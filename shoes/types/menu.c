#include "shoes/types/native.h"
#include "shoes/types/menu.h"
#include "shoes/types/menuitem.h"

/*
 * A top Level menu in menubar  -File, Edit, Help...
 */ 
// ruby
extern VALUE cShoesMenu;

FUNC_M("+menu", menu, -1);

void shoes_menu_init() {
    cShoesMenu  = rb_define_class_under(cTypes, "Menu", cNative);
    rb_define_method(cShoesMenu, "items", CASTHOOK(shoes_menu_list), 0);
    rb_define_method(cShoesMenu, "[]", CASTHOOK(shoes_menu_at), 1);
    rb_define_method(cShoesMenu, "title", CASTHOOK(shoes_menu_title), 0);
    rb_define_method(cShoesMenu, "<<", CASTHOOK(shoes_menu_append), 1);
    rb_define_method(cShoesMenu, "append", CASTHOOK(shoes_menu_append), 1);
    rb_define_method(cShoesMenu, "insert", CASTHOOK(shoes_menu_insert), 2);
    rb_define_method(cShoesMenu, "index", CASTHOOK(shoes_menu_index), 1);
    rb_define_method(cShoesMenu, "remove", CASTHOOK(shoes_menu_remove), 1);
    RUBY_M("+menu", menu, -1);
}

void shoes_menu_mark(shoes_menu *mn) {
    rb_gc_mark_maybe(mn->items);
    rb_gc_mark_maybe(mn->context);
}

static void shoes_menu_free(shoes_menu *mn) {
	if (mn->title)
	  free(mn->title);
    RUBY_CRITICAL(SHOE_FREE(mn));
}

VALUE shoes_menu_alloc(VALUE klass) {
    VALUE obj;
    shoes_menu *mn = SHOE_ALLOC(shoes_menu);
    SHOE_MEMZERO(mn, shoes_menu, 1);
    obj = Data_Wrap_Struct(klass, shoes_menu_mark, shoes_menu_free, mn);
    mn->native = NULL;
    mn->extra = NULL;
    mn->parent = NULL;
    mn->title = NULL;
    mn->context = Qnil;
    mn->items = rb_ary_new();
    return obj;
}

VALUE shoes_menu_new(VALUE text) {
  VALUE obj= shoes_menu_alloc(cShoesMenu);
  shoes_menu *mn;
  Data_Get_Struct(obj, shoes_menu, mn);
  mn->title = strdup(RSTRING_PTR(text));
  shoes_native_menu_new(mn);
  return obj;
}

// Add menuitem to the menu at the end
VALUE shoes_menu_append(VALUE self, VALUE miv) {
  shoes_menu *mn;
  shoes_menuitem *mi;
  Data_Get_Struct(self, shoes_menu, mn);
  if (rb_obj_is_kind_of(miv, cShoesMenuitem)) {
    Data_Get_Struct(miv, shoes_menuitem, mi);
    mi->parent = mn;
    shoes_native_menu_append(mn, mi);
    int cnt = RARRAY_LEN(mn->items);
    rb_ary_store(mn->items, cnt, miv);
  } else if (rb_obj_is_kind_of(miv, cShoesMenu)) {
    rb_raise(rb_eArgError, "menu cannot be appended to menu");
  } else {
    rb_raise(rb_eArgError, "not a menuitem");
  }
  return self;
}

VALUE shoes_menu_list(VALUE self) {
  shoes_menu *mn;
  Data_Get_Struct(self, shoes_menu, mn);
  return mn->items;
}

VALUE shoes_menu_title(VALUE self) {
  shoes_menu *mn;
  Data_Get_Struct(self, shoes_menu, mn);
  return rb_str_new2(mn->title);
}

// Used by several functions in the this file as well as Shoes scripts
// arg is string or int. Return is 0..n-1 or Qnil
// usage: menu.index(arg)
VALUE shoes_menu_index(VALUE self, VALUE arg) {
  shoes_menu *mn;
  Data_Get_Struct(self, shoes_menu, mn);
  if (TYPE(arg) == T_FIXNUM) {
    int pos = NUM2INT(arg);
    int cnt = RARRAY_LEN(mn->items);
    if (pos < cnt && pos >= 0)
      return arg;
  } else if (TYPE(arg) == T_STRING) {
    char *txt = RSTRING_PTR(arg);
    int cnt = RARRAY_LEN(mn->items);
    int i;
    for (i = 0; i < cnt; i++) {
      VALUE miv = rb_ary_entry(mn->items, i);
      shoes_menuitem *mi;
      Data_Get_Struct(miv, shoes_menuitem, mi);
      if (strcmp(txt,mi->title) == 0)
        return INT2NUM(i);
    
    }
  } else
    rb_raise(rb_eArgError, "index must be string or integer");
  return Qnil;
}

// arg is string or int. Returns matching menuitem object or Qnil
// usage: menu[arg]
VALUE shoes_menu_at(VALUE self, VALUE arg) {
  shoes_menu *mn;
  Data_Get_Struct(self, shoes_menu, mn);
  VALUE posv = shoes_menu_index(self, arg);
  if (!NIL_P(posv)) {
    int pos = NUM2INT(posv);
    return rb_ary_entry(mn->items, pos);
  }
  return Qnil;
}

// insert at position - others move down, ick, ick and ick!
VALUE shoes_menu_insert(VALUE self, VALUE miv, VALUE reqpos) {
  int pos = 0;
  shoes_menu *mn;
  Data_Get_Struct(self, shoes_menu, mn);
  int cnt = RARRAY_LEN(mn->items);  //cnt before
  VALUE pv = shoes_menu_index(self, reqpos);
  if (NIL_P(pv))
    pos = -1;
  else
    pos = NUM2INT(pv);
  // if pos is < 0 we can call append and return 
  if (pos < 0) {
    shoes_menu_append(self, miv);
  } else {
    // Call the native function and then diddle with the items (rb_ary)
    // to match
    shoes_menuitem *mi;
    Data_Get_Struct(miv, shoes_menuitem, mi);
    mi->parent = mn;
    shoes_native_menu_insert(mn, mi, pos);
    VALUE nary = rb_ary_new2(cnt+1); 
    int i;
    // copy up to pos
    for (i = 0; i < pos; i++) {
      rb_ary_store(nary, i, rb_ary_entry(mn->items, i));
    }
    // insert new entry at pos
    rb_ary_store(nary, pos, miv);
    // copy the trailing entries
    for (i = pos; i < cnt; i++) {
      rb_ary_store(nary, i+1, rb_ary_entry(mn->items, i));
    }
    // replace ary - pray that our gc handling is good
    mn->items = nary; 
  }
  return INT2NUM(pos);
}

// remove given menuitem from menu. arg is int or string
VALUE shoes_menu_remove(VALUE self, VALUE arg) {
  shoes_menu *mn;
  Data_Get_Struct(self, shoes_menu, mn);
  VALUE posv = shoes_menu_index(self, arg);
  if (NIL_P(posv))
    rb_raise(rb_eArgError, "menuitem not found");
  int pos = NUM2INT(posv);
  // remove the native
  shoes_native_menu_remove(mn ,pos); 
  // TODO: not documented online
  rb_ary_delete_at(mn->items, pos);
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

