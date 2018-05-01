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

// This wraooer may not be needed but it doesn't hurt very much
@implementation ShoesMenuItem
- (id) initWithMI: (shoes_menuitem *)mi
{
  if ((self = [super init]))
    wrapped = mi;
  return self;
}
@end


VALUE shoes_osx_menubar;

// -------- menubar -----

// called at shoes_native_init time() - very early in startup

void shoes_native_menu_root(NSMenu *main) {
  //fprintf(stderr, "vreating global menubar\n");
  VALUE mbv = shoes_menubar_alloc(cShoesMenubar);
  shoes_menubar *mb;
  Data_Get_Struct(mbv, shoes_menubar, mb);
  mb->native = (void *)main;
  shoes_osx_menubar = mbv;
  rb_gc_register_address(&shoes_osx_menubar);
  shoes_osx_create_apple_menu(mbv);
}


void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn) {
  
}

void shoes_native_menubar_remove(shoes_menubar *mb, int pos) {
}

void shoes_native_menubar_insert(shoes_menubar *mb, shoes_menu *mn, int pos) {
}


// called at app.open() time, optionally

VALUE shoes_native_menubar_setup(shoes_app *app, void *arg) {
  fprintf(stderr,"menubar setup called\n");
  //if (app ->have_menu == 0)
  //  return Qnil;
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
  [menu addItem: item];
  return NULL;
}

void shoes_native_menu_insert(shoes_menu *mn, shoes_menuitem *mi, int pos) {
}

void shoes_native_menu_remove(shoes_menu *mn, int pos) {
}

// -------- menuitem ------


void *shoes_native_menuitem_new(shoes_menuitem *mi) {

  NSString *title = [[NSString alloc] initWithUTF8String: mi->title];
  NSString *keystring = [[NSString alloc] initWithUTF8String: mi->key];
  NSMenuItem *item = [[NSMenuItem alloc] initWithTitle: title 
		action: @selector(menuTrigger:) 
		keyEquivalent:(NSString *)keystring];  
  [item setTarget: shoes_world->os.events];
  ShoesMenuItem *wr = [[ShoesMenuItem alloc] initWithMI: mi];
  [item setRepresentedObject: wr]; 
    
  mi->native = (void *)item;
  return mi->native;;
}

// ----- separator - like a menuitem w/o callback
void *shoes_native_menusep_new(shoes_menuitem *mi) {
  NSMenuItem *sep =  [NSMenuItem separatorItem];
  mi->native = (void *)sep;
  return mi->native;
}

void shoes_native_menuitem_set_title(shoes_menuitem *mi) {
}

void shoes_native_menuitem_enable(shoes_menuitem *mi, int state) {
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
	// Setup menubar and Shoes menu
	shoes_menubar *mb;
	Data_Get_Struct(mbv, shoes_menubar, mb);
    NSMenu *main = (NSMenu *)mb->native; 
    // Create the Native application (Apple) menu.
    NSMenu *menuApp = [[NSMenu alloc] initWithTitle: @"Apple Menu"];

    VALUE shoestext = rb_str_new2(shoes_app_name);
    VALUE shoesmenu = shoes_menu_alloc(cShoesMenu);
    shoes_menu *mn;
    Data_Get_Struct(shoesmenu, shoes_menu, mn);
    mn->title = RSTRING_PTR(shoestext);
    // set Shoes Menu in Shoes menubar
    rb_ary_store(mb->menus, 0, shoesmenu);
    mn->native = (void *)menuApp;
    
    // This native doesn't have a Shoes menuitem (yet)
    NSMenu *menuServices = [[NSMenu alloc] initWithTitle: @"Services"];
    [NSApp setServicesMenu:menuServices];    
    
    
    // Now populate with MenuItems
    NSMenuItem *menuitem;    

    //menuitem = [menuApp addItemWithTitle:@"Open..."
    //    action:@selector(openFile:) keyEquivalent:@"o"];
    //[menuitem setTarget: shoes_world->os.events];
    
    // We don't have Shoes app yet. 
    int flags = 1; // 1 means enabled
    char *key = "";
    VALUE otext = rb_str_new2("Open");
    VALUE oproc = rb_eval_string("proc { Shoes.show_selector }");
    VALUE oitem = shoes_menuitem_new(otext, flags, "o", oproc, Qnil); // calls native
    shoes_menu_append(shoesmenu, oitem);							  // calls native
   
	// Manual 
	VALUE mtext = rb_str_new2("Manual");
	VALUE mproc = rb_eval_string("proc { Shoes.show_manual }");
	VALUE mitem = shoes_menuitem_new(mtext, flags, key, mproc, Qnil);
	shoes_menu_append(shoesmenu, mitem);
	// Cobbler
	VALUE ctext = rb_str_new2("Cobbler");
	VALUE cproc = rb_eval_string("proc { Shoes.cobbler }");
	VALUE citem = shoes_menuitem_new(ctext, flags, key, cproc, Qnil);
	shoes_menu_append(shoesmenu, citem);
	// Profile - bug in profiler (#400 , @dredknight)
	VALUE ftext = rb_str_new2("Profile");
	VALUE fproc = rb_eval_string("proc { require 'shoes/profiler'; Shoes.profile(nil) }");
	VALUE fitem = shoes_menuitem_new(ftext, flags, key, fproc, Qnil);
	shoes_menu_append(shoesmenu, fitem);
	// Package
	VALUE ptext = rb_str_new2("Package");
	VALUE pproc = rb_eval_string("proc { Shoes.app_package }");
	VALUE pitem = shoes_menuitem_new(ptext, flags, key, pproc, Qnil);
	shoes_menu_append(shoesmenu, pitem);

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
    
    //[menuApp addItem: [NSMenuItem separatorItem]];
    VALUE stext = rb_str_new2("--- a seperator");
    VALUE s1item = shoes_menuitem_new(stext, flags, key, Qnil, Qnil);
    shoes_menu_append(shoesmenu, s1item);
 
    //menuitem = [[NSMenuItem alloc] initWithTitle:@"Quit"
    //    action:@selector(terminate:) keyEquivalent:@"q"];
    //[menuitem setTarget: NSApp];
    //[menuApp addItem: menuitem];
    //[menuitem release];
    VALUE qtext = rb_str_new2("Quit");
    VALUE qproc = rb_eval_string("proc { Shoes.quit }");
    VALUE qitem = shoes_menuitem_new(qtext, flags, "q", qproc, Qnil); 
    shoes_menu_append(shoesmenu, qitem);

    
    // Turn off a warning message: laugh or cry?
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-method-access"
    // TODO: This undocumented method is required if you don't 
    // load a nib file which ties us up into xcode. Lesser of two evils.
    [NSApp setAppleMenu: menuApp]; 
#pragma clang diagnostic pop
    // Add Native  Menu to Native menubar
    add_to_menubar(main, menuApp);
    [menuApp release];
}


