//
// shoes/native-gtk.c
// GTK+ code for Shoes.
//   Modified for Gtk-3.0 by Cecil Coupe (cjc) + @passenger94 
//
#ifndef GTK3
// fail only used for shoes_native_window_color will be deleted
#define GTK3
#endif
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/types/settings.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/types/text.h"
#include "shoes/types/text_link.h"
#include "shoes/types/download.h"
#include "shoes/types/event.h"
#include "shoes/internal.h"
#include "shoes/types/menubar.h"
#include "shoes/native/gtk/gtkmenus.h"


#include <fontconfig/fontconfig.h>
#ifndef RUBY_HTTP
#ifndef SHOES_GTK_WIN32
#include <curl/curl.h>
#endif
#endif
#include <pthread.h>
#include <glib/gprintf.h>

#include "gtk.h"
#include "shoes/native/gtk/gtkfixedalt.h"
#include "shoes/native/gtk/gtkentryalt.h"
#include "shoes/native/gtk/gtkcomboboxtextalt.h"
#include "shoes/native/gtk/gtkbuttonalt.h"
#include "shoes/native/gtk/gtkscrolledwindowalt.h"
#include "shoes/native/gtk/gtkprogressbaralt.h"

#define HEIGHT_PAD 0

#define SHOES_GTK_INVISIBLE_CHAR (gunichar)0x2022

/* forward declares in this file */
static void shoes_gtk_root_size(GtkWidget *widget, cairo_t *cr, gpointer data);
//static void shoes_gtk_content_size(GtkWidget *widget, GtkAllocation *size, gpointer data);
int shoes_gtk_optbox_height(shoes_app *app, int height);
void shoes_gtk_attach_menubar(shoes_app *app, shoes_settings *st);
void shoes_gtk_attach_toolbar(shoes_app *app, shoes_settings *st);

// Is mainloop poll()/select() or Timeout driven
#if defined(SHOES_GTK_WIN32) && ( !defined(GPOLL))
#define SGTMO
#else
#define SGPOLL 
#endif

int shoes_Windows_Version = 0;     // global var.

#ifdef SHOES_GTK_WIN32
int shoes_win10_gtk3_22_check();  // forward declare
#endif
/* 
 * Sigh. We need to accommodate Gnome Shells that "augment* the title bar
 * and/or provide a half baked global menu bar. Like Fedora 29 - but it's not
 * the only distro (Elementary) and it won't be the last. Not a global, yet.
*/
enum {
  OLD_SCHOOL,
  WAYLAND,
};
static int shoes_gtk_backend = OLD_SCHOOL;

/*
int shoes_gtk_set_desktop() {
  char *session = getenv("XDG_SESSION_TYPE");
  char *backend = getenv("GDK_BACKEND"); // not set normally.
  //if (gtk_get_minor_version() >= 24)
    //shoes_gtk_desktop |= GTK_3_24;
  if (session && strcmp(session, "wayland") == 0)
    shoes_gtk_desktop |= WAYLAND;
  if (backend && strcmp(backend,"wayland") == 0)
    shoes_gtk_desktop |= WAYLAND;
  printf("desktop: %s %s %d\n", backend, session, shoes_gtk_desktop);
}
*/
// ---------- fonts ------------

static VALUE shoes_make_font_list(FcFontSet *fonts, VALUE ary) {
    int i = 0;
    //printf("fontconfig says %d fonts\n", fonts->nfont);
    for (i = 0; i < fonts->nfont; i++) {
        FcValue val;
        FcPattern *p = fonts->fonts[i];
        if (FcPatternGet(p, FC_FAMILY, 0, &val) == FcResultMatch) {
            rb_ary_push(ary, rb_str_new2((char *)val.u.s));
            //printf("fc says %s\n", (char *)val.u.s);
        }    
    }
    rb_funcall(ary, rb_intern("uniq!"), 0);
    rb_funcall(ary, rb_intern("sort!"), 0);
    return ary;
}

#ifdef SHOES_GTK_WIN32
/*
 * This is only called when a shoe script uses the font(filename) command
 * so the file name is lacuna.ttf, coolvetica.ttf (Shoes splash or the
 * Shoes manual) or a user supplied font
*/
VALUE shoes_native_load_font(const char *filename) {
    VALUE allfonts, newfonts, oldfonts;
   // the Shoes api says after a font load, return an array of the font
    // name(s). FamilyName in font-speak.
    // The Shoes fontlist must be updated as side effect
    
    // Use the much faster Windows api. First remove the font if it
    // exists in Windows session space - otherwise you won't get a reload without
    // rebooting. Fun.
    int i = 0;
    while( RemoveFontResourceExA( filename, FR_PRIVATE, 0 ) )
    {
      i++;
    }
    if (i) 
      printf("removed %s %d times\n", filename, i);
    int fonts = AddFontResourceExA(filename, FR_PRIVATE, 0);
    if (fonts == 0) {
      printf("windows failed to add fonts in %s\n", filename);
      return Qnil;
    }
    allfonts = shoes_native_font_list();   // everything, include the one loaded
    oldfonts = rb_const_get(cShoes, rb_intern("FONTS"));
    newfonts = rb_funcall(allfonts, rb_intern("-"), 1, oldfonts);
    shoes_update_fonts(allfonts);
    return newfonts;
}


static int CALLBACK shoes_font_list_iter(const ENUMLOGFONTEX *font, const NEWTEXTMETRICA *pfont, DWORD type, LPARAM l) {
    rb_ary_push(l, rb_str_new2(font->elfLogFont.lfFaceName));
    return TRUE;
}

VALUE shoes_native_font_list() {
    LOGFONT font = {0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, ""};
    VALUE ary = rb_ary_new();
    HDC dc = GetDC(NULL);
    EnumFontFamiliesExA(dc, &font, (FONTENUMPROC)shoes_font_list_iter, (LPARAM)ary, 0);
    ReleaseDC(NULL, dc);
    rb_funcall(ary, rb_intern("uniq!"), 0);
    rb_funcall(ary, rb_intern("sort!"), 0);
    return ary;
}

#else  // Linux gtk3
#if 0
// Fontconfig list
VALUE shoes_native_font_list() {
    VALUE ary = rb_ary_new();
    FcConfig *fc = FcConfigGetCurrent();
    FcFontSet *fonts = FcConfigGetFonts(fc, FcSetApplication);
    if (fonts) shoes_make_font_list(fonts, ary);
    fonts = FcConfigGetFonts(fc, FcSetSystem);
    if (fonts) shoes_make_font_list(fonts, ary);
    return ary;
}

VALUE shoes_native_load_font(const char *filename) {
    FcConfig *fc = FcConfigGetCurrent();
    FcFontSet *fonts = FcFontSetCreate();
    if (!FcFileScan(fonts, NULL, NULL, NULL, (const FcChar8 *)filename, FcTrue))
        return Qnil;

    VALUE ary = rb_ary_new();
    shoes_make_font_list(fonts, ary);  // This is usually just 1 font
    FcFontSetDestroy(fonts);

    if (!FcConfigAppFontAddFile(fc, (const FcChar8 *)filename))
        return Qnil;

    // refresh the FONTS list
    shoes_update_fonts(shoes_native_font_list());
    return ary;
}
#else
// Pango list from https://www.lemoda.net/pango/list-fonts/index.html
VALUE shoes_native_font_list() {
    int i;
    PangoFontFamily ** families;
    int n_families;
    PangoFontMap * fontmap;
    VALUE ary = rb_ary_new();

    fontmap = pango_cairo_font_map_get_default();
    pango_font_map_list_families (fontmap, & families, & n_families);
    //printf ("There are %d families\n", n_families);
    for (i = 0; i < n_families; i++) {
        PangoFontFamily * family = families[i];
        const char * family_name;

        family_name = pango_font_family_get_name (family);
        //printf ("Family %d: %s\n", i, family_name);
        rb_ary_push(ary, rb_str_new2(family_name));
    }
    g_free (families);
    return ary;
}

VALUE shoes_native_load_font(const char *filename) {
    FcConfig *fc = FcConfigGetCurrent();
    FcFontSet *fonts = FcFontSetCreate();
    // issue 441 - can't be a directory or FcFileScan crashes
    if (!g_file_test(filename, G_FILE_TEST_EXISTS))
      return Qnil;
    if (g_file_test(filename, G_FILE_TEST_IS_DIR))
      return Qnil;
    if (FcFileScan(fonts, NULL, NULL, NULL, (const FcChar8 *)filename, FcTrue) 
        == FcFalse)
      return Qnil;

    VALUE ary = rb_ary_new();
    // should have a pango version below, eh?
    shoes_make_font_list(fonts, ary);
    FcFontSetDestroy(fonts);

    if (!FcConfigAppFontAddFile(fc, (const FcChar8 *)filename))
        return Qnil;

    // refresh the FONTS list
    shoes_update_fonts(shoes_native_font_list());
    return ary;
}
#endif // Pango version
#endif // Windows or Not

/*
 * Shoes 3.3.8+ may use gtk_application_new (aka GApplication) instead of gtk_init
*/

void
shoes_gtk_app_activate (GApplication *app, gpointer user_data) {
    fprintf(stderr, "shoes_gtk_app_activate called\n");
}

void shoes_gtk_app_open (GApplication *app, GFile **files,
      gint n_files, gchar *hint) {
    fprintf(stderr, "shoes_gtk_app_open called\n");
}

gboolean
shoes_gtk_app_startup(GApplication *application, gpointer user_data) {
    fprintf(stderr, "shoes_gtk_app_startup\n");
    return TRUE;
}

gboolean
shoes_gtk_app_cmdline (GApplication *application, gchar ***arguments,
      gint *exit_status)
{
  /* This doesn't seem to work as documented.
   * *exit_status is 0x0 instead of a real pointer, arguments don't seem
   * correct either. For Shoes it doesn't matter. Just tell gtk we processed
   * them so it doesn't have too. Need to do this for Gtk/Windows which has the
   * documented habit of reading the commandline twice
  */ 
  printf("command-line signal processed\n");
  return TRUE;
}


// Some globals for Gtk3 
GtkApplication *shoes_GtkApp; 

GtkCssProvider *shoes_css_provider = NULL; // user provided theme

void shoes_gtk_css_error(GtkCssProvider *provider,
               GtkCssSection  *section,
               GError         *error,
               gpointer        user_data) 
{
  fprintf(stderr,"%s\n", error->message);
}

// Process the setting for Theme and css 
void shoes_gtk_load_css(shoes_settings *st) {
  gchar *theme_path = NULL;
  if (! NIL_P(st->theme)) {
      // A theme was requested in startup.yaml
      theme_path = g_build_filename("themes", RSTRING_PTR(st->theme), 
          "gtk-3.0", "gtk.css", NULL);
      st->theme_path = rb_str_new2(theme_path);
  } else { 
    // do we have a user theme in $HOME ?
    gchar *dflt;
    const gchar *home = g_getenv("HOME");
#ifdef SHOES_GTK_WIN32
    dflt = g_build_filename(home, "AppData", "Local", "Shoes", "themes",
        "default", NULL);
#else
    dflt = g_build_filename(home, ".shoes", "themes", "default", NULL);
#endif
    GFile *df = g_file_new_for_path(dflt);
    GError *err = NULL;
    gchar *ln;
    if (FALSE == g_file_load_contents(df, NULL, &ln, NULL,NULL, &err)) {
      // This is the expected path for most people - no theme specified
      g_free(dflt);
      g_free(theme_path);
      g_object_unref(df);
      return;
    }
    gchar *p = g_strrstr(ln, "\n");
    if (p)
      *p = '\0';
    st->theme = rb_str_new2(ln);
    // build path to <theme-name>/gtk-3.0/gtk.css
    gchar *dir = g_path_get_dirname(dflt);  // without 'default'
    gchar *css = g_build_filename(dir, ln, "gtk-3.0", "gtk.css", NULL);
    st->theme_path = rb_str_new2(css);
    g_free(ln);
    g_free(css);
    g_free(dir);
    g_object_unref(df);
  }
  if (!NIL_P(st->theme) && !NIL_P(st->theme_path)) {
    g_free(theme_path);
    theme_path = strdup(RSTRING_PTR(st->theme_path));
    GError *gerr = NULL;
    shoes_css_provider = gtk_css_provider_new();
    g_signal_connect(G_OBJECT(shoes_css_provider), "parsing-error",
                     G_CALLBACK(shoes_gtk_css_error), NULL);
    gtk_css_provider_load_from_path(shoes_css_provider, theme_path, &gerr);
    if (gerr != NULL) {
      fprintf(stderr, "Failed css loading:css %s\n", gerr->message);
      g_error_free(gerr);
    }
    g_free(theme_path);
 }
}

