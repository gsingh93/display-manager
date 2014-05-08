#include <gtk/gtk.h>

#define ENTER_KEY 65293
#define UI_FILE   "gui.ui"

static GtkEntry *user_text_field;
static GtkEntry *pass_text_field;

static void login() {
    const gchar *username = gtk_entry_get_text(user_text_field);
    const gchar *password = gtk_entry_get_text(pass_text_field);
    g_print("%s %s\n", username, password);
}

static gboolean key_event(GtkWidget *widget, GdkEventKey *event) {
    if (event->keyval == ENTER_KEY) {
        login();
    }
    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, UI_FILE, NULL);

    GObject *window = gtk_builder_get_object(builder, "window");
    user_text_field = GTK_ENTRY(gtk_builder_get_object(builder,
                                                           "user_text_entry"));
    pass_text_field = GTK_ENTRY(gtk_builder_get_object(builder,
                                                           "pass_text_entry"));

    // Make full screen
    GdkScreen *screen = gdk_screen_get_default();
    gint height = gdk_screen_get_height(screen);
    gint width = gdk_screen_get_width(screen);
    gtk_widget_set_size_request(GTK_WIDGET(window), width, height);

    g_signal_connect(window, "key-release-event", G_CALLBACK(key_event), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_main();
}
