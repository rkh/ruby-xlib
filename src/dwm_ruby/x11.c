#include "x11.h"

int xerror(Display *dpy, XErrorEvent *ee) {
    if(ee->error_code == BadWindow
    ||(ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
    ||(ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
    || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
    || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
    || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
        return 0;
//    return xerrorxlib(dpy, ee); /* may call exit */
}

int xerrorstart(Display *dsply, XErrorEvent *ee, WM *winman) {
    winman->otherwm = True;
    return -1;
}

void setup(WM* winman) {
    int d;
    unsigned int i, j, mask;
    Window w;
    XModifierKeymap *modmap;
    XSetWindowAttributes wa;

    // init atoms
    winman->wmatom[WMProtocols] = XInternAtom(winman->dpy, "WM_PROTOCOLS", False);
    winman->wmatom[WMDelete] = XInternAtom(winman->dpy, "WM_DELETE_WINDOW", False);
    winman->wmatom[WMName] = XInternAtom(winman->dpy, "WM_NAME", False);
    winman->wmatom[WMState] = XInternAtom(winman->dpy, "WM_STATE", False);
    winman->netatom[NetSupported] = XInternAtom(winman->dpy, "_NET_SUPPORTED", False);
    winman->netatom[NetWMName] = XInternAtom(winman->dpy, "_NET_WM_NAME", False);
    XChangeProperty(winman->dpy, winman->root, winman->netatom[NetSupported], XA_ATOM, 32,
            PropModeReplace, (unsigned char *) winman->netatom, NetLast);

    // init geometry
    winman->sx = winman->sy = 0;
    winman->sw = DisplayWidth(winman->dpy, winman->screen);
    winman->sh = DisplayHeight(winman->dpy, winman->screen);

    // init window area
    winman->wax = winman->way = 0;
    winman->waw = winman->sw;
    winman->wah = winman->sh;

    // TODO: init modifier map
    // TODO: init cursors

    wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask
        | EnterWindowMask | LeaveWindowMask | StructureNotifyMask;
    XChangeWindowAttributes(winman->dpy, winman->root, CWEventMask | CWCursor, &wa);
    XSelectInput(winman->dpy, winman->root, wa.event_mask);
    
    // TODO: Grabkeys, Multihead-Support, Xinerama, Compiz, Multitouch :D
}

Client manage(WM* winman, Window w, XWindowAttributes *wa, Client* c) {
    XWindowChanges wc;
    Window trans;
    Status rettrans;
    long data[] = {NormalState, None};
    XEvent ev;

    c->win = w;
    c->x = wa->x;
    c->y = wa->y;
    c->w = wa->width;
    c->h = wa->height;
    c->oldborder = wa->border_width;
    if(c->w == winman->sw && c->h == winman->sh) {
        c->x = winman->sx;
        c->y = winman->sy;
        c->border = wa->border_width;
    }
    else {
        if(c->x + c->w + 2 * c->border > winman->wax + winman->waw)
            c->x = winman->wax + winman->waw - c->w - 2 * c->border;
        if(c->y + c->h + 2 * c->border > winman->way + winman->wah)
            c->y = winman->way + winman->wah - c->h - 2 * c->border;
        if(c->x < winman->wax)
            c->x = winman->wax;
        if(c->y < winman->way)
            c->y = winman->way;
        c->border = 0;
    }
    wc.border_width = c->border;
    XConfigureWindow(winman->dpy, w, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
    XSelectInput(winman->dpy, w, EnterWindowMask | FocusChangeMask 
            | PropertyChangeMask | StructureNotifyMask);
    XMoveResizeWindow(winman->dpy, c->win, c->x, c->y, c->w, c->h); // some wins need this    
    XMapWindow(winman->dpy, c->win);
    XChangeProperty(winman->dpy, c->win, winman->wmatom[WMState], 
            winman->wmatom[WMState], 32, PropModeReplace, 
            (unsigned char*)data, 2);
    winman->selected = c;
    XRaiseWindow(winman->dpy, winman->selected->win);
    XSync(winman->dpy, False);
}

void
resize(WM* winman, Client *c, int x, int y, int w, int h, int sizehints) {
	XWindowChanges wc;

	if(sizehints != 0) {
		/* set minimum possible */
		if (w < 1)
			w = 1;
		if (h < 1)
			h = 1;

		/* temporarily remove base dimensions */
		w -= c->basew;
		h -= c->baseh;

		/* adjust for aspect limits */
		if (c->minay > 0 && c->maxay > 0 && c->minax > 0 && c->maxax > 0) {
			if (w * c->maxay > h * c->maxax)
				w = h * c->maxax / c->maxay;
			else if (w * c->minay < h * c->minax)
				h = w * c->minay / c->minax;
		}

		/* adjust for increment value */
		if(c->incw)
			w -= w % c->incw;
		if(c->inch)
			h -= h % c->inch;

		/* restore base dimensions */
		w += c->basew;
		h += c->baseh;

		if(c->minw > 0 && w < c->minw)
			w = c->minw;
		if(c->minh > 0 && h < c->minh)
			h = c->minh;
		if(c->maxw > 0 && w > c->maxw)
			w = c->maxw;
		if(c->maxh > 0 && h > c->maxh)
			h = c->maxh;
	}
	if(w <= 0 || h <= 0)
		return;
	/* offscreen appearance fixes */
	if(x > winman->sw)
		x = winman->sw - w - 2 * c->border;
	if(y > winman->sh)
		y = winman->sh - h - 2 * c->border;
	if(x + w + 2 * c->border < winman->sx)
		x = winman->sx;
	if(y + h + 2 * c->border < winman->sy)
		y = winman->sy;
	if(c->x != x || c->y != y || c->w != w || c->h != h) {
		c->x = wc.x = x;
		c->y = wc.y = y;
		c->w = wc.width = w;
		c->h = wc.height = h;
		wc.border_width = c->border;
		XConfigureWindow(winman->dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
		XSync(winman->dpy, False);
	}
}

Client* query_clients(WM* winman) {
    unsigned int i, num;
    Window *wins, d1, d2;
    XWindowAttributes wa;
    Client* c;

    wins = NULL;
    if (XQueryTree(winman->dpy, winman->root, &d1, &d2, &wins, &num)) {
        winman->clients_num = num;
        c = (Client*)calloc(num, sizeof(Client));
        for (i = 0; i < num; i++) {
            XGetWindowAttributes(winman->dpy, wins[i], &wa);
            if(wa.map_state == IsViewable)
                manage(winman, wins[i], &wa, &c[i]);
        }
    }
    if(wins)
        XFree(wins);
    return c;
}

WM Init_WM() {
    WM windowmanager;
    setlocale(LC_CTYPE, "de_DE.UTF-8");
    if(!(windowmanager.dpy = XOpenDisplay(0)))
        printf("Cannot open Display!!\n");fflush(stdout);        
    windowmanager.screen = DefaultScreen(windowmanager.dpy);
    windowmanager.root = RootWindow(windowmanager.dpy, windowmanager.screen);
    
    Bool otherwm = False;
    XSetErrorHandler(xerrorstart);
    /* this causes an error if some other window manager is running */
    XSelectInput(windowmanager.dpy, windowmanager.root, SubstructureRedirectMask);
    XSync(windowmanager.dpy, False);
    if(otherwm)
        printf("Another WM is running!!\n");fflush(stdout);
    XSync(windowmanager.dpy, False);
    XSetErrorHandler(NULL);
    windowmanager.xerrorxlib = XSetErrorHandler(xerror);
    XSync(windowmanager.dpy, False);
    setup(&windowmanager);
    return windowmanager;
}

void Destroy_WM(WM* winman) {
    XUngrabKey(winman->dpy, AnyKey, AnyModifier, winman->root);
    XSetInputFocus(winman->dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XSync(winman->dpy, False);
    XCloseDisplay(winman->dpy);
}

int main() {
    WM winman;
    int i;

    printf("Start to init NOW!\n");fflush(stdout);
    winman = Init_WM();
    printf("We have a screen: %d  %d %d %d\n", winman.sx, winman.sy, winman.sw, winman.sh);
    printf("               ... and a window area: %d %d %d %d\n", winman.wax, winman.way, winman.waw, winman.wah);
    printf("Start query NOW!\n");fflush(stdout);
    winman.clients = query_clients(&winman);
    //query_clients(&winman); 
    printf("Success! We should have clients now!\n");
    for(i=0; i < winman.clients_num; i++) {
        printf("Trying client number %d name: %s \n", i, winman.clients[i].name);
        printf("Trying clienter number %d geo: %d %d %d %d\n", 
                i, winman.clients[0].x, winman.clients[0].y, winman.clients[i].w, winman.clients[0].h);
    }
    printf("Great so far! Try some resizing now...\n");fflush(stdout);
    resize(&winman, &winman.clients[0], winman.wax, winman.way, winman.waw, winman.wah, 0);
    printf("Finish for now...\n");
    return 0;
}