/* 
 * We have a multitude of env vars we could (need) to set for Windows
 * See https://developer.gnome.org/gtk3/stable/gtk-running.html
 * NOTE: not everything on that pages is true. 
 * Note that the shoes settings file may have preferences
 * and we are dependent on who compiled gtk3 and how.
 */
#include <stdlib.h>
void shoes_native_init(char *start) {
#if !defined(RUBY_HTTP) && !defined(SHOES_GTK_WIN32)
    curl_global_init(CURL_GLOBAL_ALL);
#endif

    Get_TypedStruct2(shoes_world->settings, shoes_settings, st);
    char app_id[100];
    char *rdom = RSTRING_PTR(st->rdomain);
    if (st->mdi == Qtrue) {
      sprintf(app_id, "%s",rdom); 
    } else {
      sprintf(app_id, "%s%d", rdom, getpid()); // TODO: Windows?
    }
    // set the gdk_backend ?

    if (st->backend != Qnil) {
      char *backend = RSTRING_PTR(st->backend);
      gdk_set_allowed_backends(backend);
      if (strncmp(backend, "wayland", 7) == 0) {
        fprintf(stderr, "setting wayland backend\n");
        shoes_gtk_backend = shoes_gtk_backend | WAYLAND;
      }
    } else {
       // defaults to shoes_gtk_backend == OLD_SCHOOL
#ifdef SHOES_GTK_WIN32
       gdk_set_allowed_backends("win32,x11");

       shoes_Windows_Version = shoes_win10_gtk3_22_check();
       // Gtk3 on windows should be patched to not set CSD
#endif 
#ifdef SHOES_QUARTZ
        gdk_set_allowed_backends("quartz,x11");
#endif 
#if defined(SHOES_GTK) && !defined(SHOES_GTK_WIN32)
        gdk_set_allowed_backends("x11,wayland,mir");
        const char *csd = g_getenv("GTK_CSD");
        if (csd && strcmp(csd, "1") == 0) {
          // Turn it off. TODO: be more complex with settings?
          // TODO - try actually supporting CSD menus. 
          g_setenv("GTK_CSD", "0", TRUE);
        }
#endif
    }
#if defined(SHOES_GTK_WIN32) && defined(SHOES_GDKMODS)
    /* Windows: Gdk uses loadable modules - msys2' gtk is built this way
     * set GDK_PIXBUF_MODULE_FILE - in theory the installer did this too.
     * Execpt it's nice to run shoes w/o installing it. 
     */
    gchar *dirp = g_path_get_dirname(start);
    gchar *moddir = g_build_filename(dirp, "lib", "gdk-pixbuf-2.0", "2.10.0",
        "loaders.cache",NULL);
    g_setenv("GDK_PIXBUF_MODULE_FILE", moddir, TRUE);
    //char *tp = g_getenv("GDK_PIXBUF_MODULE_FILE");
    //fprintf(stderr,"GDK_PIXBUF_MODULE_FILE = %s\n", tp);
    g_free(dirp);
    g_free(moddir);

#endif
    //fprintf(stderr,"launching %s\n", app_id);
    shoes_GtkApp = gtk_application_new (app_id, G_APPLICATION_HANDLES_COMMAND_LINE);
    // register with dbus
    if (g_application_get_is_registered((GApplication *)shoes_GtkApp))
      fprintf(stderr, "%s is already registered\n", app_id);
    if (g_application_register((GApplication *)shoes_GtkApp, NULL, NULL)) {
      //fprintf(stderr,"%s is registered\n",app_id);
      st->dbus_name = rb_str_new2(app_id);
    }
    g_signal_connect(shoes_GtkApp, "activate", G_CALLBACK (shoes_gtk_app_activate), NULL);
    g_signal_connect(shoes_GtkApp, "command-line", G_CALLBACK (shoes_gtk_app_cmdline), NULL);
    g_signal_connect(G_APPLICATION(shoes_GtkApp), "startup", G_CALLBACK(shoes_gtk_app_startup), NULL);

    shoes_gtk_load_css(st);

#ifndef ENABLE_MDI
    gtk_init(NULL,NULL); // This starts the gui w/o triggering signals
#else
    g_application_run(G_APPLICATION(shoes_GtkApp), 0, NULL); // doesn't work but could?
#endif
    
}
/* end of GApplication init  */

void shoes_native_cleanup(shoes_world_t *world) {
#if !defined(RUBY_HTTP) && !defined(SHOES_GTK_WIN32)
    curl_global_cleanup();
#endif
}

void shoes_native_quit() {
    gtk_main_quit();
}


#ifdef SHOES_GTK_WIN32
int shoes_win32_cmdvector(const char *cmdline, char ***argv) {
//  return rb_w32_cmdvector(cmdline, argv);
    return 0; // TODO: delete this function.
}

void shoes_native_get_time(SHOES_TIME *ts) {
    *ts = g_get_monotonic_time();  // Should work for GTK3 w/o win32
}

unsigned long shoes_diff_time(SHOES_TIME *start, SHOES_TIME *end) {
    return *end - *start;
}
#else
void shoes_native_get_time(SHOES_TIME *ts) {
#ifdef SHOES_GTK_OSX
    gettimeofday(ts, NULL);
#else
    clock_gettime(CLOCK_REALTIME, ts);
#endif
}

unsigned long shoes_diff_time(SHOES_TIME *start, SHOES_TIME *end) {
    unsigned long usec;
    if ((end->tv_nsec-start->tv_nsec)<0) {
        usec = (end->tv_sec-start->tv_sec - 1) * 1000;
        usec += (1000000000 + end->tv_nsec - start->tv_nsec) / 1000000;
    } else {
        usec = (end->tv_sec - start->tv_sec) * 1000;
        usec += (end->tv_nsec - start->tv_nsec) / 1000000;
    }
    return usec;
}
#endif

typedef struct {
    unsigned int name;
    VALUE obj;
    void *data;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int ret;
} shoes_gtk_msg;


static gboolean shoes_gtk_catch_message(gpointer user) {
    shoes_gtk_msg *msg = (shoes_gtk_msg *)user;
    pthread_mutex_lock(&msg->mutex);
    msg->ret = shoes_catch_message(msg->name, msg->obj, msg->data);
    pthread_cond_signal(&msg->cond);
    pthread_mutex_unlock(&msg->mutex);
    return FALSE;
}


int shoes_native_throw_message(unsigned int name, VALUE obj, void *data) {
    int ret;
    shoes_gtk_msg *msg = SHOE_ALLOC(shoes_gtk_msg);
    msg->name = name;
    msg->obj = obj;
    msg->data = data;
    pthread_mutex_init(&msg->mutex, NULL);
    pthread_cond_init(&msg->cond, NULL);
    msg->ret = 0;

    pthread_mutex_lock(&msg->mutex);
    g_idle_add_full(G_PRIORITY_DEFAULT, shoes_gtk_catch_message, msg, NULL);
    pthread_cond_wait(&msg->cond, &msg->mutex);
    ret = msg->ret;
    pthread_mutex_unlock(&msg->mutex);

    free(msg);
    return ret;
}

void shoes_native_slot_mark(SHOES_SLOT_OS *slot) {}
void shoes_native_slot_reset(SHOES_SLOT_OS *slot) {}
void shoes_native_slot_clear(shoes_canvas *canvas) {
    if (canvas->slot->vscroll) {
        GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
        gtk_adjustment_set_value(adj, gtk_adjustment_get_lower(adj));
    }
}

void shoes_native_slot_paint(SHOES_SLOT_OS *slot) {
    gtk_widget_queue_draw(slot->oscanvas);
}

void shoes_native_slot_lengthen(SHOES_SLOT_OS *slot, int height, int endy) {
    if (slot->vscroll) {
        GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(slot->vscroll));
        if (gtk_adjustment_get_upper(adj) != (gdouble)endy) {
            gtk_range_set_range(GTK_RANGE(slot->vscroll), 0., (gdouble)endy);

            if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
                gtk_widget_hide(slot->vscroll);
            else
                gtk_widget_show(slot->vscroll);
        }
    }
}

void shoes_native_slot_scroll_top(SHOES_SLOT_OS *slot) {
    if (slot->vscroll)
        gtk_range_set_value(GTK_RANGE(slot->vscroll), slot->scrolly);
}

int shoes_native_slot_gutter(SHOES_SLOT_OS *slot) {
    if (slot->vscroll) {
        GtkRequisition rnat;
        gtk_widget_get_preferred_size(slot->vscroll, NULL, &rnat);
        return rnat.width;
    }
    return 0;
}

void shoes_native_remove_item(SHOES_SLOT_OS *slot, VALUE item, char c) {
}

//
// Window-level events
//
static gboolean shoes_app_gtk_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    GdkModifierType state;
    shoes_app *app = (shoes_app *)data;

    if (!event->is_hint) {
        shoes_canvas *canvas;
        TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
        state = (GdkModifierType)event->state;
        int mods = 0;
        if (event->state & GDK_SHIFT_MASK)
          mods = mods | SHOES_MODIFY_SHIFT;
        if (event->state & GDK_CONTROL_MASK)
          mods = mods | SHOES_MODIFY_CTRL;
        int new_x, new_y;
        gtk_widget_translate_coordinates(widget, app->slot->oscanvas, event->x, event->y,
            &new_x, &new_y);
       if (app->have_menu) {
          shoes_app_motion(app, new_x, new_y + canvas->slot->scrolly, mods);
        } else {
          // TODO: Do not Hardcode offsets. 
          if (shoes_gtk_backend & WAYLAND) {
            new_y = max(0,new_y - 60);
            new_x = max(0, new_x - 29);
            //printf("mv: x: %d -> %d y: %d -> %d\n",(int)event->x, new_x, (int)event->y, new_y);
          }
          shoes_app_motion(app, new_x, new_y + canvas->slot->scrolly, mods);
        }
    }
    return TRUE;
}

#ifdef NEWCLICK
static gboolean shoes_app_gtk_button(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    shoes_app *app = (shoes_app *)data;
    shoes_canvas *canvas;
    TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
    
    int x, y;
    GdkModifierType state;
   
    gdk_window_get_device_position(gtk_widget_get_window(widget), event->device, &x, &y, &state);
    
    if (event->type == GDK_BUTTON_PRESS) {
        shoes_app_click(app, event->button, x, y);
    } else if (event->type == GDK_BUTTON_RELEASE) {
        shoes_app_release(app, event->button, x, y);
    }
    return TRUE;
}
#else
static gboolean shoes_app_gtk_button(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    shoes_app *app = (shoes_app *)data;
    shoes_canvas *canvas;
    TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
    // process modifiers
    int mods = 0;
    if (event->state & GDK_SHIFT_MASK)
      mods = mods | SHOES_MODIFY_SHIFT;
    if (event->state & GDK_CONTROL_MASK)
      mods = mods | SHOES_MODIFY_CTRL; 
/*  never get these on Linux or its a themeable thing
    if (event->state & GDK_MOD1_MASK)  
      mods = mods | SHOES_MODIFY_ALT;
    if (event->state & GDK_SUPER_MASK)
      fprintf(stderr, "super\n");
    if (event->state & GDK_HYPER_MASK)
      fprintf(stderr, "hyper\n");   
    if (event->state & GDK_META_MASK)
      fprintf(stderr, "meta\n");   
*/
 		int new_x, new_y;
		gtk_widget_translate_coordinates(widget, app->slot->oscanvas, event->x, event->y,
			&new_x, &new_y);
    if (event->type == GDK_BUTTON_PRESS) {
      if (app->have_menu) {
        shoes_app_click(app, event->button, new_x, new_y + canvas->slot->scrolly, mods);
      } else {
        // TODO: Do not Hardcode offsets. Windows? Different Theme?
        //if (gtk_get_minor_version() >= 24 && strcmp(current_desktop_session, "wayland") == 0) { // 3.24.x
        if (shoes_gtk_backend & WAYLAND) { 
          new_y = max(0,new_y - 60);
          new_x = max(0, new_x - 29);
          //printf("btn: x: %d -> %d y: %d -> %d\n",(int)event->x, new_x, (int)event->y, new_y);
        }
        shoes_app_click(app, event->button, new_x, new_y + canvas->slot->scrolly, mods);
        //shoes_app_click(app, event->button, event->x, event->y + canvas->slot->scrolly, mods);
      }
    } else if (event->type == GDK_BUTTON_RELEASE) {
      if (app->have_menu) {
        shoes_app_release(app, event->button, new_x, new_y + canvas->slot->scrolly, mods);
      } else
        shoes_app_release(app, event->button, new_x, new_y + canvas->slot->scrolly, mods);
        //shoes_app_release(app, event->button, event->x, event->y + canvas->slot->scrolly, mods);
    }
    return TRUE;
}
#endif

