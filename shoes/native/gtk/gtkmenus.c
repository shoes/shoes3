#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"
#include "shoes/canvas.h"
#include "shoes/types/menubar.h"
#include "shoes/types/menu.h"
#include "shoes/types/menuitem.h"
#include "shoes/native/gtk/gtkmenus.h"


// -------- menubar -----

//VALUE shoes_gtk_menubar = Qnil; // All apps will share. OSX behavior - live with it

void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn) {
  GtkWidget *menubar = (GtkWidget *)mb->native;
  GtkWidget *menu = (GtkWidget *)mn->native;
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menu);
}

/*
 * This is clumsy. Create a Menubar, File Menu, and File->Quit in
 * both gtk and Shoes. This happens when menus are requested for a app/window
 * The default set of menus is created here.
*/ 

VALUE shoes_native_menubar_setup(shoes_app *app) {
    GtkWidget *menubar;      
    GtkWidget *fileMenu;
    GtkWidget *fileMi;
    GtkWidget *quitMi;  
    if (app->have_menu == 0)
      return Qnil;
    if (NIL_P(app->menubar)) {
      // get the GtkWidget for the app window
      shoes_app_gtk *gk = &app->os;
      GtkWidget *root = (GtkWidget *)gk->window;
      GtkAccelGroup *accel_group = NULL; 
      accel_group = gtk_accel_group_new();
      gtk_window_add_accel_group(GTK_WINDOW(root), accel_group); // TODO
            
      // use the platform neutral calls to build Shoes menu
      // we can't do that for the menubar or we'll build it ourself
      menubar = gtk_menu_bar_new();
      VALUE mbv = shoes_menubar_alloc(cShoesMenubar);
      shoes_menubar *mb;
      Data_Get_Struct(mbv, shoes_menubar, mb);
      mb->native = (void *)menubar;
      //mb->root = (void *)root;
      //mb->accgroup = (void *)accel_group;
      // save menubar object in app object
      app->menubar = mbv;
    }
    return app->menubar;
}

void shoes_native_build_menus(shoes_app *app,VALUE mbv) {
      
      // Shoes menu
      VALUE shoestext = rb_str_new2("Shoes");
      VALUE shoesmenu = shoes_menu_new(shoestext);
      int flags = MENUITEM_ENABLE;
      char *key = "";
      // New/open Shoes.show_selector
      VALUE otext = rb_str_new2("Open");
      VALUE oproc = rb_eval_string("proc { Shoes.show_selector }");
      VALUE oitem = shoes_menuitem_new(otext,flags, key, oproc, app->canvas);
      shoes_menu_append(shoesmenu, oitem);
      // -------- separator
      VALUE stext = rb_str_new2("--- a seperator");
      VALUE s1item = shoes_menuitem_new(stext, flags, key, Qnil, app->canvas);
      shoes_menu_append(shoesmenu, s1item);
      // Manual 
      VALUE mtext = rb_str_new2("Manual");
      VALUE mproc = rb_eval_string("proc { Shoes.show_manual }");
      VALUE mitem = shoes_menuitem_new(mtext, flags, key, mproc, app->canvas);
      shoes_menu_append(shoesmenu, mitem);
      // Cobbler
      VALUE ctext = rb_str_new2("Cobbler");
      VALUE cproc = rb_eval_string("proc { Shoes.cobbler }");
      VALUE citem = shoes_menuitem_new(ctext, flags, key, cproc, app->canvas);
      shoes_menu_append(shoesmenu, citem);

      // Profile - bug in profiler (#400 , @dredknight)
      VALUE ftext = rb_str_new2("Profile");
      VALUE fproc = rb_eval_string("proc { require 'shoes/profiler'; Shoes.profile(nil) }");
      VALUE fitem = shoes_menuitem_new(ftext, flags, key, fproc, app->canvas);
      shoes_menu_append(shoesmenu, fitem);
      // Package
      VALUE ptext = rb_str_new2("Package");
      VALUE pproc = rb_eval_string("proc { Shoes.app_package }");
      VALUE pitem = shoes_menuitem_new(ptext, flags, key, pproc, app->canvas);
      shoes_menu_append(shoesmenu, pitem);
      // --------
      VALUE s2item = shoes_menuitem_new(stext, flags, key, Qnil, app->canvas);
      shoes_menu_append(shoesmenu, s2item);

      // Quit
      VALUE qtext = rb_str_new2("Quit");
      VALUE qproc = rb_eval_string("proc { Shoes.quit() }");
      VALUE qitem = shoes_menuitem_new(qtext, flags | MENUITEM_CONTROL, "q", qproc, app->canvas);      
      shoes_menu_append(shoesmenu, qitem);
      
      shoes_menubar_append(mbv, shoesmenu);
}

