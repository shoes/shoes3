#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_SETTINGS_TYPE_H
#define SHOES_SETTINGS_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget, cShoesMenubar;

typedef struct {
  VALUE app_name;
  VALUE theme;
  VALUE mdi; 
  VALUE rdomain;
  VALUE use_menus;
  VALUE dbus_name;
  VALUE monitor_list;
} shoes_settings;

// Global vars
//extern VALUE shoes_settings_globalv;
//extern shoes_settings *shoes_settings_global;

//void shoes_settings_init();
VALUE shoes_settings_alloc(VALUE klass);
VALUE shoes_settings_new(shoes_yaml_init *);
VALUE shoes_settings_dbus(VALUE self);
VALUE shoes_settings_app_name(VALUE self);
VALUE shoes_settings_get_theme(VALUE self);
VALUE shoes_settings_set_theme(VALUE self, VALUE theme);
VALUE shoes_settings_mdi(VALUE self);
VALUE shoes_settings_menu(VALUE self);
VALUE shoes_settings_rdomain(VALUE self);
VALUE shoes_settings_set_rdomain(VALUE self, VALUE name);
VALUE shoes_settings_monitors_list(VALUE self);
VALUE shoes_settings_monitor(VALUE self, VALUE idx);
#endif
