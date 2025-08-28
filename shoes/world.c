///
// shoes/world.c
// Abstract windowing for GTK, Quartz (OSX) and Win32.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"
#include "shoes/types/settings.h"

#ifdef SHOES_SIGNAL
#include <signal.h>
#ifdef SHOES_GTK_WIN32
#define SHOES_WIN32  // For this file.
#endif

void shoes_sigint() {
    shoes_native_quit();
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef SHOES_QUARTZ
extern void shoes_osx_setup_stdout();
#endif
shoes_world_t *shoes_world = NULL;

shoes_world_t *shoes_world_alloc() {
    fprintf(stderr, "[SHOES] shoes_world_alloc: Starting\n");
    shoes_world_t *world = SHOE_ALLOC(shoes_world_t);
    SHOE_MEMZERO(world, shoes_world_t, 1);
    
    fprintf(stderr, "[SHOES] shoes_world_alloc: Creating Ruby arrays\n");
    world->apps = rb_ary_new();
    world->msgs = rb_ary_new();
    world->mainloop = FALSE;
    
    fprintf(stderr, "[SHOES] shoes_world_alloc: Initializing image cache\n");
    world->image_cache = st_init_strtable();
    world->blank_image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    world->blank_cache = SHOE_ALLOC(shoes_cached_image);
    world->blank_cache->surface = world->blank_image;
    world->blank_cache->pattern = NULL;
    world->blank_cache->width = 1;
    world->blank_cache->height = 1;
    world->blank_cache->mtime = 0;
    
    fprintf(stderr, "[SHOES] shoes_world_alloc: Setting up fonts\n");
    world->default_font = pango_font_description_new();
    pango_font_description_set_family(world->default_font, "Arial");
    pango_font_description_set_absolute_size(world->default_font, 14. * PANGO_SCALE * (96./72.));
    
    fprintf(stderr, "[SHOES] shoes_world_alloc: Registering GC addresses\n");
    rb_gc_register_address(&world->apps);
    rb_gc_register_address(&world->msgs);
    
    // new settings - THIS IS THE PROBLEM! cSettings doesn't exist yet!
    fprintf(stderr, "[SHOES] shoes_world_alloc: Creating settings (cSettings=%p)\n", (void*)cSettings);
    world->settings = shoes_settings_alloc(cSettings);
    rb_gc_register_address(&world->settings);
    
    fprintf(stderr, "[SHOES] shoes_world_alloc: Done\n");
    return world;
}

int shoes_world_free_image_cache(st_data_t key, st_data_t value, st_data_t arg) {
    char *key_str = (char *)key;
    shoes_cache_entry *cached = (shoes_cache_entry *)value;
    if (cached->type != SHOES_CACHE_ALIAS && cached->image != NULL) {
        if (cached->image->pattern != NULL)
            cairo_pattern_destroy(cached->image->pattern);
        if (cached->image->surface != shoes_world->blank_image)
            cairo_surface_destroy(cached->image->surface);
        free(cached->image);
    }
    free(cached);
    free(key_str);
    return ST_CONTINUE;
}

void shoes_world_free(shoes_world_t *world) {
    shoes_native_cleanup(world);
    st_foreach(world->image_cache, CASTFOREACH(shoes_world_free_image_cache), 0);
    st_free_table(world->image_cache);
    SHOE_FREE(world->blank_cache);
    cairo_surface_destroy(world->blank_image);
    pango_font_description_free(world->default_font);
    rb_gc_unregister_address(&world->apps);
    rb_gc_unregister_address(&world->msgs);
    if (world != NULL)
        SHOE_FREE(world);
}

int shoes_ruby_embed() {
    VALUE v;
    char *argv[] = {"ruby", "-e", "1"};
    
    fprintf(stderr, "[SHOES] shoes_ruby_embed: Starting Ruby initialization\n");
    
    // Ruby 3.x initialization - simplified approach
    int argc = 0;
    char *dummy = "";
    char **args = &dummy;
    
#ifdef SHOES_WIN32
    //ruby_sysinit(0, 0);
#else
    // Initialize Ruby's view of the stack
    fprintf(stderr, "[SHOES] shoes_ruby_embed: Calling ruby_init_stack\n");
    VALUE variable_in_this_stack_frame;
    ruby_init_stack(&variable_in_this_stack_frame);
#endif
    
    fprintf(stderr, "[SHOES] shoes_ruby_embed: Calling ruby_sysinit\n");
    ruby_sysinit(&argc, &args);
    
    fprintf(stderr, "[SHOES] shoes_ruby_embed: Calling ruby_init\n");
    ruby_init();
    
    fprintf(stderr, "[SHOES] shoes_ruby_embed: Calling ruby_options with argc=3\n");
    fprintf(stderr, "[SHOES] shoes_ruby_embed: argv[0]=%s, argv[1]=%s, argv[2]=%s\n", 
            argv[0], argv[1], argv[2]);
    v = (VALUE)ruby_options(3, argv);
    
    fprintf(stderr, "[SHOES] shoes_ruby_embed: ruby_options returned, checking result\n");
    return !FIXNUM_P(v);
}

shoes_code shoes_init(SHOES_INIT_ARGS) {
    fprintf(stderr, "[SHOES] shoes_init: Starting\n");
#ifdef SHOES_SIGNAL
    signal(SIGINT,  shoes_sigint);
#ifndef SHOES_GTK_WIN32
    signal(SIGQUIT, shoes_sigint);
#endif
#endif
#ifdef SHOES_QUARTZ
    // init some OSX things our way before Ruby inits.
    fprintf(stderr, "[SHOES] shoes_init: Calling shoes_osx_setup_stdout\n");
    shoes_osx_setup_stdout();
#endif
    fprintf(stderr, "[SHOES] shoes_init: Calling shoes_ruby_embed\n");
    shoes_ruby_embed();  // initialize ruby
    
    fprintf(stderr, "[SHOES] shoes_init: Calling shoes_ruby_init\n");
    shoes_ruby_init();  // MUST be called before any Ruby API usage
    
    fprintf(stderr, "[SHOES] shoes_init: Calling shoes_world_alloc\n");
    shoes_world = shoes_world_alloc();
    fprintf(stderr, "[SHOES] shoes_init: shoes_world_alloc returned, shoes_world=%p\n", (void*)shoes_world);
    
#ifdef SHOES_WIN32
    shoes_world->os.instance = inst;
    shoes_world->os.style = style;
#endif
    // parse shoes.yaml file and update settings class/object
    fprintf(stderr, "[SHOES] shoes_init: Calling shoes_init_load_yaml\n");
    shoes_init_load_yaml(path);
    
    fprintf(stderr, "[SHOES] shoes_init: Calling shoes_native_init\n");
    shoes_native_init();

    // Defer setting FONTS constant - it causes crashes during initialization
    // TODO: Fix this properly for Ruby 3.x
    // if (cShoes != Qnil && cShoes != 0) {
    //     rb_const_set(cShoes, rb_intern("FONTS"), shoes_font_list());
    // }
    fprintf(stderr, "[SHOES] shoes_init: Done, returning SHOES_OK\n");
    return SHOES_OK;
}

void shoes_update_fonts(VALUE ary) {
#if PANGO_VERSION_MAJOR > 1 || PANGO_VERSION_MINOR >= 22
    pango_cairo_font_map_set_default(NULL);
#endif
    rb_funcall(rb_const_get(cShoes, rb_intern("FONTS")), rb_intern("replace"), 1, ary);
}

static VALUE shoes_load_begin(VALUE v) {
    char *bootup = (char *)v;
    return rb_eval_string(bootup);
}

static VALUE shoes_load_exception(VALUE v, VALUE exc) {
    return exc;
}

shoes_code shoes_load(char *path) {
    char bootup[SHOES_BUFSIZE];

    fprintf(stderr, "[SHOES] shoes_load: Starting with path=%s\n", path ? path : "(null)");
    if (path) {
        sprintf(bootup, "Shoes.visit(%%q<%s>);", path);
        fprintf(stderr, "[SHOES] shoes_load: Calling rb_rescue2 with bootup: %s\n", bootup);

        VALUE v = rb_rescue2(CASTHOOK(shoes_load_begin), (VALUE)bootup, CASTHOOK(shoes_load_exception), Qnil, rb_cObject, 0);
        fprintf(stderr, "[SHOES] shoes_load: rb_rescue2 returned, v=%p\n", (void*)v);
        
        // Check type first to avoid crashes
        if (TYPE(v) != T_NONE && TYPE(v) != T_UNDEF) {
            fprintf(stderr, "[SHOES] shoes_load: v type=%d\n", TYPE(v));
            if (rb_obj_is_kind_of(v, rb_eException)) {
                fprintf(stderr, "[SHOES] shoes_load: v is an exception, handling error\n");
                shoes_canvas_error(Qnil, v);
                rb_eval_string("Shoes.show_log");
            }
        }
    }
    fprintf(stderr, "[SHOES] shoes_load: Done, returning SHOES_OK\n");
    return SHOES_OK;
}

void shoes_set_argv(int argc, char **argv) {
    ruby_set_argv(argc, argv);
}

#ifdef SHOES_QUARTZ
static VALUE shoes_start_begin(VALUE v) {
    char str[128];
    sprintf(str, "$SHOES_URI = Shoes.args!(%d)", osx_cshoes_launch);
    return rb_eval_string(str);
}
#else
static VALUE shoes_start_begin(VALUE v) {
    return rb_eval_string("$SHOES_URI = Shoes.args!");
}
#endif

static VALUE shoes_start_exception(VALUE v, VALUE exc) {
    return exc;
}

shoes_code shoes_start(char *path, char *uri, int debug) {
    fprintf(stderr, "[SHOES] shoes_start: Starting with path=%s, uri=%s, debug=%d\n", path, uri, debug);
    shoes_code code = SHOES_OK;
    char bootup[SHOES_BUFSIZE];
    char dbstr[64];
    sprintf(dbstr, "SHOES_DEBUG=%s", (debug ? "true" : "false"));
    
    fprintf(stderr, "[SHOES] shoes_start: Evaluating debug string: %s\n", dbstr);
    rb_eval_string(dbstr);
    
    fprintf(stderr, "[SHOES] shoes_start: Building bootup string\n");
    int len = shoes_snprintf(bootup,
                             SHOES_BUFSIZE,
                             "begin;"
                             "DIR = File.expand_path(File.dirname(%%q<%s>));"
#ifdef OLD_SHOES
                             "$:.replace([DIR+'/ruby/lib/'+(ENV['SHOES_RUBY_ARCH'] || RUBY_PLATFORM), DIR+'/ruby/lib', DIR+'/lib', '.']);"
#else
                             "$:.unshift(DIR+'/lib');"
                             "$:.unshift('.');"
#endif
                             // Load minimal shoes for testing
                             "$stderr.puts 'Loading shoes_minimal from ' + DIR + '/lib';"
                             "require 'shoes_minimal';"
                             "$stderr.puts 'After require shoes_minimal';"
                             "DIR;"
                             "rescue Object => e;"
                             "$stderr.puts('Error: ' + e.message);"
                             "$stderr.puts(e.backtrace.join(\"\\n\"));"
                             "end",
                             path);

    if (len < 0 || len >= SHOES_BUFSIZE) {
        QUIT("Path to script is too long.");
    }

    fprintf(stderr, "[SHOES] shoes_start: Evaluating bootup string (len=%d)\n", len);
    // Don't print the bootup string itself - it's too verbose
    VALUE str = rb_eval_string(bootup);
    fprintf(stderr, "[SHOES] shoes_start: rb_eval_string returned, str=%p\n", (void*)str);
    
    fprintf(stderr, "[SHOES] shoes_start: Checking if str is NIL\n");
    if (NIL_P(str)) {
        fprintf(stderr, "[SHOES] shoes_start: str is NIL, returning SHOES_QUIT\n");
        return SHOES_QUIT;
    }

    fprintf(stderr, "[SHOES] shoes_start: str is not NIL, converting to string\n");
    StringValue(str);
    fprintf(stderr, "[SHOES] shoes_start: Copying to shoes_world->path\n");
    strcpy(shoes_world->path, RSTRING_PTR(str));

    fprintf(stderr, "[SHOES] shoes_start: Calling rb_rescue2 for shoes_start_begin\n");
    char *load_uri_str = NULL;
    VALUE load_uri = rb_rescue2(CASTHOOK(shoes_start_begin), Qnil, CASTHOOK(shoes_start_exception), Qnil, rb_cObject, 0);
    fprintf(stderr, "[SHOES] shoes_start: rb_rescue2 returned, load_uri=%p\n", (void*)load_uri);
    if (!RTEST(load_uri)) {
        fprintf(stderr, "[SHOES] shoes_start: load_uri is false/nil, returning SHOES_QUIT\n");
        return SHOES_QUIT;
    }
    
    // Check if it's a string first (the normal case)
    if (TYPE(load_uri) == T_STRING) {
        fprintf(stderr, "[SHOES] shoes_start: load_uri is a string\n");
        load_uri_str = RSTRING_PTR(load_uri);
    } else {
        fprintf(stderr, "[SHOES] shoes_start: load_uri type=%d, checking if exception\n", TYPE(load_uri));
        // Only check for exception if it's not a string
        fprintf(stderr, "[SHOES] shoes_start: Checking if load_uri is exception (rb_eException=%p)\n", (void*)rb_eException);
        if (rb_obj_is_kind_of(load_uri, rb_eException)) {
            fprintf(stderr, "[SHOES] shoes_start: load_uri is an exception\n");
            // QUIT_ALERT macro causes crashes in Ruby 3.x, use simpler approach
            VALUE msg = rb_funcall(load_uri, rb_intern("message"), 0);
            if (TYPE(msg) == T_STRING) {
                fprintf(stderr, "Shoes Error: %s\n", RSTRING_PTR(msg));
            } else {
                fprintf(stderr, "Shoes Error: (unable to get error message)\n");
            }
            return SHOES_QUIT;
        }
    }

    code = shoes_load(load_uri_str);
    fprintf(stderr, "[SHOES] shoes_start: shoes_load returned code=%d\n", code);
    if (code != SHOES_OK)
        goto quit;

    fprintf(stderr, "[SHOES] shoes_start: Calling shoes_app_start with apps=%p, uri=%s\n", 
            (void*)shoes_world->apps, uri);
    code = shoes_app_start(shoes_world->apps, uri);
    fprintf(stderr, "[SHOES] shoes_start: shoes_app_start returned code=%d\n", code);
quit:
    return code;
}

shoes_code shoes_final() {
    fprintf(stderr, "[SHOES] shoes_final: Starting cleanup\n");
    
    // Check if Ruby and cShoes were properly initialized before calling
    fprintf(stderr, "[SHOES] shoes_final: cShoes=%p, Qnil=%p\n", (void*)cShoes, (void*)Qnil);
    if (cShoes != Qnil && cShoes != 0) {
        fprintf(stderr, "[SHOES] shoes_final: Checking if cShoes responds to 'clean'\n");
        if (rb_respond_to(cShoes, rb_intern("clean"))) {
            fprintf(stderr, "[SHOES] shoes_final: Calling Shoes.clean\n");
            rb_funcall(cShoes, rb_intern("clean"), 0);
        } else {
            fprintf(stderr, "[SHOES] shoes_final: cShoes doesn't respond to 'clean', skipping\n");
        }
    }
    
    fprintf(stderr, "[SHOES] shoes_final: Freeing shoes_world\n");
    if (shoes_world != NULL) {
        shoes_world_free(shoes_world);
    }
    
    fprintf(stderr, "[SHOES] shoes_final: Done\n");
    return SHOES_OK;
}

/*
 * load the shoes.yaml file (if it exists and is formatted properly)
 * borrowed from https://stackoverflow.com/questions/20628099/parsing-yaml-to-values-with-libyaml-in-c
*/
#include <stdio.h>
#include <string.h>
#include <yaml.h>
// Another C global
shoes_yaml_init *shoes_config_yaml = NULL; 

int shoes_init_load_yaml(char *path) {
    yaml_parser_t parser;
    yaml_token_t token;
    if (!yaml_parser_initialize(&parser)) {
        fputs("Failed to initialize yaml parser!\n", stderr);
        return 0;
    }
    // set defaults
    shoes_config_yaml = malloc(sizeof(shoes_yaml_init));
    shoes_config_yaml->app_name = "Shoes";
    shoes_config_yaml->icon_path = "static/app-icon.png";
    shoes_config_yaml->theme_name = NULL;
    shoes_config_yaml->active = FALSE;
    shoes_config_yaml->rdomain = "com.shoesrb.shoes";
    shoes_config_yaml->use_menus = "false";
    shoes_config_yaml->mdi = "false";
    shoes_config_yaml->backend = NULL;
    shoes_config_yaml->extra1 = NULL;
    shoes_config_yaml->extra2 = NULL;
    shoes_config_yaml->osx_menutrim = "false";
    shoes_config_yaml->image_cache = "true";
    
    // check current dir (script location) first
    FILE* fh = fopen("startup.yaml", "r");
    if (fh == NULL) {
      // check where Shoes was launched from - argv[0] sort-of). 
      // We don't have DIR yet so do what it does
      char buf[200];
      sprintf(buf, "File.expand_path(File.dirname(%%q<%s>));", path);
      VALUE dpv = rb_eval_string(buf);
      char *dp = RSTRING_PTR(dpv);
      strcpy(buf, dp);
      strcat(buf,"/startup.yaml");
      fh = fopen(buf, "r");
    }
    if (fh != NULL) {
      yaml_parser_set_input_file(&parser, fh);
      /* As this is an example, I'll just use:
       *  state = 0 = expect key
       *  state = 1 = expect value
       */
      int state = 0;
      char** datap;
      char* tk;
  
      do {
          yaml_parser_scan(&parser, &token);
          switch(token.type) {
              case YAML_KEY_TOKEN:     state = 0; break;
              case YAML_VALUE_TOKEN:   state = 1; break;
              case YAML_SCALAR_TOKEN:
                  tk = (char *)token.data.scalar.value;
                  if (state == 0) {
                      /* It's safe to not use strncmp as 
                         one string is a literal */
                      if (!strcmp(tk, "App_Name")) {
                          datap = &shoes_config_yaml->app_name;
                      } else if (!strcmp(tk,"Icon_Path")) {
                          datap = &shoes_config_yaml->icon_path;
                      } else if (!strcmp(tk, "Theme")) {
                          datap = &shoes_config_yaml->theme_name;
                      } else if (!strcmp(tk,"RDomain")) {
                          datap = &shoes_config_yaml->rdomain;
                      } else if (!strcmp(tk,"Use_Menus")) {
                          datap = &shoes_config_yaml->use_menus;
                      } else if (!strcmp(tk,"MDI")) {
                          datap = &shoes_config_yaml->mdi;
                      } else if (!strcmp(tk,"Extra1")) {
                          datap = &shoes_config_yaml->extra1;
                      } else if (!strcmp(tk,"Extra2")) {
                          datap = &shoes_config_yaml->extra2;
                      } else if (!strcmp(tk, "Image_Cache")) {
                          datap = &shoes_config_yaml->image_cache;
                      } else if (!strcmp(tk,"OSX_Menu_Trim")) {
                          datap = &shoes_config_yaml->osx_menutrim;
                      } else if (!strcmp(tk,"Display_Backend")) {
                          datap = &shoes_config_yaml->backend; 
                      } else {
                          printf("Unrecognised key: %s\n", tk);
                          return 0;
                      }
                  } else {
                        *datap = strdup(tk);
                  }
                  break;
             default: break;
             }
         if (token.type != YAML_STREAM_END_TOKEN)
             yaml_token_delete(&token);
      } while (token.type != YAML_STREAM_END_TOKEN);
  
      yaml_token_delete(&token);
      yaml_parser_delete(&parser);
      fclose(fh);
    }
    shoes_config_yaml->active = TRUE;
    shoes_settings_new(shoes_config_yaml);
    return 1;
}

#ifdef __cplusplus
}
#endif
