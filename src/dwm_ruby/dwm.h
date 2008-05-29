/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance.  Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * Calls to fetch an X event from the event queue are blocking.  Due reading
 * status text from standard input, a select()-driven main loop has been
 * implemented which selects for reads on the X connection and STDIN_FILENO to
 * handle all data smoothly. The event handlers of dwm are organized in an
 * array which is accessed whenever a new event has been fetched. This allows
 * event dispatching in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag.  Clients are organized in a global
 * doubly-linked client list, the focus history is remembered through a global
 * stack list. Each client contains an array of Bools of the same size as the
 * global tags array to indicate the tags of a client.  
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <pthread.h>

/* macros */
#define BUTTONMASK		(ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask)		(mask & ~(numlockmask | LockMask))
#define LENGTH(x)		(sizeof x / sizeof x[0])
#define MAXTAGLEN		16
#define MOUSEMASK		(BUTTONMASK | PointerMotionMask)


/* enums */
extern enum { CurNormal, CurResize, CurMove, CurLast };	/* cursor */
extern enum { ColBorder, ColFG, ColBG, ColLast };		/* color */
extern enum { NetSupported, NetWMName, NetLast };		/* EWMH atoms */
extern enum { WMProtocols, WMDelete, WMName, WMState, WMLast };/* default atoms */

/* typedefs */
typedef struct Client_t Client;

struct Client_t {
	char name[256];
	int x, y, w, h;
	int rx, ry, rw, rh; /* revert geometry */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay;
	long flags;
	unsigned int border, oldborder;
	Bool isbanned, isfixed, ismax, isfloating, wasfloating;
	Bool *tags;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
};

typedef struct {
	int x, y, w, h;
	unsigned long norm[ColLast];
	unsigned long sel[ColLast];
	Drawable drawable;
	GC gc;
	struct {
		int ascent;
		int descent;
		int height;
		XFontSet set;
		XFontStruct *xfont;
	} font;
} DC; /* draw context */

typedef struct Key_t {
	unsigned long mod;
	KeySym keysym;
	void (*func)(const char *arg);
	const char *arg;
} Key;

extern void arrange(void);
extern void attach(Client *c);
extern void attachstack(Client *c);
extern void ban(Client *c);
extern void buttonpress(XEvent *e);
extern void checkotherwm(void);
extern void cleanup(void);
extern void configure(Client *c);
extern void configurenotify(XEvent *e);
extern void configurerequest(XEvent *e);
extern void destroynotify(XEvent *e);
extern void detach(Client *c);
extern void detachstack(Client *c);
extern void *emallocz(unsigned int size);
extern void enternotify(XEvent *e);
extern void eprint(const char *errstr, ...);
extern void expose(XEvent *e);
extern void focus(Client *c);
extern void focusin(XEvent *e);
extern Client *getclient(Window w);
extern long getstate(Window w);
extern void grabkeys(void);
extern Bool isoccupied(unsigned int t);
extern Bool isprotodel(Client *c);
extern void keypress(XEvent *e);
extern void killclient(const char *arg);
extern void leavenotify(XEvent *e);
extern void manage(Window w, XWindowAttributes *wa);
extern void mappingnotify(XEvent *e);
extern void maprequest(XEvent *e);
extern void movemouse(Client *c);
extern Client *nexttiled(Client *c);
extern void propertynotify(XEvent *e);
extern void quit(const char *arg);
extern void resize(Client *c, int x, int y, int w, int h, Bool sizehints);
extern void restack(void);
extern void scan(void);
extern void setclientstate(Client *c, long state);
extern void setup(void);
extern void spawn(const char *arg);
extern void unban(Client *c);
extern void unmanage(Client *c);
extern void unmapnotify(XEvent *e);
extern void updatesizehints(Client *c);
extern int xerror(Display *dpy, XErrorEvent *ee);
extern int xerrordummy(Display *dsply, XErrorEvent *ee);
extern int xerrorstart(Display *dsply, XErrorEvent *ee);
extern void* run_launcher(char* nil);
extern int maininit();
extern int mainquit();

