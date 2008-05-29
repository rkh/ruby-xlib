%module x11
%{
#include "x11.h"
%}

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
    unsigned int clients_num;
} WM;

extern void resize(WM* winman, Client *c, int x, int y, int w, int h, Bool sizehints);
extern Client* query_clients(WM* winman);
extern WM Init_WM();
extern void Destroy_WM(WM* winman);

