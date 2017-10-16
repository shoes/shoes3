#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/color.h"
#include "shoes/internal.h"

#include "shoes/native/gtk.h"
#include "shoes/native/gtk/gtkscrolledwindowalt.h"
#include "shoes/native/gtk/gtkeditbox.h"
extern VALUE cColor;
static char *css_template = "textview {\n font: %s;\n color: %s;\n}\n";
                                 
SHOES_CONTROL_REF shoes_native_edit_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    GtkTextBuffer *buffer;
    GtkWidget* textview = gtk_text_view_new();
    SHOES_CONTROL_REF ref = gtk_scrolled_window_alt_new(NULL, NULL);
    

#if 1
    // default css values
    char *font = "Arial 12";
    char *color = "black";
    int do_sub = 0;
    int have_color = 0;
    if (RTEST(ATTR(attr, font))) {
      font = RSTRING_PTR(ATTR(attr, font));
      do_sub = 1;
    }
    if (RTEST(ATTR(attr, stroke))) {
      color = RSTRING_PTR(ATTR(attr, stroke));
      do_sub = 1;
    }
    if (do_sub) {
      /* Change default font and color through widget css */
      GtkCssProvider *provider;
      GtkStyleContext *context;
      char new_css[100]; 
      sprintf(new_css, css_template, font, color);
      printf("css: %s", new_css);
      provider = gtk_css_provider_new ();
      gtk_css_provider_load_from_data(provider, new_css, -1, NULL);
      context = gtk_widget_get_style_context (textview);
      gtk_style_context_add_provider (context,
            GTK_STYLE_PROVIDER (provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      // check what's really in provider ?
      printf("prov: %s\n", gtk_css_provider_to_string(provider));
    }
#else
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
          // override color doesn't work, deprecated if it did.
          gtk_widget_override_color(ref, GTK_STATE_FLAG_NORMAL, &gclr);
      }
    }
#endif
    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, _(msg), -1);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ref),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(ref), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(ref), textview);

    g_signal_connect(G_OBJECT(buffer), "changed",
                     G_CALLBACK(shoes_widget_changed),
                     (gpointer)self);

    return ref;
}

VALUE shoes_native_edit_box_get_text(SHOES_CONTROL_REF ref) {
    GtkWidget *textview;
    GTK_CHILD(textview, ref);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    GtkTextIter begin, end;
    gtk_text_buffer_get_bounds(buffer, &begin, &end);
    return rb_str_new2(gtk_text_buffer_get_text(buffer, &begin, &end, TRUE));
}

void shoes_native_edit_box_set_text(SHOES_CONTROL_REF ref, char *msg) {
    GtkWidget *textview;
    GTK_CHILD(textview, ref);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, _(msg), -1);
}

void shoes_native_edit_box_append(SHOES_CONTROL_REF ref, char *msg) {
    GtkWidget *textview;
    GTK_CHILD(textview, ref);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    GtkTextIter begin, end;
    gtk_text_buffer_get_bounds(buffer, &begin, &end);
    gtk_text_buffer_insert(buffer, &end, msg, strlen(msg));
    gtk_text_buffer_get_bounds(buffer, &begin, &end);

}

void shoes_native_edit_box_scroll_to_end(SHOES_CONTROL_REF ref) {
    GtkWidget *textview;
    GTK_CHILD(textview, ref);
    GtkTextIter end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_get_end_iter (buffer, &end);
    /* get the current ( cursor )mark name */
    GtkTextMark *insert_mark = gtk_text_buffer_get_insert (buffer);

    /* move mark and selection bound to the end */
    gtk_text_buffer_place_cursor(buffer, &end);

    /* scroll to the end view */
    gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (textview),
                                  insert_mark, 0.0, TRUE, 0.0, 1.0);
}
