#include <libgen.h> // dirname()
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>

static GtkEntry *user_text_field;
static GtkEntry *pass_text_field;
static GtkLabel *status_label;

#define UI_FILE     "gui.ui"
#define WINDOW_ID   "window"
#define USERNAME_ID "username_text_entry"
#define PASSWORD_ID "password_text_entry"
#define STATUS_ID   "status_label"

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    char ui_file_path[256];
    if (readlink("/proc/self/exe", ui_file_path, sizeof(ui_file_path)) == -1) {
        printf("Error: could not get location of binary");
        exit(1);
    }

    dirname(ui_file_path);
    strcat(ui_file_path, "/" UI_FILE);
    GtkBuilder *builder = gtk_builder_new_from_file(ui_file_path);
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, WINDOW_ID));
    user_text_field = GTK_ENTRY(gtk_builder_get_object(builder, USERNAME_ID));
    pass_text_field = GTK_ENTRY(gtk_builder_get_object(builder, PASSWORD_ID));
    status_label = GTK_LABEL(gtk_builder_get_object(builder, STATUS_ID));

    // Make full screen
    GdkScreen *screen = gdk_screen_get_default();
    gint height = gdk_screen_get_height(screen);
    gint width = gdk_screen_get_width(screen);
    gtk_widget_set_size_request(GTK_WIDGET(window), width, height);
    gtk_widget_show(window);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_main();

    return 0;
}
