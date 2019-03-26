#ifndef SHOES_METHODS_M_H
#define SHOES_METHODS_M_H

/* Deprecated Extension API TODO: Find these (canvas.*?) and replace */
//#define GET_STRUCT(ele, var) \
//  shoes_##ele *var; \
//  Data_Get_Struct(self, shoes_##ele, var)
  
  
/*
 * New Extension API 
 */
// creates the rb_data_type_t part of underlaying C foundation of a becoming ruby object
#define TypedData_Type_New(wrapped) \
const rb_data_type_t wrapped##_type = { \
    #wrapped "_type", \
    { \
      (void (*)(void *))wrapped##_mark, \
      (void (*)(void *))wrapped##_free, \
      (size_t (*)(const void *))sizeof(wrapped), \
    }, \
    0, 0, \
    RUBY_TYPED_FREE_IMMEDIATELY, \
}

// unwraps a ruby object (implicit self), declare var of type wrapped
#define Get_TypedStruct(wrapped, var) \
  wrapped *var; \
  TypedData_Get_Struct(self, wrapped, &wrapped##_type, var)

// unwraps a ruby object (rbObject), declare var of type wrapped
#define Get_TypedStruct2(rbObject, wrapped, var) \
  wrapped *var; \
  TypedData_Get_Struct(rbObject, wrapped, &wrapped##_type, var)

// unwraps a ruby object (rbObject), "returns" the wrapped struct
#define Get_TypedStruct3(rbObject, wrapped) \
  (wrapped*)rb_check_typeddata((rbObject), (&wrapped##_type))
  

#define FUNC_M(name, func, argn) \
  VALUE \
  shoes_canvas_c_##func(int argc, VALUE *argv, VALUE self) \
  { \
    VALUE canvas, obj; \
    shoes_canvas *self_t; \
    Data_Get_Struct(self, shoes_canvas, self_t); \
    char *n = name; \
    if (rb_ary_entry(self_t->app->nesting, 0) == self || \
         ((rb_obj_is_kind_of(self, cWidget) || self == self_t->app->nestslot) && \
          RARRAY_LEN(self_t->app->nesting) > 0)) \
      canvas = rb_ary_entry(self_t->app->nesting, RARRAY_LEN(self_t->app->nesting) - 1); \
    else \
      canvas = self; \
    if (!rb_obj_is_kind_of(canvas, cCanvas)) \
      return ts_funcall2(canvas, rb_intern(n + 1), argc, argv); \
    obj = call_cfunc(CASTHOOK(shoes_canvas_##func), canvas, argn, argc, argv); \
    if (n[0] == '+' && RARRAY_LEN(self_t->app->nesting) == 0) shoes_canvas_repaint_all(self); \
    return obj; \
  } \
  VALUE \
  shoes_app_c_##func(int argc, VALUE *argv, VALUE self) \
  { \
    VALUE canvas; \
    char *n = name; \
    shoes_app *app; \
    Data_Get_Struct(self, shoes_app, app); \
    if (RARRAY_LEN(app->nesting) > 0) \
      canvas = rb_ary_entry(app->nesting, RARRAY_LEN(app->nesting) - 1); \
    else \
      canvas = app->canvas; \
    if (!rb_obj_is_kind_of(canvas, cCanvas)) \
      return ts_funcall2(canvas, rb_intern(n + 1), argc, argv); \
    return shoes_canvas_c_##func(argc, argv, canvas); \
  }
  
// TODO another change needed when canvas changes.
// TODO: hardcoded quess of width based on string length

#define SETUP_CONTROL_T(dh, dw, flex) \
  char *msg = ""; \
  int len = dw ? dw : 200; \
  shoes_canvas *canvas; \
  shoes_place place; \
  VALUE text = Qnil, ck = rb_obj_class(c); \
  Get_TypedStruct2(self, shoes_control, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  text = ATTR(self_t->attr, text); \
  if (!NIL_P(text)) { \
    text = shoes_native_to_s(text); \
    msg = RSTRING_PTR(text); \
    if (flex) len = ((int)RSTRING_LEN(text) * 8) + 32; \
  } \
  shoes_place_decide(&place, c, self_t->attr, len, 28 + dh, REL_CANVAS, TRUE)
  
#define SETUP_CONTROL(dh, dw, flex) \
  char *msg = ""; \
  int len = dw ? dw : 200; \
  shoes_control *self_t; \
  shoes_canvas *canvas; \
  shoes_place place; \
  VALUE text = Qnil, ck = rb_obj_class(c); \
  Data_Get_Struct(self, shoes_control, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  text = ATTR(self_t->attr, text); \
  if (!NIL_P(text)) { \
    text = shoes_native_to_s(text); \
    msg = RSTRING_PTR(text); \
    if (flex) len = ((int)RSTRING_LEN(text) * 8) + 32; \
  } \
  shoes_place_decide(&place, c, self_t->attr, len, 28 + dh, REL_CANVAS, TRUE)

//
// Macros for setting up drawing
//
#define SETUP_DRAWING(self_type, rel, dw, dh) \
  self_type *self_t; \
  shoes_place place; \
  shoes_canvas *canvas; \
  Data_Get_Struct(self, self_type, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  if (ATTR(self_t->attr, hidden) == Qtrue) return self; \
  shoes_place_decide(&place, c, self_t->attr, dw, dh, rel, REL_COORDS(rel) == REL_CANVAS)

//
// Defines a redirecting function which applies the element or transformation
// to the currently active canvas.  This is used in place of the old instance_eval
// and ensures that you have access to the App's instance variables while
// assembling elements in a layout.
//
// TODO: these need to change when canvas is converted to new macros
//
#define FUNC_T(name, func, argn) \
  VALUE \
  shoes_canvas_c_##func(int argc, VALUE *argv, VALUE self) \
  { \
    VALUE canvas, obj; \
    shoes_canvas *self_t; \
    if (RTYPEDDATA_P(self)) \
      self_t = (shoes_canvas *)RTYPEDDATA_DATA(self); \
    else \
      Data_Get_Struct(self, shoes_canvas, self_t); \
    char *n = name; \
    if (rb_ary_entry(self_t->app->nesting, 0) == self || \
         ((rb_obj_is_kind_of(self, cWidget) || self == self_t->app->nestslot) && \
          RARRAY_LEN(self_t->app->nesting) > 0)) \
      canvas = rb_ary_entry(self_t->app->nesting, RARRAY_LEN(self_t->app->nesting) - 1); \
    else \
      canvas = self; \
    if (!rb_obj_is_kind_of(canvas, cCanvas)) \
      return ts_funcall2(canvas, rb_intern(n + 1), argc, argv); \
    obj = call_cfunc(CASTHOOK(shoes_canvas_##func), canvas, argn, argc, argv); \
    if (n[0] == '+' && RARRAY_LEN(self_t->app->nesting) == 0) shoes_canvas_repaint_all(self); \
    return obj; \
  } \
  VALUE \
  shoes_app_c_##func(int argc, VALUE *argv, VALUE self) \
  { \
    VALUE canvas; \
    char *n = name; \
    shoes_app *app; \
    if (RTYPEDDATA_P(self)) \
      app = (shoes_app *)RTYPEDDATA_DATA(self); \
    else \
      Data_Get_Struct(self, shoes_app, app); \
    if (RARRAY_LEN(app->nesting) > 0) \
      canvas = rb_ary_entry(app->nesting, RARRAY_LEN(app->nesting) - 1); \
    else \
      canvas = app->canvas; \
    if (!rb_obj_is_kind_of(canvas, cCanvas)) \
      return ts_funcall2(canvas, rb_intern(n + 1), argc, argv); \
    return shoes_canvas_c_##func(argc, argv, canvas); \
  }


/* 
 * TODO in all following macros
 * Temporary "if (RTYPEDDATA_P(self) else ..." while fixing TypedData new API 
 * should be "Get_TypedStruct(shoes_##ele, self_t)" when fixed 
*/
//
// Transformations
//
#define TRANS_COMMON(ele, repaint) \
  VALUE \
  shoes_##ele##_transform(VALUE self, VALUE _m) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    ID m = SYM2ID(_m); \
    if (m == s_center || m == s_corner) \
    { \
      self_t->st = shoes_transform_detach(self_t->st); \
      self_t->st->mode = m; \
    } \
    else \
    { \
      rb_raise(rb_eArgError, "transform must be called with either :center or :corner."); \
    } \
    return self; \
  } \
  VALUE \
  shoes_##ele##_translate(VALUE self, VALUE _x, VALUE _y) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    double x, y; \
    x = NUM2DBL(_x); \
    y = NUM2DBL(_y); \
    self_t->st = shoes_transform_detach(self_t->st); \
    cairo_matrix_translate(&self_t->st->tf, x, y); \
    return self; \
  } \
  VALUE \
  shoes_##ele##_rotate(VALUE self, VALUE _deg) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    double rad; \
    rad = NUM2DBL(_deg) * SHOES_RAD2PI; \
    self_t->st = shoes_transform_detach(self_t->st); \
    cairo_matrix_rotate(&self_t->st->tf, -rad); \
    if (repaint) shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  VALUE \
  shoes_##ele##_scale(int argc, VALUE *argv, VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    VALUE _sx, _sy; \
    double sx, sy; \
    rb_scan_args(argc, argv, "11", &_sx, &_sy); \
    sx = NUM2DBL(_sx); \
    if (NIL_P(_sy)) sy = sx; \
    else            sy = NUM2DBL(_sy); \
    self_t->st = shoes_transform_detach(self_t->st); \
    cairo_matrix_scale(&self_t->st->tf, sx, sy); \
    if (repaint) shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  VALUE \
  shoes_##ele##_skew(int argc, VALUE *argv, VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    cairo_matrix_t matrix; \
    VALUE _sx, _sy; \
    double sx, sy; \
    rb_scan_args(argc, argv, "11", &_sx, &_sy); \
    sx = NUM2DBL(_sx) * SHOES_RAD2PI; \
    sy = 0.0; \
    if (!NIL_P(_sy)) sy = NUM2DBL(_sy) * SHOES_RAD2PI; \
    cairo_matrix_init(&matrix, 1.0, sy, sx, 1.0, 0.0, 0.0); \
    self_t->st = shoes_transform_detach(self_t->st); \
    cairo_matrix_multiply(&self_t->st->tf, &self_t->st->tf, &matrix); \
    if (repaint) shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  }

#define REPLACE_COMMON_T(ele) \
  VALUE \
  shoes_##ele##_replace(int argc, VALUE *argv, VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    long i; \
    VALUE texts, attr, block; \
    attr = Qnil; \
    texts = rb_ary_new(); \
    for (i = 0; i < argc; i++) \
    { \
      if (rb_obj_is_kind_of(argv[i], rb_cHash)) \
        attr = argv[i]; \
      else \
        rb_ary_push(texts, argv[i]); \
    } \
    self_t->texts = texts; \
    if (!NIL_P(attr)) self_t->attr = attr; \
    block = shoes_find_textblock(self); \
    Get_TypedStruct2(block, shoes_textblock, block_t); \
    shoes_textblock_uncache(block_t, TRUE); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  }

#define REPLACE_COMMON(ele) \
  VALUE \
  shoes_##ele##_replace(int argc, VALUE *argv, VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    long i; \
    shoes_textblock *block_t; \
    VALUE texts, attr, block; \
    attr = Qnil; \
    texts = rb_ary_new(); \
    for (i = 0; i < argc; i++) \
    { \
      if (rb_obj_is_kind_of(argv[i], rb_cHash)) \
        attr = argv[i]; \
      else \
        rb_ary_push(texts, argv[i]); \
    } \
    self_t->texts = texts; \
    if (!NIL_P(attr)) self_t->attr = attr; \
    block = shoes_find_textblock(self); \
    Data_Get_Struct(block, shoes_textblock, block_t); \
    shoes_textblock_uncache(block_t, TRUE); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  }

//
// Common methods
//
#define EVENT_COMMON(ele, est, sym) \
  VALUE \
  shoes_##ele##_##sym(int argc, VALUE *argv, VALUE self) \
  { \
    shoes_##est *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##est, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##est, self_t); \
    VALUE str = Qnil, blk = Qnil; \
  \
    rb_scan_args(argc, argv, "01&", &str, &blk); \
    if (NIL_P(self_t->attr)) self_t->attr = rb_hash_new(); \
    rb_hash_aset(self_t->attr, ID2SYM(s_##sym), NIL_P(blk) ? str : blk ); \
    return self; \
  }

#define CLASS_COMMON(ele) \
  VALUE \
  shoes_##ele##_style(int argc, VALUE *argv, VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    rb_arg_list args; \
    switch (rb_parse_args(argc, argv, "h,", &args)) { \
      case 1: \
        if (NIL_P(self_t->attr)) self_t->attr = rb_hash_new(); \
        rb_funcall(self_t->attr, s_update, 1, args.a[0]); \
        shoes_canvas_repaint_all(self_t->parent); \
      break; \
      case 2: return rb_obj_freeze(rb_obj_dup(self_t->attr)); \
    } \
    return self; \
  } \
  \
  VALUE \
  shoes_##ele##_displace(VALUE self, VALUE x, VALUE y) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    ATTRSET(self_t->attr, displace_left, x); \
    ATTRSET(self_t->attr, displace_top, y); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  \
  VALUE \
  shoes_##ele##_move(VALUE self, VALUE x, VALUE y) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    ATTRSET(self_t->attr, left, x); \
    ATTRSET(self_t->attr, top, y); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  }

#define CLASS_COMMON2(ele) \
  VALUE \
  shoes_##ele##_hide(VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    ATTRSET(self_t->attr, hidden, Qtrue); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  \
  VALUE \
  shoes_##ele##_show(VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    ATTRSET(self_t->attr, hidden, Qfalse); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  \
  VALUE \
  shoes_##ele##_toggle(VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    ATTRSET(self_t->attr, hidden, ATTR(self_t->attr, hidden) == Qtrue ? Qfalse : Qtrue); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  \
  VALUE \
  shoes_##ele##_is_hidden(VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    if (RTEST(ATTR(self_t->attr, hidden))) \
      return ATTR(self_t->attr, hidden); \
    else return Qfalse; \
  } \
  CLASS_COMMON(ele); \
  EVENT_COMMON(ele, ele, change); \
  EVENT_COMMON(ele, ele, click); \
  EVENT_COMMON(ele, ele, release); \
  EVENT_COMMON(ele, ele, hover); \
  EVENT_COMMON(ele, ele, leave);

#define PLACE_COMMON(ele) \
  VALUE \
  shoes_##ele##_get_parent(VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    return self_t->parent; \
  } \
  \
  VALUE \
  shoes_##ele##_get_left(VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    shoes_canvas *canvas = NULL; \
    if (!NIL_P(self_t->parent)) { \
      Data_Get_Struct(self_t->parent, shoes_canvas, canvas); \
    } else { \
      Data_Get_Struct(self, shoes_canvas, canvas); \
    } \
    return INT2NUM(self_t->place.x - CPX(canvas)); \
  } \
  \
  VALUE \
  shoes_##ele##_get_top(VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    shoes_canvas *canvas = NULL; \
    if (!NIL_P(self_t->parent)) { \
      Data_Get_Struct(self_t->parent, shoes_canvas, canvas); \
    } else { \
      Data_Get_Struct(self, shoes_canvas, canvas); \
    } \
    return INT2NUM(self_t->place.y - CPY(canvas)); \
  } \
  \
  VALUE \
  shoes_##ele##_get_height(VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    return INT2NUM(self_t->place.h); \
  } \
  \
  VALUE \
  shoes_##ele##_get_width(VALUE self) \
  { \
    shoes_##ele *self_t; \
    if (RTYPEDDATA_P(self)) \
      TypedData_Get_Struct(self, shoes_##ele, RTYPEDDATA_TYPE(self), self_t); \
    else \
      Data_Get_Struct(self, shoes_##ele, self_t); \
    return INT2NUM(self_t->place.w); \
  }
  
//
// Macros for setting up drawing
// TODO change when canvas is converted
//
#define SETUP_DRAWING_T(self_type, rel, dw, dh) \
  shoes_place place; \
  shoes_canvas *canvas; \
  Get_TypedStruct(self_type, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  if (ATTR(self_t->attr, hidden) == Qtrue) return self; \
  shoes_place_decide(&place, c, self_t->attr, dw, dh, rel, REL_COORDS(rel) == REL_CANVAS)

#define SETUP_CANVAS() \
  shoes_canvas *canvas; \
  cairo_t *cr; \
  if (RTYPEDDATA_P(self)) \
    canvas = (shoes_canvas *)RTYPEDDATA_DATA(self); \
   else \
    Data_Get_Struct(self, shoes_canvas, canvas); \
  cr = CCR(canvas)
   
#define SETUP_IMAGE() \
  shoes_place place; \
  shoes_image *image; \
  Data_Get_Struct(self, shoes_image, image); \
  shoes_image_ensure_dup(image); \
  shoes_place_exact(&place, attr, 0, 0); \
  if (NIL_P(attr)) attr = image->attr; \
  else if (!NIL_P(image->attr)) attr = rb_funcall(image->attr, s_merge, 1, attr);


#endif 
