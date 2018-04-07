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
extern VALUE shoes_settings_globalv;
//extern shoes_settings *shoes_settings_global;

//void shoes_settings_init();
VALUE shoes_settings_alloc(VALUE klass);
VALUE shoes_settings_new(shoes_yaml_init *);
VALUE shoes_settings_dbus(VALUE self);
#endif