static gboolean shoes_app_gtk_wheel(GtkWidget *widget, GdkEventScroll *event, gpointer data) {
    ID wheel;
    shoes_app *app = (shoes_app *)data;

    switch (event->direction) {
        case GDK_SCROLL_UP:
            wheel = s_up;
            break;
        case GDK_SCROLL_DOWN:
            wheel = s_down;
            break;
        case GDK_SCROLL_LEFT:
            wheel = s_left;
            break;
        case GDK_SCROLL_RIGHT:
            wheel = s_right;
            break;
        default:
            return TRUE;
    }
    // process modifiers
    int mods = 0;
    if (event->state & GDK_SHIFT_MASK)
      mods = mods | SHOES_MODIFY_SHIFT;
    if (event->state & GDK_CONTROL_MASK)
      mods = mods | SHOES_MODIFY_CTRL; 
 		int new_x, new_y;
		gtk_widget_translate_coordinates(widget, app->slot->oscanvas, event->x, event->y,
			&new_x, &new_y);
    if (app->have_menu) 
      shoes_app_wheel(app, wheel, event->x, event->y - app->mb_height, mods);
    else {
      if (shoes_gtk_backend & WAYLAND) {
        new_y = max(0,new_y - 60);
        new_x = max(0, new_x - 29);
        //printf("whl: x: %d -> %d y: %d -> %d\n",(int)event->x, new_x, (int)event->y, new_y);
      }
      shoes_app_wheel(app, wheel, event->x, event->y, mods);
    }
    return TRUE;
}

#if 0
// TODO: unused? called when window was maximized (window adornment button)
// considered to be a hack.
static void shoes_gtk_resized_max(shoes_app *app, int width, int height) {
    shoes_canvas *canvas;
    TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
#ifdef SZBUG
    fprintf(stderr,"shoes_gtk_resized_max: %d %d\n", width, height);
#endif
    if (canvas->slot->vscroll &&
            (height != canvas->slot->scrollh || width != canvas->slot->scrollw)) {
        GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
        //gtk_widget_set_size_request(canvas->slot->vscroll, -1, height);
        gtk_widget_set_size_request(canvas->slot->vscroll, -1, height);

        //gtk_widget_set_size_request(GTK_CONTAINER(widget), canvas->app->width, size->height);
        GtkAllocation alloc;
        gtk_widget_get_allocation((GtkWidget *)canvas->slot->vscroll, &alloc);  
#ifdef SZBUG
       fprintf(stderr, "alloc: %d %d %d %d\n\n", alloc.x, alloc.y, alloc.width, alloc.height);
#endif
        gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas), canvas->slot->vscroll,
                    (width - alloc.width)-100, 0);
        gtk_adjustment_set_page_size(adj, height);
        gtk_adjustment_set_page_increment(adj, height - 32);

        if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
            gtk_widget_hide(canvas->slot->vscroll);
        else {
            gtk_widget_show(canvas->slot->vscroll);
        }
        canvas->slot->scrollh = height;
        canvas->slot->scrollw = width;
    }
}
#endif

// called only by **Window** signal handler for "size-allocate"
static void shoes_app_gtk_size(GtkWidget *widget, cairo_t *cr, gpointer data) {
    shoes_app *app = (shoes_app *)data;
    gtk_window_get_size(GTK_WINDOW(app->os.window), &app->width, &app->height);
#ifdef SZBUG
    fprintf(stderr,"shoes_app_gtk_size: wid: %d hgt: %d\n", app->width, app->height);
#if 0 // only exists in gtk 3.12 and above which aint' windows
    if (gtk_window_is_maximized(widget)) {
      // because of maimize button and asynch behaviour app->w/h are incorrect here.
      fprintf(stderr,"resized max %d, %d\n", app->width, app->height);
    }
#endif
#endif
    // This calls shoes_native_canvas_resize - a no-opp in gtk
    shoes_canvas_size(app->canvas, app->width, app->height);
}

#define KEY_SYM(name, sym) \
  else if (event->keyval == GDK_KEY_##name) \
    v = ID2SYM(rb_intern("" # sym))

static gboolean shoes_app_gtk_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    VALUE v = Qnil;
    guint modifiers = event->state;
    shoes_app *app = (shoes_app *)data;
    if (event->keyval == GDK_KEY_Return) {
        v = rb_str_new2("\n");
    }
    KEY_SYM(BackSpace, backspace);	// GTK3 <bs> has length of 1. Go figure.
    KEY_SYM(Escape, escape);
    else if (event->length > 0) {
        if ((event->state & GDK_CONTROL_MASK) || (event->state & GDK_MOD1_MASK)) {
            gint len;
            gunichar ch;
            char chbuf[7] = {0};

            ch = gdk_keyval_to_unicode(event->keyval);
            len = g_unichar_to_utf8(ch, chbuf);
            chbuf[len] = '\0';

            v = ID2SYM(rb_intern(chbuf));
            if (modifiers & GDK_SHIFT_MASK)
              modifiers ^= GDK_SHIFT_MASK;
        } else {
            if (event->string[0] == '\r' && event->length == 1)
                v = rb_str_new2("\n");
            else
                v = rb_str_new(event->string, event->length);
        }
    }
    KEY_SYM(Insert, insert);
    KEY_SYM(Delete, delete);
    KEY_SYM(Tab, tab);
    KEY_SYM(ISO_Left_Tab, tab);
    KEY_SYM(Page_Up, page_up);
    KEY_SYM(Page_Down, page_down);
    KEY_SYM(Home, home);
    KEY_SYM(End, end);
    KEY_SYM(Left, left);
    KEY_SYM(Up, up);
    KEY_SYM(Right, right);
    KEY_SYM(Down, down);
    KEY_SYM(F1, f1);
    KEY_SYM(F2, f2);
    KEY_SYM(F3, f3);
    KEY_SYM(F4, f4);
    KEY_SYM(F5, f5);
    KEY_SYM(F6, f6);
    KEY_SYM(F7, f7);
    KEY_SYM(F8, f8);
    KEY_SYM(F9, f9);
    KEY_SYM(F10, f10);
    KEY_SYM(F11, f11);
    KEY_SYM(F12, f12);

    if (v != Qnil) {
        if (event->type == GDK_KEY_PRESS) {
            shoes_app_keydown(app, v);

            if (event->keyval == GDK_KEY_Return)
                if ((event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0)
                    v = ID2SYM(rb_intern("enter"));
            if (SYMBOL_P(v)) {
                if (modifiers & GDK_MOD1_MASK)
                    KEY_STATE(alt);
                if (modifiers & GDK_SHIFT_MASK)
                    KEY_STATE(shift);
                if (modifiers & GDK_CONTROL_MASK)
                    KEY_STATE(control);
            }

            shoes_app_keypress(app, v);
        } else {
            shoes_app_keyup(app, v);
        }
    }

    return FALSE;
}

// data arg is a VALUE - an App or a Canvas
static gboolean shoes_canvas_gtk_touch(GtkWidget *widget, GdkEventTouch *event, gpointer data) {
    shoes_app *app; 
    shoes_canvas *canvas;
    if (data == NULL || (VALUE)data == Qnil) {
      fprintf(stderr," Touch: NIL for app/canavas\n");
      return FALSE;
    }
    if (rb_obj_is_kind_of((VALUE)data, cApp)) {
      TypedData_Get_Struct((VALUE)data, shoes_app, &shoes_app_type, app);
      TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
    } else if (rb_obj_is_kind_of((VALUE)data, cCanvas)) {
      TypedData_Get_Struct((VALUE)data, shoes_canvas, &shoes_canvas_type, canvas);
    } else {
      return FALSE;
    }
    // Is the scrollbar showing ? 
    shoes_slot_gtk *slot = canvas->slot;
    GtkWidget *sb = slot->vscroll;
    if (sb == NULL) {
      fprintf(stderr, "Touch: No scrollbar\n");
      return FALSE;
    }
    if (event->type == GDK_TOUCH_BEGIN) {
      //fprintf(stderr, "Touch Begin:\n");
      app->touch_x = event->x;
      app->touch_y = event->y;
    } else if (event->type == GDK_TOUCH_END) {
      app->touch_x = 0.0;
      app->touch_y = 0.0;
      //fprintf(stderr, "Touch End:\n");
    } else if (event->type == GDK_TOUCH_UPDATE) {
      GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(sb));
      gdouble adjv = gtk_adjustment_get_value(adj);
      //gdouble dy = (int)event->y - app->touch_y;
      gdouble dy = (gdouble)app->touch_y - event->y;
      gdouble newp = adjv + dy;
      if (dy)
        gtk_adjustment_set_value(adj, newp);
      //fprintf(stderr, "Touch_Update: %f: %f %f\n", adjv, event->y, dy);
    } else if (event->type == GDK_TOUCH_CANCEL) {
      app->touch_x = 0.0;
      app->touch_y = 0.0;
      fprintf(stderr, "Touch Cancel:\n");
    } else {
      fprintf(stderr, "Touch UNKNOWN\n");
    }
    return FALSE; // false => We did not handle the event?
}

static void shoes_gtk_app_drag_begin(GtkGestureDrag *gesture,
    gdouble x, gdouble y, gpointer data) {
    shoes_app *app = (shoes_app *)data;
    fprintf(stderr, "Drag Begin, %d,%d\n", (int)x, (int)y);
 }

static gboolean shoes_app_gtk_quit(GtkWidget *widget, GdkEvent *event, gpointer data) {
    shoes_app *app = (shoes_app *)data;
    if (shoes_app_remove(app))
        gtk_main_quit();
    return FALSE;
}

static void shoes_canvas_gtk_paint_children(GtkWidget *widget, gpointer data) {
    shoes_canvas *canvas = (shoes_canvas *)data;
#ifdef SZBUG
	fprintf(stderr, "Propagate draw widget: %lx\n", (unsigned long)widget);
#endif
    gtk_container_propagate_draw(GTK_CONTAINER(canvas->slot->oscanvas), widget,
                                 canvas->slot->drawevent);
}

// called from Gtk signal handlers for 'slots'
static void shoes_canvas_gtk_paint(GtkWidget *widget, cairo_t *cr, gpointer data) {
    VALUE c = (VALUE)data;
    shoes_canvas *canvas;
    TypedData_Get_Struct(c, shoes_canvas, &shoes_canvas_type, canvas);
    canvas->slot->drawevent = cr;		// stash it for the children

    // getting widget dirty area, already clipped
    cairo_rectangle_int_t rect;
    gdk_cairo_get_clip_rectangle(cr, &rect);
#ifdef SZBUG
	fprintf(stderr, "shoes_canvas_gtk_paint triggered: %lx %lx %d %d %d %d\n", 
			(unsigned long)canvas->slot->oscanvas,
			(unsigned long)cr,
			rect.x, rect.y, rect.width, rect.height);
#endif

    shoes_canvas_paint(c);
    // Gtk3 doc says gtk_container_foreach is preferable over gtk_container_forall
    gtk_container_foreach(GTK_CONTAINER(widget), shoes_canvas_gtk_paint_children, canvas);

    canvas->slot->drawevent = NULL;
}

static void shoes_canvas_gtk_size(GtkWidget *widget, GtkAllocation *size, gpointer data) {
    VALUE c = (VALUE)data;
    shoes_canvas *canvas;
    TypedData_Get_Struct(c, shoes_canvas, &shoes_canvas_type, canvas);
#ifdef SZBUG
    fprintf(stderr,"shoes_canvas_gtk_size: %d %d %d %d\n", size->x, size->y, size->width, size->height);
#endif
    if (canvas->slot->vscroll &&
            (size->height != canvas->slot->scrollh || size->width != canvas->slot->scrollw)) {
        GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
        gtk_widget_set_size_request(canvas->slot->vscroll, -1, size->height);
        
        //gtk_widget_set_size_request(GTK_CONTAINER(widget), canvas->app->width, size->height);
        GtkAllocation alloc;
        gtk_widget_get_allocation((GtkWidget *)canvas->slot->vscroll, &alloc);  
#ifdef SZBUG
        fprintf(stderr, "alloc: %d %d %d %d\n\n", alloc.x, alloc.y, alloc.width, alloc.height);
#endif
        if (canvas->slot->oscanvas != widget) 
          fprintf(stderr,"shoes_canvas_gtk_size: ooops\n");
          
        gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas), canvas->slot->vscroll,
                       size->width - alloc.width, 0);
        gtk_adjustment_set_page_size(adj, size->height);
        gtk_adjustment_set_page_increment(adj, size->height - 32);

        if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
            gtk_widget_hide(canvas->slot->vscroll);
        else {
            gtk_widget_show(canvas->slot->vscroll);
        }
        canvas->slot->scrollh = size->height;
        canvas->slot->scrollw = size->width;
    }
}

