#include <ruby.h>
#include <x11.h>

// Main module
VALUE mX11,

// Classes
  cClient, cWM,
  cEvent, cKeyEvent, cButtonEvent, cMotionEvent, cCrossingEvent, cFocusEvent,
  cFocusChangeEvent, cCreateWindowEvent, cDestroyWindowEvent, cUnmapEvent, cMapRequestEvent,
  cResizeRequestEvent, cConfigureRequestEvent;

#define numKeyMasks 13

// Symbols
static ID id_client, id_x, id_y, id_button, id_keycode, id_keysym, id_state, id_mods,
          id_keymasks[numKeyMasks];
// id_Button1,id_Button2,id_Button3,id_Button4,id_Button5,
// id_Shift,id_Lock,id_Control,
// id_Mod1,id_Mod2,id_Mod3,id_Mod4,id_Mod5;

static VALUE client_make(VALUE klass, Client* c); // I need this alot
static VALUE event_wrap(VALUE klass, XEvent* ev);


static VALUE x11_error_handler(VALUE self) {
  return rb_iv_get(mX11,"error_handler");
}

static VALUE x11_error_handler_set(VALUE self, VALUE proc) {
  if (!(NIL_P(proc) || RTEST(rb_respond_to(proc,rb_intern("call"))))) {
    rb_raise(rb_eArgError,"Error handler must be a Proc or nil");
    return Qnil;
  }
  rb_iv_set(mX11,"error_handler",proc);
  return proc;
}

static int x11_internal_error_handler(Display* dpy, XErrorEvent* err) {
  char message[255];
  VALUE handler = rb_iv_get(mX11,"error_handler");

  XGetErrorText(err->display, err->error_code, message, 254);
  if (NIL_P(handler) || !RTEST(rb_respond_to(handler,rb_intern("call")))) {
    fprintf(stderr,"X11 Error: %s\n", message);
  } else {
    rb_funcall(handler, rb_intern("call"), 2, INT2NUM(err->error_code), rb_str_new2(message));
  }
  return 0; // ignored
}


// Allocate a new WindowManager object
// using the supplied Init_WM
// Also do a first query
static VALUE wm_alloc(VALUE klass) {
    WM* newwm;
    VALUE obj;
    newwm = Init_WM();
    newwm->clients = query_clients(newwm);
    obj = Data_Wrap_Struct(klass, 0, Destroy_WM, newwm);
    return obj;
}

static VALUE wm_query(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    newwm->clients = query_clients(newwm);
    return Qnil;
}

static VALUE wm_sx(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    return INT2NUM(newwm->sx);
}

static VALUE wm_sy(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    return INT2NUM(newwm->sy);
}

static VALUE wm_sw(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    return INT2NUM(newwm->sw);
}

static VALUE wm_sh(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    return INT2NUM(newwm->sh);
}

static VALUE wm_wax(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    return INT2NUM(newwm->wax);
}

static VALUE wm_wax_set(VALUE self, VALUE newval) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    newwm->wax = NUM2INT(newval);
    return Qnil;
}

static VALUE wm_way(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    return INT2NUM(newwm->way);
}

static VALUE wm_way_set(VALUE self, VALUE newval) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    newwm->way = NUM2INT(newval);
    return Qnil;
}

static VALUE wm_waw(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    return INT2NUM(newwm->waw);
}

static VALUE wm_waw_set(VALUE self, VALUE newval) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    newwm->waw = NUM2INT(newval);
    return Qnil;
}

static VALUE wm_wah(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    return INT2NUM(newwm->wah);
}

static VALUE wm_wah_set(VALUE self, VALUE newval) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    newwm->wah = NUM2INT(newval);
    return Qnil;
}

static VALUE wm_num_clients(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    return INT2NUM(newwm->clients_num);
}

static VALUE wm_selected(VALUE self) {
    WM *newwm;
    XEvent event_return, event_saviour;
    int i;

    Data_Get_Struct(self, WM, newwm);

    // Cycle through focus events and get the most recent
    XCheckTypedEvent(newwm->dpy, FocusIn, &event_return);
    /*
    while (XCheckTypedEvent(newwm->dpy, FocusIn, &event_return))
    */
        event_saviour = event_return; 
    for (i=0; i<newwm->clients_num; i++)
       if (&newwm->clients[i].win == &event_saviour.xany.window)
           newwm->selected = &newwm->clients[i];
    return client_make(cClient, newwm->selected);
}

