#include "shoes/types/native.h"
#include "shoes/types/menu.h"
#include "shoes/types/menuitem.h"

/*
 * A top Level menu in menubar  -File, Edit, Help...
 */ 
// ruby
VALUE cMenu;

FUNC_M("+menu", menu, -1);

void shoes_menu_init() {
    cMenu  = rb_define_class_under(cTypes, "Menu", cNative);
    rb_define_method(cMenu, "[]", CASTHOOK(shoes_menu_get), 1);
    rb_define_method(cMenu, "[]=", CASTHOOK(shoes_menu_set), 2);
    rb_define_method(cMenu, "<<", CASTHOOK(shoes_menu_append), 1);
    RUBY_M("+menu", menu, -1);
}

void shoes_menu_mark(shoes_menu *mn) {
    rb_gc_mark_maybe(mn->items);
}

static void shoes_menu_free(shoes_menu *mn) {
    RUBY_CRITICAL(SHOE_FREE(mn));
}

VALUE shoes_menu_alloc(VALUE klass) {
    VALUE obj;
    shoes_menu *mn = SHOE_ALLOC(shoes_menu);
    SHOE_MEMZERO(mn, shoes_menu, 1);
    obj = Data_Wrap_Struct(klass, shoes_menu_mark, shoes_menu_free, mn);
    mn->title = NULL;
    mn->items = rb_ary_new();
    return obj;
}

VALUE shoes_menu_new(VALUE text) {
  VALUE obj= shoes_menu_alloc(cMenu);
  shoes_menu *mn;
  Data_Get_Struct(obj, shoes_menu, mn);
  mn->title = RSTRING_PTR(text);
  shoes_native_menu_new(mn);
  return obj;
}

VALUE shoes_menu_append(VALUE self, VALUE miv) {
  shoes_menu *mn;
  shoes_menuitem *mi;
  // TODO scream holy hell if miv is not a cMenuitem
  Data_Get_Struct(self, shoes_menu, mn);
  Data_Get_Struct(miv, shoes_menuitem, mi);
  shoes_native_menu_append(mn, mi);
  return self;
}

VALUE shoes_menu_get(VALUE self, VALUE arg) {
  return Qnil;
}

VALUE shoes_menu_set(VALUE self, VALUE arg, VALUE val) {
  return Qnil;
}

// canvas - Shoes usage:  menu "Help"
VALUE shoes_canvas_menu(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE text = Qnil, attr = Qnil, menu;
    

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
    menu = shoes_menu_new(text);
    
    return menu;
}

