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
    shoes_css_apply((GtkWidget*)ref, attr, css_template);  

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
