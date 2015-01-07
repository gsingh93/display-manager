#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include "pam.h"

#define ENTER_KEY    65293
#define UI_FILE      "gui.ui"

static GtkEntry *user_text_field;
static GtkEntry *pass_text_field;

static gboolean key_event(GtkWidget *widget, GdkEventKey *event) {
    if (event->keyval == ENTER_KEY) {
        const gchar *username = gtk_entry_get_text(user_text_field);
        const gchar *password = gtk_entry_get_text(pass_text_field);

        pid_t child_pid;
        login(username, password, &child_pid);
        gtk_widget_hide(widget);

        // Wait for child process to finish (wait for logout)
        int status;
        waitpid(child_pid, &status, 0); // TODO: Handle errors

        logout();
    }
    return FALSE;
}

void start_x_server() {
    int pid = fork();
    if (pid == 0) {
        char *cmd = "/usr/bin/X :0 vt01";
        execl("/bin/bash", "/bin/bash", "-c", cmd, NULL);
        printf("Failed to start X server");
        exit(1);
    } else {
        // TODO: Wait for X server to start
        sleep(1);
    }
}

int main(int argc, char *argv[]) {
    bool testing = true;
    if (!testing) {
        start_x_server();
    }

    gtk_init(&argc, &argv);

    GtkBuilder *builder = gtk_builder_new_from_file(UI_FILE);
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
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

    return 0;
}
