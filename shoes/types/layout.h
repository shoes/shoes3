#ifndef SHOES_LAYOUT_TYPE_H
#define SHOES_LAYOUT_TYPE_H

typedef struct {
  VALUE delegate;
  VALUE canvas;  
  // fields below belong to the C crafted default layout manager, what ever that
  // turns out to be. 
  int x;
  int y;
} shoes_layout;

extern VALUE cLayout;
void shoes_layout_init();
VALUE shoes_layout_new(VALUE attr, VALUE parent);
// slot like methods:
VALUE shoes_layout_insert(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_delete_at(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_clear(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_refresh(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_add_rule(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_finish(int argc, VALUE *argv, VALUE self);

// canvas calls these, delegate to usr or the secret layout
void shoes_layout_cleared(shoes_canvas *canvas);
void shoes_layout_add_ele(shoes_canvas *canvas, VALUE ele);
VALUE shoes_layout_delete_ele(shoes_canvas *canvas, VALUE ele);

//  TODO: delagate methods for the secret default manager. 
void shoes_layout_default_add(shoes_canvas *canvas, VALUE ele);
void shoes_layout_default_clear(shoes_canvas *canvas);
#endif
