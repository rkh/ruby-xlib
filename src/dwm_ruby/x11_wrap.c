#include <ruby.h>
#include <x11.h>

VALUE mrubyx11, cClient, cKey, cWM;

// Allocate a new WindowManager object
// using the supplied Init_WM
static VALUE wm_alloc(VALUE klass) {
    WM* newwm;
    VALUE obj;
    newwm = Init_WM();
    obj = Data_Wrap_Struct(klass, 0, Destroy_WM, newwm);
    return obj;
}

// Query for clients in our WindowManager
static VALUE wm_initialize (VALUE self) {
    WM *newwm;
    Data_Get_Struct(self, WM, newwm);
    newwm->clients = query_clients(newwm);
    return self;
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

static void client_free(void *p) {
    free(p);
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
//    return rb_ary_new4(newwm->clients_num, newwm->clients);
}

static VALUE client_name(VALUE self) {
    Client *c;
    Data_Get_Struct(self, Client, c);
    return rb_str_new2(c->name);
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
    rb_define_method(cWM, "screenx", wm_sx, 0);
    rb_define_method(cWM, "screeny", wm_sy, 0);
    rb_define_method(cWM, "screenw", wm_sw, 0);
    rb_define_method(cWM, "screenh", wm_sh, 0);
    rb_define_method(cWM, "windowareax", wm_wax, 0);
    rb_define_method(cWM, "windowareay", wm_way, 0);
    rb_define_method(cWM, "windowareaw", wm_waw, 0);
    rb_define_method(cWM, "windowareah", wm_wah, 0);
    rb_define_method(cWM, "windowareax=", wm_wax_set, 1);
    rb_define_method(cWM, "windowareay=", wm_way_set, 1);
    rb_define_method(cWM, "windowareaw=", wm_waw_set, 1);
    rb_define_method(cWM, "windowareah=", wm_wah_set, 1);
    rb_define_method(cWM, "num_clients", wm_num_clients, 0);
    rb_define_method(cWM, "clients", wm_clients, 0);

    rb_define_method(cClient, "size=", client_size_set, 1);
    rb_define_method(cClient, "size", client_size, 0);
    rb_define_method(cClient, "name", client_name, 0);
    rb_define_method(cClient, "x", client_x, 0);
    rb_define_method(cClient, "y", client_y, 0);
    rb_define_method(cClient, "w", client_w, 0);
    rb_define_method(cClient, "h", client_h, 0);
    rb_define_method(cClient, "x=", client_x_set, 1);
    rb_define_method(cClient, "y=", client_y_set, 1);
    rb_define_method(cClient, "w=", client_w_set, 1);
    rb_define_method(cClient, "h=", client_h_set, 1);
}


