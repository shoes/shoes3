#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"
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
void shoes_native_menubar_quit(void *extra) {
  //fprintf(stderr, "Dummy quit called\n");
  shoes_native_quit();
}

// TODO: calling this will segfault
VALUE shoes_native_menubar_make_block(char *code) {
  VALUE block;
  block = rb_eval_string(code);
#if 0
  int argc = 1;
  VALUE argv[2];
  argv[0] = rb_str_new2(code);
  argv[1] = Qnil;
  rb_scan_args(argc, argv, "0&", &block);
#endif
  return block;
}

VALUE shoes_native_menubar_setup(shoes_app *app) {
    GtkWidget *menubar;      
    GtkWidget *fileMenu;
    GtkWidget *fileMi;
    GtkWidget *quitMi;  
    if (app->have_menu == 0)
      return Qnil;
    //if (! NIL_P(shoes_gtk_menubar)) {
    //  app->menubar = shoes_gtk_menubar;
    //  return shoes_gtk_menubar;
    //}
    if (NIL_P(app->menubar)) {
#if 0 //     
      menubar = gtk_menu_bar_new();
      fileMenu = gtk_menu_new();
    
      fileMi = gtk_menu_item_new_with_label("Shoes");
      quitMi = gtk_menu_item_new_with_label("Quit");
    
      gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu);
      gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quitMi);
      gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi);
      g_signal_connect(G_OBJECT(quitMi), "activate",
            G_CALLBACK(shoes_native_menubar_quit), NULL); 
            
      // Thats gtk's menus - now setup Shoes Objects to match
      // without calling ourself inappropriately
      VALUE mbv = shoes_menubar_alloc(cShoesMenubar);
      shoes_menubar *mb;
      Data_Get_Struct(mbv, shoes_menubar, mb);
      mb->native = (void *)menubar;
      // save menubar object in app object
      app->menubar = mbv;
      
      VALUE mnv = shoes_menu_alloc(cShoesMenu);
      shoes_menu *mn;
      Data_Get_Struct(mnv, shoes_menu, mn);
      mn->title = "File";
      mn->native = (void *)fileMenu;
      // link mn to the menubar
      rb_ary_push(mb->menus, mnv);
      
      VALUE miv = shoes_menuitem_alloc(cShoesMenuitem);
      shoes_menuitem *mi;
      Data_Get_Struct(miv, shoes_menuitem, mi);
      mi->title = "Quit";
      mi->key = "";
      //mi->block = Qnil; // TODO, for now we use the signal handler above
      mi->native = (void *)quitMi;
      mi->block = shoes_native_menubar_make_block("proc { Shoes.quit }");
      g_signal_connect(G_OBJECT(mi->native), "activate",
          G_CALLBACK(shoes_native_menuitem_callback), (gpointer) mi);
      mi->context = app->canvas;
      // add to Shoes menu
      rb_ary_push(mn->items, miv);
#else      
      // use the platform neutral calls to build Shoes menu
      // we can't do that for the menubar or we'll call ourself
      menubar = gtk_menu_bar_new();
      VALUE mbv = shoes_menubar_alloc(cShoesMenubar);
      shoes_menubar *mb;
      Data_Get_Struct(mbv, shoes_menubar, mb);
      mb->native = (void *)menubar;
      // save menubar object in app object
      app->menubar = mbv;
      
      // Shoes menu
      VALUE shoestext = rb_str_new2("Shoes");
      VALUE shoesmenu = shoes_menu_new(shoestext);
      
      // New/open Shoes.show_selector
      VALUE otext = rb_str_new2("Open");
      VALUE oproc = rb_eval_string("proc { Shoes.show_selector }");
      VALUE oitem = shoes_menuitem_new(otext, Qnil, oproc, app->canvas);
      shoes_menu_append(shoesmenu, oitem);
      // -------- separator
      VALUE stext = rb_str_new2("--- a seperator");
      VALUE s1item = shoes_menuitem_new(stext, Qnil, Qnil, Qnil);
      shoes_menu_append(shoesmenu, s1item);
      // Manual 
      VALUE mtext = rb_str_new2("Manual");
      VALUE mproc = rb_eval_string("proc { Shoes.show_manual }");
      VALUE mitem = shoes_menuitem_new(mtext, Qnil, mproc, app->canvas);
      shoes_menu_append(shoesmenu, mitem);
      // Cobbler
      VALUE ctext = rb_str_new2("Cobbler");
      VALUE cproc = rb_eval_string("proc { Shoes.cobbler }");
      VALUE citem = shoes_menuitem_new(ctext, Qnil, cproc, app->canvas);
      shoes_menu_append(shoesmenu, citem);
      // Profile - bug in profiler (#400 , @dredknight)
      VALUE ftext = rb_str_new2("Profile");
      VALUE fproc = rb_eval_string("proc { require 'shoes/profiler'; Shoes.profile(nil) }");
      VALUE fitem = shoes_menuitem_new(ftext, Qnil, fproc, app->canvas);
      shoes_menu_append(shoesmenu, fitem);
      // Package
      VALUE ptext = rb_str_new2("Package");
      VALUE pproc = rb_eval_string("proc { Shoes.app_package }");
      VALUE pitem = shoes_menuitem_new(ptext, Qnil, pproc, app->canvas);
      shoes_menu_append(shoesmenu, pitem);
      // --------
      VALUE s2item = shoes_menuitem_new(stext, Qnil, Qnil, Qnil);
      shoes_menu_append(shoesmenu, s2item);
      // Quit
      VALUE qtext = rb_str_new2("Quit");
      VALUE qproc = rb_eval_string("proc { Shoes.quit() }");
      VALUE qitem = shoes_menuitem_new(qtext, Qnil, qproc, app->canvas);      
      shoes_menu_append(shoesmenu, qitem);
      
      shoes_menubar_append(mbv, shoesmenu);
#endif 
    }
    return app->menubar;
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

// -------- menuitem ------

void shoes_native_menuitem_callback(GtkWidget *wid, gpointer extra) {
  shoes_menuitem *mi = (shoes_menuitem *)extra;
  //fprintf(stderr,"menu %s triggered\n", mi->title);
  shoes_safe_block(mi->context, mi->block, rb_ary_new3(1, mi->context));
}

void *shoes_native_menuitem_new(shoes_menuitem *mi) {
  GtkWidget *gmi;
  gmi = gtk_menu_item_new_with_label(mi->title);
  if (mi->key) {
    // TODO: acceleraters like 'control_q or shift_control_i'
    int ctlk = 0;
    int shiftk = 0;
    if (strncpy(mi->key,"control_",8) == 0) {
    }
  }
  mi->native = (void *)gmi;
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
