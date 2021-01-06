#include <gtk/gtk.h>

// gtk builder
GtkBuilder *builder;
// outer window objects
GObject *window, *full_window_box;
// menu bar objects
GObject *MenuBar, *menu_file, *menu_file_runnewtask, *menu_file_exit;
GObject *menu_options, *menu_options_ontop;
GObject *menu_view, *menu_view_refreshnow;
GObject *menu_help, *menu_help_about;
// content window objects
GObject *content_window_box, *tabbed_windows;
// process objects
GObject *process_window, *process_title, *process_title_buttons, *process_name, *process_pid, *process_cpu, *process_memory, *process_user;
GObject *process_list_window, *process_scrolling_window;
// performance objects
GObject *performance_window, *cpuGraphArea, *memGraphArea;
// users objects
GObject *users_window;

GError *error = NULL;

static void
print_hello(GtkWidget *widget,
            gpointer data)
{
    g_print("Hello World hey!\n");
}

int main(int argc,
         char *argv[])
{

    gtk_init(&argc, &argv);

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new();
    if (gtk_builder_add_from_file(builder, "builder.ui", &error) == 0)
    {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    /* Connect signal handlers to the constructed widgets. */
    window = gtk_builder_get_object(builder, "window");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_main();

    return 0;
}