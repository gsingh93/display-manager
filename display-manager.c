#include <libgen.h> // dirname()
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include "pam.h"

#define ENTER_KEY    65293
#define ESC_KEY      65307
#define UI_FILE      "gui.ui"
#define DISPLAY      ":1"
#define VT           "vt01"

static bool testing = false;

static GtkEntry *user_text_field;
static GtkEntry *pass_text_field;
static GtkLabel *status_label;

static pthread_t login_thread;
static pid_t x_server_pid;

static FILE *log_fd;

static void* login_func(void *data) {
    GtkWidget *widget = GTK_WIDGET(data);
    const gchar *username = gtk_entry_get_text(user_text_field);
    const gchar *password = gtk_entry_get_text(pass_text_field);

    fprintf(log_fd, "Attempting to log in user %s\n", username);
    gtk_label_set_text(status_label, "Logging in...");
    pid_t child_pid;
    if (login(username, password, &child_pid)) {
        gtk_widget_hide(widget);

        // Wait for child process to finish (wait for logout)
        int status;
        waitpid(child_pid, &status, 0); // TODO: Handle errors
        gtk_widget_show(widget);

        gtk_label_set_text(status_label, "");

        fprintf(log_fd, "Session ended, logging out\n");
        logout();
    } else {
        fprintf(log_fd, "Login error\n");
        gtk_label_set_text(status_label, "Login error");
    }
    gtk_entry_set_text(pass_text_field, "");

    return NULL;
}

static gboolean key_event(GtkWidget *widget, GdkEventKey *event) {
    if (event->keyval == ENTER_KEY) {
        pthread_create(&login_thread, NULL, login_func, (void*) widget);
    } else if (event->keyval == ESC_KEY) {
        fprintf(log_fd, "User pressed ESC. Quitting\n");
        gtk_main_quit();
    }
    return FALSE;
}

static void start_x_server(const char *display, const char *vt) {
    x_server_pid = fork();
    if (x_server_pid == 0) {
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "/usr/bin/X %s %s", display, vt);
        execl("/bin/bash", "/bin/bash", "-c", cmd, NULL);
        fprintf(log_fd, "Failed to start X server");
        exit(1);
    } else {
        fprintf(log_fd, "X Server PID is %d\n", x_server_pid);
        // TODO: Wait for X server to start
        sleep(1);
    }
}

static void stop_x_server() {
    if (x_server_pid != 0) {
        kill(x_server_pid, SIGKILL);
    }
}

static void sig_handler(int signo) {
    fprintf(log_fd, "Caught signal: %d", signo);
    stop_x_server();
}

int main(int argc, char *argv[]) {
    log_fd = fopen("dm.log", "w");
    setvbuf(log_fd, NULL, _IONBF, 0);
    fprintf(log_fd, "Starting display manager\n");

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char strtime[32];
    strftime(strtime, sizeof(strtime), "%c", &tm);
    fprintf(log_fd, "Time: %s\n", strtime);

    const char *display = DISPLAY;
    const char *vt = VT;
    if (argc == 3) {
        display = argv[1];
        vt = argv[2];
    }
    fprintf(log_fd, "display: %s\n", display);
    fprintf(log_fd, "vt: %s\n", vt);

    signal(SIGABRT, sig_handler);
    signal(SIGALRM, sig_handler);
    signal(SIGHUP, sig_handler);
    signal(SIGILL, sig_handler);
    signal(SIGPIPE, sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGSEGV, sig_handler);
    signal(SIGTRAP, sig_handler);

    if (!testing) {
        fprintf(log_fd, "Starting X server\n");
        start_x_server(display, vt);
    }
    setenv("DISPLAY", display, true);

    gtk_init(&argc, &argv);
    fprintf(log_fd, "Initialized GTK\n");

    char ui_file_path[256];
    if (readlink("/proc/self/exe", ui_file_path, sizeof(ui_file_path)) == -1) {
        printf("Error: could not get location of binary");
        exit(1);
    }

    dirname(ui_file_path);
    strcat(ui_file_path, "/" UI_FILE);
    GtkBuilder *builder = gtk_builder_new_from_file(ui_file_path);
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    user_text_field = GTK_ENTRY(gtk_builder_get_object(builder,
                                                       "user_text_entry"));
    pass_text_field = GTK_ENTRY(gtk_builder_get_object(builder,
                                                       "pass_text_entry"));
    status_label = GTK_LABEL(gtk_builder_get_object(builder, "status_label"));

    // Make full screen
    GdkScreen *screen = gdk_screen_get_default();
    gint height = gdk_screen_get_height(screen);
    gint width = gdk_screen_get_width(screen);
    gtk_widget_set_size_request(GTK_WIDGET(window), width, height);

    g_signal_connect(window, "key-release-event", G_CALLBACK(key_event), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    fprintf(log_fd, "Starting main GTK loop\n");
    gtk_main();

    stop_x_server();
    fclose(log_fd);

    return 0;
}
