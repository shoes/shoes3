#include "shoes/types/native.h"
#include "shoes/types/event.h"

// ruby
VALUE cNative;

PLACE_COMMON(control);
CLASS_COMMON(control);
EVENT_COMMON(control, control, click);
EVENT_COMMON(control, control, change);

/* TODO: May need to refactor all types depending on native to use either 
* types/native_<type>{.c,.h} or types/native/<type>{.c,.h}. That would be 
* proper etiquette. For now, just using shoes_0_native_type_init().
*/

// Many widgets depend on cNative, so initialization needs to be triggered first.
void shoes_0_native_type_init() {
#ifdef NEW_MACRO_CONTROL
    cNative  = rb_define_class_under(cTypes, "Native", rb_cData);
#else
    cNative  = rb_define_class_under(cTypes, "Native", rb_cObject);
    rb_define_alloc_func(cNative, shoes_control_alloc);
#endif    
    rb_define_method(cNative, "app", CASTHOOK(shoes_canvas_get_app), 0);
    rb_define_method(cNative, "parent", CASTHOOK(shoes_control_get_parent), 0);
    rb_define_method(cNative, "style", CASTHOOK(shoes_control_style), -1);
    rb_define_method(cNative, "displace", CASTHOOK(shoes_control_displace), 2);
    rb_define_method(cNative, "focus", CASTHOOK(shoes_control_focus), 0);
    rb_define_method(cNative, "hide", CASTHOOK(shoes_control_hide), 0);
    rb_define_method(cNative, "show", CASTHOOK(shoes_control_show), 0);
    rb_define_method(cNative, "state=", CASTHOOK(shoes_control_set_state), 1);
    rb_define_method(cNative, "state", CASTHOOK(shoes_control_get_state), 0);
    rb_define_method(cNative, "move", CASTHOOK(shoes_control_move), 2);
    rb_define_method(cNative, "top", CASTHOOK(shoes_control_get_top), 0);
    rb_define_method(cNative, "left", CASTHOOK(shoes_control_get_left), 0);
    rb_define_method(cNative, "width", CASTHOOK(shoes_control_get_width), 0);
    rb_define_method(cNative, "height", CASTHOOK(shoes_control_get_height), 0);
    rb_define_method(cNative, "remove", CASTHOOK(shoes_control_remove), 0);
}

// ruby
void shoes_control_mark(shoes_control *control) {
    rb_gc_mark_maybe(control->parent);
    rb_gc_mark_maybe(control->attr);
}

void shoes_control_free(shoes_control *control) {
    if (control->ref != NULL) shoes_native_control_free(control->ref);
    RUBY_CRITICAL(free(control));
}

#ifdef NEW_MACRO_CONTROL
// creates struct shoes_control_type
TypedData_Type_New(shoes_control);
//#undef Data_Get_Struct
#undef Data_Wrap_Struct
#endif

VALUE shoes_control_alloc(VALUE klass) {
    VALUE obj;
    shoes_control *control = SHOE_ALLOC(shoes_control);
    SHOE_MEMZERO(control, shoes_control, 1);
#ifdef NEW_MACRO_CONTROL
    obj = TypedData_Wrap_Struct(klass, &shoes_control_type, control);
#else
    obj = Data_Wrap_Struct(klass, shoes_control_mark, shoes_control_free, control);
#endif
    control->attr = Qnil;
    control->parent = Qnil;
    return obj;
}

VALUE shoes_control_new(VALUE klass, VALUE attr, VALUE parent) {
    VALUE obj = shoes_control_alloc(klass);
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(obj, shoes_control, control);
#else
    shoes_control *control;
    Data_Get_Struct(obj, shoes_control, control);
#endif
    control->attr = attr;
    control->parent = parent;
    return obj;
}



VALUE shoes_control_focus(VALUE self) {
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
//  ATTRSET(self_t->attr, hidden, Qtrue);
    if (self_t->ref != NULL) shoes_native_control_focus(self_t->ref);
    return self;
}

VALUE shoes_control_get_state(VALUE self) {
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
    return ATTR(self_t->attr, state);
}

static VALUE shoes_control_try_state(shoes_control *self_t, VALUE state) {
    unsigned char cstate;
    if (NIL_P(state))
        cstate = CONTROL_NORMAL;
    else if (TYPE(state) == T_STRING) {
        if (strncmp(RSTRING_PTR(state), "disabled", 8) == 0)
            cstate = CONTROL_DISABLED;
        else if (strncmp(RSTRING_PTR(state), "readonly", 8) == 0)
            cstate = CONTROL_READONLY;
        else {
            shoes_error("control can't have :state of %s\n", RSTRING_PTR(state));
            return Qfalse;
        }
    } else return Qfalse;

    if (self_t->ref != NULL) {
        if (cstate == CONTROL_NORMAL)
            shoes_native_control_state(self_t->ref, TRUE, TRUE);
        else if (cstate == CONTROL_DISABLED)
            shoes_native_control_state(self_t->ref, FALSE, TRUE);
        else if (cstate == CONTROL_READONLY)
            shoes_native_control_state(self_t->ref, TRUE, FALSE);
    }
    return Qtrue;
}

VALUE shoes_control_set_state(VALUE self, VALUE state) {
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
    if (shoes_control_try_state(self_t, state))
        ATTRSET(self_t->attr, state, state);
    return self;
}

VALUE shoes_control_temporary_hide(VALUE self) {
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
    if (self_t->ref != NULL) shoes_control_hide_ref(self_t->ref);
    return self;
}

VALUE shoes_control_hide(VALUE self) {
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
    ATTRSET(self_t->attr, hidden, Qtrue);
    if (self_t->ref != NULL) shoes_control_hide_ref(self_t->ref);
    return self;
}

