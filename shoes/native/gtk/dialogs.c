
#ifndef GTK3
// fail only used for shoes_native_window_color will be deleted
#define GTK3
#endif
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/types/settings.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/types/text.h"
#include "shoes/types/text_link.h"
#include "shoes/types/download.h"
#include "shoes/types/event.h"
#include "shoes/internal.h"
#include "shoes/types/menubar.h"
#include "shoes/native/gtk/gtkmenus.h"


/* --------------- dialogs -----------*/
extern GtkCssProvider *shoes_css_provider; // user provided theme


#if defined(GTK3)
VALUE shoes_native_window_color(shoes_app *app) {
    GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(APP_WINDOW(app)));
    GdkRGBA bg;
#ifdef BSD // assumes Gtk 3.22
    gtk_style_context_lookup_color(style, (char *)NULL, &bg);
#else
    gtk_style_context_lookup_color(style, GTK_STATE_NORMAL, &bg);
#endif
    return shoes_color_new((int)(bg.red * 255), (int)(bg.green * 255),
                           (int)(bg.blue * 255), SHOES_COLOR_OPAQUE);
}

VALUE shoes_native_dialog_color(shoes_app *app) {
    GdkRGBA bg;
    GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(APP_WINDOW(app)));
#ifdef BSD // assumes Gtk 3.22
    gtk_style_context_lookup_color(style, (char *)NULL, &bg);
#else
    gtk_style_context_lookup_color(style, GTK_STATE_NORMAL, &bg);
#endif
    return shoes_color_new((int)(bg.red * 255), (int)(bg.green * 255),
                           (int)(bg.blue * 255), SHOES_COLOR_OPAQUE);
}
#else
VALUE shoes_native_window_color(shoes_app *app) {
    GtkStyle *style = gtk_widget_get_style(GTK_WIDGET(APP_WINDOW(app)));
    GdkColor bg = style->bg[GTK_STATE_NORMAL];
    return shoes_color_new(bg.red / 257, bg.green / 257, bg.blue / 257, SHOES_COLOR_OPAQUE);
}

VALUE shoes_native_dialog_color(shoes_app *app) {
    GtkStyle *style = gtk_widget_get_style(GTK_WIDGET(APP_WINDOW(app)));
    GdkColor bg = style->bg[GTK_STATE_NORMAL];
    return shoes_color_new(bg.red / 257, bg.green / 257, bg.blue / 257, SHOES_COLOR_OPAQUE);
}
#endif