static VALUE wm_clients(VALUE self) {
    WM *newwm;
    //Client *c;
    VALUE obj;
    VALUE arr;
    int i;

    arr = rb_ary_new();
    Data_Get_Struct(self, WM, newwm);
    for (i=0; i<newwm->clients_num; i++) {
        obj = client_make(cClient, &newwm->clients[i]);
        rb_ary_push(arr, obj);
    }
    return arr;
}

static VALUE wm_pending_event_count(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    return INT2NUM(XPending(newwm->dpy));
}

static VALUE wm_event_next_source(VALUE self) {
    WM *newwm;
    int c;

    Data_Get_Struct(self, WM, newwm);
    c = event_next_source(newwm);
    if (c > 0)
        return client_make(cClient, &newwm->clients[c]);
    else
        return Qnil;
}

static VALUE wm_event_next_type(VALUE self) {
    WM *newwm;

    Data_Get_Struct(self, WM, newwm);
    if (event_next_type(newwm) != NULL)
        return rb_str_new2(event_next_type(newwm));
    else {
        return Qnil;
    }
}

static VALUE wm_event_pop(VALUE self) {
    WM *newwm;

    Data_Get_Struct(self, WM, newwm);
    event_pop(newwm);
    return Qnil;
}

static KeySym keysym_to_c(VALUE r_keysym) {
  return XStringToKeysym(StringValueCStr(r_keysym));
}

static int keymask_to_c(VALUE r_mods) {
  int mods = 0, i;
  for (i = 0; i < numKeyMasks; i++) {
    if (RTEST(rb_funcall(r_mods,rb_intern("key?"),1,ID2SYM(id_keymasks[i])))) {
      mods = mods | (1 << i);
    }
  }
  return mods;
}

static Client* client_to_c(VALUE r_client) {
  Client* c;
  Data_Get_Struct(r_client,Client,c);
  return c;
}

static WM* wm_to_c(VALUE r_wm) {
  WM* wm;
  Data_Get_Struct(r_wm,WM,wm);
  return wm;
}

static VALUE wm_manage(VALUE self, VALUE r_client, VALUE x, VALUE y, VALUE w, VALUE h) {
  WM* wm = wm_to_c(self);
  Client* c = client_to_c(r_client);
  XWindowAttributes wa;
  wa.x      = NUM2INT(x);
  wa.y      = NUM2INT(y);
  wa.width  = NUM2INT(w);
  wa.height = NUM2INT(h);
  manage_client(wm,&wa,c);
  return Qtrue;
}

VALUE r_client_for_window(WM* wm, Window w) {
  Client* c;
  c = client_ftw(wm,w);
  if (!c && manageable_p(wm,w)) {
    c = (Client*) calloc(1,sizeof(Client));
    init_client(wm,w,c);
  }
  return c ? client_make(cClient,c) : Qnil;
}

static VALUE wm_get_event(VALUE self) {
  WM* wm;
  XEvent* pev;
  VALUE r_ev,r_mods,r_keysym;
  int i;
  
  Data_Get_Struct(self,WM,wm);
  pev = (XEvent*) calloc(1,sizeof(XEvent));
  XNextEvent(wm->dpy,pev);
  r_ev = event_wrap(cEvent,pev);


  switch(pev->type) {

  case ButtonPress:
  case ButtonRelease:
    rb_ivar_set(r_ev, id_client, r_client_for_window(wm,pev->xbutton.window));
    rb_ivar_set(r_ev,id_x,INT2NUM(pev->xbutton.x));
    rb_ivar_set(r_ev,id_y,INT2NUM(pev->xbutton.y));
    break;

  case KeyPress:
  case KeyRelease:
    rb_ivar_set(r_ev, id_client, r_client_for_window(wm,pev->xkey.window));
    rb_ivar_set(r_ev,id_keycode,INT2NUM(pev->xkey.keycode));
    rb_ivar_set(r_ev,id_state,INT2NUM(pev->xkey.state));

    r_keysym = rb_str_new2(XKeysymToString(XLookupKeysym((XKeyEvent*)pev,0)));
    r_mods = rb_hash_new();
    for (i = 0; i < numKeyMasks; i++) {
      if (pev->xkey.state & (1 << i))
        rb_hash_aset(r_mods,ID2SYM(id_keymasks[i]),Qtrue);
    }

    rb_ivar_set(r_ev,id_keysym,r_keysym);
    rb_ivar_set(r_ev,id_mods,r_mods);
    break;

  case ConfigureNotify:
    rb_ivar_set(r_ev, id_client, r_client_for_window(wm,pev->xconfigure.window));
    break;

  case FocusIn:
  case FocusOut:
    rb_ivar_set(r_ev, id_client, r_client_for_window(wm,pev->xfocus.window));
    break;

  case EnterNotify:
  case LeaveNotify:
    rb_ivar_set(r_ev, id_client, r_client_for_window(wm,pev->xcrossing.window));
    break;
    
  case CreateNotify:
    rb_ivar_set(r_ev, id_client, r_client_for_window(wm,pev->xcreatewindow.window));
    break;
    
  case DestroyNotify:
    rb_ivar_set(r_ev, id_client, r_client_for_window(wm,pev->xdestroywindow.window));
    break;

  case MapRequest:
    rb_ivar_set(r_ev, id_client, r_client_for_window(wm,pev->xmaprequest.window));
    break;
  }
  return r_ev;
}

