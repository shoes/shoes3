#include "shoes/types/native.h"
#include "shoes/types/menuitem.h"
#include "shoes/app.h"
/*
 * A top Level menu in menubar  -File, Edit, Help...
 */ 
// ruby
VALUE cShoesMenuitem;

#ifdef NEW_MACRO_APP
FUNC_T("+menuitem", menuitem, -1);
#else
FUNC_M("+menuitem", menuitem, -1);
#endif

void shoes_menuitem_init() {
#ifdef NEW_MACRO_MENUITEM
    cShoesMenuitem  = rb_define_class_under(cTypes, "Menuitem", rb_cData);
#else
    cShoesMenuitem  = rb_define_class_under(cTypes, "Menuitem", rb_cObject);
#endif
    rb_define_method(cShoesMenuitem, "title", CASTHOOK(shoes_menuitem_gettitle), 0);
    rb_define_method(cShoesMenuitem, "title=", CASTHOOK(shoes_menuitem_settitle), 1);
    rb_define_method(cShoesMenuitem, "key", CASTHOOK(shoes_menuitem_getkey), 0);
    rb_define_method(cShoesMenuitem, "key=", CASTHOOK(shoes_menuitem_setkey), 1);
    rb_define_method(cShoesMenuitem, "block=", CASTHOOK(shoes_menuitem_setblk), 1);
    rb_define_method(cShoesMenuitem, "enable=", CASTHOOK(shoes_menuitem_setenable), 1);
    RUBY_M("+menuitem", menuitem, -1);
}

void shoes_menuitem_mark(shoes_menuitem *mi) {
    rb_gc_mark_maybe(mi->block);
    rb_gc_mark_maybe(mi->context);
}

static void shoes_menuitem_free(shoes_menuitem *mi) {
	if (mi->title) free(mi->title);
	if (mi->key) free(mi->key);
  if (mi->id) 
    free(mi->id);
  RUBY_CRITICAL(SHOE_FREE(mi));
}

#ifdef NEW_MACRO_MENUITEM
// creates struct shoes_app_type
TypedData_Type_New(shoes_menuitem);
#endif


VALUE shoes_menuitem_alloc(VALUE klass) {
    VALUE obj;
    shoes_menuitem *mi = SHOE_ALLOC(shoes_menuitem);
    SHOE_MEMZERO(mi, shoes_menuitem, 1);
#ifdef NEW_MACRO_MENUITEM
    obj = TypedData_Wrap_Struct(klass, &shoes_menuitem_type, mi);
#else
    obj = Data_Wrap_Struct(klass, shoes_menuitem_mark, shoes_menuitem_free, mi);
#endif
    mi->native = NULL;
    mi->title = NULL;
    mi->state = 0;
    mi->key = NULL;
    mi->block = Qnil;
    mi->context = Qnil;
    mi->id = NULL;
    return obj;
}

VALUE shoes_menuitem_new(VALUE text, int flags, char *key, VALUE blk, VALUE canvas) {
  VALUE obj= shoes_menuitem_alloc(cShoesMenuitem);
#ifdef NEW_MACRO_MENUITEM
  Get_TypedStruct2(obj, shoes_menuitem, mi);
#else
  shoes_menuitem *mi;
  Data_Get_Struct(obj, shoes_menuitem, mi);
#endif
  //shoes_canvas *cvs;
  //Data_Get_Struct(canvas, shoes_canvas, cvs);
  //shoes_app *app = cvs->app;  
  mi->title = strdup(RSTRING_PTR(text));
  if (strncmp(mi->title, "---", 3) == 0) {
    mi->key = "";
    mi->block = Qnil;
    mi->state = MENUITEM_ENABLE;
    mi->native = shoes_native_menusep_new(mi);
    mi->context = canvas;  // never called
  } else {
    mi->key = strdup(key);
    mi->state = flags;
    mi->block = blk;
    mi->context = canvas; // Note: can be Qnil on osx
    mi->native = shoes_native_menuitem_new(mi);
  }
  return obj;
}


VALUE shoes_menuitem_gettitle(VALUE self) {
#ifdef NEW_MACRO_MENUITEM
  Get_TypedStruct2(self, shoes_menuitem, mi);
#else
  shoes_menuitem *mi;
  Data_Get_Struct(self, shoes_menuitem, mi);
#endif
  if (mi->title == 0)
    return rb_str_new2("---");
  else
   return rb_str_new2(mi->title);
}

VALUE shoes_menuitem_settitle(VALUE self, VALUE text) {
#ifdef NEW_MACRO_MENUITEM
  Get_TypedStruct2(self, shoes_menuitem, mi);
#else
  shoes_menuitem *mi;
  Data_Get_Struct(self, shoes_menuitem, mi);
#endif
  if (mi->title)
    free(mi->title);
  mi->title = strdup(RSTRING_PTR(text));
  shoes_native_menuitem_set_title(mi);
  return Qnil;
}


