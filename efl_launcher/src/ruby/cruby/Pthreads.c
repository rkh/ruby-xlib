#include <pthread.h>
#include <ruby.h>



VALUE Pthreads = Qnil;

VALUE method_pthreads(VALUE self, VALUE program)
{
    pthread_t run;
    pthread_create(&run, NULL, (void *(*)(void*))system, StringValuePtr(program));
    pthread_detach(run);
    return Qnil;
}

void Init_Pthreads() {
    Pthreads = rb_define_module("Pthreads");
    rb_define_method(Pthreads, "pthreads", method_pthreads, 1);    
}    
    
