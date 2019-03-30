
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/types/text.h"
#include "shoes/types/text_link.h"
#include "shoes/types/download.h"
#include "shoes/types/event.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtkcss.h"

/* 
 * TODO: Print more intelligent errors - should it ever get called.
*/ 
void shoes_css_parse_error (GtkCssProvider *provider,
               GtkCssSection  *section,
               GError         *error,
               gpointer        user_data) 
{
  fprintf(stderr,"css parse error\n");
}

// terminal uses this:
void shoes_css_apply_font(GtkWidget *widget, char *fontarg, char *css_template) 
{
  /* Change default font and color through widget css */
  GtkCssProvider *provider;
  GtkStyleContext *context;
  char new_css[100]; 
  sprintf(new_css, css_template, fontarg, "black");
  //printf("css: %s", new_css);
  provider = gtk_css_provider_new ();
  g_signal_connect(G_OBJECT(provider), "parsing-error",
                 G_CALLBACK(shoes_css_parse_error),
                 (gpointer)NULL);
  gtk_css_provider_load_from_data(provider, new_css, -1, NULL);
  context = gtk_widget_get_style_context (widget);
  gtk_style_context_add_provider (context,
        GTK_STYLE_PROVIDER (provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  // check what's really in provider ?
  //printf("provider has: %s\n", gtk_css_provider_to_string(provider));
}

// terminal uses this:
void shoes_css_apply_font_and_colors(GtkWidget *widget, char *fontarg, 
    char *fgclr, char* bgclr, char *css_template) 
{
  /* Change default font and color through widget css */
  GtkCssProvider *provider;
  GtkStyleContext *context;
  char new_css[100]; 
  sprintf(new_css, css_template, fontarg, fgclr, bgclr);
  //printf("css: %s", new_css);
  provider = gtk_css_provider_new ();
  g_signal_connect(G_OBJECT(provider), "parsing-error",
                 G_CALLBACK(shoes_css_parse_error),
                 (gpointer)NULL);
  gtk_css_provider_load_from_data(provider, new_css, -1, NULL);
  context = gtk_widget_get_style_context (widget);
  gtk_style_context_add_provider (context,
        GTK_STYLE_PROVIDER (provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  // check what's really in provider ?
  //printf("provider has: %s\n", gtk_css_provider_to_string(provider));
}

// The more general api using Shoes attr
void shoes_css_apply(GtkWidget *widget, VALUE attr, char *css_template)
{
    // default css values
    char *font = "Arial 12";
    char color[40] = "black";
    int do_sub = 0;
    VALUE vclr = Qnil;
    VALUE vfont = ATTR(attr, font);
    if (! NIL_P(vfont)) {
      font = RSTRING_PTR(vfont);
      do_sub = 1;
    }
    if (RTEST(ATTR(attr, stroke))) {
      vclr = (ATTR(attr, stroke));
      // That's a Shoes color turn it into a css rgba
      Get_TypedStruct2(vclr, shoes_color, scolor); 
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
                     (gpointer)NULL);
      gtk_css_provider_load_from_data(provider, new_css, -1, NULL);
      context = gtk_widget_get_style_context (widget);
      gtk_style_context_add_provider (context,
            GTK_STYLE_PROVIDER (provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      // check what's really in provider ?
      //printf("provider has: %s\n", gtk_css_provider_to_string(provider));
    }
}
