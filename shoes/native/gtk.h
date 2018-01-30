#define GTK_CHILD(child, ptr) \
  GList *children = gtk_container_get_children(GTK_CONTAINER(ptr)); \
  child = children->data
// some functions that only gtkwidets know about in gtk.c
void shoes_css_parse_error (GtkCssProvider *provider,
               GtkCssSection  *section,
               GError         *error,
               gpointer        user_data);