static void shoes_canvas_gtk_scroll(GtkRange *r, gpointer data) {
    VALUE c = (VALUE)data;
    shoes_canvas *canvas;
    TypedData_Get_Struct(c, shoes_canvas, &shoes_canvas_type, canvas);
    canvas->slot->scrolly = (int)gtk_range_get_value(r);
    shoes_slot_repaint(canvas->app->slot);
}

/* ----------- mainloop handling -------------- */

/*
 * Some notes about select and the gtk mainloop
 * select() blocks the calling thread (Gtk) until the fds are ready 
 * OR the timeout expires. In Shoes, there maybe other ruby threads running
 * because of user requests (download is threaded). Thise threads won't block
 * HERE but there work could unblock one of the fds that select() is waiting
 * on - that would wake the GUI up (as does the timeout)
*/
#ifdef SGPOLL
#ifndef SHOES_GTK_WIN32
static gint shoes_app_g_poll(GPollFD *fds, guint nfds, gint timeout) {
    struct timeval tv;

    rb_fdset_t rset, wset, xset;
    GPollFD *f;
    int ready;
    int maxfd = 0;

    rb_fd_init(&rset); // was FD_ZERO()
    rb_fd_init(&wset);
    rb_fd_init(&xset);

    for (f = fds; f < &fds[nfds]; ++f)
        if (f->fd >= 0) {
            if (f->events & G_IO_IN)
                //FD_SET (f->fd, &rset);
                rb_fd_set(f->fd, &rset);
            if (f->events & G_IO_OUT)
                //FD_SET (f->fd, &wset);
                rb_fd_set(f->fd, &wset);
            if (f->events & G_IO_PRI)
                //FD_SET (f->fd, &xset);
                rb_fd_set(f->fd, &xset);
            if (f->fd > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
                maxfd = f->fd;
        }

    //
    // If we poll indefinitely, then the window updates will
    // pile up for as long as Ruby is churning away.
    //
    // Give Ruby half-seconds in which to work, in order to
    // keep it from completely blocking the GUI.

    if (timeout == -1 || timeout > 500)
        timeout = 500;

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    ready = rb_thread_fd_select (maxfd + 1, &rset, &wset, &xset, &tv);
    if (ready > 0)
        for (f = fds; f < &fds[nfds]; ++f) {
            f->revents = 0;
            if (f->fd >= 0) {
                //if (FD_ISSET (f->fd, &rset))
                if (rb_fd_isset (f->fd, &rset))
                    f->revents |= G_IO_IN;
                //if (FD_ISSET (f->fd, &wset))
                if (rb_fd_isset (f->fd, &wset))
                    f->revents |= G_IO_OUT;
                //if (FD_ISSET (f->fd, &xset))
                if (rb_fd_isset (f->fd, &xset))
                    f->revents |= G_IO_PRI;
            }
        }
    // Free the allocated storage from rb_fd_init
    rb_fd_term(&rset);
    rb_fd_term(&wset);
    rb_fd_term(&xset);
    return ready;
}
#else
/*
 * Windows: Gtk and Ruby could differ on what an 'fd' is? They could differ
 * in what the bit flags are for events and revents. Since we are polling
 * ruby primarily we need to use it's definitions/macros of C library things
 * 
 * NOTE: as of July 13, 2018 this doesn't crash - also doesn't work well
 * 
*/

static gint shoes_app_g_poll(GPollFD *fds, guint nfds, gint timeout) {
    struct timeval tv;
    fd_set rset, wset, xset; 
    GPollFD *f;
    int ready;
    int maxfd = 0;

    FD_ZERO(&rset); 
    FD_ZERO(&wset);
    FD_ZERO(&xset);

    int i;
    // In Windows/Gtk, GPollFD->fd could be a handle (Gtk/Gio/glib), or a small int (ruby)
    // Watch out for maxfd - it's wrong, according to select() doc.
    for (i = 0; i < nfds; i++) {
        f = &fds[i];
        if (f->fd >= 0) {
            if (f->events & G_IO_IN)
                FD_SET(f->fd, &rset);
            if (f->events & G_IO_OUT)
                FD_SET(f->fd, &wset);
            if (f->events & G_IO_PRI)
                FD_SET(f->fd, &xset);
            if (f->fd > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
                maxfd = max (maxfd, i+1);
        }
    }
    //
    // If we poll indefinitely, then the window updates will
    // pile up for as long as Ruby is churning away.
    //
    // Give Ruby half-seconds in which to work, in order to
    // keep it from completely blocking the GUI. 
    //
    // WARNING timeout can be (is) 0 on Windows and Linux
    if (timeout == -1 || timeout > 500)
        timeout = 500;

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    //ready = rb_fd_select (maxfd + 1, &rset, &wset, &xset, &tv); // crash
    ready = select (maxfd + 1, &rset, &wset, &xset, &tv); 
    if (ready < 0) {
      // fails a  lot on Windows/Gtk
      // fprintf(stderr, "loop fail\n");
    }
    if (ready > 0) {
        for (f = fds; f < &fds[nfds]; ++f) {
            f->revents = 0;
            if (f->fd >= 0) {
                if (FD_ISSET (f->fd, &rset))
                    f->revents |= G_IO_IN;
                if (FD_ISSET (f->fd, &wset))
                    f->revents |= G_IO_OUT;
                if (FD_ISSET (f->fd, &xset))
                    f->revents |= G_IO_PRI;
            }
        }
    }

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&xset);

    return ready;
}

#endif
#endif

int win_current_tmo = 10;  // only used on Windows until bug is fixed

#ifdef SGTMO
/*
static GSource *gtkrb_source;
static GSource *gtkrb_init_source();
static GSourceFuncs gtkrb_func_tbl;

static GSource *gtkrb_init_source()
{
  // fill in the struct
  gtkrb_source = g_source_new(&gtkrb_func_tbl, (guint) sizeof(gtkrb_func_tbl));
}
*/
static int win_default_tmo = 10;

static gboolean gtkrb_idle(gpointer opt) { // opt points to win_current_tmo
    int t = *((int *)opt);
    if (t != win_default_tmo) {
      // change requested - requires new timer
      fprintf(stderr, "new tmo: %d\n", t);
      win_default_tmo = t;
      g_timeout_add(win_default_tmo, gtkrb_idle, &win_current_tmo);
      rb_thread_schedule();
      return 0; // cancel the current(old) timer
    }
    rb_thread_schedule();
    return 1; // keep current timer
}
#endif

void shoes_native_loop() {
#ifdef SGPOLL
    g_main_context_set_poll_func(g_main_context_default(), shoes_app_g_poll);
#else
    /* Win32 (should work for Linux too when finished)
     * Build: struct GSourceFuncs
     * Call: g_source_new() with that struct
     * Call: g_source_attach() with that
     */
#ifdef GIDLE
    //gtkrb_source = gtkrb_init_source();
    //g_source_attach(gtkrb_source, (gpointer) NULL);
    fprintf(stderr,"Using idle loop\n");
    g_idle_add(gtkrb_idle, NULL);
#else
    //fprintf(stderr,"Using timeout loop\n");
    g_timeout_add(win_default_tmo, gtkrb_idle, &win_current_tmo);
#endif
#endif
    GLOBAL_APP(app);
    if (APP_WINDOW(app)) gtk_main();
}

shoes_code shoes_native_app_cursor(shoes_app *app, ID cursor) {
    if (app->os.window == NULL || gtk_widget_get_window(app->os.window)== NULL || app->cursor == cursor)
        goto done;
    GdkCursor *c;
    if (gtk_get_minor_version() >= 16) {
      // TODO: muliple-moniter support needs? 
      GdkDisplay *dsp = gdk_display_get_default();
      if (cursor == s_hand_cursor || cursor == s_link) {
          c = gdk_cursor_new_for_display(dsp, GDK_HAND2);
      } else if (cursor == s_arrow_cursor) {
          c = gdk_cursor_new_for_display(dsp, GDK_ARROW);
      } else if (cursor == s_text_cursor) {
          c = gdk_cursor_new_for_display(dsp, GDK_XTERM);
      } else if (cursor == s_watch_cursor) {
          c = gdk_cursor_new_for_display(dsp, GDK_WATCH);
      } else
          goto done;
    } else {
      // Windows gtk may be older than 3.16
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
      if (cursor == s_hand_cursor || cursor == s_link) {
          c = gdk_cursor_new(GDK_HAND2);
      } else if (cursor == s_arrow_cursor) {
          c = gdk_cursor_new(GDK_ARROW);
      } else if (cursor == s_text_cursor) {
          c = gdk_cursor_new(GDK_XTERM);
      } else if (cursor == s_watch_cursor) {
          c = gdk_cursor_new(GDK_WATCH);
      } else
          goto done;
#pragma GCC diagnostic pop
    }
    gdk_window_set_cursor(gtk_widget_get_window(app->os.window), c);
    app->cursor = cursor;

done:
    return SHOES_OK;
}

void shoes_native_app_title(shoes_app *app, char *msg) {
    gtk_window_set_title(GTK_WINDOW(app->os.window), _(msg));
    //gtk_window_set_title(GTK_WINDOW(app->os.window), msg);
}

void shoes_native_app_resize_window(shoes_app *app) {
    if ((app->os.window != NULL) && (app->width > 0 && app->height > 0)) {
        gtk_widget_set_size_request((GtkWidget *) app->os.window, app->width, app->height);
    }
}

VALUE shoes_native_get_resizable(shoes_app *app) {
    return gtk_window_get_resizable(GTK_WINDOW(app->os.window));
}

void shoes_native_set_resizable(shoes_app *app, int resizable) {
    gboolean state;
    state = resizable ? TRUE : FALSE;
    if (gtk_window_get_resizable(GTK_WINDOW(app->os.window)) != state)
        gtk_window_set_resizable(GTK_WINDOW(app->os.window), state);
}


void shoes_native_app_fullscreen(shoes_app *app, char yn) {
    gtk_window_set_keep_above(GTK_WINDOW(app->os.window), (gboolean)yn);
    if (yn)
        gtk_window_fullscreen(GTK_WINDOW(app->os.window));
    else
        gtk_window_unfullscreen(GTK_WINDOW(app->os.window));
}

// new in 3.2.19
void shoes_native_app_set_icon(shoes_app *app, char *icon_path) {
    // replace default icon
    gboolean err;
    GtkWindow *win;
    if (app->have_menu)
      win = (GtkWindow *)app->os.window;
    else
      win = (GtkWindow *) app->slot->oscanvas;  // TODO: needed? 
    err = gtk_window_set_icon_from_file(win, icon_path, NULL);
    err = gtk_window_set_default_icon_from_file(icon_path, NULL);
}

// new in 3.2.19. TODO 3.3.7 update for use_menu app->os.window
void shoes_native_app_set_wtitle(shoes_app *app, char *wtitle) {
    //gtk_window_set_title(GTK_WINDOW(app->slot->oscanvas), _(wtitle));
    GtkWindow *win;
    if (app->have_menu)
      win = (GtkWindow *)app->os.window;
    else
      win = (GtkWindow *) app->slot->oscanvas;  // TODO: needed? 
    gtk_window_set_title(GTK_WINDOW(win), _(wtitle));
}

// new in 3.3.3 - opacity  can use the older api on old Gtk3 
void shoes_native_app_set_opacity(shoes_app *app, double opacity) {
  if (gtk_get_minor_version() >= 8)
    gtk_widget_set_opacity(GTK_WIDGET(app->os.window), opacity);
  else 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gtk_window_set_opacity(GTK_WINDOW(app->os.window), opacity);
#pragma GCC diagnostic pop
}

double shoes_native_app_get_opacity(shoes_app *app) {
 if (gtk_get_minor_version() >= 8)
    return gtk_widget_get_opacity(GTK_WIDGET(app->os.window));
  else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    return gtk_window_get_opacity(GTK_WINDOW(app->os.window));
#pragma GCC diagnostic pop
}

void shoes_native_app_set_decoration(shoes_app *app, gboolean decorated) {
    gtk_window_set_decorated(GTK_WINDOW(app->os.window), decorated);
}

int shoes_native_app_get_decoration(shoes_app *app) {
    return gtk_window_get_decorated(GTK_WINDOW(app->os.window)) == TRUE;
}

// new in 3.3.4
void shoes_native_app_get_window_position(shoes_app *app) {
    gtk_window_get_position(GTK_WINDOW(app->os.window), &app->x, &app->y);
}

void shoes_native_app_window_move(shoes_app *app, int x, int y) {
    gtk_window_move(GTK_WINDOW(app->os.window), app->x = x, app->y = y);
}

/*
 *  All apps windows can have a menubur and then the old shoes space below. 
 *  it's optional and the default is no menu for backwards compatibilty
 *  That causes some pixel tweaking and redundency. 
 */
 
/* TODO fixes bug #349 
 * seems like overkill or incomplete 
*/
#if !GTK_CHECK_VERSION(3,12,0)
// forward declares 
int shoes_gtk_is_maximized(shoes_app *app, int width, int height); 
void shoes_gtk_set_max(shoes_app *app);
#endif

gboolean shoes_app_gtk_configure_event(GtkWidget *widget, GdkEvent *evt, gpointer data) {
  shoes_app *app = (shoes_app *)data;
  if (widget == app->os.window) {  // GtkWindow
    //gtk_widget_set_size_request(widget, evt->configure.width, evt->configure.height);
    shoes_canvas *canvas;
    TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
#if GTK_CHECK_VERSION(3,12,0)
    if (gtk_window_is_maximized((GtkWindow *)widget)) {
#else
    if (shoes_gtk_is_maximized(app, evt->configure.width, evt->configure.height)) {
#endif
    //if (canvas->slot->vscroll && evt->configure.height != app->height) { 
          GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
          gtk_widget_set_size_request(canvas->slot->vscroll, -1, evt->configure.height);
          
          GtkAllocation alloc;
          gtk_widget_get_allocation((GtkWidget *)canvas->slot->vscroll, &alloc);  
#ifdef SZBUG
          fprintf(stderr, "shoes_app_gtk_configure_menu %d, %d, %d, %d\n", 
              evt->configure.x, evt->configure.y, evt->configure.width, evt->configure.height);
          fprintf(stderr, "cfg alloc: %d %d %d %d\n\n", alloc.x, alloc.y, alloc.width, alloc.height);
#endif
          gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas), canvas->slot->vscroll,
                         evt->configure.width - alloc.width, 0);
          gtk_adjustment_set_page_size(adj, evt->configure.height);
          gtk_adjustment_set_page_increment(adj, evt->configure.height - 32);
  
          if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
              gtk_widget_hide(canvas->slot->vscroll);
          else {
              gtk_widget_show(canvas->slot->vscroll);
          }
      }
  }
  app->width = evt->configure.width;
  app->height = evt->configure.height;
  return FALSE;
}
 
