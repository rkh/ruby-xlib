#include "x11.h"

char* Xevents[] = {
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

/*
 * This handles just some initial setup directly related to the 
 * Window manager object which is passed as a pointer
 */
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
    // This last part disallows X to map windows itself and transfers 
    // responsability to the WindowManager. We don't need that...    
/*
    wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask
        | EnterWindowMask | LeaveWindowMask | StructureNotifyMask;
    XChangeWindowAttributes(winman->dpy, winman->root, CWEventMask | CWCursor, &wa);
    XSelectInput(winman->dpy, winman->root, wa.event_mask);
*/  
    // TODO: Grabkeys, Multihead-Support, Xinerama, Compiz, Multitouch :D
}

/*
 * This will initialize a new client object at the adress c,
 * its manager set to winman. The window itself and it's attributes 
 * must be passed as well. This function will be called each time a
 * window is added to the field of managed windows in the WM object
 */
Client manage(WM* winman, Window w, XWindowAttributes *wa, Client* c) {
    XWindowChanges wc;
    Window trans;
    Status rettrans;
    long data[] = {NormalState, None};
    XEvent ev;
    XClassHint* ch = XAllocClassHint();
    XTextProperty text_prop_ret;

    c->win = w;
    c->manager = winman;
    XGetClassHint(winman->dpy, c->win, ch);
    
    /* Try what we can to get values for Windows */
    sprintf(c->class,"%s",ch->res_class ? ch->res_class : "EMPTY");
    sprintf(c->name,"%s",ch->res_name ? ch->res_name : "EMPTY");
    if (XGetWMName(winman->dpy, c->win, &text_prop_ret)) {
        sprintf(c->name, "%s", 
                (strcmp(c->name, "")==0) ? text_prop_ret.value : c->name);
    }

    if(ch->res_class) XFree(ch->res_class);
    if(ch->res_name) XFree(ch->res_name);
    XFree(ch);
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

/*
 * This function creates a simple border around a client
 */
void border_client(Client* c, int w) {
    XWindowChanges wc;

    c->border = w;
    wc.border_width = w;
    XConfigureWindow(c->manager->dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
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
    XConfigureWindow(c->manager->dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
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
void process_event(WM* winman) {
    XEvent ev;
    XWindowAttributes wa;
    int i,j;

    if (XPending(winman->dpy))
        XNextEvent(winman->dpy, &ev);
        switch (ev.type) {
            case (MapRequest):
                winman->clients_num += 1;
                // remember: if realloc fails, it keeps the origin-block intact, 
                // so we can just do it directly :)
                winman->clients = realloc(winman->clients, sizeof(Client)*winman->clients_num);
                manage(winman, ev.xany.window, &wa, &winman->clients[winman->clients_num-1]);
                break;
            case (UnmapNotify || DestroyNotify):
                for(i=0; i<winman->clients_num; i++)
                    if (winman->clients[i].win == ev.xany.window) {
                        winman->clients_num -= 1;
                        for(j=i+1; j<winman->clients_num; j++)
                            winman->clients[j-1] = winman->clients[j];
                        winman->clients = realloc(winman->clients, sizeof(Client)*winman->clients_num); 
                    }
                break;
            default:
                break;
        }
}

/*
 * This returns whether there is an event waiting for us
 */
char event_pending(WM* winman) {
    if (XPending(winman->dpy)) return 1;
    return 0;
}

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
 * This might do better for client stuff
 */
void update_query(WM* winman) {
    unsigned int i,j,num;
    char found = 0;
    Window *wins, d1, d2;
    XWindowAttributes wa;
    Client* c;

    wins = NULL;
    XQueryTree(winman->dpy, winman->root, &d1, &d2, &wins, &num);

    // Are we missing windowS?
    if (winman->clients_num < num) {        
        winman->clients = (Client*)realloc(winman->clients, sizeof(Client)*num);     // Resize array
        for (i=0;i<num;i++) {
            found = 0;
            for(j=0;j<winman->clients_num;j++) {
                if (winman->clients[j].win == wins[i]) {
                    // If we find the win, stop here, we don't need to handle it
                    found = 1;
                    break;
                }
            }
            if (found == 0) {
                XGetWindowAttributes(winman->dpy, wins[i], &wa);
                // If we haven't found the win, manage it and post-inc the client number
                manage(winman, wins[i], &wa, &winman->clients[winman->clients_num++]);
            }
            if (winman->clients_num == num) break;  // If the new client number is equal to num, stop here
        }
    }/* else if (winman->clients_num > num) {
        for(i=0; i<winman->clients_num; i++)
            if (winman->clients[i].win == ev.xany.window) {
                winman->clients_num -= 1;
                for(j=i+1; j<winman->clients_num; j++)
                    winman->clients[j-1] = winman->clients[j];
                winman->clients = realloc(winman->clients, sizeof(Client)*winman->clients_num); 
            }
    }*/
    if(wins)
        XFree(wins);
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

/*
 * This is a setup-function to fill the field
 * of managed clients in the WM object.
 * This should only be called during setup!
 */
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
    
    XSetErrorHandler(NULL);
    windowmanager->xerrorxlib = XSetErrorHandler(xerror);
    XSync(windowmanager->dpy, False);
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
                i, winman->clients[0].x, winman->clients[0].y, winman->clients[i].w, winman->clients[0].h);
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

    printf("Start handling of pending events for a few seconds...\n");
    for(i=100; i>0; --i) {
        process_event(winman);
        usleep(10000);
    }

    printf("Try query update...\n");
    update_query(winman);

    printf("Finish for now...\n");
    return 0;
}

