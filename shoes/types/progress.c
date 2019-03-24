#include "shoes/types/native.h"
#include "shoes/types/progress.h"
#include "shoes/app.h"

// ruby
VALUE cProgress;

#ifdef NEW_MACRO_APP
FUNC_T("+progress", progress, -1);
#else
FUNC_M("+progress", progress, -1);
#endif

void shoes_progress_init() {
    cProgress  = rb_define_class_under(cTypes, "Progress", cNative);
    rb_define_method(cProgress, "draw", CASTHOOK(shoes_progress_draw), 2);
    rb_define_method(cProgress, "fraction", CASTHOOK(shoes_progress_get_fraction), 0);
    rb_define_method(cProgress, "fraction=", CASTHOOK(shoes_progress_set_fraction), 1);
    rb_define_method(cProgress, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cProgress, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);

    RUBY_M("+progress", progress, -1);
}

// ruby
VALUE shoes_progress_draw(VALUE self, VALUE c, VALUE actual) {
#ifdef NEW_MACRO_CONTROL
    SETUP_CONTROL_T(0, 0, FALSE);
#else
    SETUP_CONTROL(0, 0, FALSE);
#endif
    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            self_t->ref = shoes_native_progress(self, canvas, &place, self_t->attr, msg);
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        } else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }

    FINISH();

    return self;
}

VALUE shoes_progress_get_fraction(VALUE self) {
    double perc = 0.;
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    GET_STRUCT(control, self_t);
#endif
    if (self_t->ref != NULL)
        perc = shoes_native_progress_get_fraction(self_t->ref);
    return rb_float_new(perc);
}

VALUE shoes_progress_set_fraction(VALUE self, VALUE _perc) {
    double perc = min(max(NUM2DBL(_perc), 0.0), 1.0);
#ifdef NEW_MACRO_CONTROL
    Get_TypedStruct2(self, shoes_control, self_t);
#else
    GET_STRUCT(control, self_t);
#endif
    if (self_t->ref != NULL)
        shoes_native_progress_set_fraction(self_t->ref, perc);
    return self;
}

// canvas
VALUE shoes_canvas_progress(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE progress;
    SETUP_CANVAS();

    rb_parse_args(argc, argv, "|h", &args);
    progress = shoes_control_new(cProgress, args.a[0], self);
    shoes_add_ele(canvas, progress);
    return progress;
}