shoes_code shoes_native_app_open(shoes_app *app, char *path, int dialog, shoes_settings *st) {
#if 0 //!defined(SHOES_GTK_WIN32)
    char icon_path[SHOES_BUFSIZE];
    sprintf(icon_path, "%s/static/app-icon.png", shoes_world->path);
    gtk_window_set_default_icon_from_file(icon_path, NULL);
#endif
    GtkWidget *window;       // Root window 
    shoes_app_gtk *gk = &app->os; //lexical - typing shortcut

   char icon_path[SHOES_BUFSIZE];
    if (st->icon_path == Qnil) {
      sprintf(icon_path, "%s/static/app-icon.png", shoes_world->path);
    } else {
      char *ip = RSTRING_PTR(st->icon_path);
      if (*ip == '/' || ip[1] ==':')  
        strcpy(icon_path, ip);
      else
       sprintf(icon_path, "%s/%s", shoes_world->path, ip);
    }
    gtk_window_set_default_icon_from_file(icon_path, NULL);
    
    // The Good old way, menu-less
#if 0 //#ifdef ENABLE_MDI
    // don't do this if gtk_init() is used.
    window = gtk_application_window_new(shoes_GtkApp);
#else
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif
    gk->window = window;
    app->slot->oscanvas = window;
    // theme the window
    if (shoes_css_provider != NULL) {
      gtk_style_context_add_provider(gtk_widget_get_style_context(window),
          GTK_STYLE_PROVIDER(shoes_css_provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
        
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    if (!app->resizable) {
        gtk_widget_set_size_request(window, app->width, app->height);
        gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    } else if (app->minwidth < app->width || app->minheight < app->height) {
        GdkGeometry hints;
        hints.min_width = app->minwidth;
        hints.min_height = app->minheight;
#ifdef SZBUG
        fprintf(stderr,"resize hints: %d, %d\n", hints.min_width, hints.min_height);
#endif
        gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL,
                                      &hints, GDK_HINT_MIN_SIZE);
    }
    gtk_window_set_default_size(GTK_WINDOW(window), app->width, app->height);

    gtk_window_get_position(GTK_WINDOW(window), &app->x, &app->y);
    g_signal_connect(G_OBJECT(window), "size-allocate",
                     G_CALLBACK(shoes_app_gtk_size), app);
    g_signal_connect(G_OBJECT(window), "motion-notify-event",
                     G_CALLBACK(shoes_app_gtk_motion), app);
    g_signal_connect(G_OBJECT(window), "button-press-event",
                     G_CALLBACK(shoes_app_gtk_button), app);
    g_signal_connect(G_OBJECT(window), "button-release-event",
                     G_CALLBACK(shoes_app_gtk_button), app);
    g_signal_connect(G_OBJECT(window), "scroll-event",
                     G_CALLBACK(shoes_app_gtk_wheel), app);
    g_signal_connect(G_OBJECT(window), "key-press-event",
                     G_CALLBACK(shoes_app_gtk_keypress), app);
    g_signal_connect(G_OBJECT(window), "key-release-event",
                     G_CALLBACK(shoes_app_gtk_keypress), app);
    g_signal_connect(G_OBJECT(window), "delete-event",
                       G_CALLBACK(shoes_app_gtk_quit), app);
    g_signal_connect(G_OBJECT(window), "configure-event",   // bug #349
                     G_CALLBACK(shoes_app_gtk_configure_event), app);
#if 1   
    g_signal_connect(G_OBJECT(window), "touch-event",
                     G_CALLBACK(shoes_canvas_gtk_touch), (gpointer)app->self);
#endif
#if 0
    g_signal_connect(GTK_WIDGET(window), "drag-begin",
                    G_CALLBACK(shoes_gtk_app_drag_begin), app);
#endif                     
    if (app->fullscreen) shoes_native_app_fullscreen(app, 1);

    gtk_window_set_decorated(GTK_WINDOW(window), app->decorated);
#if GTK_CHECK_VERSION(3,8,0)
    gtk_widget_set_opacity(GTK_WIDGET(window), app->opacity);
#endif
    // ORIG: gtk_widget_set_events(window, GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    gtk_widget_add_events(window, GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK | 
        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_TOUCH_MASK);


    // If the user asked for a specific Screen (Monitor) for the Window
    if (app->monitor >= 0) {
      shoes_native_monitor_set(app);
    }
#if ! GTK_CHECK_VERSION(3,12,0)
    shoes_gtk_set_max(app);
#endif
    return SHOES_OK;
}

void shoes_native_app_show(shoes_app *app) {
    gtk_widget_show_all(app->os.window);
}


void shoes_native_app_close(shoes_app *app) {
    shoes_app_gtk_quit(app->os.window, NULL, (gpointer)app);
    gtk_widget_destroy(app->os.window);
    app->os.window = NULL;
}

// Below function doesn't work - /etc/alternatives doesn't exist.
// TODO: Appears to not be used at Shoe/ruby level
void shoes_native_browser_open(char *url) {
    VALUE browser = rb_str_new2("/etc/alternatives/x-www-browser '");
    rb_str_cat2(browser, url);
    rb_str_cat2(browser, "' 2>/dev/null &");
    shoes_sys(RSTRING_PTR(browser), 1);
}

 
void shoes_native_slot_init(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width, int height, int scrolls, int toplevel) {
    shoes_canvas *canvas;
    SHOES_SLOT_OS *slot;
    TypedData_Get_Struct(c, shoes_canvas, &shoes_canvas_type, canvas);

    // TODO: probably not needed. 
    if (canvas->app->have_menu) {
		fprintf(stderr, "normal slot init --> \n");
		shoes_slot_init_menu(c, parent, x, y, width, height, scrolls, toplevel);
		return;
    }
    slot = shoes_slot_alloc(canvas, parent, toplevel);

    /* Subclassing GtkFixed so we can override gtk3 size management which creates
       problems with slot height being always tied to inside widgets cumulative heights
       creating heights overflow with no scrollbar !
    */
    slot->oscanvas = gtkfixed_alt_new(width, height); 
#ifdef SZBUG
    fprintf(stderr,"shoes_native_slot_init topleve: %d, slot->canvas %lx\n", toplevel, (unsigned long)slot->oscanvas);
#endif
    g_signal_connect(G_OBJECT(slot->oscanvas), "draw",
                     G_CALLBACK(shoes_canvas_gtk_paint), (gpointer)c);
    g_signal_connect(G_OBJECT(slot->oscanvas), "size-allocate",
                     G_CALLBACK(shoes_canvas_gtk_size), (gpointer)c);
#if 1   
    g_signal_connect(G_OBJECT(slot->oscanvas), "touch-event",
                     G_CALLBACK(shoes_canvas_gtk_touch), (gpointer)c);
#endif
    INFO("shoes_native_slot_init(%lu)\n", c);

    if (toplevel) {
        gtk_container_add(GTK_CONTAINER(parent->oscanvas), slot->oscanvas);
    } else {
        gtk_fixed_put(GTK_FIXED(parent->oscanvas), slot->oscanvas, x, y);
    }
    slot->scrollh = slot->scrollw = 0;
    slot->vscroll = NULL;
    if (scrolls) {
        slot->vscroll = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,NULL);
        GtkAdjustment *adj;
        adj = gtk_range_get_adjustment(GTK_RANGE(slot->vscroll));
        gtk_adjustment_set_step_increment(adj, 16);
        gtk_adjustment_set_page_increment(adj, height - 32);
        slot->drawevent = NULL;

        g_signal_connect(G_OBJECT(slot->vscroll), "value-changed",
                         G_CALLBACK(shoes_canvas_gtk_scroll), (gpointer)c);
        gtk_fixed_put(GTK_FIXED(slot->oscanvas), slot->vscroll, -100, -100);

        gtk_widget_set_size_request(slot->oscanvas, width, height);
        //gtk_widget_set_size_request(slot->oscanvas, canvas->app->minwidth, canvas->app->minheight);

        if (!toplevel) ATTRSET(canvas->attr, wheel, scrolls);
    }

    if (toplevel)
        shoes_canvas_size(c, width, height);
    else {
        gtk_widget_show_all(slot->oscanvas);
        canvas->width = 100;
        canvas->height = 100;
    }
}

