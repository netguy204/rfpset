#include "fpset.h"
#include <ruby.h>

static VALUE rfpset_bonjour(VALUE self) {
  return rb_str_new2("hello world!");
}

void Init_rfpset() {
  VALUE klass = rb_define_class("FPSet", rb_cObject);
  rb_define_singleton_method(klass, "bonjour", rfpset_bonjour, 0);
}
