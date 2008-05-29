#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <locale.h>

enum { WMProtocols, WMDelete, WMName, WMState, WMLast };/* default atoms */
enum { NetSupported, NetWMName, NetLast };      /* EWMH atoms */

typedef struct Client_t Client;

struct Client_t {
    char name[256];
    int x,y,w,h;
    int rx,ry,rw,rh; //revert geo
    int basew,baseh,incw,inch,maxw,maxh,minw,minh;
    int minax,maxax,minay,maxay;
    long flags;
    unsigned int border, oldborder;
    Bool isbanned, isfixed, ismax, isfloating, wasfloating;
    Window win;
};

typedef struct Key_t {
    unsigned long mod;
    KeySym keysym;
    const char *arg;
    void (*func)(const char *arg);
} Key;

typedef struct WM_t {
    int screen, sx, sy, sw, sh, wax, way, waw, wah;
    Client *selected;
    Client* order;
    Display *dpy;
    Window root;
    Bool running; 
    Bool otherwm;
    int (*xerrorxlib)(Display *, XErrorEvent *);
    Atom wmatom[WMLast];
    Atom netatom[NetLast];
    Client* clients;
} WM;

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
    c->x = 0;
    c->y = 0;
    c->w = 2*wa->width;
    c->h = 2*wa->height;
    c->oldborder = wa->border_width;
    if(c->w == winman->sw && c->h == winman->sh) {
        c->x = 0;
        c->y = 0;
        c->border = wa->border_width;
    }
    else {
/*        
        if(c->x + c->w + 2 * c->border > winman->wax + winman->waw)
            c->x = winman->wax + winman->waw - c->w - 2 * c->border;
        if(c->y + c->h + 2 * c->border > winman->way + winman->wah)
            c->y = winman->way + winman->wah - c->h - 2 * c->border;
        if(c->x < winman->wax)
            c->x = winman->wax;
        if(c->y < winman->way)
            c->y = winman->way;
        c->border = 0;*/
    }
    wc.border_width = c->border;
    XConfigureWindow(winman->dpy, w, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
    //configure(c);
    //updatesizehints(c);
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
    //while(XCheckMaskEvent(winman->dpy, EnterWindowMask, &ev));
}

Client* query_clients(WM* winman) {
    unsigned int i, num;
    Window *wins, d1, d2;
    XWindowAttributes wa;
    Client* c;

    wins = NULL;
    if (XQueryTree(winman->dpy, winman->root, &d1, &d2, &wins, &num)) {
        c = calloc(num, sizeof(Client));
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
    printf("Start to init NOW!\n");fflush(stdout);
    winman = Init_WM();
    printf("Start query NOW!\n");fflush(stdout);
    winman.clients = query_clients(&winman);
    printf("Success! We should have clients now!\n");
    printf("Trying the first clients name: %s \n", winman.clients[0].name);
    printf("Trying the first clients geo: %d %d %d %d\n", 
            winman.clients[0].x, winman.clients[0].y, winman.clients[0].w, winman.clients[0].h);
    printf("Finish for now...");
    return 0;
}

