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
    rb_define_method(cSettings, "dbus", CASTHOOK(shoes_settings_dbus),0);
    //rb_define_method(cShoesMenubar, "[]", CASTHOOK(shoes_menubar_at), 1);
    //RUBY_M("+settings", settings, -1);
}


void shoes_settings_mark(shoes_settings *st) {
    rb_gc_mark_maybe(st->app_name);
    rb_gc_mark_maybe(st->theme);
    rb_gc_mark_maybe(st->mdi);
    rb_gc_mark_maybe(st->rdomain);
    rb_gc_mark_maybe(st->mdi);
    rb_gc_mark_maybe(st->use_menus);
    rb_gc_mark_maybe(st->dbus_name);
}

static void shoes_settings_free(shoes_settings *st) {
    RUBY_CRITICAL(SHOE_FREE(st));
}

VALUE shoes_settings_alloc(VALUE klass) {
    VALUE obj;
    shoes_settings *st = SHOE_ALLOC(shoes_settings);
    SHOE_MEMZERO(st, shoes_settings, 1);
    obj = Data_Wrap_Struct(klass, shoes_settings_mark, shoes_settings_free, st);
    st->app_name = Qnil;
    st->theme = Qnil;
    st->mdi = Qnil;
    st->rdomain = Qnil;
    st->use_menus = Qnil;
    st->dbus_name = Qnil;
    return obj;
}

/*
 * This should only be called once, early in Shoes startup
 * Save in a global ruby object - not a Shoes GUI object. 
 * There is a one time, small bit of memory that is not free-ed. 
*/
VALUE shoes_settings_globalv = Qnil;
shoes_settings *shoes_settings_global;

VALUE shoes_settings_new(shoes_yaml_init *yml) {
  if (!NIL_P(shoes_settings_globalv))
    return shoes_settings_globalv;

  shoes_settings_globalv = shoes_settings_alloc(cSettings);
  shoes_settings *st;
  Data_Get_Struct(shoes_settings_globalv, shoes_settings, st);
  st->app_name = rb_str_new2(yml->app_name);
  if (yml->theme_name == NULL)
    st->theme = Qnil;
  else
    st->theme = rb_str_new2(yml->theme_name);
  st->rdomain = rb_str_new2(yml->rdomain);
  if (! strcmp(yml->mdi,"true")) 
    st->mdi = Qtrue;
  else
    st->mdi = Qnil;
  if (! strcmp(yml->use_menus, "true"))
    st->use_menus = Qtrue;
  else
    st->use_menus = Qnil;
  free(yml);
  shoes_settings_global = st;
  return shoes_settings_globalv; 
}

VALUE shoes_settings_dbus(VALUE self) {
  shoes_settings *st;
  Data_Get_Struct(self, shoes_settings, st);
  return st->dbus_name;
}

// Not canvas visible

