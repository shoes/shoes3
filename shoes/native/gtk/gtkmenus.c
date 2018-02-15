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
void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn) {
  GtkWidget *menubar = (GtkWidget *)mb->native;
  GtkWidget *menu = (GtkWidget *)mn->native;
  // TODO problems here:
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menu);
}

/*
 * This is clumsy. Create a Menubar, File Menu, and File->Quit in
 * both gtk and Shoes. This happens when menus are requested for a app/window
 * The default set of menus is created here.
*/ 
void shoes_native_menubar_quit(void *extra) {
  fprintf(stderr, "Dummy quit called\n");

}

VALUE shoes_native_menubar_setup(shoes_app *app) {
    GtkWidget *menubar;      
    GtkWidget *fileMenu;
    GtkWidget *fileMi;
    GtkWidget *quitMi;  
    if (app->have_menu == 0)
      return Qnil;
    if (NIL_P(app->menubar)) {
    
      menubar = gtk_menu_bar_new();
      fileMenu = gtk_menu_new();
    
      fileMi = gtk_menu_item_new_with_label("File");
      quitMi = gtk_menu_item_new_with_label("Quit");
    
      gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu);
      gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quitMi);
      gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi);
      g_signal_connect(G_OBJECT(quitMi), "activate",
            G_CALLBACK(shoes_native_menubar_quit), NULL); 
            
      // Thats gtk's menus - now setup Shoes Objects to match
      // without calling ourself inappropriately
      VALUE mbv = shoes_menubar_alloc(cMenubar);
      shoes_menubar *mb;
      Data_Get_Struct(mbv, shoes_menubar, mb);
      mb->native = (void *)menubar;
      // save menubar object in app object
      app->menubar = mbv;
      
      VALUE mnv = shoes_menu_alloc(cMenu);
      shoes_menu *mn;
      Data_Get_Struct(mnv, shoes_menu, mn);
      mn->title = "File";
      mn->native = (void *)fileMenu;
      // link mn to the menubar
      rb_ary_push(mb->menus, mnv);
      
      VALUE miv = shoes_menuitem_alloc(cMenuitem);
      shoes_menuitem *mi;
      Data_Get_Struct(miv, shoes_menuitem, mi);
      mi->title = "Quit";
      mi->key = "";
      mi->block = Qnil; // TODO, for now we use the signal handler above
      // add to menu
      rb_ary_push(mn->items, miv);
    }
    return app->menubar;
}



// ------- menu ------
void *shoes_native_menu_new(VALUE mnv, char *name) {
  GtkWidget *menu;
  GtkWidget *mi;
  shoes_menu *mn;
  Data_Get_Struct(mnv, shoes_menu, mn);
  menu = gtk_menu_new();
  mi = gtk_menu_item_new_with_label(name);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi), menu);
  mn->native = (void *)menu;
}

void *shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi) {
  gtk_menu_shell_append(GTK_MENU_SHELL(GTK_WIDGET(mn->native)), (GtkWidget *)mi->native);  
}

// -------- menuitem ------

void shoes_native_menuitem_callback(void *extra) {
  shoes_menuitem *mi = (shoes_menuitem *)extra;
  // TODO: rb_eval mi->block
  fprintf(stderr,"menu %s triggered\n", mi->title);
}

void *shoes_native_menuitem_new(shoes_menuitem *mi) {
  GtkWidget *gmi;
  // Todo: acceleraters like 'control_q'
  gmi = gtk_menu_item_new_with_label(mi->title);
  mi->native = (void *)gmi;
  // TODO: setup gsignal callback
  if (! NIL_P(mi->block)) {
    g_signal_connect(G_OBJECT(gmi), "activate",
            G_CALLBACK(shoes_native_menuitem_callback), mi);   }
  return (void *)mi->native;
}

