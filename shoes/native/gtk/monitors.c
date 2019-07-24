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

