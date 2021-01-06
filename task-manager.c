#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <cairo.h>

sem_t mutexTask;

int graphQueue = 12;
int circCPU[12], circMem[12];
int circCPUFront = -1, circMemFront = -1;
int circCPURear = -1, circMemRear = -1;

//Variables for Task-Manager.
GtkBuilder *TKManagerUI;
GObject *TaskManagerWindow, *MenuBar, *menu_file, *menu_view, *menu_options, *menu_help;
GObject *file_newcon, *file_run, *file_exit;
GObject *options_ontop, *options_minclose, *options_color;
GObject *view_refreshnow, *view_refreshspeed;
GObject *help_about;
GtkWidget *graphCPU, *graphMem;
GtkLabel *cpuusage, *memusage;
GtkWidget *vp;
GtkWidget *scrolledWindow;
GtkWidget *box, *tempIn[200], *tempOut[200];
GtkWidget *but[200], *taskData[200][5];
GtkWidget *sortCommand, *sortPID, *sortCPU, *sortMem, *sortUser;
//Variable for Task-Manager ends.

//SOCKET DECLARATIONS
int alwaysOnTop = 0, minClose = 0;
int taskSock = -1, memSock = -1, cpuSock = -1, runSock = -1;
char taskBuffer[512], memBuffer[512], cpuBuffer[512], runBuffer[512];
char taskCommand[512];
int currSort = 1;
int refreshRate = 20;
int refreshStop = 0;
int countTask = 0, countCPU = 0, countMem = 0;

static void printMenuActivatedData(GtkWidget *widget, gpointer data)
{
    g_print("\n%s was activated.\n", (gchar *)data);
}

void shutDown(GtkWidget *widget, gpointer data)
{
    strcpy(taskBuffer, "BYE");
    gtk_widget_destroy(TaskManagerWindow);
    gtk_main_quit();
}

void onTop()
{
    if (alwaysOnTop == 0)
    {
        gtk_window_set_keep_above(TaskManagerWindow, TRUE);
        alwaysOnTop = 1;
    }
    else
    {
        gtk_window_set_keep_above(TaskManagerWindow, FALSE);
        alwaysOnTop = 0;
    }
}

void createNewTaskWindow(GtkWidget *widget, gpointer data)
{
    //SETTING UP NEW CONNECTION WINDOW
    gtk_widget_set_sensitive(TaskManagerWindow, FALSE);

    GtkWidget *box;
    GtkWidget *content_area;

    // task_window = gtk_dialog_new();

    // new_connection_ui = gtk_builder_new();
    // gtk_builder_add_from_file(new_connection_ui, "ui-xml/new-connection-dialogue.ui", NULL);

    // box = gtk_builder_get_object(new_connection_ui, "new-task-box");

    // task_okay = gtk_builder_get_object(new_connection_ui, "okay-button1");
    // g_signal_connect(task_okay, "clicked", G_CALLBACK(executeTask), new_connection_ui);

    // task_cancel = gtk_builder_get_object(new_connection_ui, "cancel-button1");
    // g_signal_connect(task_cancel, "clicked", G_CALLBACK(closeNewConnectionWindow), task_window);

    // content_area = gtk_dialog_get_content_area(GTK_DIALOG(task_window));
    // //gtk_widget_reparent(box,GTK_CONTAINER(content_area));
    // gtk_container_add(GTK_CONTAINER(content_area), box);

    // g_signal_connect(task_window, "destroy", G_CALLBACK(closeNewConnectionWindow), task_window);
    // gtk_window_set_title(task_window, "Run a New Task");
    // gtk_window_set_skip_taskbar_hint(task_window, TRUE);
    // gtk_window_set_resizable(GTK_WINDOW(task_window), FALSE);

    // gtk_widget_show_all(task_window);

    //gtk_widget_grab_focus(GTK_WIDGET(new_connection_window));
    //SETTING UP NEW CONNECTION WINDOW ends.
}

void minimiseOnClose()
{
    // if (minClose == 0)
    // {
    //     g_signal_connect(TaskManagerWindow, "delete-event", G_CALLBACK(gtk_window_iconify), NULL);
    //     minClose = 1;
    // }
    // else
    // {
    //     g_signal_connect(TaskManagerWindow, "delete-event", G_CALLBACK(delete_eventMod), NULL);
    //     minClose = 0;
    // }
}

void refreshNow()
{
    strcpy(taskBuffer, taskCommand);
    strcpy(cpuBuffer, "top -bn1 | grep \"Cpu(s)\" | sed \"s/.*, *\\([0-9.]*\\)%* id.*/\\1/\" | awk \'{printf(\"%0.1f\",100-$1);}\'");
    // receiveCPUUsage(cpuSock, cpuBuffer);
    strcpy(memBuffer, "free -m | grep Mem | awk \'{printf(\"%0.1f\",$3/$2*100)}\'");
    // sendMessageOverMemSocket(memSock, memBuffer);
    // receiveMemUsage(memSock, memBuffer);
}

