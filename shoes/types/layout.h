#ifndef SHOES_LAYOUT_TYPE_H
#define SHOES_LAYOUT_TYPE_H

typedef struct {
  VALUE delegate;
  VALUE canvas;
  // below belongs to the C crafted default layout manager, what ever that
  // is. A Toy for now.
  int x;
  int y;
} shoes_layout;
extern VALUE cLayout;
extern void shoes_layout_init();
extern VALUE shoes_layout_new(VALUE attr, VALUE parent);
extern VALUE shoes_layout_add_rule(VALUE self, VALUE rule);
extern VALUE shoes_layout_compute(VALUE self);

extern void shoes_layout_add_ele(shoes_canvas *canvas, VALUE ele);
extern VALUE shoes_layout_delete_ele(shoes_canvas *canvas, VALUE ele);

#endif
