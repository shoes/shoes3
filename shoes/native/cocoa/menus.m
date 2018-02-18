#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"
#include "shoes/types/menubar.h"
#include "shoes/types/menu.h"
#include "shoes/types/menuitem.h"
#include "shoes/native/cocoa/menus.h"



VALUE shoes_osx_menubar;

// -------- menubar -----

// called at shoes_native_init time( )very early
void shoes_native_menu_root(NSMenu *main) {
  fprintf(stderr, "saving root menubar\n");
  // build the App menu (with the quit) and the Edit Menu
  // natives and with Shoes equivalent 
  VALUE mbv = shoes_menubar_alloc(cShoesMenubar);
  shoes_menubar *mb;
  Data_Get_Struct(mbv, shoes_menubar, mb);
  mb->native = (void *)main;
  shoes_osx_menubar = mbv;
  shoes_osx_create_apple_menu(mbv);
}


void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn) {
  
}

/*
 * 
*/ 
void shoes_native_menubar_quit(void *extra) {
  //fprintf(stderr, "Dummy quit called\n");
  shoes_native_quit();
}

// TODO: calling this will segfault
VALUE shoes_native_menubar_make_block(char *code) {
  int argc = 1;
  VALUE argv[2];
  argv[0] = rb_str_new2(code);
  argv[1] = Qnil;
  VALUE block = Qnil;
  rb_scan_args(argc, argv, "0&", &block);
  return block;
}

// called at app.open() time, optionally

VALUE shoes_native_menubar_setup(shoes_app *app) {
  fprintf(stderr,"menubar setup called\n");
  if (app ->have_menu == 0)
    return Qnil;
  if (NIL_P(app->menubar)) {
	  app->menubar = shoes_osx_menubar;  
  }
  return app->menubar;
}



// ------- menu ------
void *shoes_native_menu_new(shoes_menu *mn) {
  NSString *title = [[NSString alloc] initWithUTF8String: mn->title];
  NSMenu *menu = [[NSMenu alloc] initWithTitle: title];
  mn->native = (void *)menu;
  return NULL;
}

void *shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi) {
  NSMenu *menu = (NSMenu *)mn->native;
  NSMenuItem *item = (NSMenuItem *)mi->native;
  [menu addItem:item];
  return NULL;
}

// -------- menuitem ------

// TODO: parse the 'control_q" string into something menuitem can use
NSString *shoes_osx_menu_keystring(char *str) {
	NSString *nstr = @"";
	return nstr;
}


void *shoes_native_menuitem_new(shoes_menuitem *mi) {
  NSString *title = [[NSString alloc] initWithUTF8String: mi->title];
  NSString *keystring = shoes_osx_menu_keystring(mi->key);
  NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:title 
		action:@selector(shoes_native_menuitem_callback) 
		keyEquivalent:(NSString *)keystring];
  [item setTarget:shoes_world->os.events];
  mi->native = (void *)item;
  return NULL;
}

// ------- default menu creaters -------

void
add_to_menubar(NSMenu *main, NSMenu *menu)
{
    NSMenuItem *dummyItem = [[NSMenuItem alloc] initWithTitle:@""
        action:nil keyEquivalent:@""];
    [dummyItem setSubmenu:menu];
    [main addItem:dummyItem];
    [dummyItem release];
}

void shoes_osx_create_apple_menu(VALUE mbv) {
	shoes_menubar *mb;
	Data_Get_Struct(mbv, shoes_menubar, mb);
	NSMenu *main = (NSMenu *)mb->native; 
    NSMenuItem *menuitem;
    // Create the application (Apple) menu.
    NSMenu *menuApp = [[NSMenu alloc] initWithTitle: @"Apple Menu"];

    NSMenu *menuServices = [[NSMenu alloc] initWithTitle: @"Services"];
    [NSApp setServicesMenu:menuServices];

    menuitem = [menuApp addItemWithTitle:@"Open..."
        action:@selector(openFile:) keyEquivalent:@"o"];
    [menuitem setTarget: shoes_world->os.events];
#if 0    
    menuitem = [menuApp addItemWithTitle:@"Package..."
        action:@selector(package:) keyEquivalent:@"P"];
    [menuitem setTarget: shoes_world->os.events];
#endif    
    [menuApp addItemWithTitle:@"Preferences..." action:nil keyEquivalent:@""];
    [menuApp addItem: [NSMenuItem separatorItem]];
    menuitem = [[NSMenuItem alloc] initWithTitle: @"Services"
        action:nil keyEquivalent:@""];
    [menuitem setSubmenu:menuServices];
    [menuApp addItem: menuitem];
    [menuitem release];
    [menuApp addItem: [NSMenuItem separatorItem]];
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Hide"
        action:@selector(hide:) keyEquivalent:@""];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    [menuitem release];
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Hide Others"
        action:@selector(hideOtherApplications:) keyEquivalent:@""];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    [menuitem release];
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Show All"
        action:@selector(unhideAllApplications:) keyEquivalent:@""];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    [menuitem release];
    [menuApp addItem: [NSMenuItem separatorItem]];
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Quit"
        action:@selector(terminate:) keyEquivalent:@"q"];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    [menuitem release];
    // Turn off a warning message: laugh or cry?
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-method-access"
    // TODO: This undocumented method is required if you don't 
    // load a nib file which ties us up into xcode. Lesser of two evils.
    [NSApp setAppleMenu: menuApp]; 
#pragma clang diagnostic pop
    add_to_menubar(main, menuApp);
    [menuApp release];
}