static VALUE wm_grab_key(VALUE self, VALUE r_keysym, VALUE r_mods) {
  KeySym keysym = keysym_to_c(r_keysym);
  KeyCode keycode;
  int mods = keymask_to_c(r_mods);
  WM* wm;

  Data_Get_Struct(self,WM,wm);
  keycode = XKeysymToKeycode(wm->dpy, keysym);
  XGrabKey(wm->dpy, keycode, mods, wm->root, True, GrabModeAsync, GrabModeAsync);
  return Qnil;
}

static VALUE wm_ungrab_key(VALUE self, VALUE r_keysym, VALUE r_mods) {
  KeySym keysym = keysym_to_c(r_keysym);
  KeyCode keycode;
  int mods = keymask_to_c(r_mods);
  WM* wm;

  Data_Get_Struct(self,WM,wm);
  keycode = XKeysymToKeycode(wm->dpy, keysym);
  XUngrabKey(wm->dpy, keycode, mods, wm->root);
  return Qnil;
}

static void client_free(void *p) {
    //free(p);
}

static VALUE client_alloc(VALUE klass) {
    Client *c;
    VALUE obj;
    c = (Client*)calloc(1, sizeof(Client));
    obj = Data_Wrap_Struct(klass, 0, client_free, c);
    return obj;
}

static VALUE client_make(VALUE klass, Client* c) {
    VALUE obj;
    obj = Data_Wrap_Struct(klass, 0, client_free, c);
    return obj;
}

static VALUE client_wid(VALUE self) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    return INT2NUM(c->win);
}

static VALUE client_name(VALUE self) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    return rb_str_new2(c->name);
}

static VALUE client_class(VALUE self) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    return rb_str_new2(c->class);
}

static VALUE client_border(VALUE self) {
    Client* c;
    Data_Get_Struct(self, Client, c);
    return INT2NUM(c->border);
}

static VALUE client_border_set(VALUE self, VALUE width) {
    Client* c;
    Data_Get_Struct(self, Client, c);
    border_client(c, NUM2INT(width));
    return Qnil;
}

static VALUE client_border_dset(VALUE self) {
    Client* c;
    Data_Get_Struct(self, Client, c);
    unborder_client(c);
    return Qnil;
}

static VALUE client_x(VALUE self) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    return INT2NUM(c->x);
} 

static VALUE client_x_set(VALUE self, VALUE nv) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    resize(c->manager, c, NUM2INT(nv), c->y, c->w, c->h, False);

    return Qnil; 
} 

static VALUE client_y(VALUE self) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    return INT2NUM(c->y);
} 

static VALUE client_y_set(VALUE self, VALUE nv) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    resize(c->manager, c, c->x, NUM2INT(nv), c->w, c->h, False);

    return Qnil; 
} 

static VALUE client_w(VALUE self) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    return INT2NUM(c->w);
} 

static VALUE client_w_set(VALUE self, VALUE nv) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    resize(c->manager, c, c->x, c->y, NUM2INT(nv), c->h, False);

    return Qnil; 
} 

static VALUE client_h(VALUE self) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    return INT2NUM(c->h);
} 

static VALUE client_h_set(VALUE self, VALUE nv) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    resize(c->manager, c, c->x, c->y, c->w, NUM2INT(nv), False);

    return Qnil; 
} 

