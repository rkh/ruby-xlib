#include <ruby.h>
#include <x11.h>

VALUE mrubyx11, cClient, cKey, cWM;
static VALUE client_make(VALUE klass, Client* c); // I need this alot

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

// Free our WindowManager
static void wm_free(void *p) {
    Destroy_WM(p);
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
    Data_Get_Struct(self, WM, newwm);
    return client_make(cClient, newwm->selected);
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

static VALUE wm_event_pending(VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    if (event_pending(newwm))
        return Qtrue;
    else
        return Qfalse;
}

static VALUE wm_event_next_source(VALUE self) {
    WM *newwm;
    int c;

    Data_Get_Struct(self, WM, newwm);
    c = event_next_source(newwm);
    if (c == -1) {
        rb_raise(rb_eArgError, "no pending events - check first!");
        return Qnil;
    }
    if (c != -2)
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
        rb_raise(rb_eArgError, "no pending events - check first!");
        return Qnil;
    }
}

static VALUE wm_event_pop(VALUE self) {
    WM *newwm;

    Data_Get_Struct(self, WM, newwm);
    event_pop(newwm);
    return Qnil;
}

static VALUE client_name(VALUE self) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    return rb_str_new2(c->name);
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
    
    if (RARRAY(valarray)->len >= 4) {
        x = RARRAY(valarray)->ptr[0];
        y = RARRAY(valarray)->ptr[1];
        w = RARRAY(valarray)->ptr[2];
        h = RARRAY(valarray)->ptr[3];
        resize(c->manager, c, x, y, w, h, False);
    }
    else
        rb_raise(rb_eArgError, "wrong number of arguments [x,y,w,h]");
    return Qnil;
}

void Init_x11() {
    mrubyx11 = rb_define_module("X11");

    cWM = rb_define_class_under(mrubyx11, "WindowManager", rb_cObject);
    rb_define_alloc_func(cWM, wm_alloc);

    cClient = rb_define_class_under(cWM, "Client", rb_cObject);
    rb_define_alloc_func(cClient, client_alloc);

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
    rb_define_method(cWM, "event_pending?", wm_event_pending, 0);
    rb_define_method(cWM, "next_event", wm_event_next_type, 0);
    rb_define_method(cWM, "next_event_source", wm_event_next_source, 0);
    rb_define_method(cWM, "event_pop", wm_event_pop, 0);

    rb_define_method(cClient, "size=", client_size_set, 1);
    rb_define_method(cClient, "size", client_size, 0);
    rb_define_method(cClient, "name", client_name, 0);
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
}


