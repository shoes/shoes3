#include "shoes/types/native.h"
#include "shoes/types/menubar.h"
#include "shoes/types/menu.h"
#include "shoes/types/menuitem.h"
#include "shoes/app.h"
/*
 * The menubar 
 */ 
// ruby
extern VALUE cShoesMenubar;

FUNC_M("+menubar", menubar, -1);

void shoes_menubar_init() {
    cShoesMenubar  = rb_define_class_under(cTypes, "Menubar", rb_cObject);
    rb_define_alloc_func(cShoesMenubar, shoes_menubar_alloc);
    rb_define_method(cShoesMenubar, "menus", CASTHOOK(shoes_menubar_list),0);
    rb_define_method(cShoesMenubar, "[]", CASTHOOK(shoes_menubar_at), 1);
    rb_define_method(cShoesMenubar, "<<", CASTHOOK(shoes_menubar_append), 1);
    rb_define_method(cShoesMenubar, "append", CASTHOOK(shoes_menubar_append), 1);
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
 * Calling new will create a minimal menu setup Shoes, Shoes->Quit (etc)
 * OR retrieve the one all ready created for the app/window
*/
VALUE shoes_menubar_new(VALUE canvas) {
    shoes_app *app;
    shoes_canvas *cvs;
    Data_Get_Struct(canvas, shoes_canvas, cvs);
    app = cvs->app;
    if (NIL_P(app->menubar)) {
      // Should not happen in real life, but
      app->menubar = shoes_native_menubar_setup(app);
    }
    return app->menubar;
}


VALUE shoes_menubar_append(VALUE self, VALUE menu) {
  if (rb_obj_is_kind_of(menu, cShoesMenu)) {
    shoes_menubar *mb;
    shoes_menu *mn;
    Data_Get_Struct(self, shoes_menubar, mb);
    Data_Get_Struct(menu, shoes_menu, mn);
    shoes_native_menubar_append(mb, mn);
    int cnt = RARRAY_LEN(mb->menus);
    rb_ary_store(mb->menus, cnt, menu);
  } else {
    rb_raise(rb_eArgError, "menubar append - not a menu");
  }
  return Qtrue;
}

VALUE shoes_menubar_list(VALUE self) {
  shoes_menubar *mb;
  Data_Get_Struct(self, shoes_menubar, mb);
  return mb->menus;
}


VALUE shoes_menubar_at(VALUE self, VALUE arg) {
  shoes_menubar *mb;
  Data_Get_Struct(self, shoes_menubar, mb);
  if (TYPE(arg) == T_FIXNUM)
    return rb_ary_entry(mb->menus, NUM2INT(arg));
  else if (TYPE(arg) == T_STRING) {
    char *txt = RSTRING_PTR(arg);
    int cnt = RARRAY_LEN(mb->menus);
    int i;
    for (i = 0; i < cnt; i++) {
      VALUE mnv = rb_ary_entry(mb->menus, i);
      shoes_menu *mn;
      Data_Get_Struct(mnv, shoes_menu, mn);
      if (mn->title == 0) continue;
      if (strcmp(txt,mn->title) == 0)
        return mnv;    
    }
  } else
    rb_raise(rb_eArgError, "index must be string or integer");
  return Qnil;
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

