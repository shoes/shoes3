#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"
#include "shoes/types/menubar.h"
#include "shoes/types/menu.h"
#include "shoes/types/menuitem.h"
#include "shoes/types/settings.h"
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

// OSX has a funny setup at the top.
void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn) {
  NSMenu *main = (NSMenu *)mb->native;
  NSMenu *menu = (NSMenu *)mn->native;
  NSMenuItem *dummyItem = [[NSMenuItem alloc] initWithTitle:@""
        action: nil keyEquivalent:@""];
  [dummyItem setSubmenu: menu];
  [main addItem:dummyItem];
  [dummyItem release];  
  
}

// remove whole menu from menubar
void shoes_native_menubar_remove(shoes_menubar *mb, int pos) {
  NSMenu *main = (NSMenu *)mb->native;
  [main removeItemAtIndex: pos];

}

void shoes_native_menubar_insert(shoes_menubar *mb, shoes_menu *mn, int pos) {
  NSMenu *main = (NSMenu *)mb->native;
  NSMenu *menu = (NSMenu *)mn->native;
  NSMenuItem *dummyItem = [[NSMenuItem alloc] initWithTitle:@""
        action:nil keyEquivalent:@""];
  [dummyItem setSubmenu: menu];
  [main insertItem: dummyItem atIndex: pos];
  [dummyItem release];
}


// called at app.open() time, optionally

VALUE shoes_native_menubar_setup(shoes_app *app, void *arg) {
  //fprintf(stderr,"menubar setup called\n");
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
  [menu setAutoenablesItems: NO];
  return NULL;
}

void shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi) {
  if (mi->state & NO_NATIVE)
    return;
  NSMenu *menu = (NSMenu *)mn->native;
  NSMenuItem *item = (NSMenuItem *)mi->native;
  [menu addItem: item];
  //fprintf(stderr, "append %s to menu %s\n", mi->title, mn->title);
}

void shoes_native_menu_insert(shoes_menu *mn, shoes_menuitem *mi, int pos) {
  //fprintf(stderr, "insert %s into %s at pos %d\n", mi->title, mn->title, pos);
  NSMenu *menu = (void *)mn->native;
  [menu insertItem: (NSMenuItem *)mi->native atIndex: pos];
  return;
}

void shoes_native_menu_remove(shoes_menu *mn, int pos) {
  //fprintf(stderr, "Menu %s, delete %d\n", mn->title, pos);
  NSMenu *menu = (void *)mn->native;
  [menu removeItemAtIndex: pos];
}

// -------- menuitem ------


void *shoes_native_menuitem_new(shoes_menuitem *mi) {
  if (mi->state & NO_NATIVE)
    return (void *)mi;
  NSEventModifierFlags nflags = 0;
  if (mi->state & MENUITEM_CONTROL)
    nflags |= NSCommandKeyMask;
  if (mi->state & MENUITEM_SHIFT)  
    nflags |= NSShiftKeyMask;
  if (mi->state &  MENUITEM_ALT)
    nflags |= NSAlternateKeyMask;
    
  NSString *title = [[NSString alloc] initWithUTF8String: mi->title];
  NSString *keystring = [[NSString alloc] initWithUTF8String: mi->key];
  NSMenuItem *item = [[NSMenuItem alloc] initWithTitle: title 
		action: @selector(menuTrigger:) 
		keyEquivalent:(NSString *)keystring];  
  [item setTarget: shoes_world->os.events];
  [item setKeyEquivalentModifierMask: nflags];
  ShoesMenuItem *wr = [[ShoesMenuItem alloc] initWithMI: mi];
  [item setRepresentedObject: wr]; 
  [item setEnabled: (mi->state & MENUITEM_ENABLE ? YES : NO)];
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
  NSMenuItem *item = (NSMenuItem *)mi->native;
  NSString *title = [[NSString alloc] initWithUTF8String: mi->title];
  [item setTitle: title];
}

void shoes_native_menuitem_enable(shoes_menuitem *mi, int state) {
  NSMenuItem *item = (NSMenuItem *)mi->native;
  if (item) {
    [item setEnabled: (state ? YES : NO)];
  }
}

