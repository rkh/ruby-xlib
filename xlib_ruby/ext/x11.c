#include "x11.h"

static char* Xevents[] = {
	[ButtonPress] = "buttonpress",
	[ConfigureRequest] = "configurerequest",
	[ConfigureNotify] = "configurenotify",
	[DestroyNotify] = "destroynotify",
	[EnterNotify] = "enternotify",
	[Expose] = "expose",
	[FocusIn] = "focusin",
	[KeyPress] = "keypress",
	[LeaveNotify] = "leavenotify",
	[MappingNotify] = "mappingnotify",
	[MapRequest] = "maprequest",
	[PropertyNotify] = "propertynotify",
	[UnmapNotify] = "unmapnotify"
};

/*
 * This handles just some initial setup directly related to the 
 * Window manager object which is passed as a pointer
 */
void setup(WM* winman) {
	/* init atoms */
	winman->wmatom[WMProtocols] =   XInternAtom(winman->dpy, "WM_PROTOCOLS", False);
	winman->wmatom[WMDelete] =      XInternAtom(winman->dpy, "WM_DELETE_WINDOW", False);
	winman->wmatom[WMName] =        XInternAtom(winman->dpy, "WM_NAME", False);
	winman->wmatom[WMState] =       XInternAtom(winman->dpy, "WM_STATE", False);
	winman->netatom[NetSupported] = XInternAtom(winman->dpy, "_NET_SUPPORTED", False);
	winman->netatom[NetWMName] =    XInternAtom(winman->dpy, "_NET_WM_NAME", False);
  
  // init geometry
  winman->sx = winman->sy = 0;
  winman->sw = DisplayWidth(winman->dpy, winman->screen);
  winman->sh = DisplayHeight(winman->dpy, winman->screen);

  // init window area
  winman->wax = winman->way = 0;
  winman->waw = winman->sw;
  winman->wah = winman->sh;

  // TODO: Grabkeys, Multihead-Support, Xinerama, Compiz, Multitouch :D
}

/*
 * This will initialize a new client object at the adress c,
 * its manager set to winman. The window itself and it's attributes 
 * must be passed as well. This function will be called each time a
 * window is added to the field of managed windows in the WM object
 */
void manage(WM* winman, Window w, XWindowAttributes *wa, Client* c) {
    XWindowChanges wc;
    XClassHint* ch = XAllocClassHint();
    char* win_name;

    c->win = w;
    c->manager = winman;

    if (XGetClassHint(winman->dpy, c->win, ch)) {
        sprintf(c->class,"%s", ch->res_class);
        if(ch->res_class) XFree(ch->res_class);
        if(ch->res_name) XFree(ch->res_name);
    }
    if (XFetchName(winman->dpy, w, &win_name)) {
        sprintf(c->name,"%s",win_name);
        XFree(win_name);
    }

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
    //XConfigureWindow(winman->dpy, w, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
    fprintf(stderr,"manage:XConfigureWindow w=%x border=%i\n", (int)w, wc.border_width);
    //XConfigureWindow(winman->dpy, w, CWBorderWidth, &wc);
    XSelectInput(winman->dpy, w, EnterWindowMask | FocusChangeMask 
            | PropertyChangeMask | StructureNotifyMask);
    XMoveResizeWindow(winman->dpy, c->win, c->x, c->y, c->w, c->h); // some wins need this    
    XMapWindow(winman->dpy, c->win);
    XSetInputFocus(winman->dpy, c->win, RevertToNone, CurrentTime); // focus new windows    
    winman->selected = c;
    XSync(winman->dpy, False);
}

/*
 * This function creates a simple border around a client
 */
void border_client(Client* c, int w) {
    XWindowChanges wc;

    c->border = w;
    wc.border_width = w;
    fprintf(stderr,"border_client:XConfigureWindow border=%i\n", wc.border_width);
    XConfigureWindow(c->manager->dpy, c->win, CWBorderWidth, &wc);
    XMoveResizeWindow(c->manager->dpy, c->win, c->x, c->y, c->w, c->h); // some wins need this
    XSync(c->manager->dpy, False);
}

/* 
 * This function resets the client's border to the preset size
 */
