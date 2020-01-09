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
#include "shoes/types/settings.h"

extern int shoes_app_serial_num;

// -------- menubar -----

#if 0
void shoes_gtk_check_quit(shoes_app *app) {
  //VALUE mnv = shoes_menubar_at(app->menubar, INT2NUM(0));
  VALUE mnv = shoes_menubar_at(app->menubar, rb_str_new2("Shoes"));
  if (NIL_P(mnv)) {
    fprintf(stderr,"Shoes menu is nil\n");
    return;
  }
  VALUE miv = shoes_menu_at(mnv, rb_str_new2("Quit"));
  if (NIL_P(miv)) {
    fprintf(stderr,"Quit item is nil\n");
    return;
  }
  fprintf(stderr,"Quit OK\n");
}
#endif

/*
 * Builds a minimal Shoes menu.
*/ 
VALUE shoes_native_menubar_setup(shoes_app *app, void *gtkmb) {
    GtkWidget *menubar = gtkmb;

    if (gtkmb == NULL) {
      menubar = gtk_menu_bar_new(); 
      app->os.menubar = menubar;
      app->have_menu = TRUE;
    } else {
      menubar = (GtkWidget *)gtkmb;
    }
    if (NIL_P(app->menubar)) {
      // get the GtkWidget for the app window
      shoes_app_gtk *gk = &app->os;
      GtkWidget *root = (GtkWidget *)gk->window;
      GtkAccelGroup *accel_group = NULL; 
      accel_group = gtk_accel_group_new();
      gtk_window_add_accel_group(GTK_WINDOW(root), accel_group); 
      app->os.accel_group = accel_group;
            
      // use the platform neutral calls to build Shoes menu
      // we can't do that for the menubar so we'll build it ourself
      VALUE mbv = shoes_menubar_alloc(cShoesMenubar);
      Get_TypedStruct2(mbv, shoes_menubar, mb);
      mb->context = app->canvas;
      mb->native = (void *)menubar;
      
      // save menubar object in app object
      app->menubar = mbv;
    }
#if 0
    // Flyby menubar creation: DOES NOT WORK reliably. 
    // When menubar was not specified at App creation (or not in settings)
    // but called by canvas method,  then build a minimal menu .
    if (gtkmb == NULL) {
      Get_TypedStruct2(shoes_world->settings, shoes_settings, st);
      VALUE shoestext = st->app_name;
      VALUE shoesmenu = shoes_menu_new(shoestext);
      shoes_menubar_append(app->menubar, shoesmenu);
      gtk_box_pack_start(GTK_BOX(app->os.opt_container), menubar, FALSE, FALSE, 0);
      GtkRequisition reqmin, reqnat;
      gtk_widget_get_preferred_size(app->os.opt_container, &reqmin, &reqnat);
      fprintf(stderr, "optbox h: %d\n", reqmin.height); // 38 for me. 
      //gtk_widget_set_size_request(app->os.menubar, reqmin.width, reqmin.height); 
      //gtk_widget_show_all(app->os.menubar); // no effect
      //app->mb_height = shoes_gtk_optbox_height(app, app->height); // any pos hgt will do
    }
#endif
    return app->menubar;
}

// Sets up the default Shoes menus
void shoes_native_build_menus(shoes_app *app, VALUE mbv) {
      
      // Shoes menu
      //VALUE shoestext = rb_str_new2("Shoes");
      Get_TypedStruct2(shoes_world->settings, shoes_settings, st);
      VALUE shoestext = st->app_name;
      VALUE shoesmenu = shoes_menu_new(shoestext);
      int flags = MENUITEM_ENABLE;
      char *key = "";
      // New/open Shoes.show_selector
      VALUE otext = rb_str_new2("Open");
      VALUE oproc = rb_eval_string("proc { Shoes.show_selector }");
      VALUE oitem = shoes_menuitem_new(otext,flags|MENUITEM_CONTROL, "o", oproc, app->canvas);
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
      VALUE qproc = rb_eval_string("proc { Shoes.quit }");
      VALUE qitem = shoes_menuitem_new(qtext, flags | MENUITEM_CONTROL, "q", qproc, app->canvas); 

      shoes_menu_append(shoesmenu, qitem);
      shoes_menubar_append(mbv, shoesmenu);
      //shoes_gtk_check_quit(app);
}

#if 0
void shoes_native_menubar_dump(GList *list) {
  GList *l;
  char *label;
  for (l = list; l != NULL; l = l->next)
  {
    // do something with l->data
    GtkWidget *widget = l->data;
    GtkMenuItem *mn = l->data;
    label = gtk_menu_item_get_label(mn);
    fprintf(stderr,"item: %s\n", label);
  }
}
#endif

