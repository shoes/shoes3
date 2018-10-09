#ifndef SHOES_LAYOUT_TYPE_H
#define SHOES_LAYOUT_TYPE_H

typedef struct {
  VALUE delegate;
  VALUE canvas;  // TODO is this used? 
  // fields below belong to the C crafted default layout manager, what ever that
  // is. 
  int x;
  int y;
} shoes_layout;

extern VALUE cLayout;
void shoes_layout_init();
VALUE shoes_layout_new(VALUE attr, VALUE parent);
// slot like methods:
VALUE shoes_layout_append(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_prepend(VALUE self, VALUE ele);
VALUE shoes_layout_add_rule(VALUE self, VALUE rule);
VALUE shoes_layout_compute(VALUE self);

void shoes_layout_add_ele(shoes_canvas *canvas, VALUE ele);
void shoes_layout_clear(shoes_canvas *canvas);
VALUE shoes_layout_delete_ele(shoes_canvas *canvas, VALUE ele);

//  methods for the default manager. TODO: Write it.
void shoes_layout_default_add(shoes_canvas *canvas, VALUE ele);
void shoes_layout_default_clear(shoes_canvas *canvas);
#endif