VALUE shoes_control_temporary_show(VALUE self) {
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
    if (self_t->ref != NULL) shoes_control_show_ref(self_t->ref);
    return self;
}

VALUE shoes_control_show(VALUE self) {
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
    ATTRSET(self_t->attr, hidden, Qfalse);
    if (self_t->ref != NULL) shoes_control_show_ref(self_t->ref);
    return self;
}

VALUE shoes_control_remove(VALUE self) {
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
    shoes_canvas_remove_item(self_t->parent, self, 1, 0);
    
    shoes_canvas *canvas;
#ifdef NEW_MACRO_CANVAS
    TypedData_Get_Struct(self_t->parent, shoes_canvas, &shoes_canvas_type, canvas);
#else
    Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
#endif
    if (self_t->ref != NULL) {
        SHOES_CONTROL_REF ref = self_t->ref;
        if (rb_obj_is_kind_of(self, cRadio)) {
          shoes_radio_remove_group(ref, canvas->app->groups);
        }  
        self_t->ref = NULL;
        shoes_native_control_remove(ref, canvas);
    }
    return self;
}

void shoes_control_check_styles(shoes_control *self_t) {
    VALUE x = ATTR(self_t->attr, state);
    shoes_control_try_state(self_t, x);
}

void shoes_control_send(VALUE self, ID event) {
    VALUE click;
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
    VALUE sendevt = Qtrue;
    shoes_canvas *parent_canvas;
#ifdef NEW_MACRO_CANVAS
    TypedData_Get_Struct(self_t->parent, shoes_canvas, &shoes_canvas_type, parent_canvas);
#else
    Data_Get_Struct(self_t->parent, shoes_canvas, parent_canvas);
#endif
   // do we have an event overide? 
    if (parent_canvas->app->use_event_handler && event == s_click) {
      //fprintf(stderr, "C: button click seeks permission\n");
      shoes_app *app = parent_canvas->app;
      shoes_canvas *app_canvas;
#ifdef NEW_MACRO_CANVAS
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, app_canvas);
#else
      Data_Get_Struct(app->canvas, shoes_canvas, app_canvas);
#endif
      VALUE evtproc = ATTR(app_canvas->attr, event);
      if (! NIL_P(evtproc)) {
        // TODO:  verify selt_t->place.name is accurate, somehow
        int x,y,w,h  = 0;
        x = self_t->place.x;
        y = self_t->place.y;
        h = self_t->place.h;
        w = self_t->place.w;
        // use 
        VALUE evt = shoes_event_new_widget(cShoesEvent, s_btn_activate, self, 1, x, y, w, h, Qnil, Qnil);
        shoes_safe_block(app->canvas, evtproc, rb_ary_new3(1, evt));
#ifdef NEW_MACRO_EVENT
        Get_TypedStruct2(evt, shoes_event, tevent);
#else
        shoes_event *tevent;
        Data_Get_Struct(evt, shoes_event, tevent);
#endif
        sendevt = shoes_event_contrain_TF(tevent->accept);
      } else
        fprintf(stderr, "shoes_control_send: doesn't have event - but it should\n");
    }
    if ((sendevt == Qtrue) && !NIL_P(self_t->attr)) {
        // TODO: bug here for replay
        click = rb_hash_aref(self_t->attr, ID2SYM(event));
        if (!NIL_P(click))
            shoes_safe_block(self_t->parent, click, rb_ary_new3(1, self));
    }
}


VALUE shoes_control_get_tooltip(VALUE self) {
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
    return shoes_native_control_get_tooltip(self_t->ref);
}

VALUE shoes_control_set_tooltip(VALUE self, VALUE tooltip) {
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    shoes_control *self_t;
    Data_Get_Struct(self, shoes_control, self_t);
#endif
    if (self_t->ref != NULL)
        shoes_native_control_set_tooltip(self_t->ref, tooltip);
    return self;
}


void shoes_control_hide_ref(SHOES_CONTROL_REF ref) {
    if (ref != NULL) shoes_native_control_hide(ref);
}

void shoes_control_show_ref(SHOES_CONTROL_REF ref) {
    if (ref != NULL) shoes_native_control_show(ref);
}

// called by low level when feeding events
VALUE shoes_control_is_here(VALUE self, int x, int y) {
#ifdef NEW_MACRO_CONTROL
  Get_TypedStruct2(self, shoes_control, ctl);
#else
  shoes_control *ctl;
  Data_Get_Struct(self, shoes_control, ctl);
#endif
  if (IS_INSIDE(ctl, x, y)) 
    return Qtrue;
  else 
    return Qnil;
}


// canvas
VALUE shoes_canvas_hide(VALUE self) {
    shoes_canvas *self_t;
#ifdef NEW_MACRO_CANVAS
    TypedData_Get_Struct(self, shoes_canvas, &shoes_canvas_type, self_t);
#else
    Data_Get_Struct(self, shoes_canvas, self_t);
#endif
    ATTRSET(self_t->attr, hidden, Qtrue);
    shoes_canvas_ccall(self, shoes_control_temporary_hide, shoes_native_control_hide, 1);
    shoes_canvas_repaint_all(self);
    return self;
}

VALUE shoes_canvas_show(VALUE self) {
    shoes_canvas *self_t;
#ifdef NEW_MACRO_CANVAS
    TypedData_Get_Struct(self, shoes_canvas, &shoes_canvas_type, self_t);
#else
    Data_Get_Struct(self, shoes_canvas, self_t);
#endif
    ATTRSET(self_t->attr, hidden, Qfalse);
    shoes_canvas_ccall(self, shoes_control_temporary_show, shoes_native_control_show, 1);
    shoes_canvas_repaint_all(self);
    return self;
}