void shoes_native_menubar_update(VALUE canvasv) {
  shoes_canvas *canvas;
  TypedData_Get_Struct(canvasv, shoes_canvas, &shoes_canvas_type, canvas);
  shoes_app *app = canvas->app; 
  gtk_widget_show_all(app->os.window); // TODO Kind of works; Narrow scope?
}

void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn) {
  //fprintf(stderr, "mbar: append %s to mbar\n", mn->title);
  GtkWidget *menubar = (GtkWidget *)mb->native;
  GtkWidget *menu = (GtkWidget *)mn->native; // or extra?
  // add the app accel group to the menu
  shoes_canvas *canvas;
  TypedData_Get_Struct(mb->context, shoes_canvas, &shoes_canvas_type, canvas);
  shoes_app *app = canvas->app;
  gtk_menu_set_accel_group((GtkMenu *)mn->extra, app->os.accel_group);
  
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menu);
  shoes_native_menubar_update(mb->context);
}

void shoes_native_menubar_remove(shoes_menubar *mb, int pos) {
  VALUE mnv = rb_ary_entry(mb->menus, pos);
  Get_TypedStruct2(mnv, shoes_menu, mn);
  //fprintf(stderr, "mbar: remove %s at %d\n", mn->title, pos);
  gtk_container_remove(GTK_CONTAINER(mb->native), GTK_WIDGET(mn->native));
}

void shoes_native_menubar_insert(shoes_menubar *mb, shoes_menu *mn, int pos) {
  //fprintf(stderr, "mbar: insert menu %s at pos %d\n", mn->title, pos);
  // add the app accel group to the menu
  shoes_canvas *canvas;
  TypedData_Get_Struct(mb->context, shoes_canvas, &shoes_canvas_type, canvas);
  shoes_app *app = canvas->app;
  gtk_menu_set_accel_group((GtkMenu *)mn->extra, app->os.accel_group);

  gtk_menu_shell_insert(GTK_MENU_SHELL(GTK_WIDGET(mb->native)), 
    (GtkWidget *)mn->native, pos);    
  shoes_native_menubar_update(mb->context);
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
  char path[100];
  sprintf(path, "<AppID%d>/%s", shoes_app_serial_num, mn->title);
  gtk_menu_set_accel_path((GtkMenu *)menu, path);
  return mn->native;
}
#if 0
void shoes_native_menu_dump(GList *list) {
  GList *l;
  char *label;
  for (l = list; l != NULL; l = l->next)
  {
    // do something with l->data
    GtkWidget *widget = l->data;
    GtkMenuItem *mn = l->data;
    label = gtk_menu_item_get_label(mn);
    fprintf(stderr,"item: %s\n", label);
  }
}
#endif

void shoes_native_menu_update(VALUE canvasv) {
  shoes_canvas *canvas;
  TypedData_Get_Struct(canvasv, shoes_canvas, &shoes_canvas_type, canvas);
  shoes_app *app = canvas->app; 
  shoes_app_gtk *gk = &app->os; 
  gtk_widget_show_all(gk->window); // TODO Kind of works; Narrow scope?
}

void shoes_gtk_accel_path(shoes_menu *mn, shoes_menuitem *mi) {
  if (mi->key && strlen(mi->key)) { 
   char path[100];   
   sprintf(path,"<AppID%d>/%s/%s", shoes_app_serial_num, mn->title, mi->title);
   gtk_menu_item_set_accel_path((GtkMenuItem *)mn->native, path);
   guint gkey = gdk_keyval_from_name(mi->key);
    gint gflags = 0;
    if (mi->state & MENUITEM_CONTROL)
      gflags = gflags | GDK_CONTROL_MASK;
    if (mi->state & MENUITEM_SHIFT)
      gflags = gflags | GDK_SHIFT_MASK;
    if (mi->state & MENUITEM_ALT)
      gflags = gflags | GDK_MOD1_MASK;

    GtkAccelKey oldkey;
    if (gtk_accel_map_lookup_entry(path, &oldkey)) {
      // replace
      gtk_accel_map_change_entry(path, gkey, gflags, TRUE);
    } else {
      // insert
      gtk_accel_map_add_entry(path, gkey, gflags);
    }
    //gtk_widget_add_accelerator(gmi, "activate", app->os.accel_group, 
    //  gkey, gflags, GTK_ACCEL_VISIBLE); 
 }
}

void shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi) {
  // does menuitem have accel key? We can create the path now
  if (mi->key && (mi->state & (MENUITEM_CONTROL|MENUITEM_SHIFT|MENUITEM_ALT))) {
    shoes_gtk_accel_path(mn, mi);
  }
  // Is there a block? We need to activate it here
  if (! NIL_P(mi->block)) {
    g_signal_connect(G_OBJECT(mi->native), "activate",
            G_CALLBACK(shoes_native_menuitem_callback), (gpointer) mi);
  }
  
  gtk_menu_shell_append(GTK_MENU_SHELL(GTK_WIDGET(mn->extra)), (GtkWidget *)mi->native);
  // trigger something to update? 
  shoes_native_menu_update(mi->context);
}

void shoes_native_menu_insert(shoes_menu *mn, shoes_menuitem *mi, int pos) {
  //fprintf(stderr, "insert %s into %s at pos %d\n", mi->title, mn->title, pos);
  if (mi->key && (mi->state & (MENUITEM_CONTROL|MENUITEM_SHIFT|MENUITEM_ALT))) {
    shoes_gtk_accel_path(mn, mi);
  }
  // Is there a block? We need to activate it here
  if (! NIL_P(mi->block)) {
    g_signal_connect(G_OBJECT(mi->native), "activate",
            G_CALLBACK(shoes_native_menuitem_callback), (gpointer) mi);
  }
   gtk_menu_shell_insert(GTK_MENU_SHELL(GTK_WIDGET(mn->extra)), 
    (GtkWidget *)mi->native, pos);
  shoes_native_menu_update(mi->context);
}

void shoes_native_menu_remove(shoes_menu *mn, int pos) {
  VALUE miv = rb_ary_entry(mn->items, pos);
  Get_TypedStruct2(miv, shoes_menuitem, mi);
 // fprintf(stderr, "Menu %s, delete %s\n", mn->title, mi->title);
  gtk_container_remove(GTK_CONTAINER(mn->extra), GTK_WIDGET(mi->native));
}

// -------- menuitem ------

void shoes_native_menuitem_callback(GtkWidget *wid, gpointer extra) {
  shoes_menuitem *mi = (shoes_menuitem *)extra;
  //fprintf(stderr,"menu %s triggered\n", mi->title);
  shoes_safe_block(mi->context, mi->block, rb_ary_new3(1, mi->context));
}

void shoes_gtk_accel_set(char path, guint key, GdkModifierType mods) {
}

void *shoes_native_menuitem_new(shoes_menuitem *mi) {
  GtkWidget *gmi;
  gmi = gtk_menu_item_new_with_label(mi->title);
  mi->native = (void *)gmi;
#if 0
  shoes_canvas *canvas;
  TypedData_Get_Struct(mi->context, shoes_canvas, &shoes_canvas_type, canvas);
  shoes_app *app = canvas->app;
  if (mi->key && strlen(mi->key) && mi->state > MENUITEM_ENABLE) { 
    guint gkey = gdk_keyval_from_name(mi->key);
    gint gflags = 0;
    if (mi->state & MENUITEM_CONTROL)
      gflags = gflags | GDK_CONTROL_MASK;
    if (mi->state & MENUITEM_SHIFT)
      gflags = gflags | GDK_SHIFT_MASK;
    if (mi->state & MENUITEM_ALT)
      gflags = gflags | GDK_MOD1_MASK;
    
    gtk_widget_add_accelerator(gmi, "activate", app->os.accel_group, 
      gkey, gflags, GTK_ACCEL_VISIBLE); 
  }
  if (! NIL_P(mi->block)) {
    g_signal_connect(G_OBJECT(gmi), "activate",
            G_CALLBACK(shoes_native_menuitem_callback), (gpointer) mi);
  }
#endif
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

void shoes_native_menuitem_set_key(shoes_menuitem *mi, int newflags, char *newkey) {
  int mask = MENUITEM_ALT | MENUITEM_CONTROL | MENUITEM_SHIFT;
  mi->state = newflags;
  if (mi->key) {
    free(mi->key);
    mi->key = strdup(newkey);
  }
  if (mi->key && strlen(mi->key) && (mi->state & mask)) {
    guint gkey = gdk_keyval_from_name(mi->key);
    gint gflags = 0;
    if (mi->state & MENUITEM_CONTROL)
      gflags = gflags | GDK_CONTROL_MASK;
    if (mi->state & MENUITEM_SHIFT)
      gflags = gflags | GDK_SHIFT_MASK;
    if (mi->state & MENUITEM_ALT)
      gflags = gflags | GDK_MOD1_MASK;
    
    char path[100];   
    shoes_menu *mn = mi->parent;
    sprintf(path,"<AppID%d>/%s/%s", shoes_app_serial_num, mn->title, mi->title);    
    gtk_menu_item_set_accel_path((GtkMenuItem *)mi->native, path); 
    gtk_accel_map_change_entry(path, gkey, gflags, TRUE);

  }
}
