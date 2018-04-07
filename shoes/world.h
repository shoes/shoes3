//
// shoes/world.h
//
// The shoes_world struct contains global information about the environment which is shared between
// app windows. Expanded in Shoes 3.3.7.
//
#ifndef SHOES_WORLD_H
#define SHOES_WORLD_H

#include "shoes/config.h"
#include "shoes/ruby.h"
#include "shoes/code.h"

#ifdef __cplusplus
extern "C" {
#endif

SHOES_EXTERN typedef struct _shoes_world_t {
    SHOES_WORLD_OS os;
    int mainloop;
    char path[SHOES_BUFSIZE];
    VALUE apps, msgs;
    st_table *image_cache;
    guint thread_event;
    cairo_surface_t *blank_image;
    shoes_cached_image *blank_cache;
    PangoFontDescription *default_font;
    // Shoes 3,3.7 adds a bunch of globals and defaults
    VALUE settings;
} shoes_world_t;

extern SHOES_EXTERN shoes_world_t *shoes_world;

// rarely used ? but needed
#define GLOBAL_APP(appvar) \
  shoes_app *appvar = NULL; \
  if (RARRAY_LEN(shoes_world->apps) > 0) \
    Data_Get_Struct(rb_ary_entry(shoes_world->apps, 0), shoes_app, appvar)\

// gtk uses this for mKernel methods - app is optional
// defines 3 variables that callers need to know about. Ick!
#define GTK_APP_VAR(appvar) \
  char * title_##appvar = "Shoes"; \
  GtkWindow *window_##appvar = NULL; \
  shoes_app *appvar = NULL; \
  if (RARRAY_LEN(shoes_world->apps) > 0) { \
    VALUE actual_app = rb_funcall2(self, rb_intern("app"), 0, NULL); \
    Data_Get_Struct(actual_app, shoes_app, appvar); \
    title_##appvar = RSTRING_PTR(app->title); \
    window_##appvar = APP_WINDOW(app);\
  }\

#define OSX_APP_VAR(appvar) \
  char * title_##appvar = "Shoes"; \
  shoes_app *appvar = NULL; \
  if (RARRAY_LEN(shoes_world->apps) > 0) { \
    VALUE actual_app = rb_funcall2(self, rb_intern("app"), 0, NULL); \
    Data_Get_Struct(actual_app, shoes_app, appvar); \
    title_##appvar = RSTRING_PTR(app->title); \
  }\


// no longer used - TODO: remove after testing.
#define ACTUAL_APP_NOPE(appvar) \
  shoes_app *appvar = NULL; \
  VALUE actual_app = rb_funcall2(self, rb_intern("app"), 0, NULL); \
  Data_Get_Struct(actual_app, shoes_app, appvar);

#define ROUND(x) ((x) >= 0 ? (int)round((x)+0.5) : (int)round((x)-0.5))

//
// Shoes World
//
SHOES_EXTERN shoes_world_t *shoes_world_alloc(void);
SHOES_EXTERN void shoes_world_free(shoes_world_t *);
void shoes_update_fonts(VALUE);

//
// Shoes
//
SHOES_EXTERN shoes_code shoes_init(SHOES_INIT_ARGS);
SHOES_EXTERN shoes_code shoes_load(char *);
SHOES_EXTERN shoes_code shoes_start(char *, char *, int);
#if defined(SHOES_WIN32) || defined(SHOES_GTK_WIN32)
SHOES_EXTERN int shoes_win32_cmdvector(const char *, char ***);
#endif
SHOES_EXTERN void shoes_set_argv(int, char **);
SHOES_EXTERN shoes_code shoes_final(void);

/*
 * Shoes.yaml
*/ 
typedef struct _shoes_yaml_init {
    int   active;
    char* app_name;
    char* theme_name;
    char* rdomain;
    char* use_menus;
    char* mdi;
} shoes_yaml_init;

extern shoes_yaml_init *shoes_config_yaml;

SHOES_EXTERN int shoes_init_load_yaml();
SHOES_EXTERN char *shoes_app_name;

#ifdef __cplusplus
}
#endif

#endif