VALUE shoes_menuitem_getkey(VALUE self) {
#ifdef NEW_MACRO_MENUITEM
  Get_TypedStruct2(self, shoes_menuitem, mi);
#else
  shoes_menuitem *mi;
  Data_Get_Struct(self, shoes_menuitem, mi);
#endif
  int flags = mi->state;
  VALUE outstr = rb_str_new2("");
  if (flags & MENUITEM_CONTROL)
    rb_str_cat2(outstr,"control_");
  if (flags & MENUITEM_SHIFT) 
    rb_str_cat2(outstr,"shift_");
  if (flags & MENUITEM_ALT)
    rb_str_cat2(outstr,"alt_");
  rb_str_cat2(outstr, mi->key);
  return outstr;
}

VALUE shoes_menuitem_setkey(VALUE self, VALUE keystr) {
#ifdef NEW_MACRO_MENUITEM
  Get_TypedStruct2(self, shoes_menuitem, mi);
#else
  shoes_menuitem *mi;
  Data_Get_Struct(self, shoes_menuitem, mi);
#endif
  int enable = mi->state & MENUITEM_ENABLE;
  char outkey[4];
  int flags = shoes_menuitem_parse_key(keystr, outkey);
  shoes_native_menuitem_set_key(mi, (enable | flags), outkey);
  return Qnil;
}


VALUE shoes_menuitem_setblk(VALUE self, VALUE block) {
  // TODO: may not be possible, all platforms? 
  if (rb_obj_is_kind_of(block, rb_cProc)) {
#ifdef NEW_MACRO_MENUITEM
    Get_TypedStruct2(self, shoes_menuitem, mi);
#else
    shoes_menuitem *mi;
    Data_Get_Struct(self, shoes_menuitem, mi);
#endif
    mi->block = block;
  } else 
    rb_raise(rb_eArgError, "not a proc");
  return Qnil;
}

VALUE shoes_menuitem_setenable(VALUE self, VALUE state) {
  // TODO: may not be possible, all platforms?
  int ns = 1;
  if (TYPE(state) == T_TRUE)
    ns = 1;
  else if (TYPE(state) == T_FALSE)
    ns = 0;
  else if (TYPE(state) == T_NIL)
    ns = 0;
  else
    rb_raise(rb_eArgError, "menuitem enable must be boolean");
#ifdef NEW_MACRO_MENUITEM
  Get_TypedStruct2(self, shoes_menuitem, mi);
#else
  shoes_menuitem *mi;
  Data_Get_Struct(self, shoes_menuitem, mi);
#endif
  shoes_native_menuitem_enable(mi, ns);
  mi->state = ns;
  return Qnil;
}

// helpers

int shoes_menuitem_parse_key(VALUE keystr, char *outkey) {
  int flags = 0;
  char *phrase = RSTRING_PTR(keystr);
  if (strstr(phrase, "shift_")) 
    flags = flags | MENUITEM_SHIFT;
  if (strstr(phrase, "control_"))
    flags = flags | MENUITEM_CONTROL; // Apple fan on OSX
  if (strstr(phrase, "alt_"))
    flags = flags | MENUITEM_ALT;
  if (flags) {
    char *sep = strrchr(phrase, '_');
    sep++;
    if (*sep) {
      strcpy(outkey, sep); 
      return flags;
    }
  }
  rb_raise(rb_eArgError,"key: string is not formatted properly");
  return flags;
}

// canvas - Shoes usage:  menuitem "Quit" , key: 'control_q'  do Shoes.quit end
//    just like a button. 
VALUE shoes_canvas_menuitem(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE text = Qnil, attr = Qnil, blk = Qnil, keystr = Qnil;
    VALUE menuitem = Qnil;
    VALUE enablev = Qnil;
    int flags = 0;
    switch (rb_parse_args(argc, argv, "s|h,|h", &args)) {
        case 1:
            text = args.a[0];
            attr = args.a[1];
            break;

        case 2:
            attr = args.a[0];
            break;
    }
    
    flags = flags | MENUITEM_ENABLE;  // default
    enablev = shoes_hash_get(attr, rb_intern("enable"));
    if (!NIL_P(enablev)) {
      if (TYPE(enablev) == T_FALSE)
        flags = flags ^ MENUITEM_ENABLE; 
    }

    keystr = shoes_hash_get(attr, rb_intern("key"));
    char key[] = {0,0,0,0};
    if (! NIL_P(keystr)) {
      int kflags = shoes_menuitem_parse_key(keystr, key);
      flags = flags | kflags;
    }
    
    if (rb_block_given_p()) {
        //ATTRSET(attr, click, rb_block_proc());
        blk = rb_block_proc();
    }
    menuitem = shoes_menuitem_new(text, flags, key, blk, self);
    
    return menuitem;
}

