#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"

VALUE shoes_native_menubar_setup(shoes_app *app);
void shoes_native_menu_root(NSMenu *main);
void shoes_osx_create_apple_menu(VALUE mbv);
