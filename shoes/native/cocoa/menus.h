#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"
#include "shoes/types/menuitem.h"

@interface ShoesMenuItem: NSObject
{
@public
  shoes_menuitem *wrapped;
}
@end


VALUE shoes_native_menubar_setup(shoes_app *app, void *arg);
void shoes_native_menu_root(NSMenu *main);
void shoes_osx_create_apple_menu(VALUE mbv);
