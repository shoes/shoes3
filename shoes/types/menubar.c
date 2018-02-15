#include "shoes/types/native.h"
#include "shoes/types/menubar.h"
#include "shoes/types/menu.h"
#include "shoes/app.h"
/*
 * The menubar 
 */ 
// ruby
VALUE cMenubar;

FUNC_M("+menubar", menubar, -1);

void shoes_menubar_init() {
    cMenubar  = rb_define_class_under(cTypes, "Menubar", rb_cObject);
    rb_define_alloc_func(cMenubar, shoes_menubar_alloc);
    rb_define_method(cMenubar, "[]", CASTHOOK(shoes_menubar_get), 1);
    rb_define_method(cMenubar, "[]=", CASTHOOK(shoes_menubar_set), 2);
    rb_define_method(cMenubar, "<<", CASTHOOK(shoes_menubar_append), 1);
    RUBY_M("+menubar", menubar, -1);
}


void shoes_menubar_mark(shoes_menubar *mb) {
    rb_gc_mark_maybe(mb->menus);
}

static void shoes_menubar_free(shoes_menubar *mb) {
    RUBY_CRITICAL(SHOE_FREE(mb));
}

VALUE shoes_menubar_alloc(VALUE klass) {
    VALUE obj;
    shoes_menubar *mb = SHOE_ALLOC(shoes_menubar);
    SHOE_MEMZERO(mb, shoes_menubar, 1);
    obj = Data_Wrap_Struct(klass, shoes_menubar_mark, shoes_menubar_free, mb);
    mb->native = NULL;
    mb->menus = rb_ary_new();
    return obj;
}

/*
 * Calling new will create a minimal menu setup File, File->Quit (gtk)
 * or retrieve the one all ready created for the app/window
*/
VALUE shoes_menubar_new(VALUE canvas) {
    shoes_app *app;
    shoes_canvas *cvs;
    Data_Get_Struct(canvas, shoes_canvas, cvs);
    app = cvs->app;
    if (NIL_P(app->menubar)) {
      // Should not happen in real life, but
      VALUE mbv = shoes_menubar_alloc(cMenubar);
      shoes_menubar *mb;
      Data_Get_Struct(mbv, shoes_menubar, mb);
      mb->native = shoes_native_menubar_setup(app);
      app->menubar = mbv;
    }
    return app->menubar;
}

// returns menu at idx
VALUE shoes_menubar_get(VALUE self, VALUE idx) {
  return Qnil;
}

// replaces menu at idx
VALUE shoes_menubar_set(VALUE self, VALUE idx, VALUE menu) {
  return Qnil;
}

VALUE shoes_menubar_append(VALUE self, VALUE menu) {
  shoes_menubar *mb;
  shoes_menu *mn;
  // TODO: check if menu is a cMenu object
  Data_Get_Struct(self, shoes_menubar, mb);
  Data_Get_Struct(menu, shoes_menu, mn);
  shoes_native_menubar_append(mb, mn);
}

/*
 *  canvas - The returned menu bar has contents 
 *  For gtk it's File->Quit
 *  For cocoa (global menu) it's more than gtk
*/
VALUE shoes_canvas_menubar(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE text = Qnil, attr = Qnil, menubar;
    
#if 0
    switch (rb_parse_args(argc, argv, "s|h,|h", &args)) {
        case 1:
            text = args.a[0];
            attr = args.a[1];
            break;

        case 2:
            attr = args.a[0];
            break;
    }

    if (!NIL_P(text))
        ATTRSET(attr, text, text);

    if (rb_block_given_p())
        ATTRSET(attr, click, rb_block_proc());
#endif
    return shoes_menubar_new(self); 
  
}