static VALUE client_size(VALUE self) {
    Client *c; 
    int sizes[4];

    Data_Get_Struct(self, Client, c);
    sizes[0] = c->x;
    sizes[1] = c->y;
    sizes[2] = c->w;
    sizes[3] = c->h;
    return rb_ary_new3(4, INT2NUM(sizes[0]), INT2NUM(sizes[1]), INT2NUM(sizes[2]), INT2NUM(sizes[3]));
}

static VALUE client_raise(VALUE self) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    raise_client(c);
    return client_make(cClient, c);
}

static VALUE client_ban(VALUE self) {
    Client* c;
    Data_Get_Struct(self, Client, c);
    ban_client(c);
    return Qnil;
}

static VALUE client_unban(VALUE self) {
    Client* c;
    Data_Get_Struct(self, Client, c);
    unban_client(c);
    return client_make(cClient, c);
}

static VALUE client_size_set(VALUE self, VALUE valarray) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    int x,y,w,h;
    
    if (RARRAY_LEN(valarray) >= 4) {
        x = RARRAY_PTR(valarray)[0];
        y = RARRAY_PTR(valarray)[1];
        w = RARRAY_PTR(valarray)[2];
        h = RARRAY_PTR(valarray)[3];
        resize(c->manager, c, x, y, w, h, False);
    }
    else
        rb_raise(rb_eArgError, "wrong number of arguments [x,y,w,h]");
    return Qnil;
}

static VALUE event_alloc(VALUE klass) {
  XEvent *ev = (XEvent*) calloc(1,sizeof(XEvent));
  return Data_Wrap_Struct(klass, 0, free, ev);
}

static VALUE event_wrap(VALUE klass, XEvent* ev) {
  return Data_Wrap_Struct(klass, 0, free, ev);
}

static VALUE event_type(VALUE self) {
  XEvent* ev;
  Data_Get_Struct(self,XEvent,ev);
  //if (ev->type < LASTEvent && !NIL_P(id_events[ev->type])) {
  //  return id_events[ev->type];
  //} else {
    return INT2NUM(ev->type);
    //}
}

static VALUE event_serial(VALUE self) {
  XEvent* ev;
  Data_Get_Struct(self,XEvent,ev);
  return INT2NUM(ev->xany.serial);
}

static VALUE event_wid(VALUE self) {
  XEvent* ev;
  Data_Get_Struct(self,XEvent,ev);
  return INT2NUM(ev->xany.window);
}

static VALUE event_client(VALUE self) {
  return rb_ivar_get(self,id_client);
}

static VALUE event_button(VALUE self) {
  return rb_ivar_get(self,id_button);
}

static VALUE event_x(VALUE self) {
  return rb_ivar_get(self,id_x);
}

static VALUE event_y(VALUE self) {
  return rb_ivar_get(self,id_y);
}

static VALUE event_keycode(VALUE self) {
  return rb_ivar_get(self,id_keycode);
}

static VALUE event_keysym(VALUE self) {
  return rb_ivar_get(self,id_keysym);
}

static VALUE event_state(VALUE self) {
  return rb_ivar_get(self,id_state);
}

static VALUE event_mods(VALUE self) {
  return rb_ivar_get(self,id_mods);
}

