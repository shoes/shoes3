#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
//#include "shoes/native/native.h"

#ifndef SHOES_SETTINGS_TYPE_H
#define SHOES_SETTINGS_TYPE_H

//#define NEW_MACRO_SETTINGS

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget, cShoesMenubar;

typedef struct {
  VALUE app_name;
  VALUE icon_path;
  VALUE theme;
  VALUE theme_path;
  VALUE mdi; 
  VALUE rdomain;
  VALUE use_menus;
  VALUE dbus_name;
  VALUE backend;
  VALUE extra1;
  VALUE extra2;
  VALUE osx_menutrim; 
  VALUE image_cache;
} shoes_settings;

typedef struct {
  int x;
  int y;
  int width;
  int height;
} shoes_monitor_t;

#ifdef NEW_MACRO_SETTINGS
extern const rb_data_type_t shoes_settings_type;
#endif


VALUE shoes_settings_alloc(VALUE klass);
VALUE shoes_settings_new(shoes_yaml_init *);
VALUE shoes_settings_dbus(VALUE self);
VALUE shoes_settings_app_name(VALUE self);
VALUE shoes_settings_set_app_name(VALUE self, VALUE name);
VALUE shoes_settings_app_icon(VALUE self);
VALUE shoes_settings_set_app_icon(VALUE self, VALUE path);
VALUE shoes_settings_get_theme(VALUE self);
VALUE shoes_settings_set_theme(VALUE self, VALUE theme);
VALUE shoes_settings_mdi(VALUE self);
VALUE shoes_settings_menu(VALUE self);
VALUE shoes_settings_rdomain(VALUE self);
VALUE shoes_settings_set_rdomain(VALUE self, VALUE name);
VALUE shoes_setting_display_backend(VALUE self);
VALUE shoes_setting_extra1(VALUE self);
VALUE shoes_setting_extra2(VALUE self);
VALUE shoes_setting_get_wintmo(VALUE self);
VALUE shoes_setting_set_wintmo(VALUE self, VALUE msec);

VALUE shoes_settings_monitor_count(VALUE self);
VALUE shoes_settings_monitor_geometry(VALUE self, VALUE idx);
VALUE shoes_settings_monitor_default(VALUE self);

// Monitor Natives
extern int shoes_native_monitor_count(); 
extern int shoes_native_monitor_default();
extern void shoes_native_monitor_geometry(int , shoes_monitor_t *r);
extern void shoes_native_monitor_set(shoes_app *app);
extern int shoes_native_monitor_get(shoes_app *app);
#endif
