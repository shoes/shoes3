#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/internal.h"
#include "shoes/types/settings.h"
#include "shoes/native/gtk/gtksystray.h"

/* 
 * Many problems here - Gtk3 deprecates what works on Windows (and linux)
 * In order to use the more forward /useful method for linux we need to 
 * use g_application which requires a unique dbus address for the Process
 * like "shoes-#{pid}" perhaps. Gapplication/GtkApplication requires
 * much more work in Shoes. 
 * 
 * Meantime we need to turn off the whining about deprecations.
 * Then see if we can pick at runtime which one we want
 * 
 * ref:
 * https://stackoverflow.com/questions/3378560/how-to-disable-gcc-warnings-for-a-few-lines-of-code
*/

extern GtkApplication *shoes_GtkApp;

#if !defined(SHOES_GTK_WIN32) // i.e. not Windows
static void shoes_native_systray_gapp(char *title, char *message, char *path) {
  GNotification *note;
  note = g_notification_new (title);
  g_notification_set_body (note, message);
  GFile *iconf = g_file_new_for_path (path);
  GIcon *icon = g_file_icon_new (iconf);
  g_notification_set_icon(note, icon);  //TODO: could get icon from settings
  Get_TypedStruct2(shoes_world->settings, shoes_settings, st);
  char *nm = RSTRING_PTR(st->app_name);
  g_application_send_notification (G_APPLICATION(shoes_GtkApp), RSTRING_PTR(st->app_name), note);
}
#endif
// Always compile the old version (gtk_status_icon)
// use gtk_status_icon for Windows, deprecated but GNotification doesn't work
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static GtkStatusIcon *stsicon = NULL;
static char *stspath = NULL;
static void shoes_native_systray_old(char *title, char *message, char *path) {
    if (stsicon == NULL) {
        stsicon = gtk_status_icon_new_from_file(path);
        stspath = path;
    }
    // detect change of icon
    if (strcmp(path, stspath)) {
        stspath = path;
        gtk_status_icon_set_from_file (stsicon, stspath);
    }
    gtk_status_icon_set_title(stsicon, title);
    gtk_status_icon_set_tooltip_text(stsicon, message);
}
#pragma GCC diagnostic pop

void shoes_native_systray(char *title, char *message, char *path) {
#ifdef SHOES_GTK_WIN32
  // always call the older stuff for Windows
  shoes_native_systray_old(title, message, path);
#else
  shoes_native_systray_gapp(title, message, path);
#endif
#if 0
  shoes_native_systray_old(title, message, path);
#endif
}