void unborder_client(Client* c) {
    XWindowChanges wc;

    c->border = c->oldborder;
    wc.border_width = c->border;
    fprintf(stderr,"unborder_client:XConfigureWindow border=%i\n", wc.border_width);
    XConfigureWindow(c->manager->dpy, c->win, CWBorderWidth, &wc);
    XMoveResizeWindow(c->manager->dpy, c->win, c->x, c->y, c->w, c->h); // some wins need this
    XSync(c->manager->dpy, False);
}

/*
 * This function raises a client
 */
void raise_client(Client* c) {
    //XWindowChanges wc;
    //XConfigureWindow(c->manager->dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
    //XSelectInput(c->manager->dpy, c->win, EnterWindowMask | FocusChangeMask
    //        | PropertyChangeMask | StructureNotifyMask);
    XMoveResizeWindow(c->manager->dpy, c->win, c->x, c->y, c->w, c->h); // some wins need this
    XMapWindow(c->manager->dpy, c->win);
    c->manager->selected = c;
    XRaiseWindow(c->manager->dpy, c->win);
    XSync(c->manager->dpy, False);
}

/*
 * This fucntion will try to process 
 * the next pending event, if existant.
 * This is just for the main testing
 */
/*
void process_event(WM* winman) {
    XEvent ev;
    XWindowAttributes wa;
    unsigned int i,j;
    Window *wins, d1, d2;

    if (XPending(winman->dpy))
        XNextEvent(winman->dpy, &ev);
        if ((ev.type == UnmapNotify) || (ev.type == DestroyNotify)) {
                for(i=0; i<winman->clients_num; i++)
                    if (winman->clients[i].win == ev.xany.window) {
                        winman->clients_num -= 1;
                        for(j=i+1; j<winman->clients_num; j++)
                            winman->clients[j-1] = winman->clients[j];
                        winman->clients = realloc(winman->clients, sizeof(Client)*winman->clients_num);
                    }
        }
        else {
            wins = NULL;
            XQueryTree(winman->dpy, winman->root, &d1, &d2, &wins, &j)
            if (j != winman->clients_num)
                winman->clients_num = j;
                winman->clients = realloc(winman->clients, sizeof(Client)*winman->clients_num);
                for (i = 0; i < j; i++) {
                    XGetWindowAttributes(winman->dpy, wins[i], &wa);
                    if(wa.map_state == IsViewable)
                        manage(winman, wins[i], &wa, &winman->clients[i]);
                }
        }
}
*/

/*
 * This shows which source caused the event.
 * Has to be called prior to handling the actual event!!
 */
int event_next_source(WM* winman) {
    XEvent ev;
    int i;

    if (XPending(winman->dpy)) {
        XPeekEvent(winman->dpy, &ev);
        for(i=0; i<winman->clients_num; i++) {
            if (winman->clients[i].win == ev.xany.window)
                return i;
        }
        return -2;
    }
    return -1;
}

/*
 * This returns the events name 
 */
char* event_next_type(WM* winman) {
    XEvent ev;
    
    if (XPending(winman->dpy)) {
        XPeekEvent(winman->dpy, &ev);
        return Xevents[ev.type];
    }
    return NULL;
}

/*
 * This removes an event from the queue
 */
void event_pop(WM* winman) {
    XEvent ev;
    
    if (XPending(winman->dpy))
        XNextEvent(winman->dpy, &ev);
}

/*
 * This function resizes a client
 */
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
    fprintf(stderr,"resize:XConfigureWindow x=%i y=%i width=%i height=%i border=%i\n", wc.x,wc.y,wc.width,wc.height,wc.border_width);
		XConfigureWindow(winman->dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
		XSync(winman->dpy, False);
	}
}

/*
 * This function "minimizes"
 */
void ban_client(Client* c) {
    if (c->isbanned)
        return;
    XMoveWindow(c->manager->dpy, c->win, c->manager->sw + 2, c->manager->sh + 2);
    c->isbanned = True;
    XSync(c->manager->dpy, False);
}

/*
 * This function "un-minimizes"
 */
void unban_client(Client*c) {
    if (!(c->isbanned))
        return;
    XMoveWindow(c->manager->dpy, c->win, c->x, c->y);
    c->isbanned = False;
    XSync(c->manager->dpy, False);
}

long
getstate(WM* winman, Window w) {
	int format, status;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	status = XGetWindowProperty(winman->dpy, w, winman->wmatom[WMState], 0L, 2L, False, winman->wmatom[WMState],
			&real, &format, &n, &extra, (unsigned char **)&p);
	if(status != Success)
		return -1;
	if(n != 0)
		result = *p;
	XFree(p);
	return result;
}

