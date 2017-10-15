#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/internal.h"

#include "shoes/native/gtk/gtkentryalt.h"
#include "shoes/native/gtk/gtkeditline.h"
extern VALUE cColor;
SHOES_CONTROL_REF shoes_native_edit_line(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_entry_alt_new();

    if (RTEST(ATTR(attr, secret))) shoes_native_secrecy(ref);
    
    if (RTEST(ATTR(attr, font))) {
      char *fontstr = RSTRING_PTR(ATTR(attr, font));
      PangoFontDescription *fontdesc = NULL;
      fontdesc = pango_font_description_from_string(fontstr);
      // deprecated in gtk 3.16 - use private css - ugh. 
      gtk_widget_override_font(ref, fontdesc);
    }
    if (RTEST(ATTR(attr, stroke))) {
      VALUE fgclr = ATTR(attr, stroke);
      if (TYPE(fgclr) == T_STRING) 
          fgclr = shoes_color_parse(cColor, fgclr);  // convert string to cColor
      if (rb_obj_is_kind_of(fgclr, cColor)) { 
          shoes_color *color; 
          Data_Get_Struct(fgclr, shoes_color, color); 
          GdkRGBA gclr; 
          gclr.red = color->r / 255.0;
          gclr.green = color->g / 255.0; 
          gclr.blue = color->b / 255.0;
          gclr.alpha = color->a / 255.0;
          gtk_widget_override_color(ref, GTK_STATE_FLAG_NORMAL, &gclr);
      }
    }

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    gtk_entry_set_text(GTK_ENTRY(ref), _(msg));

    g_signal_connect(G_OBJECT(ref), "changed",
                     G_CALLBACK(shoes_widget_changed),
                     (gpointer)self);
    // cjc: try to intercept \n  bug 860 @ shoes4
    g_signal_connect(G_OBJECT(ref), "activate",
                     G_CALLBACK(shoes_native_enterkey), // fix name?
                     (gpointer)self);

    return ref;
}

VALUE shoes_native_edit_line_get_text(SHOES_CONTROL_REF ref) {
    return rb_str_new2(gtk_entry_get_text(GTK_ENTRY(ref)));
}

void shoes_native_edit_line_set_text(SHOES_CONTROL_REF ref, char *msg) {
    gtk_entry_set_text(GTK_ENTRY(ref), _(msg));
}

VALUE shoes_native_edit_line_cursor_to_end(SHOES_CONTROL_REF ref) {
    gtk_editable_set_position(GTK_EDITABLE(ref), -1);
    return Qnil;
}

void shoes_native_enterkey(GtkWidget *ref, gpointer data) {
    VALUE self = (VALUE)data;
    GET_STRUCT(control, self_t);
    VALUE click = ATTR(self_t->attr, donekey);
    if (!NIL_P(click)) {
        shoes_safe_block(self_t->parent, click, rb_ary_new3(1, self));
    }
}
