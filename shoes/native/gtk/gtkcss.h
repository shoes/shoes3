#ifndef SHOES_GTK_CSSH
#define SHOES_GTK_CSSH 
void shoes_css_parse_error (GtkCssProvider *provider,
               GtkCssSection  *section,
               GError         *error,
               gpointer        user_data);
void shoes_css_apply(GtkWidget *widget, VALUE attr, char *css_template);
#endif
