#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"

VALUE shoes_native_menubar_setup(shoes_app *app);
void shoes_native_build_menus(shoes_app *app, VALUE mbv);
void shoes_native_menuitem_callback(GtkWidget *wid, gpointer extra);
void shoes_native_menuitem_insert(shoes_menu *mn, shoes_menuitem *mi, int pos);
