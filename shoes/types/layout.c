/*
 * layout.c 
*/
#include "shoes/app.h"
#include "shoes/canvas.h"
#include "shoes/ruby.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/version.h"
#include "shoes/types/types.h"
#include "shoes/types/settings.h"
#include "shoes/types/layout.h"
#include <math.h>
//
// Shoes::Layout needs to be a class so it can be subclassed
//
extern VALUE cButton, cBackground;

FUNC_M("+layout", layout, -1);

void shoes_layout_init() {
  cLayout = rb_define_class_under(cTypes, "Layout", cNative);
  rb_define_method(cLayout, "rule", CASTHOOK(shoes_layout_add_rule), 1);
  rb_define_method(cLayout, "compute", CASTHOOK(shoes_layout_compute), 1);
  
  RUBY_M("+layout", layout, -1);
}

void shoes_layout_mark(shoes_layout *ly) {
  rb_gc_mark_maybe(ly->canvas);
  rb_gc_mark_maybe(ly->delegate);
}

static void shoes_layout_free(shoes_layout *ly) {
  RUBY_CRITICAL(SHOE_FREE(ly));
}

VALUE shoes_layout_alloc(VALUE klass) {
  VALUE obj;
  shoes_layout *ly = SHOE_ALLOC(shoes_layout);
  SHOE_MEMZERO(ly, shoes_layout, 1);
  obj = Data_Wrap_Struct(klass, shoes_layout_mark, shoes_layout_free, ly);
  // set fields ? 
  ly->x = 10;
  ly->y = 10;
  ly->delegate = Qnil;
  ly->canvas = Qnil;
  return obj;
}

VALUE shoes_layout_new(VALUE attr, VALUE parent) {
    fprintf(stderr, "shoes_layout_new called\n");
    VALUE obj = shoes_layout_alloc(cLayout);
    shoes_layout *ly;
    Data_Get_Struct(obj, shoes_layout, ly);
    // Most of shoes thinks its a Flow
    VALUE canvas = shoes_slot_new(cFlow, attr, parent);
    ly->canvas = canvas;
    shoes_canvas *cvs;
    Data_Get_Struct(canvas, shoes_canvas, cvs);
    // get manager from attr, put in delegate.
    VALUE mgr;
    ID s_manager = rb_intern ("manager");
    mgr = ATTR(attr, manager);
    if (! NIL_P(mgr))
      ly->delegate = mgr;
    cvs->layout_mgr = obj; // me
    return obj;
}

// called from inside shoes (shoes_add_ele)
void shoes_layout_add_ele(shoes_canvas *canvas, VALUE ele) {
  if (rb_obj_is_kind_of(ele, cBackground)) {
    fprintf(stderr, "skipping background widget\n");
    return;
  }
  // Find a delegate or use the Toy ?
  if (canvas->layout_mgr != Qnil) {
    shoes_layout *ly;
    Data_Get_Struct(canvas->layout_mgr, shoes_layout, ly);
    if (! NIL_P(ly->delegate)) {
      fprintf(stderr,"Delegating\n");
      VALUE del = ly->delegate;
      ID s_addw = rb_intern("add_widget");
      if (rb_respond_to(del, s_addw))
        rb_funcall(del, s_addw, 1, ele);
      //return;
    }
  }
  if (rb_obj_is_kind_of(ele, cButton)) {
    fprintf(stderr, "shoes_layout_add Button\n");
  } else if (rb_obj_is_kind_of(ele, cPara)) {
    fprintf(stderr,"shoes layout_add Para\n");
  } else {
    fprintf(stderr, "shoes_layout_add something\n");
  }
  return; 
}

VALUE shoes_layout_delete_ele(shoes_canvas *canvas, VALUE ele) {
  fprintf(stderr,"shoes_layout_delete called\n");
}

extern VALUE shoes_layout_add_rule(VALUE self, VALUE rule) {
  fprintf(stderr,"shoes_layout_add_rule called\n");
}

extern VALUE shoes_layout_compute(VALUE self) {
  fprintf(stderr, "shoes_layout_compute called\n");
}