void createRefreshRateWindow(GtkWidget *widget, gpointer data)
{
    //SETTING UP NEW CONNECTION WINDOW
    gtk_widget_set_sensitive(TaskManagerWindow, FALSE);

    // GtkWidget *box;
    // GtkWidget *content_area;

    // refresh_window = gtk_dialog_new();

    // new_connection_ui = gtk_builder_new();
    // gtk_builder_add_from_file(new_connection_ui, "ui-xml/new-connection-dialogue.ui", NULL);

    // box = gtk_builder_get_object(new_connection_ui, "refresh-speed-box");

    // refresh_okay = gtk_builder_get_object(new_connection_ui, "okay-button2");
    // g_signal_connect(refresh_okay, "clicked", G_CALLBACK(setRefreshRate), new_connection_ui);

    // refresh_cancel = gtk_builder_get_object(new_connection_ui, "cancel-button");
    // g_signal_connect(refresh_cancel, "clicked", G_CALLBACK(closeNewConnectionWindow), refresh_window);

    // content_area = gtk_dialog_get_content_area(GTK_DIALOG(refresh_window));
    // //gtk_widget_reparent(box,GTK_CONTAINER(content_area));
    // gtk_container_add(GTK_CONTAINER(content_area), box);

    // g_signal_connect(refresh_window, "destroy", G_CALLBACK(closeNewConnectionWindow), refresh_window);
    // gtk_window_set_title(refresh_window, "Set Refresh Rate");
    // gtk_window_set_skip_taskbar_hint(refresh_window, TRUE);
    // gtk_window_set_resizable(GTK_WINDOW(refresh_window), FALSE);

    // gtk_widget_show_all(refresh_window);

    //gtk_widget_grab_focus(GTK_WIDGET(new_connection_window));
    //SETTING UP NEW CONNECTION WINDOW ends.
}

int main(int argc, char *argv[])
{
    //  * create task manager window
    TKManagerUI = gtk_builder_new();
    gtk_builder_add_from_file(TaskManagerWindow, "ui-xml/task-manager-ui.ui", NULL);

    TaskManagerWindow = GTK_WIDGET(gtk_builder_get_object(TKManagerUI, "task-manager"));
    g_signal_connect(TaskManagerWindow, "destroy", G_CALLBACK(shutDown), NULL);

    MenuBar = GTK_WIDGET(gtk_builder_get_object(TKManagerUI, "menubar"));

    menu_file = gtk_builder_get_object(TKManagerUI, "menuitem-file");
    g_signal_connect(menu_file, "activate", G_CALLBACK(printMenuActivatedData), "Menu-File");

    menu_view = gtk_builder_get_object(TKManagerUI, "menuitem-view");
    g_signal_connect(menu_view, "activate", G_CALLBACK(printMenuActivatedData), "Menu-View");

    menu_options = gtk_builder_get_object(TKManagerUI, "menuitem-options");
    g_signal_connect(menu_options, "activate", G_CALLBACK(printMenuActivatedData), "Menu-Options");

    menu_help = gtk_builder_get_object(TKManagerUI, "menuitem-help");
    g_signal_connect(menu_help, "activate", G_CALLBACK(printMenuActivatedData), "Menu-Help");

    // file_newcon = gtk_builder_get_object(TKManagerUI, "menu-file-new");
    // g_signal_connect(file_newcon, "activate", G_CALLBACK(createNewConnectionWindow), NULL);

    file_run = gtk_builder_get_object(TKManagerUI, "menu-file-run");
    g_signal_connect(file_run, "activate", G_CALLBACK(createNewTaskWindow), "File-Run");
    gtk_widget_set_sensitive(file_run, FALSE);

    file_exit = gtk_builder_get_object(TKManagerUI, "menu-file-exit");
    g_signal_connect(file_exit, "activate", G_CALLBACK(shutDown), NULL);

    options_ontop = gtk_builder_get_object(TKManagerUI, "menu-options-ontop");
    g_signal_connect(options_ontop, "activate", G_CALLBACK(onTop), "Options-OnTop");

    options_minclose = gtk_builder_get_object(TKManagerUI, "menu-options-minclose");
    g_signal_connect(options_minclose, "activate", G_CALLBACK(minimiseOnClose), "Options-MinClose");

    options_color = gtk_builder_get_object(TKManagerUI, "menu-options-color");
    g_signal_connect(options_color, "activate", G_CALLBACK(printMenuActivatedData), "Options-Color");
    gtk_widget_set_sensitive(options_color, FALSE);

    view_refreshnow = gtk_builder_get_object(TKManagerUI, "menu-view-refreshnow");
    g_signal_connect(view_refreshnow, "activate", G_CALLBACK(refreshNow), "View-Refresh-Now");
    gtk_widget_set_sensitive(view_refreshnow, FALSE);

    view_refreshspeed = gtk_builder_get_object(TKManagerUI, "menu-view-refreshspeed");
    g_signal_connect(view_refreshspeed, "activate", G_CALLBACK(createRefreshRateWindow), "View-Refresh-Speed");
    gtk_widget_set_sensitive(view_refreshspeed, FALSE);

    help_about = gtk_builder_get_object(TKManagerUI, "menu-help-about");
    g_signal_connect(help_about, "activate", G_CALLBACK(printMenuActivatedData), "Help-About");

    return 0;
}