void shoes_native_menubar_remove(shoes_menubar *mb, int pos) {
  fprintf(stderr,"menubar delete at %d\n", pos);
}

void shoes_native_menubar_insert(shoes_menubar *mb, shoes_menu *mn, int pos) {
  fprintf(stderr, "add menu %s at pos %d\n", mn->title, pos);
}

// ------- menu ------
void *shoes_native_menu_new(shoes_menu *mn) {
  GtkWidget *menu;
  GtkWidget *mi;
  menu = gtk_menu_new();
  mi = gtk_menu_item_new_with_label(mn->title);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi), menu);
  mn->native = (void *)mi; 
  mn->extra = (void *)menu;
  return mn->native;
}

void *shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi) {
  
  gtk_menu_shell_append(GTK_MENU_SHELL(GTK_WIDGET(mn->extra)), (GtkWidget *)mi->native);  
  return NULL;
}

void shoes_native_menu_insert(shoes_menu *mn, shoes_menuitem *mi, int pos) {
  fprintf(stderr, "insert %s into %s at pos %d\n", mi->title, mn->title, pos);
}

void shoes_native_menu_remove(shoes_menu *mn, int pos) {
  fprintf(stderr, "Menu %s, delete %d\n", mn->title, pos);
}

// -------- menuitem ------

void shoes_native_menuitem_callback(GtkWidget *wid, gpointer extra) {
  shoes_menuitem *mi = (shoes_menuitem *)extra;
  //fprintf(stderr,"menu %s triggered\n", mi->title);
  shoes_safe_block(mi->context, mi->block, rb_ary_new3(1, mi->context));
}

void *shoes_native_menuitem_new(shoes_menuitem *mi) {
  GtkWidget *gmi;
  gmi = gtk_menu_item_new_with_label(mi->title);
  mi->native = (void *)gmi;
  shoes_canvas *canvas;
  Data_Get_Struct(mi->context, shoes_canvas, canvas);
  shoes_app *app = canvas->app;
  shoes_app_gtk *gk = &app->os;
 
  if (mi->key && strlen(mi->key) && mi->state > MENUITEM_ENABLE) { 
    guint gkey = gdk_keyval_from_name(mi->key);
    gint gflags = 0;
    if (mi->state & MENUITEM_CONTROL)
      gflags = gflags | GDK_CONTROL_MASK;
    if (mi->state & MENUITEM_SHIFT)
      gflags = gflags | GDK_SHIFT_MASK;
    if (mi->state & MENUITEM_ALT)
      gflags = gflags | GDK_MOD1_MASK;
    gtk_widget_add_accelerator(gmi, "activate", gk->accel_group, 
      gkey, gflags, GTK_ACCEL_VISIBLE); 

  }
  if (! NIL_P(mi->block)) {
    g_signal_connect(G_OBJECT(gmi), "activate",
            G_CALLBACK(shoes_native_menuitem_callback), (gpointer) mi);
  }
  return (void *)mi->native;
}

// ----- separator - like a menuitem w/o callback
void *shoes_native_menusep_new(shoes_menuitem *mi) {
  GtkWidget *gmi = gtk_separator_menu_item_new();
  mi->native = (void *)gmi;
  return mi->native;
}

void shoes_native_menuitem_set_title(shoes_menuitem *mi) {
  gtk_menu_item_set_label ((GtkMenuItem *) mi->native, mi->title);
}

void shoes_native_menuitem_enable(shoes_menuitem *mi, int state) {
  gtk_widget_set_sensitive((GtkWidget *)mi->native, state);
}

void shoes_native_menuitem_set_key(shoes_menuitem *mi) {
}
