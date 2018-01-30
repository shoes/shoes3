#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtkswitch.h"
#include "shoes/native/gtk.h"
extern VALUE cColor;
static char *css_template = "GtkSwitch {\n font: %s;\n color: %s;\n}\n";

SHOES_CONTROL_REF shoes_native_switch(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_switch_new();

    if (!NIL_P(shoes_hash_get(attr, rb_intern("active")))) {
        gtk_switch_set_active(GTK_SWITCH(ref), shoes_hash_get(attr, rb_intern("active")) == Qtrue);
    }
    // change font and color/stroke

    // default css values
    char *font = "Arial 12";
    char color[40] = "black";
    int do_sub = 0;
    int have_color = 0;
    VALUE vclr = Qnil;
    VALUE vfont = ATTR(attr, font);
    if (! NIL_P(vfont)) {
      font = RSTRING_PTR(vfont);
      do_sub = 1;
    }
    if (RTEST(ATTR(attr, stroke))) {
      vclr = (ATTR(attr, stroke));
      // Thatis a Shoes color, turn it into a css rgba
      shoes_color *scolor; 
      Data_Get_Struct(vclr, shoes_color, scolor); 
      sprintf(color, "rgba(%d,%d,%d,%d)", scolor->r, scolor->g, scolor->b,
          scolor->a);        
      do_sub = 1;
    }
    if (do_sub) {
      /* Change default font and color through widget css */
      GtkCssProvider *provider;
      GtkStyleContext *context;
      char new_css[100]; 
      sprintf(new_css, css_template, font, color);
      //printf("css: %s", new_css);
      provider = gtk_css_provider_new ();
      g_signal_connect(G_OBJECT(provider), "parsing-error",
                     G_CALLBACK(shoes_css_parse_error),
                     (gpointer)self);
      gtk_css_provider_load_from_data(provider, new_css, -1, NULL);
      context = gtk_widget_get_style_context ((GtkWidget *)ref);
      gtk_style_context_add_provider (context,
            GTK_STYLE_PROVIDER (provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      // check what's really in provider ?
      //printf("provider has: %s\n", gtk_css_provider_to_string(provider));
    }    
    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    g_signal_connect(G_OBJECT(ref), "notify::active",
                     G_CALLBACK(shoes_native_activate),
                     (gpointer)self);

    return ref;
}

void shoes_native_switch_set_active(SHOES_CONTROL_REF ref, int activate) {
    gtk_switch_set_active(GTK_SWITCH(ref), activate);
}

VALUE shoes_native_switch_get_active(SHOES_CONTROL_REF ref) {
    return gtk_switch_get_active(GTK_SWITCH(ref)) ? Qtrue : Qfalse;
}

static void shoes_native_activate(GObject *switcher, GParamSpec *pspec, gpointer data) {
    VALUE self = (VALUE)data;

    shoes_control_send(self, s_active);
}