void shoes_native_slot_destroy(shoes_canvas *canvas, shoes_canvas *pc) {
    if (canvas->slot->vscroll)
        gtk_container_remove(GTK_CONTAINER(canvas->slot->oscanvas), canvas->slot->vscroll);
    gtk_container_remove(GTK_CONTAINER(pc->slot->oscanvas), canvas->slot->oscanvas);
}

cairo_t *shoes_native_cairo_create(shoes_canvas *canvas) {
    GdkWindow *win = gtk_widget_get_window(canvas->slot->oscanvas);
    cairo_t *cr = gdk_cairo_create(win);
    if (canvas->slot->drawevent != NULL &&
            gtk_cairo_should_draw_window(canvas->slot->drawevent, win)) {
        cairo_rectangle_int_t alloc;
        gtk_widget_get_allocation((GtkWidget *)canvas->slot->oscanvas, &alloc);
        cairo_region_t *region = cairo_region_create_rectangle(&alloc);
//    cairo_rectangle_int_t w_rect = {canvas->place.ix, canvas->place.iy, canvas->place.w, canvas->place.h};
        cairo_rectangle_int_t w_rect;
        gdk_cairo_get_clip_rectangle(cr, &w_rect);
        cairo_region_intersect_rectangle(region, &w_rect);
        gdk_cairo_region(cr, region);
        cairo_clip(cr);
        cairo_region_destroy(region);

        cairo_translate(cr, alloc.x, alloc.y - canvas->slot->scrolly);
    }
    return cr;
}

void shoes_native_cairo_destroy(shoes_canvas *canvas) {
}

void shoes_native_group_clear(SHOES_GROUP_OS *group) {
    //group->radios = NULL;
    //group->layout = NULL;
}

void shoes_native_canvas_place(shoes_canvas *self_t, shoes_canvas *pc) {
    int x, y, newy;

    GtkAllocation a;
    gtk_widget_get_allocation(self_t->slot->oscanvas, &a);
    gtk_widget_translate_coordinates(self_t->slot->oscanvas, pc->slot->oscanvas, 0, 0, &x, &y);
    newy = (self_t->place.iy + self_t->place.dy) - pc->slot->scrolly;

    if (x != self_t->place.ix + self_t->place.dx || y != newy)
        gtk_fixed_move(GTK_FIXED(pc->slot->oscanvas), self_t->slot->oscanvas,
                       self_t->place.ix + self_t->place.dx, newy);
    if (a.width != self_t->place.iw || a.height != self_t->place.ih)
        gtk_widget_set_size_request(self_t->slot->oscanvas, self_t->place.iw, self_t->place.ih);
}

void shoes_native_canvas_resize(shoes_canvas *canvas) {
#if 0
  if (canvas->app->have_menu) {
    int w = canvas->width;
    int h = canvas->height;
#ifdef SZBUG
    fprintf(stderr,"shoes_native_canvas_resize: %d, %d\n", w, h);
#endif
    // gtk_widget_queue_resize(canvas->app->os.shoes_window);  // nope
    gtk_widget_set_size_request(canvas->slot->oscanvas, w, h); // won't shrink
    GtkAllocation rect;
    rect.width = w;
    rect.height = h;
    rect.x = 0;
    rect.y = 29;
    //gtk_widget_set_allocation(canvas->slot->oscanvas, &rect); // not as good,
  }
#endif
}

/*
 * one shot timer for start{} on slot. Canvas internal use.
*/
static gboolean start_wait(gpointer data) {
    VALUE rbcanvas = (VALUE)data;
    shoes_canvas *canvas;
    TypedData_Get_Struct(rbcanvas, shoes_canvas, &shoes_canvas_type, canvas);

    shoes_safe_block(rbcanvas, ATTR(canvas->attr, start), rb_ary_new3(1, rbcanvas));
    return FALSE; // timeout will be stopped and destroyed
}

void shoes_native_canvas_oneshot(int ms, VALUE canvas) {
    g_timeout_add_full(G_PRIORITY_HIGH, 1, start_wait, (gpointer)canvas, NULL);
}

void shoes_widget_changed(GtkWidget *ref, gpointer data) {
    VALUE self = (VALUE)data;
    shoes_control_send(self, s_change);
}

void shoes_native_control_hide(SHOES_CONTROL_REF ref) {
    gtk_widget_hide(ref);
}

void shoes_native_control_show(SHOES_CONTROL_REF ref) {
    gtk_widget_show(ref);
}

void shoes_native_control_position(SHOES_CONTROL_REF ref, shoes_place *p1, VALUE self,
                              shoes_canvas *canvas, shoes_place *p2) {
    PLACE_COORDS();
    gtk_widget_set_size_request(ref, p2->iw, p2->ih);
    gtk_fixed_put(GTK_FIXED(canvas->slot->oscanvas), ref, p2->ix + p2->dx, p2->iy + p2->dy);
    gtk_widget_show_all(ref);
}

void shoes_native_control_repaint(SHOES_CONTROL_REF ref, shoes_place *p1,
                             shoes_canvas *canvas, shoes_place *p2) {
    p2->iy -= canvas->slot->scrolly;
    if (CHANGED_COORDS()) {
        PLACE_COORDS();
        gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas),
                       ref, p2->ix + p2->dx, p2->iy + p2->dy);
        gtk_widget_set_size_request(ref, p2->iw, p2->ih);
    }
    p2->iy += canvas->slot->scrolly;
}

void shoes_native_control_focus(SHOES_CONTROL_REF ref) {
    if (gtk_widget_get_can_focus(ref)) gtk_widget_grab_focus(ref);
}

void shoes_native_control_state(SHOES_CONTROL_REF ref, gboolean sensitive, gboolean setting) {
    gtk_widget_set_sensitive(ref, sensitive);
    if (GTK_IS_EDITABLE(ref))
        gtk_editable_set_editable(GTK_EDITABLE(ref), setting);
    else if (GTK_IS_SCROLLED_WINDOW(ref)) {
        GtkWidget *textview;
        GTK_CHILD(textview, ref);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), setting);
    }
}

void shoes_native_control_remove(SHOES_CONTROL_REF ref, shoes_canvas *canvas) {
    gtk_container_remove(GTK_CONTAINER(canvas->slot->oscanvas), ref);
}

void shoes_native_control_free(SHOES_CONTROL_REF ref) {
    //
    // no need to free gtk widgets, since gtk seems
    // to garbage collect them fine.  and memory
    // addresses often get reused.
    //
}

void shoes_native_control_set_tooltip(SHOES_CONTROL_REF ref, VALUE tooltip) {
    gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_native_to_s(tooltip)));
}

VALUE shoes_native_control_get_tooltip(SHOES_CONTROL_REF ref) {
    return rb_str_new2(gtk_widget_get_tooltip_text(GTK_WIDGET(ref)));
}

void shoes_native_secrecy(SHOES_CONTROL_REF ref) {
    gtk_entry_set_visibility(GTK_ENTRY(ref), FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(ref), SHOES_GTK_INVISIBLE_CHAR);
}

VALUE shoes_native_clipboard_get(shoes_app *app) {
    //GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    if (gtk_clipboard_wait_is_text_available(primary)) {
        gchar *string = gtk_clipboard_wait_for_text(primary);
        return rb_str_new2(string);
    }
    return Qnil;
}

void shoes_native_clipboard_set(shoes_app *app, VALUE string) {
    //GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(primary, RSTRING_PTR(string), RSTRING_LEN(string));
}

VALUE shoes_native_to_s(VALUE text) {
    text = rb_funcall(text, s_to_s, 0);
    return text;
}



#ifdef SHOES_GTK_WIN32
// hat tip: https://justcheckingonall.wordpress.com/2008/08/29/console-window-win32-app/
#include <stdio.h>
#include <io.h>
#include <fcntl.h>


// called from main.c(skel) on Windows - works fine
static FILE* shoes_console_out = NULL;
static FILE* shoes_console_in = NULL;

