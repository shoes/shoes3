#ifndef SHOES_LAYOUT_TYPE_H
#define SHOES_LAYOUT_TYPE_H

// C level layouts. Not all are implemented. 
typedef enum {
  Layout_None,
  Layout_VFL,
  Layout_Grid,
} Layout_Types;

typedef struct {
  VALUE delegate;   // user Written Ruby layout class
  VALUE canvas;  
  int cache_w;      // used to synthesize 'size' event
  int cache_h;
  // fields below belong to the C crafted layout manager(s), what ever that
  // turns out to be. 
  Layout_Types mgr;
  VALUE views;      // VFL speak. Aka Widgets, Shoes elements
  VALUE metrics;    // VFL speak. sort of like a macro. 
  VALUE rbconstraints; // VFL parse results, ruby-ized.  not used ? 
  void *root;       // whatever the layout wants.
} shoes_layout;

// all drawables do/should implement this at the top - slightly safer
// than deconstructing shoes_canvas for everything. Treat as read-only
typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
} shoes_abstract;

extern VALUE cLayout;
void shoes_layout_init();
VALUE shoes_layout_new(VALUE attr, VALUE parent);
// user visible, slot like methods:
VALUE shoes_layout_insert(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_delete_at(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_clear(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_add_rules(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_finish(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_get_height(VALUE self);
VALUE shoes_layout_get_width(VALUE self);
VALUE shoes_layout_start(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_style(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_parse_vfl(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_get_constraints(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_append_constraints(int argc, VALUE *argv, VALUE self);

// canvas calls these, delegate to usr written (ruby) or the new C crafted layouts
void shoes_layout_size(shoes_canvas *canvas, int pass);
void shoes_layout_cleared(shoes_canvas *canvas);
void shoes_layout_add_ele(shoes_canvas *canvas, VALUE ele);
VALUE shoes_layout_delete_ele(shoes_canvas *canvas, VALUE ele);

//  TODO: delegate methods for the new C internal layot(s). 
void shoes_layout_internal_setup(shoes_layout *lay, shoes_canvas *canvas,
  VALUE attr);
void shoes_layout_internal_add(shoes_canvas *canvas, VALUE ele);
VALUE shoes_layout_internal_delete_at(shoes_layout *lay, shoes_canvas *canvas,
      VALUE ele, int pos);
void shoes_layout_internal_clear(shoes_canvas *canvas);
void shoes_layout_internal_size(shoes_layout *lay, shoes_canvas *canvas, int pass);
void shoes_layout_internal_finish(shoes_layout *lay, shoes_canvas *canvas);
VALUE shoes_layout_internal_rules(shoes_layout *lay, shoes_canvas *canvas, VALUE arg);
#endif
