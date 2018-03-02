#include "shoes/types/native.h"
#include "shoes/app.h"
#include "shoes/types/settings.h"
/*
 * The settings object
 */ 
// ruby
VALUE cSettings;

//FUNC_M("+settings", settings, -1);

void shoes_settings_init() {
    cSettings  = rb_define_class_under(cShoes, "Settings", rb_cObject);
    rb_define_alloc_func(cSettings, shoes_settings_alloc);
    //rb_define_method(cShoesMenubar, "menus", CASTHOOK(shoes_menubar_list),0);
    //rb_define_method(cShoesMenubar, "[]", CASTHOOK(shoes_menubar_at), 1);
    //RUBY_M("+settings", settings, -1);
}


void shoes_settings_mark(shoes_settings *st) {
    //rb_gc_mark_maybe(mb->menus);
    //rb_gc_mark_maybe(mb->context);
}

static void shoes_settings_free(shoes_settings *st) {
    RUBY_CRITICAL(SHOE_FREE(st));
}

VALUE shoes_settings_alloc(VALUE klass) {
    VALUE obj;
    shoes_settings *st = SHOE_ALLOC(shoes_settings);
    SHOE_MEMZERO(st, shoes_settings, 1);
    obj = Data_Wrap_Struct(klass, shoes_settings_mark, shoes_settings_free, st);
    //mb->native = NULL;
    //mb->menus = rb_ary_new();
    //mb->context = Qnil;
    return obj;
}

/*
 * This should only be called once, early in Shoes startup
 * Save in a global ruby object - not really a Shoes object. 
*/
VALUE shoes_setting_global = Qnil;

VALUE shoes_settings_new(shoes_yaml_init *yml) {
  if (!NIL_P(shoes_setting_global))
    return shoes_setting_global;

  shoes_setting_global = shoes_settings_alloc(cSettings);
  /* 
   *  Convert the strings in the yaml struct to object settings and
   * C globals we night need
  */
  
  return shoes_setting_global; 
}

// Not canvas visible? 