VALUE shoes_dialog_alert(int argc, VALUE *argv, VALUE self) {
    GTK_APP_VAR(app);
    //char atitle[50]; // bug432 
    char atitle[192];
    g_sprintf(atitle, "%s says", title_app);
    rb_arg_list args;
    rb_parse_args(argc, argv, "S|h", &args);
    char *msg = RSTRING_PTR(shoes_native_to_s(args.a[0]));

    gchar *format_string = "<span size='larger'>%s</span>\n\n%s";
    if (argc == 2) {
        if (RTEST(ATTR(args.a[1], title))) {
            VALUE tmpstr = ATTR(args.a[1], title);
            strcpy(atitle,RSTRING_PTR(shoes_native_to_s(tmpstr)));
        } else {
            g_stpcpy(atitle," ");
        }
    }

    GtkWidget *dialog = gtk_message_dialog_new_with_markup(
                            window_app, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                            format_string, atitle, msg );

    // theme the window
    if (shoes_css_provider != NULL) {
      gtk_style_context_add_provider(gtk_widget_get_style_context(dialog),
          GTK_STYLE_PROVIDER(shoes_css_provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return Qnil;
}

VALUE shoes_dialog_ask(int argc, VALUE *argv, VALUE self) {
    char atitle[192];
    GTK_APP_VAR(app);

    VALUE answer = Qnil;
    rb_arg_list args;
    rb_parse_args(argc, argv, "S|h", &args);

    switch(argc) {
        case 1:
            sprintf(atitle, "%s asks", title_app);
            break;
        case 2:
            if (RTEST(ATTR(args.a[1], title))) {
                VALUE tmpstr = ATTR(args.a[1], title);
                strcpy(atitle, RSTRING_PTR(shoes_native_to_s(tmpstr)));
            } else {
                g_stpcpy(atitle," ");
            }
            break;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons(atitle, window_app, GTK_DIALOG_MODAL,
                        _("_Cancel"), GTK_RESPONSE_CANCEL, _("_OK"), GTK_RESPONSE_OK, NULL);

    // theme the window
    if (shoes_css_provider != NULL) {
      gtk_style_context_add_provider(gtk_widget_get_style_context(dialog),
          GTK_STYLE_PROVIDER(shoes_css_provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);
    gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 6);
    GtkWidget *question = gtk_label_new(RSTRING_PTR(shoes_native_to_s(args.a[0])));
    // TODO: is this really needed?
    if (gtk_get_minor_version() < 14){
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
     gtk_misc_set_alignment(GTK_MISC(question), 0, 0);
#pragma GCC diagnostic pop
    }
    GtkWidget *_answer = gtk_entry_new();
    if (RTEST(ATTR(args.a[1], secret))) shoes_native_secrecy(_answer);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), question, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), _answer, FALSE, TRUE, 3);

    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        const gchar *txt = gtk_entry_get_text(GTK_ENTRY(_answer));
        answer = rb_str_new2(txt);
    }
    gtk_widget_destroy(dialog);
    return answer;
}


VALUE shoes_dialog_confirm(int argc, VALUE *argv, VALUE self) {
    VALUE answer = Qfalse;
    char atitle[192];
    GTK_APP_VAR(app);
    //char *apptitle = RSTRING_PTR(app->title);
    rb_arg_list args;
    rb_parse_args(argc, argv, "S|h", &args);
    VALUE quiz = shoes_native_to_s(args.a[0]);

    switch(argc) {
        case 1:
            sprintf(atitle, "%s asks", title_app);
            break;
        case 2:
            if (RTEST(ATTR(args.a[1], title))) {
                VALUE tmpstr = ATTR(args.a[1], title);
                strcpy(atitle, RSTRING_PTR(shoes_native_to_s(tmpstr)));
            } else {
                g_stpcpy(atitle," ");
            }
            break;
    }



    GtkWidget *dialog = gtk_dialog_new_with_buttons(atitle, window_app, GTK_DIALOG_MODAL,
                        _("_Cancel"), GTK_RESPONSE_CANCEL, _("_OK"), GTK_RESPONSE_OK, NULL);
    // theme the window
    if (shoes_css_provider != NULL) {
      gtk_style_context_add_provider(gtk_widget_get_style_context(dialog),
          GTK_STYLE_PROVIDER(shoes_css_provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);
    gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 6);

    GtkWidget *question = gtk_label_new(RSTRING_PTR(quiz));
    // TODO is misc really needed?
    if (gtk_get_minor_version() < 14){
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
     gtk_misc_set_alignment(GTK_MISC(question), 0, 0);
#pragma GCC diagnostic pop
    }
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), question, FALSE, FALSE, 3);

    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK)
        answer = Qtrue;
    gtk_widget_destroy(dialog);
    return answer;

}