void Init_x11() {

  // Main module
  mX11 = rb_define_module("X11");

  rb_define_singleton_method(mX11, "error_handler", x11_error_handler, 0);
  rb_define_singleton_method(mX11, "error_handler=", x11_error_handler_set, 1);

  XSetErrorHandler(x11_internal_error_handler);
  
  // WindowManager
    
  cWM = rb_define_class_under(mX11, "WindowManager", rb_cObject);
  rb_define_alloc_func(cWM, wm_alloc);

  rb_define_method(cWM, "query", wm_query, 0);
  rb_define_method(cWM, "selected", wm_selected, 0);
  rb_define_method(cWM, "screenxpos", wm_sx, 0);
  rb_define_method(cWM, "screenypos", wm_sy, 0);
  rb_define_method(cWM, "screenwidth", wm_sw, 0);
  rb_define_method(cWM, "screenheight", wm_sh, 0);
  rb_define_method(cWM, "windowareaxpos", wm_wax, 0);
  rb_define_method(cWM, "windowareaypos", wm_way, 0);
  rb_define_method(cWM, "windowareawidth", wm_waw, 0);
  rb_define_method(cWM, "windowareaheight", wm_wah, 0);
  rb_define_method(cWM, "windowareaxpos=", wm_wax_set, 1);
  rb_define_method(cWM, "windowareaypos=", wm_way_set, 1);
  rb_define_method(cWM, "windowareawidth=", wm_waw_set, 1);
  rb_define_method(cWM, "windowareaheight=", wm_wah_set, 1);
  rb_define_method(cWM, "number_of_clients", wm_num_clients, 0);
  rb_define_method(cWM, "clients", wm_clients, 0);
  rb_define_method(cWM, "pending_event_count", wm_pending_event_count, 0);
  rb_define_method(cWM, "next_event", wm_event_next_type, 0);
  rb_define_method(cWM, "next_event_source", wm_event_next_source, 0);
  rb_define_method(cWM, "event_pop", wm_event_pop, 0);
  rb_define_method(cWM, "get_event", wm_get_event, 0);
  rb_define_method(cWM, "get_grab_key", wm_grab_key, 2);
  rb_define_method(cWM, "get_ungrab_key", wm_ungrab_key, 2);
  rb_define_method(cWM, "manage", wm_manage, 5);


  // Client

  cClient = rb_define_class_under(cWM, "Client", rb_cObject);
  rb_define_alloc_func(cClient, client_alloc);
    
  rb_define_method(cClient, "size=", client_size_set, 1);
  rb_define_method(cClient, "size", client_size, 0);
  rb_define_method(cClient, "name", client_name, 0);
  rb_define_method(cClient, "window_class", client_class, 0);
  rb_define_method(cClient, "wid", client_wid, 0);

  rb_define_method(cClient, "xpos", client_x, 0);
  rb_define_method(cClient, "ypos", client_y, 0);
  rb_define_method(cClient, "width", client_w, 0);
  rb_define_method(cClient, "height", client_h, 0);
  rb_define_method(cClient, "xpos=", client_x_set, 1);
  rb_define_method(cClient, "ypos=", client_y_set, 1);
  rb_define_method(cClient, "width=", client_w_set, 1);
  rb_define_method(cClient, "height=", client_h_set, 1);
  rb_define_method(cClient, "raise", client_raise, 0);
  rb_define_method(cClient, "ban", client_ban, 0);
  rb_define_method(cClient, "unban", client_unban, 0);
  rb_define_method(cClient, "border", client_border, 0);
  rb_define_method(cClient, "border=", client_border_set, 1);
  rb_define_method(cClient, "border_reset", client_border_dset, 0);


  // Event

  id_keymasks[0]  = rb_intern("Shift");
  id_keymasks[1]  = rb_intern("Lock");
  id_keymasks[2]  = rb_intern("Control");
  id_keymasks[3]  = rb_intern("Mod1");
  id_keymasks[4]  = rb_intern("Mod2");
  id_keymasks[5]  = rb_intern("Mod3");
  id_keymasks[6]  = rb_intern("Mod4");
  id_keymasks[7]  = rb_intern("Mod5");
  id_keymasks[8]  = rb_intern("Button1");
  id_keymasks[9]  = rb_intern("Button2");
  id_keymasks[10] = rb_intern("Button3");
  id_keymasks[11] = rb_intern("Button4");
  id_keymasks[12] = rb_intern("Button5");
  
  id_client = rb_intern("client");
  id_x = rb_intern("x");
  id_y = rb_intern("y");
  id_button = rb_intern("button");
  id_keycode = rb_intern("keycode");
  id_keysym = rb_intern("keysym");
  id_state = rb_intern("state");
  id_mods = rb_intern("mods");
  
  cEvent = rb_define_class_under(mX11,"Event",rb_cObject);
  rb_define_alloc_func(cEvent, event_alloc);

  rb_define_method(cEvent,"type",event_type,0);
  rb_define_method(cEvent,"serial",event_serial,0);
  rb_define_method(cEvent,"wid",event_wid,0);
  rb_define_method(cEvent,"client",event_client,0);
  rb_define_method(cEvent,"button",event_button,0);
  rb_define_method(cEvent,"x",event_x,0);
  rb_define_method(cEvent,"y",event_y,0);
  rb_define_method(cEvent,"keycode",event_keycode,0);
  rb_define_method(cEvent,"keysym",event_keysym,0);
  rb_define_method(cEvent,"state",event_state,0);
  rb_define_method(cEvent,"mods",event_mods,0);
}