/*
 * This is a setup-function to fill the field
 * of managed clients in the WM object.
 * This should only be called during setup!
 */
Client* query_clients(WM* winman) {
    unsigned int i, num;
    Window *wins, d1, d2;
    XWindowAttributes wa;
    Client* c = (Client*)malloc(sizeof(Client));

    wins = NULL;
    if (XQueryTree(winman->dpy, winman->root, &d1, &d2, &wins, &num)) {
        winman->clients_num = 0;
        c = (Client*)calloc(num, sizeof(Client));
        for (i = 0; i < num; i++) {
          if (XGetWindowAttributes(winman->dpy, wins[i], &wa) &&
              (wa.map_state == IsViewable || getstate(winman,wins[i]) == IconicState)) {
            winman->clients_num += 1;
            manage(winman, wins[i], &wa, &c[i]);
          }
        }
    }
    if(wins)
        XFree(wins);
    return c;
}

/* 
 * This will initialize a WM object and return its adress
 */
WM* Init_WM() {
    WM* windowmanager;
    windowmanager = calloc(1,sizeof(WM));
    setlocale(LC_CTYPE, "de_DE.UTF-8");
    if(!(windowmanager->dpy = XOpenDisplay(0)))
        printf("Cannot open Display!!\n");fflush(stdout);
    windowmanager->screen = DefaultScreen(windowmanager->dpy);
    if(!(windowmanager->dpy = XOpenDisplay(0x0))) return NULL;
    windowmanager->root = RootWindow(windowmanager->dpy, windowmanager->screen);
    
    /*
    XSetErrorHandler(NULL);
    windowmanager->xerrorxlib = XSetErrorHandler(xerror);
    XSync(windowmanager->dpy, False);
    */
    setup(windowmanager);
    return windowmanager;
}

/*
 * This will free all Client objects and the WM itself.
 * This will grow with each added feature.
 */
void Destroy_WM(WM* winman) {
    XUngrabKey(winman->dpy, AnyKey, AnyModifier, winman->root);
    XSetInputFocus(winman->dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XSync(winman->dpy, False);
    XCloseDisplay(winman->dpy);
    free(winman->clients); 
    free(winman);
}

/*
 * Here we have a test routine to test the C code by itself
 */
int main() {
    WM* winman;   
    int i;

    printf("Start to init NOW!\n");fflush(stdout);
    winman = Init_WM();
    printf("We have a screen: %d  %d %d %d\n", winman->sx, winman->sy, winman->sw, winman->sh);
    printf("         ... and a window area: %d %d %d %d\n", winman->wax, winman->way, winman->waw, winman->wah);
    printf("Start query NOW!\n");fflush(stdout);
    winman->clients = query_clients(winman);
    printf("Success! We should have clients now!\n");
    printf("         And they should all have the WM ptr... (trying just the first): %d %d %d %d\n", 
            winman->clients[0].manager->sx, winman->clients[0].manager->sy, 
            winman->clients[0].manager->sw, winman->clients[0].manager->sh);
    for(i=0; i < winman->clients_num; i++) {
        printf("                  Trying client number %d name: %s \n", i, winman->clients[i].name);
        printf("                           Geo: %d %d %d %d\n", 
                winman->clients[0].x, winman->clients[0].y, winman->clients[i].w, winman->clients[0].h);
    }
    
    printf("Great so far! Try some resizing now...\n");fflush(stdout);
    resize(winman, &winman->clients[0], winman->wax, winman->way, winman->waw, winman->wah, 0);
    printf("Good. Set a border around our main client now...");
    border_client(&winman->clients[0], 9);
    
    printf("Try to raise in cycles as well...\n");
    for(i=0; i < winman->clients_num; i++) {
        raise_client(&winman->clients[i]);
        printf("         Client %d should be raised now...\n", i);fflush(stdout);
        usleep(1000000);
    }

    printf("Ban - unban each client ...\n");
    for(i=0; i < winman->clients_num; i++) {
        ban_client(&winman->clients[i]);
        printf("         Client %d is banned? %d \n", i, winman->clients[i].isbanned);fflush(stdout);
        usleep(1000000);
        unban_client(&winman->clients[i]);
        printf("            ... and unbanned? %d \n", winman->clients[i].isbanned);fflush(stdout);
    }

    printf("Finish for now...\n");
    Destroy_WM(winman);

    return 0;
}