VALUE shoes_dialog_color(VALUE self, VALUE title) {
    VALUE color = Qnil;
    GTK_APP_VAR(app);
    title = shoes_native_to_s(title);
    GtkWidget *dialog = gtk_color_chooser_dialog_new(RSTRING_PTR(title), NULL);

    // theme the window
    if (shoes_css_provider != NULL) {
      gtk_style_context_add_provider(gtk_widget_get_style_context(dialog),
          GTK_STYLE_PROVIDER(shoes_css_provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_OK) {
        GdkRGBA _color;
        gtk_color_chooser_get_rgba((GtkColorChooser *)dialog, &_color);
        color = shoes_color_new((int)(_color.red*255), (int)(_color.green*255),
                                (int)(_color.blue*255), (int)(_color.alpha*255));
    }

    gtk_widget_destroy(dialog);
    return color;
}

VALUE shoes_dialog_chooser(VALUE self, char *title, GtkFileChooserAction act, const gchar *button, VALUE attr) {
    VALUE path = Qnil;
#if 0
  GTK_APP_VAR(app);
#else
  //VALUE clsv = rb_funcall2(self, rb_intern("inspect"), 0, Qnil);
  //char *clsname = RSTRING_PTR(clsv);
  //printf("self is %s - > ", clsname);
  char * title_app = "Shoes"; 
  GtkWindow *window_app = NULL; 
  shoes_app *app = NULL; 
  if ( rb_obj_is_kind_of(self,cApp)) {
      // Normal 
      app = Get_TypedStruct3(self, shoes_app);
      title_app = RSTRING_PTR(app->title); 
      window_app = APP_WINDOW(app);
  } else {
    // Is it Shoes splash? 
    if (RARRAY_LEN(shoes_world->apps) > 0) { 
      VALUE actual_app = rb_ary_entry(shoes_world->apps, 0);
      //app = Get_TypedStruct3(self, shoes_app); // dies here
      app = Get_TypedStruct3(actual_app, shoes_app); 
      title_app = RSTRING_PTR(app->title); 
      window_app = APP_WINDOW(app);
    } else {
      // outside an app and not splash - no window. Gtk complains but runs. 
      /*
      VALUE actual_app = rb_funcall2(self, rb_intern("app"), 0, NULL); // this creates a window
      Data_Get_Struct(actual_app, shoes_app, app); 
      title_app = RSTRING_PTR(app->title); 
      window_app = APP_WINDOW(app);
      */
    }
  }
#endif
    if (!NIL_P(attr) && !NIL_P(shoes_hash_get(attr, rb_intern("title"))))
        title = strdup(RSTRING_PTR(shoes_hash_get(attr, rb_intern("title"))));
    GtkWidget *dialog = gtk_file_chooser_dialog_new(title, window_app, act,
                        _("_Cancel"), GTK_RESPONSE_CANCEL, button, GTK_RESPONSE_ACCEPT, NULL);
    // theme the window
    if (shoes_css_provider != NULL) {
      gtk_style_context_add_provider(gtk_widget_get_style_context(dialog),
          GTK_STYLE_PROVIDER(shoes_css_provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
    if (act == GTK_FILE_CHOOSER_ACTION_SAVE)
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    if(RTEST(shoes_hash_get(attr, rb_intern("save"))))
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
                                          RSTRING_PTR(shoes_hash_get(attr, rb_intern("save"))));
    if(RTEST(shoes_hash_get(attr, rb_intern("types"))) && TYPE(shoes_hash_get(attr, rb_intern("types"))) == T_HASH) {
        VALUE hsh = shoes_hash_get(attr, rb_intern("types"));
        VALUE keys = rb_funcall(hsh, s_keys, 0);
        int i;
        for(i = 0; i < RARRAY_LEN(keys); i++) {
            VALUE key = rb_ary_entry(keys, i);
            VALUE val = rb_hash_aref(hsh, key);
            GtkFileFilter *ff = gtk_file_filter_new();
            gtk_file_filter_set_name(ff, RSTRING_PTR(key));
            gtk_file_filter_add_pattern(ff, RSTRING_PTR(val));
            gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ff);
        }
    }
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        path = rb_str_new2(filename);
    }
    if (!NIL_P(attr) && !NIL_P(shoes_hash_get(attr, rb_intern("title"))))
        SHOE_FREE(title);
    gtk_widget_destroy(dialog);
    return path;
}

VALUE shoes_dialog_open(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
#if 0
    VALUE attr = Qnil;
    char *title;
    switch (rb_parse_args(argc, argv, "|h", &args)) {
        case 0:
            title = strdup("Open file...");
            break;
        case 1:
            attr = args.a[0];
            title = strdup(RSTRING_PTR(shoes_hash_get(attr, rb_intern("title"))));
            break;
    }
    shoes_dialog_chooser(self, title, GTK_FILE_CHOOSER_ACTION_OPEN,
                         _("_Open"), args.a[0]);
    free(title);
    return;
#else
    rb_parse_args(argc, argv, "|h", &args);
    return shoes_dialog_chooser(self, "Open file...", GTK_FILE_CHOOSER_ACTION_OPEN,
                                _("_Open"), args.a[0]);
#endif
}

VALUE shoes_dialog_save(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    rb_parse_args(argc, argv, "|h", &args);
    return shoes_dialog_chooser(self, "Save file...", GTK_FILE_CHOOSER_ACTION_SAVE,
                                _("_Save"), args.a[0]);
}

VALUE shoes_dialog_open_folder(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    rb_parse_args(argc, argv, "|h", &args);
    return shoes_dialog_chooser(self, "Open folder...", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                _("_Open"), args.a[0]);
}

VALUE shoes_dialog_save_folder(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    rb_parse_args(argc, argv, "|h", &args);
    return shoes_dialog_chooser(self, "Save folder...", GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
                                _("_Save"), args.a[0]);
}