int shoes_win32_console() {

    if (AllocConsole() == 0) {
        // cshoes.exe can get here
        printf("Already have console\n");
        return 0;
    }

    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((intptr_t) handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;
    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((intptr_t) handle_in, _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;

    //* stash handles
    shoes_console_out = hf_out;
    shoes_console_in = hf_in;
    return 1;
}

// Called by Shoes after ruby/gtk/shoes is initialized and running
void shoes_native_terminal(char *dir_path, int monitor, int columns, int row,
    int fontsize, char *fg, char *bg, char *title) {
    // has a console been setup by --console flag?
    if (shoes_console_out == NULL) {
        if (shoes_win32_console() == 0) // cshoes.exe can do this
            return;
    }
    // convert the (cached) FILE * for what ruby wants for fd[0], [1]...
    if (dup2(_fileno(shoes_console_out), 1) == -1)
        printf("failed dup2 of stdout\n");
    if (dup2(_fileno(shoes_console_out), 2) == -1)
        printf("failed dup2 of stderr\n");
    if (dup2(_fileno(shoes_console_in), 0) == -1)
        printf("failed dup2 of stdin\n");
    printf("created win32 console\n");
    return;
}

// For bug #428 
int shoes_win10_gtk3_22_check() {
    if (gtk_get_minor_version() < 22)
      return 0;
    // borrowed from
    // https://stackoverflow.com/questions/32115255/c-how-to-detect-windows-10
    int ret = 0;
    NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
    OSVERSIONINFOEXW osInfo;

    *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

    if (NULL != RtlGetVersion)
    {
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        RtlGetVersion(&osInfo);
        ret = osInfo.dwMajorVersion;
    }
    //printf("windows version %i\n", ret); // win 7 returns '6' go figure
    return ret == 10;
}
#else
/*
int shoes_native_console(char *app_path)
{
  //printf("init gtk console\n");
  shoes_native_app_console(app_path);
  printf("gtk\010k\t console \t\tcreated\n"); //test \b \t in string
  return 1;
}
*/
#endif

/*
 *  ------- monitor handling --------
 *  As usual, gtk 3.22 'fixes' things with deprecations. 
 * 
 *  Currrently using pre 3.22 API.
 *    Screen width is the sum of all monitors. Height is the max of all 
 *    monitors. Do not address monitors just set x. 
*/
#if GTK_MINOR_VERSION >= 22
GdkMonitor *shoes_gtk_monitors[8] = {NULL};

// Fill in the table;
void shoes_gtk_check_monitors() {
  GdkDisplay *display;
  display = gdk_display_get_default();
  int cnt = gdk_display_get_n_monitors(display);
  for (int i=0; i < cnt; i++) {
    shoes_gtk_monitors[i] = gdk_display_get_monitor(display, i);
  }
}

int shoes_gtk_get_monitor(GdkMonitor *mon) {
  GdkDisplay *display;
  display = gdk_display_get_default();
  int n;
  for (n = 0; n < gdk_display_get_n_monitors(display); n++) {
    if (shoes_gtk_monitors[n] == mon)
      return n;
  }
  // error if we get here.
  return 0;
}
#endif

int shoes_native_monitor_default() {
  GdkDisplay *display;
  display = gdk_display_get_default();
#if GTK_MINOR_VERSION >= 22
  if (shoes_gtk_monitors[0] == NULL) shoes_gtk_check_monitors();
  GdkMonitor *monitor;
  monitor = gdk_display_get_primary_monitor(display);
  int mon;
  mon = shoes_gtk_get_monitor(monitor);
  return mon;
#else
  GdkScreen *screen;
  screen = gdk_display_get_default_screen(display);
  int mon;
  mon = gdk_screen_get_primary_monitor(screen);
  return mon;
#endif
}

void shoes_native_monitor_geometry(int idx, shoes_monitor_t *geo) {

  GdkScreen *screen;
  screen = gdk_screen_get_default();
  GdkRectangle r;
  // workarea approximates visibleFrame on OSX
#if GTK_MINOR_VERSION >= 22
  if (shoes_gtk_monitors[0] == NULL) shoes_gtk_check_monitors();
  GdkMonitor *mon = shoes_gtk_monitors[idx];
  //gdk_monitor_get_geometry(mon, &r);
  gdk_monitor_get_workarea(mon, &r);
#else
  //gdk_screen_get_monitor_geometry(screen, idx,  &r);
  gdk_screen_get_monitor_workarea(screen, idx, &r);
#endif
  geo->x = r.x;
  geo->y = r.y;
  geo->width = r.width;
  geo->height = r.height;
}

int shoes_native_monitor_count() {
  int cnt = 0;
#if GTK_MINOR_VERSION >= 22
  GdkDisplay *display;
  display = gdk_display_get_default();
  cnt = gdk_display_get_n_monitors(display);
#else
  GdkScreen *screen;
  screen = gdk_screen_get_default();
  cnt = gdk_screen_get_n_monitors(screen);
#endif
  return cnt;
}

// Sets/moves the window onto monitor 
void shoes_native_monitor_set(shoes_app *app) {
  GtkWindow *window = (GtkWindow *)app->os.window;
  GdkDisplay *display;
  GdkScreen *screen;
  screen = gdk_screen_get_default();  
  display = gdk_screen_get_display(screen);
  // sanity check
  int cnt = 0;
#if GTK_MINOR_VERSION >= 22
  if (shoes_gtk_monitors[0] == NULL) shoes_gtk_check_monitors();
  cnt = gdk_display_get_n_monitors(display);
#else
  cnt = gdk_screen_get_n_monitors(screen);
#endif
  int realmon; 
  if (app->monitor < cnt && app->monitor >= 0)
    realmon = app->monitor;
  else {
    realmon = 0;
  }
  /*
  fprintf(stderr, "Screen w: %d x h: %d\n", gdk_screen_get_width(screen),
      gdk_screen_get_height(screen));
  fprintf(stderr, "Switch to monitor %d of %d\n", realmon, cnt);
  */ 
  GdkRectangle r;
#if GTK_MINOR_VERSION >= 22
  GdkMonitor *mon = shoes_gtk_monitors[realmon];
  gdk_monitor_get_geometry(mon, &r);
#else
  gdk_screen_get_monitor_geometry(screen, realmon,  &r);
#endif
  gtk_window_move(window, app->x + r.x, app->y);
  // TODO: do a better job of positioning. Worth the effort?
}

// returns the Shoes monitor number that the window is on
int shoes_native_monitor_get(shoes_app *app) {
  GtkWindow *window = (GtkWindow *)app->os.window;
  GdkDisplay *display;
  GdkScreen *screen;
  screen = gtk_window_get_screen(window);
  // determine which monitor that is. Remember: Gtk screen holds Gtk monitors
  int x, y, cnt, i;
  gtk_window_get_position (window, &x, &y);
#if GTK_MINOR_VERSION >= 22
  if (shoes_gtk_monitors[0] == NULL) shoes_gtk_check_monitors();
  display = gdk_display_get_default();
  cnt = gdk_display_get_n_monitors(display);
  for (i = 0; i < cnt; i++) {
    GdkRectangle r;
    GdkMonitor *mon = shoes_gtk_monitors[i];
    gdk_monitor_get_geometry(mon, &r);
    if ((x >= r.x) && (x <= (r.x +r.width)) && (y >= r.y) && (y <= (r.y +r.height)))
      return i;
  }
  // should never get here, but if it does:
  return shoes_gtk_get_monitor(shoes_gtk_monitors[0]);
#else
  cnt = gdk_screen_get_n_monitors(screen);
  for (i = 0; i < cnt; i++) {
    GdkRectangle r;
    gdk_screen_get_monitor_geometry(screen, i,  &r);
    if ((x >= r.x) && (x <= (r.x +r.width)) && (y >= r.y) && (y <= (r.y +r.height)))
      return i;
  }
  // should never get here, but if it does:
  return gdk_screen_get_primary_monitor(screen);
#endif
}


/*  ------- have_menu == true and sizing ---------- */

// hack for windows (gtk < 3.12) - the 65 below is the taskbar height. Hack!!
#if ! GTK_CHECK_VERSION(3,12,0)
void shoes_gtk_set_max(shoes_app *app) {
  int m = shoes_native_monitor_get(app);
  shoes_monitor_t geo;
  shoes_native_monitor_geometry(m, &geo);
  app->os.maxwidth = geo.width;
  app->os.maxheight = geo.height;
  //fprintf(stderr, "max wid: %d hgh: %d\n", geo.width, geo.height);
}

int shoes_gtk_is_maximized(shoes_app *app, int wid, int hgt) {
  int t = wid >= app->os.maxwidth && hgt >= (app->os.maxheight-65);
  return t; 
}
#endif

// called only by **GtkWindow** signal handler for *size-allocate*
static void shoes_gtk_root_size(GtkWidget *widget, cairo_t *cr, gpointer data) {
    shoes_app *app = (shoes_app *)data;
    int width, height;
    if (widget != app->os.window)
      fprintf(stderr, "widget != app->os.window\n");
    gtk_window_get_size(GTK_WINDOW(app->os.window), &width, &height);
#ifdef SZBUG
    fprintf(stderr,"shoes_gtk_root_size: wid: %d hgt: %d\n", width, height);
#endif
    app->width = width;
    // remove height of menubar container. Variable height now means canvas hgt
    app->height = height = (height - shoes_gtk_optbox_height(app, height));
    // process a resize & paint of Shoes content widget, i.e. top level shoes canvas
    shoes_canvas *canvas;
    TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
    
    // set new size of the content Widget. We are are a gtk container.
    GtkAllocation cvs_alloc;
    if (shoes_Windows_Version && (gtk_minor_version == 24)) {
      cvs_alloc.x = 0;
      cvs_alloc.y = app->mb_height;    // TODO: may not be needed 
    } else {
      cvs_alloc.x = 0; 
      cvs_alloc.y = app->mb_height;
    }
    cvs_alloc.width = width;
    cvs_alloc.height = height + app->mb_height;
    // This works for manually resizing the window with the mouse
    // doesn't draw all internals 
    //gtk_widget_size_allocate (app->os.shoes_window, &cvs_alloc); 
#ifndef GTK_CANVAS_SIZE    
    if (canvas->slot->vscroll && 
        (height != canvas->slot->scrollh || width != canvas->slot->scrollw)) {
      GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
      gtk_widget_set_size_request(canvas->slot->vscroll, -1, height);
      
      GtkAllocation alloc;
      gtk_widget_get_allocation((GtkWidget *)canvas->slot->vscroll, &alloc);  
#ifdef SZBUG
      fprintf(stderr, "sb alloc: %d %d %d %d\n\n", alloc.x, alloc.y, alloc.width, alloc.height);
#endif
      gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas), canvas->slot->vscroll,
                     width - alloc.width, 0);
      gtk_adjustment_set_page_size(adj, height);
      gtk_adjustment_set_page_increment(adj, height - 32);
      if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
          gtk_widget_hide(canvas->slot->vscroll);
      else {
          gtk_widget_show(canvas->slot->vscroll);
      }
      canvas->slot->scrollh = height; 
      canvas->slot->scrollw = width;
    }
#endif
    shoes_canvas_size(app->canvas, app->width, app->height);
}

#if 1
/*
 *  Called by **canvas->slot** signal handler for *size-allocate* - toplevel window.
*/
static void shoes_gtk_content_size(GtkWidget *widget, GtkAllocation *size, gpointer data) {
#ifdef SZBUG
    fprintf(stderr,"shoes_gtk_content_size widget: %lx wide: %d hgt: %d\n", 
			(unsigned long)widget,
			size->width, size->height);
#endif
#ifdef GTK_CANVAS_SIZE
    VALUE c = (VALUE)data;
    shoes_canvas *canvas;
    TypedData_Get_Struct(c, shoes_canvas, &shoes_canvas_type, canvas);
    if (canvas->slot->vscroll) {
            // && (size->height != canvas->slot->scrollh || size->width != canvas->slot->scrollw)) {
        if (size->height != canvas->slot->scrollh) {
          GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
          gtk_widget_set_size_request(canvas->slot->vscroll, -1, size->height);
          
          //gtk_widget_set_size_request(GTK_CONTAINER(widget), canvas->app->width, size->height);
          GtkAllocation alloc;
          gtk_widget_get_allocation((GtkWidget *)canvas->slot->vscroll, &alloc);  
#ifdef SZBUG
          fprintf(stderr, "alloc: %d %d %d %d\n\n", alloc.x, alloc.y, alloc.width, alloc.height);
#endif
          gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas), canvas->slot->vscroll,
                         size->width - alloc.width, 0);
          gtk_adjustment_set_page_size(adj, size->height);
          gtk_adjustment_set_page_increment(adj, size->height - 32);
  
          if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
              gtk_widget_hide(canvas->slot->vscroll);
          else {
              gtk_widget_show(canvas->slot->vscroll);
          }
          canvas->slot->scrollh = size->height;
      } else if (size->width != canvas->slot->scrollw) {
          canvas->slot->scrollw = size->width;
      }
    }
#endif
}
#endif // hide from compiler

/* TODO: sort of fixes bug #349 depends on gtk 3.12 or higher (Boo Windows)
 * seems like overkill or incomplete - it gets called a lot. 
*/
gboolean shoes_app_gtk_configure_menu(GtkWidget *widget, GdkEvent *evt, gpointer data) {
  shoes_app *app = (shoes_app *)data;
  if (widget == app->os.window) {  // GtkWindow
    //gtk_widget_set_size_request(widget, evt->configure.width, evt->configure.height);
    shoes_canvas *canvas;
    TypedData_Get_Struct(app->canvas, shoes_canvas, &shoes_canvas_type, canvas);
#if GTK_CHECK_VERSION(3,12,0)
    if (gtk_window_is_maximized((GtkWindow *)widget)) {
#else
    if (shoes_gtk_is_maximized(app, evt->configure.width, evt->configure.height)) {
#endif
      GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
      gtk_widget_set_size_request(canvas->slot->vscroll, -1, evt->configure.height);
      
      GtkAllocation alloc;
      gtk_widget_get_allocation((GtkWidget *)canvas->slot->vscroll, &alloc);  
#ifdef SZBUG
      fprintf(stderr, "shoes_app_gtk_configure_menu %d, %d, %d, %d\n", 
	    evt->configure.x, evt->configure.y, evt->configure.width, evt->configure.height);
      fprintf(stderr, "cfg alloc: %d %d %d %d\n\n", alloc.x, alloc.y, alloc.width, alloc.height);
#endif
      gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas), canvas->slot->vscroll,
				     evt->configure.width - alloc.width, 0);
      gtk_adjustment_set_page_size(adj, evt->configure.height);
      gtk_adjustment_set_page_increment(adj, evt->configure.height - 32);

      if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
	  gtk_widget_hide(canvas->slot->vscroll);
      else {
	  gtk_widget_show(canvas->slot->vscroll);
      }
    } else {
#ifdef SZBUG
      printf(stderr, "shoes_app_gtk_configure_menu: dragged %d, %d, %d, %d\\n",
	  evt->configure.x, evt->configure.y, evt->configure.width, evt->configure.height);
      GtkWidget *content = app->os.shoes_window;
#endif
    }
  }
  app->width = evt->configure.width;
  app->height = evt->configure.height;
  return FALSE;
}