void 
shoes_native_menuitem_set_key(shoes_menuitem *mi, int newflags, char *newkey)
{
  int enabled = newflags & MENUITEM_ENABLE;
  NSEventModifierFlags nflags = 0;
  if (newflags & MENUITEM_CONTROL)
    nflags |= NSCommandKeyMask;
  if (newflags & MENUITEM_SHIFT)  
    nflags |= NSShiftKeyMask;
  if (newflags &  MENUITEM_ALT)
    nflags |= NSAlternateKeyMask;
  NSString *key = [[NSString alloc] initWithUTF8String: newkey];
  mi->state = (newflags | enabled);
  if (mi->key) {
    free(mi->key);
    mi->key = strdup(newkey);
  }
  NSMenuItem *item = (NSMenuItem *)mi->native;
  [item setKeyEquivalent: key];
  [item setKeyEquivalentModifierMask: nflags];
  //[[item menu] itemChanged: item];
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
   // NSMenu *menuApp = [[NSMenu alloc] initWithTitle: @"Apple Menu"];
    NSMenu *menuApp = [[NSMenu alloc] initWithTitle: @"MyApp"];
    [menuApp setAutoenablesItems: NO]; // Beware

    shoes_settings *st;
    Data_Get_Struct(shoes_world->settings, shoes_settings, st);
    VALUE shoestext = st->app_name;
    VALUE shoesmenu = shoes_menu_alloc(cShoesMenu);
    shoes_menu *mn;
    Data_Get_Struct(shoesmenu, shoes_menu, mn);
    mn->title = strdup(RSTRING_PTR(shoestext));
    // set Shoes Menu in Shoes menubar
    rb_ary_store(mb->menus, 0, shoesmenu);
    mn->native = (void *)menuApp;
    
    // This native doesn't have a Shoes menuitem (yet)
    NSMenu *menuServices = [[NSMenu alloc] initWithTitle: @"Services"];
    [NSApp setServicesMenu:menuServices];    
    
    
    // Now populate with MenuItems unless told not to..
    NSMenuItem *menuitem;    
	int flags = MENUITEM_ENABLE; 
	char *key = "";

    if ( st->osx_menutrim == Qfalse) {    
	    //menuitem = [menuApp addItemWithTitle:@"Open..."
	    //    action:@selector(openFile:) keyEquivalent:@"o"];
	    //[menuitem setTarget: shoes_world->os.events];
	    
	    // We don't have Shoes app yet. 
	    VALUE otext = rb_str_new2("Open");
	    VALUE oproc = rb_eval_string("proc { Shoes.show_selector }");
	    VALUE oitem = shoes_menuitem_new(otext, flags | MENUITEM_CONTROL, "o", oproc, Qnil);
	    shoes_menu_append(shoesmenu, oitem);
	
	    // Console (log)
	    VALUE lgtext = rb_str_new2("Console");
	    VALUE lgproc = rb_eval_string("proc { Shoes.show_log }");
	    VALUE lgitem = shoes_menuitem_new(lgtext, flags | MENUITEM_CONTROL, "/", lgproc, Qnil);
	    shoes_menu_append(shoesmenu, lgitem);
	  
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
	    // Preferences 
	    //[menuApp addItemWithTitle:@"Preferences..." action:nil keyEquivalent:@""];
	    VALUE pftext = rb_str_new2("Preferences...");
	    VALUE pfitem = shoes_menuitem_new(pftext, 0, "", Qnil, Qnil);
	    shoes_menu_append(shoesmenu, pfitem);
    }
    //[menuApp addItem: [NSMenuItem separatorItem]];
    VALUE stext = rb_str_new2("--- a seperator");
    VALUE s3item = shoes_menuitem_new(stext, flags, key, Qnil, Qnil);
    shoes_menu_append(shoesmenu, s3item);
    
    VALUE srvtext = rb_str_new2("Services");
    VALUE srvitem = shoes_menuitem_new(srvtext, flags | NO_NATIVE, "", Qnil, Qnil);
    shoes_menuitem *srvmi;
    Data_Get_Struct(srvitem, shoes_menuitem, srvmi);
    menuitem = [[NSMenuItem alloc] initWithTitle: @"Services"
        action:nil keyEquivalent:@""];
    [menuitem setSubmenu:menuServices];
    [menuApp addItem: menuitem];
    srvmi->native = (void *)menuitem;
    shoes_menu_append(shoesmenu, srvitem);
    [menuitem release];
    
    //[menuApp addItem: [NSMenuItem separatorItem]];
    //VALUE stext = rb_str_new2("--- a seperator");
    VALUE s2item = shoes_menuitem_new(stext, flags, key, Qnil, Qnil);
    shoes_menu_append(shoesmenu, s2item);
    
    VALUE hdtext = rb_str_new2("Hide");
    VALUE hditem = shoes_menuitem_new(hdtext, flags | NO_NATIVE, "", Qnil, Qnil);
    shoes_menuitem *hdmi;
    Data_Get_Struct(hditem, shoes_menuitem, hdmi);
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Hide"
        action:@selector(hide:) keyEquivalent:@""];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    hdmi->native = (void *)menuitem;
    shoes_menu_append(shoesmenu, hditem);
    [menuitem release];
    
    VALUE hotext = rb_str_new2("Hide Others");
    VALUE hoitem = shoes_menuitem_new(hotext, flags | NO_NATIVE, "", Qnil, Qnil);
    shoes_menuitem *homi;
    Data_Get_Struct(hoitem, shoes_menuitem, homi);
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Hide Others"
        action:@selector(hideOtherApplications:) keyEquivalent:@""];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    homi->native = (void *)menuitem;
    shoes_menu_append(shoesmenu, hoitem);
    [menuitem release];
    
    VALUE satext = rb_str_new2("Show All");
    VALUE saitem = shoes_menuitem_new(satext, flags | NO_NATIVE, "", Qnil, Qnil);
    shoes_menuitem *sami;
    Data_Get_Struct(saitem, shoes_menuitem, sami);
    menuitem = [[NSMenuItem alloc] initWithTitle:@"Show All"
        action:@selector(unhideAllApplications:) keyEquivalent:@""];
    [menuitem setTarget: NSApp];
    [menuApp addItem: menuitem];
    sami->native = (void *)menuitem;
    shoes_menu_append(shoesmenu, saitem); 
    [menuitem release];
    
    //[menuApp addItem: [NSMenuItem separatorItem]];
    VALUE s1item = shoes_menuitem_new(stext, flags, key, Qnil, Qnil);
    shoes_menu_append(shoesmenu, s1item);
 
    //menuitem = [[NSMenuItem alloc] initWithTitle:@"Quit"
    //    action:@selector(terminate:) keyEquivalent:@"q"];
    //[menuitem setTarget: NSApp];
    //[menuApp addItem: menuitem];
    //[menuitem release];
    VALUE qtext = rb_str_new2("Quit");
    VALUE qproc = rb_eval_string("proc { Shoes.quit }");
    VALUE qitem = shoes_menuitem_new(qtext, MENUITEM_ENABLE | MENUITEM_CONTROL, "q", qproc, Qnil); 
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


