#ifndef SHOES_GTK_CSSH
#define SHOES_GTK_CSSH 
void shoes_css_parse_error (GtkCssProvider *provider,
               GtkCssSection  *section,
               GError         *error,
               gpointer        user_data);
void shoes_css_apply(GtkWidget *widget, VALUE attr, char *css_template);
void shoes_css_apply_font(GtkWidget *widget, char *fontarg, char *css_template);
void shoes_css_apply_font_and_colors(GtkWidget *widget, char *fontarg, 
    char *fgclr, char *bgclr, char *css_template);
#endif