void shoes_slot_init_menu(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width, int height, int scrolls, int toplevel) {
  shoes_canvas *canvas;
  SHOES_SLOT_OS *slot;
  TypedData_Get_Struct(c, shoes_canvas, &shoes_canvas_type, canvas);
  
  slot = shoes_slot_alloc(canvas, parent, toplevel);
  
  /* Subclassing GtkFixed so we can override gtk3 size management which creates
     problems with slot height being always tied to inside widgets cumulative heights
     creating heights overflow with no scrollbar !
  */
  if (toplevel)
    slot->oscanvas = canvas->app->os.shoes_window; // created in app_open
  else
    slot->oscanvas = gtkfixed_alt_new(width, height);
#ifdef SZBUG
  fprintf(stderr,"shoes_slot_init_menu toplevel: %d, slot->oscanvas: %lx\n", toplevel, (unsigned long)slot->oscanvas); 
#endif
  g_signal_connect(GTK_WIDGET(slot->oscanvas), "draw",
                   G_CALLBACK(shoes_canvas_gtk_paint), (gpointer)c);
#if 1
    g_signal_connect(GTK_WIDGET(slot->oscanvas), "touch-event",
                    G_CALLBACK(shoes_canvas_gtk_touch), (gpointer)c);
#endif
#ifdef GTK_CANVAS_SIZE  
  g_signal_connect(GTK_WIDGET(slot->oscanvas), "size-allocate",
                   G_CALLBACK(shoes_gtk_content_size), (gpointer)c);
#endif
  INFO("shoes_slot_init_menu(%lu)\n", c);
  
  if (toplevel) {
    //gtk_container_add(GTK_CONTAINER(parent->oscanvas), slot->oscanvas);
  } else {
    gtk_fixed_put(GTK_FIXED(parent->oscanvas), slot->oscanvas, x, y);
  }
  slot->scrollh = slot->scrollw = 0;
  slot->vscroll = NULL;
  if (scrolls) {
    slot->vscroll = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,NULL);
    GtkAdjustment *adj;
    adj = gtk_range_get_adjustment(GTK_RANGE(slot->vscroll));
    gtk_adjustment_set_step_increment(adj, 16);
    gtk_adjustment_set_page_increment(adj, height - 32);
    slot->drawevent = NULL;

    g_signal_connect(GTK_WIDGET(slot->vscroll), "value-changed",
                     G_CALLBACK(shoes_canvas_gtk_scroll), (gpointer)c);
    gtk_fixed_put(GTK_FIXED(slot->oscanvas), slot->vscroll, -100, -100);
    gtk_widget_set_size_request(slot->oscanvas, width, height);

    if (!toplevel) 
      ATTRSET(canvas->attr, wheel, scrolls);
  }
  
  if (toplevel) {
    //gtk_widget_show_all(slot->oscanvas);
    shoes_canvas_size(c, width, height);
    //shoes_canvas_repaint_all(slot->oscanvas); //crash - not realized yet?
  } else {
    gtk_widget_show_all(slot->oscanvas);
    //gtk_widget_show_all(parent->oscanvas);
    canvas->width = 100;
    canvas->height = 100;
  }
}

/*
 *  compute the height of the vbox containing the menubar and toolbar
 *  Argument height can be < 0, 0, or > 0. If zero, we guess what the
 *  height could be. If >0 its the real hgt of the outer window
 *  if < 0 then we'll grab the real windows height. 
*/
int shoes_gtk_optbox_height(shoes_app *app, int height) {
  int hgt = 0;
  if (height == 0) { //guess 
    hgt = 38; // works for me. not really important.
  } else if (height >  0) {
    // get the size of the option box from gtk
    GtkAllocation alloc;
    gtk_widget_get_allocation(app->os.opt_container, &alloc);
    hgt = alloc.height;
  } else {
    // compute by window.h - shoes_window.h 
    int width;
    int height;
    GtkAllocation alloc;
    gtk_window_get_size(GTK_WINDOW(app->os.window), &width, &height);
    gtk_widget_get_allocation(app->os.shoes_window, &alloc);
    hgt = height - alloc.height;
  }
  //fprintf(stderr, "optbox hgt %d -> %d\n", height, hgt);
  return hgt;
}

static void
shoes_gtk_swipe_gesture_swept (GtkGestureSwipe *gesture,
                     gdouble          velocity_x,
                     gdouble          velocity_y,
                     gpointer       *data)
{
  shoes_app *app = (shoes_app *)data;
  gdouble swipe_x = velocity_x / 10;
  gdouble swipe_y = velocity_y / 10;
  //gtk_widget_queue_draw (widget);
  fprintf(stderr, "Gesture: swipe\n");
}

/*
 *  All apps windows can have a menubur and then the old shoes space below. 
 *  it's optional and the default is no menu for backwards compatibilty
 *  That causes some pixel tweaking and redundency. 
 */
 
shoes_code shoes_native_app_open_menu(shoes_app *app, char *path, int dialog, shoes_settings *st) {
    GtkWidget *window;       // Root window 
        
    char icon_path[SHOES_BUFSIZE];
    if (st->icon_path == Qnil)
      sprintf(icon_path, "%s/static/app-icon.png", shoes_world->path);
    else {
      char *ip = RSTRING_PTR(st->icon_path);
      if (*ip == '/' || ip[1] ==':')  
        strcpy(icon_path, ip);
      else
       sprintf(icon_path, "%s/%s", shoes_world->path, ip);
    }
    gtk_window_set_default_icon_from_file(icon_path, NULL);
    /*
     *       App Window
     * +-----------------------+ 
     * | menu .. menu..        |\      
     * +-----------------------+ \ 
     * | toolitem...toolitem.. | / Optionbox 
     * +-----------------------+/           \
     * |                       |             \
     * |  shoes content        |  +-----------+ vbox
     * |                       | /
     * +-----------------------+/
     * 
     * appwindow := vbox
     * vbox := optbox shoes_window
     * optbox := [menubar] [toolbar]
     * NOTE: toolbar is not implemented and may not be done that way.
     */
    GtkWidget *vbox;         // contents of root window
    GtkWidget *optbox;       // contents of option box (menubar, toolbar)
    GtkWidget *shoes_window; // bottom of vbox where shoes does its thing.
    
#if 0 //#ifdef ENABLE_MDI
    window = gtk_application_window_new(shoes_GtkApp);
#else
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif
    // theme the window
    if (shoes_css_provider != NULL) {
      gtk_style_context_add_provider(gtk_widget_get_style_context(window),
          GTK_STYLE_PROVIDER(shoes_css_provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);
    }

    vbox =  gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    optbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(vbox), optbox);

    // Gtk Container of the fixed variety. Note the alt version!!
    shoes_window = gtkfixed_alt_new(app->width, app->height); 
    gtk_box_pack_start(GTK_BOX(vbox), shoes_window, FALSE, FALSE, 0);
    
    app->os.window = window;
    app->os.vlayout = vbox;
    app->os.opt_container = optbox;
    app->os.shoes_window = shoes_window;
    app->slot->oscanvas = shoes_window;
#ifdef SZBUG
    fprintf(stderr,"shoes_native_app_open_menu slot->canvas %lx\n", (unsigned long)app->slot->oscanvas);
#endif
    if (app->have_menu) 
      shoes_gtk_attach_menubar(app, st); 
    shoes_gtk_attach_toolbar(app, st);  // TODO: not really the right place to do this
    //guess size since it's not realized yet. It will be fixed up later
    app->mb_height = shoes_gtk_optbox_height(app, 0);  
         
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   // commit https://github.com/shoes/shoes/commit/4e7982ddcc8713298b6959804dab8d20111c0038
    if (!app->resizable) {
        gtk_widget_set_size_request(window, app->width, app->height + app->mb_height);
        gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    } else if (app->minwidth < app->width || app->minheight < app->height + app->mb_height) {
        GdkGeometry hints;
        hints.min_width = max(app->minwidth, 100);
        hints.min_height = max(app->minheight + app->mb_height, 100);
#ifdef SZBUG
        fprintf(stderr,"resize hints: %d, %d\n", hints.min_width, hints.min_height);
#endif
        gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL,
                                      &hints, GDK_HINT_MIN_SIZE);
    }
    gtk_window_set_default_size(GTK_WINDOW(window), app->width, app->height + app->mb_height);

    gtk_window_get_position(GTK_WINDOW(window), &app->x, &app->y);
    // get optionbox height - still not realized but closer to truth.
    GtkRequisition reqmin, reqnat;
    gtk_widget_get_preferred_size(optbox, &reqmin, &reqnat);
    // fprintf(stderr, "optbox h: %d\n", reqmin.height); // 38 for me. 
    app->mb_height = reqmin.height;

    g_signal_connect(GTK_WINDOW(window), "size-allocate",   // need this for manual resizing.
                     G_CALLBACK(shoes_gtk_root_size), app);
    g_signal_connect(GTK_WINDOW(window), "motion-notify-event",
                     G_CALLBACK(shoes_app_gtk_motion), app);
    g_signal_connect(GTK_WINDOW(window), "button-press-event",
                     G_CALLBACK(shoes_app_gtk_button), app);
    g_signal_connect(GTK_WINDOW(window), "button-release-event",
                     G_CALLBACK(shoes_app_gtk_button), app);
    g_signal_connect(GTK_WINDOW(window), "scroll-event",
                     G_CALLBACK(shoes_app_gtk_wheel), app);
    g_signal_connect(GTK_WINDOW(window), "key-press-event",
                     G_CALLBACK(shoes_app_gtk_keypress), app);
    g_signal_connect(GTK_WINDOW(window), "key-release-event",
                     G_CALLBACK(shoes_app_gtk_keypress), app);
    g_signal_connect(GTK_WINDOW(window), "delete-event",
                     G_CALLBACK(shoes_app_gtk_quit), app);
    g_signal_connect(GTK_WINDOW(window), "configure-event",   // bug #349
                     G_CALLBACK(shoes_app_gtk_configure_menu), app);
#if 1
    g_signal_connect(GTK_WIDGET(window), "touch-event",
                    G_CALLBACK(shoes_canvas_gtk_touch), app);
#endif
#if 0
    g_signal_connect(GTK_WIDGET(window), "drag-begin",
                    G_CALLBACK(shoes_gtk_app_drag_begin), app);
#endif
#if 0
     // swipe setup
    GtkGesture *gesture = gtk_gesture_swipe_new (shoes_window);       
    g_signal_connect (gesture, "swipe",
                        G_CALLBACK (shoes_gtk_swipe_gesture_swept), app);
    gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (gesture),
                                                  GTK_PHASE_CAPTURE ); 
#endif    
    if (app->fullscreen) shoes_native_app_fullscreen(app, 1);

    gtk_window_set_decorated(GTK_WINDOW(window), app->decorated);
#if GTK_CHECK_VERSION(3,8,0)
    gtk_widget_set_opacity(GTK_WIDGET(window), app->opacity);
#endif
    // ORIG: gtk_widget_set_events(window, GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    gtk_widget_add_events(window, GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK |
        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_TOUCH_MASK);


    // If the user asked for a specific Screen (Monitor) for the Window
    if (app->monitor >= 0) {
      shoes_native_monitor_set(app);
    }
#if ! GTK_CHECK_VERSION(3,12,0)
    shoes_gtk_set_max(app);
#endif
    return SHOES_OK;
}

void shoes_gtk_attach_menubar(shoes_app *app, shoes_settings *st) {
  GtkWidget *optbox = app->os.opt_container;    // contents of option box (menubar, toolbar)
  GtkWidget *menubar;   
  
  menubar = gtk_menu_bar_new();
  app->os.menubar = menubar;
  app->have_menu = TRUE;
  gtk_box_pack_start(GTK_BOX(optbox), menubar, FALSE, FALSE, 0);
  
  // now we can add the default Shoes menus
  VALUE mbv = shoes_native_menubar_setup(app, menubar);
  shoes_native_build_menus(app, mbv);
  app->menubar = mbv;
}

void shoes_gtk_attach_toolbar(shoes_app *app, shoes_settings *st) {
}
