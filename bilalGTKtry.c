#include <gtk/gtk.h>
#include <stdio.h>
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

//Variables for New-Connection.
GtkBuilder *new_connection_ui;
GObject *new_connection_window, *new_connection_okay, *new_connection_cancel;
GObject *refresh_window, *refresh_okay, *refresh_cancel;
GObject *task_window, *task_okay, *task_cancel;

//  * gtk builder
GtkBuilder *builder;
//  * outer window objects
GObject *window, *full_window_box;
//  * menu bar objects
GObject *MenuBar, *menu_file, *menu_file_runnewtask, *menu_file_exit;
GObject *menu_options, *menu_options_ontop;
GObject *menu_view, *menu_view_refreshnow;
GObject *menu_help, *menu_help_about;
//  * content window objects
GObject *content_window_box, *tabbed_windows;
//  * process objects
GObject *process_window, *process_title, *process_title_buttons, *process_name, *process_pid, *process_cpu, *process_memory, *process_user;
GObject *process_list_window, *process_scrolling_window;
//  * performance objects
GObject *performance_window, *cpuGraphArea, *memGraphArea;
//  * users objects
GObject *users_window;

GError *error = NULL;

GtkWidget *graphCPU, *graphMem;
GObject *cpuusage, *memusage;
GtkWidget *vp;
GtkWidget *scrolledWindow;
GtkWidget *box, *tempIn[200], *tempOut[200];
GtkWidget *but[200], *taskData[200][5];
GtkWidget *sortCommand, *sortPID, *sortCPU, *sortMem, *sortUser;

int alwaysOnTop = 0;
char taskBuffer[512], memBuffer[512], cpuBuffer[512], runBuffer[512];
char taskCommand[512];
int currSort = 1;
int refreshRate = 2;
int refreshStop = 0;
int countTask = 0, countCPU = 0, countMem = 0;

//  * helper functions
static void printMenuActivatedData(GtkWidget *widget, gpointer data)
{
    g_print("\n%s was activated.\n", (gchar *)data);
}

gpointer mainCallBack(gpointer data)
{
    gtk_main();
    return 0;
}

void killTask(GtkWidget *widget, char data[])
{
    GList *children,*iter;
	int i=0;
	children=gtk_container_get_children(GTK_CONTAINER(widget));
	children=gtk_container_get_children(GTK_CONTAINER(children->data));
	children=g_list_next(children);
	
	char kill[512];
	sprintf(kill,"kill -9 %s",gtk_label_get_text(children->data));
	system(kill);
	gtk_widget_hide(gtk_widget_get_parent(GTK_WIDGET(widget)));
}

static gboolean drawCPUGraph(GtkWidget *widget, cairo_t *cr, gpointer data)
{

    int i = 0;
    float prevX;
    /* set bg color as white */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    /* set color for rectangle */
    cairo_set_source_rgb(cr, 0.07, 0.49, 0.73);
    /* set the line width */
    cairo_set_line_width(cr, 2.5);
    /* draw the rectangle's path beginning at 3,3 */
    cairo_rectangle(cr, 0, 0, 400, 200);
    /* stroke the rectangle's path with the chosen color so it's actually visible */
    cairo_stroke(cr);

    /*drawing checkered boxes through graph */
    cairo_set_source_rgb(cr, 0.85, 0.92, 0.96);
    /* set the line width */
    cairo_set_line_width(cr, 0.8);
    /* draw the rectangle's path beginning at 3,3 */
    cairo_rectangle(cr, 50, 0, 50, 200);
    cairo_rectangle(cr, 100, 0, 100, 200);
    cairo_rectangle(cr, 150, 0, 150, 200);
    cairo_rectangle(cr, 200, 0, 200, 200);
    cairo_rectangle(cr, 250, 0, 250, 200);
    cairo_rectangle(cr, 300, 0, 300, 200);
    cairo_rectangle(cr, 350, 0, 350, 200);

    cairo_rectangle(cr, 0, 40, 400, 40);
    cairo_rectangle(cr, 0, 80, 400, 80);
    cairo_rectangle(cr, 0, 120, 400, 120);
    cairo_rectangle(cr, 0, 160, 400, 160);
    /* stroke the rectangle's path with the chosen color so it's actually visible */
    cairo_stroke(cr);

    if (circCPURear == -1)
        return;

    prevX = 400 / (graphQueue - 2);
    cairo_set_source_rgb(cr, 0.85, 0.92, 0.96);
    cairo_set_line_width(cr, 2);
    cairo_arc(cr, prevX - 400 / (graphQueue - 2), (float)(200 - (float)(2 * circCPU[circCPURear])), 5, 0, 2 * G_PI);
    cairo_fill(cr);

    if (circCPURear <= circCPUFront)
    {
        for (i = circCPURear + 1; i <= circCPUFront; i++)
        {
            cairo_set_source_rgb(cr, 0.85, 0.92, 0.96);
            cairo_set_line_width(cr, 2);
            cairo_arc(cr, prevX, (float)(200 - (float)(2 * circCPU[i])), 5, 0, 2 * G_PI);
            cairo_fill(cr);

            /* setup for graph lines */
            cairo_set_source_rgb(cr, 0.09, 0.50, 0.74);
            /* set the line width */
            cairo_set_line_width(cr, 1.5);
            cairo_move_to(cr, prevX - 400 / (graphQueue - 2), (float)(200 - (float)(2 * circCPU[i - 1])));
            cairo_line_to(cr, prevX, (float)(200 - (float)(2 * circCPU[i])));
            cairo_stroke(cr);
            prevX += 400 / (graphQueue - 2);
        }
    }
    else
    {
        for (i = circCPURear + 1; i <= graphQueue - 1; i++)
        {
            cairo_set_source_rgb(cr, 0.85, 0.92, 0.96);
            cairo_set_line_width(cr, 2);
            cairo_arc(cr, prevX, (float)(200 - (float)(2 * circCPU[i])), 5, 0, 2 * G_PI);
            cairo_fill(cr);

            /* setup for graph lines */
            cairo_set_source_rgb(cr, 0.09, 0.50, 0.74);
            /* set the line width */
            cairo_set_line_width(cr, 1.5);
            cairo_move_to(cr, prevX - 400 / (graphQueue - 2), (float)(200 - (float)(2 * circCPU[i - 1])));
            cairo_line_to(cr, prevX, (float)(200 - (float)(2 * circCPU[i])));
            cairo_stroke(cr);
            prevX += 400 / (graphQueue - 2);
        }

        for (i = 0; i <= circCPUFront; i++)
        {
            cairo_set_source_rgb(cr, 0.85, 0.92, 0.96);
            cairo_set_line_width(cr, 2);
            cairo_arc(cr, prevX, (float)(200 - (float)(2 * circCPU[i])), 5, 0, 2 * G_PI);
            cairo_fill(cr);

            /* setup for graph lines */
            cairo_set_source_rgb(cr, 0.09, 0.50, 0.74);
            /* set the line width */
            cairo_set_line_width(cr, 1.5);
            if (i == 0)
                cairo_move_to(cr, prevX - 400 / (graphQueue - 2), (float)(200 - (float)(2 * circCPU[graphQueue - 1])));
            else
                cairo_move_to(cr, prevX - 400 / (graphQueue - 2), (float)(200 - (float)(2 * circCPU[i - 1])));
            cairo_line_to(cr, prevX, (float)(200 - (float)(2 * circCPU[i])));
            cairo_stroke(cr);
            prevX += 400 / (graphQueue - 2);
        }
    }
}

static gboolean drawMemGraph(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    int i = 0;
    float prevX;
    /* set bg color as white */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    /* set color for rectangle */
    cairo_set_source_rgb(cr, 0.07, 0.49, 0.73);
    /* set the line width */
    cairo_set_line_width(cr, 2.5);
    /* draw the rectangle's path beginning at 3,3 */
    cairo_rectangle(cr, 0, 0, 400, 200);
    /* stroke the rectangle's path with the chosen color so it's actually visible */
    cairo_stroke(cr);

    /*drawing checkered boxes through graph */
    cairo_set_source_rgb(cr, 0.85, 0.92, 0.96);
    /* set the line width */
    cairo_set_line_width(cr, 0.8);
    /* draw the rectangle's path beginning at 3,3 */
    cairo_rectangle(cr, 50, 0, 50, 200);
    cairo_rectangle(cr, 100, 0, 100, 200);
    cairo_rectangle(cr, 150, 0, 150, 200);
    cairo_rectangle(cr, 200, 0, 200, 200);
    cairo_rectangle(cr, 250, 0, 250, 200);
    cairo_rectangle(cr, 300, 0, 300, 200);
    cairo_rectangle(cr, 350, 0, 350, 200);

    cairo_rectangle(cr, 0, 40, 400, 40);
    cairo_rectangle(cr, 0, 80, 400, 80);
    cairo_rectangle(cr, 0, 120, 400, 120);
    cairo_rectangle(cr, 0, 160, 400, 160);
    /* stroke the rectangle's path with the chosen color so it's actually visible */
    cairo_stroke(cr);

    if (circMemRear == -1)
        return;

    prevX = 400 / (graphQueue - 2);
    cairo_set_source_rgb(cr, 0.85, 0.92, 0.96);
    cairo_set_line_width(cr, 2);
    cairo_arc(cr, prevX - 400 / (graphQueue - 2), (float)(200 - (float)(2 * circMem[circMemRear])), 5, 0, 2 * G_PI);
    cairo_fill(cr);

    if (circMemRear <= circMemFront)
    {
        for (i = circMemRear + 1; i <= circMemFront; i++)
        {
            cairo_set_source_rgb(cr, 0.85, 0.92, 0.96);
            cairo_set_line_width(cr, 2);
            cairo_arc(cr, prevX, (float)(200 - (float)(2 * circMem[i])), 5, 0, 2 * G_PI);
            cairo_fill(cr);

            /* setup for graph lines */
            cairo_set_source_rgb(cr, 0.09, 0.50, 0.74);
            /* set the line width */
            cairo_set_line_width(cr, 1.5);
            cairo_move_to(cr, prevX - 400 / (graphQueue - 2), (float)(200 - (float)(2 * circMem[i - 1])));
            cairo_line_to(cr, prevX, (float)(200 - (float)(2 * circMem[i])));
            cairo_stroke(cr);
            prevX += 400 / (graphQueue - 2);
        }
    }
    else
    {
        for (i = circMemRear + 1; i <= graphQueue - 1; i++)
        {
            cairo_set_source_rgb(cr, 0.85, 0.92, 0.96);
            cairo_set_line_width(cr, 2);
            cairo_arc(cr, prevX, (float)(200 - (float)(2 * circMem[i])), 5, 0, 2 * G_PI);
            cairo_fill(cr);

            /* setup for graph lines */
            cairo_set_source_rgb(cr, 0.09, 0.50, 0.74);
            /* set the line width */
            cairo_set_line_width(cr, 1.5);
            cairo_move_to(cr, prevX - 400 / (graphQueue - 2), (float)(200 - (float)(2 * circMem[i - 1])));
            cairo_line_to(cr, prevX, (float)(200 - (float)(2 * circMem[i])));
            cairo_stroke(cr);
            prevX += 400 / (graphQueue - 2);
        }

        for (i = 0; i <= circMemFront; i++)
        {
            cairo_set_source_rgb(cr, 0.85, 0.92, 0.96);
            cairo_set_line_width(cr, 2);
            cairo_arc(cr, prevX, (float)(200 - (float)(2 * circMem[i])), 5, 0, 2 * G_PI);
            cairo_fill(cr);

            /* setup for graph lines */
            cairo_set_source_rgb(cr, 0.09, 0.50, 0.74);
            /* set the line width */
            cairo_set_line_width(cr, 1.5);
            if (i == 0)
                cairo_move_to(cr, prevX - 400 / (graphQueue - 2), (float)(200 - (float)(2 * circMem[graphQueue - 1])));
            else
                cairo_move_to(cr, prevX - 400 / (graphQueue - 2), (float)(200 - (float)(2 * circMem[i - 1])));
            cairo_line_to(cr, prevX, (float)(200 - (float)(2 * circMem[i])));
            cairo_stroke(cr);
            prevX += 400 / (graphQueue - 2);
        }
    }
}

void pushIntoCircularQueue(int circular[], int *front, int *rear, int value)
{

    if (*front == -1)
    {
        *front = *rear = 0;
    }
    else if (*front == (*rear) - 1)
    {
        (*front)++;
        if (*rear == graphQueue - 1)
        {
            *rear = -1;
        }
        (*rear)++;
    }
    else if (*rear == 0 && *front == graphQueue - 1)
    {
        *front = 0;
        (*rear)++;
    }
    else if (*front == graphQueue - 1)
    {
        *front = 0;
    }
    else
    {
        (*front)++;
    }
    circular[*front] = value;
}

int receiveDataAndCreateTasks(char cmd[])
{
    char command[100], pid[50], cpu[50], mem[50], user[80];
    int i = 0, j = 0;
	FILE *fp;
	if ((fp = popen(cmd, "r")) == NULL)
	{
		printf("Error opening pipe");
	}
	while (fgets(taskBuffer, 512, fp))
	{
		sscanf(taskBuffer, "%s%s%s%s%s", command, pid, cpu, mem, user);
        if(strcmp(command,"sh")!=0&&i<200){
            gtk_label_set_text(taskData[i][0], command);
            gtk_label_set_text(taskData[i][1], pid);
            gtk_label_set_text(taskData[i][2], cpu);
            gtk_label_set_text(taskData[i][3], mem);
            gtk_label_set_text(taskData[i][4], user);

            gtk_widget_show(GTK_WIDGET(tempOut[i]));
            i++;
        }
        
	}
    

    for (; i < 200; i++)
    {
        gtk_widget_hide(GTK_WIDGET(tempOut[i]));
    }

    return 1;
}

int threadedSendReceiveTasks()
{
    char cmd[] = "whoami | xargs top -b -n 1 -u | awk '{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}' | sort -k 1";
    receiveDataAndCreateTasks(cmd);
    return 1;
}

int receiveCPUUsage()
{
    char cmd[] = "top -bn1 | grep \"Cpu(s)\" | sed \"s/.*, *\\([0-9.]*\\)%* id.*/\\1/\" | awk \'{printf(\"%0.1f\",100-$1);}\'";
	FILE *fp;
	if ((fp = popen(cmd, "r")) == NULL)
	{
		printf("Error opening pipe!\n");
	}
	while (fgets(cpuBuffer, 512, fp) != NULL);
    
    float value = atof(cpuBuffer);
    char usage[5];
    pushIntoCircularQueue(circCPU, &circCPUFront, &circCPURear, value);
    sprintf(cpuBuffer, "%0.1f%s", value, "%");
    gtk_widget_queue_draw(graphCPU);

    gtk_label_set_text(cpuusage, cpuBuffer);
    return 1;
}

int threadedSendReceiveCPU()
{
    receiveCPUUsage();
    return 1;
}

int receiveMemUsage()
{
    char cmd[] = "free -m | grep Mem | awk \'{printf(\"%0.1f\",$3/$2*100)}\'";
	FILE *fp;
	if ((fp = popen(cmd, "r")) == NULL)
	{
		printf("Error opening pipe!\n");
	}
	while (fgets(memBuffer, 512, fp) != NULL);
    float value = atof(memBuffer);
    char usage[5];
    pushIntoCircularQueue(circMem, &circMemFront, &circMemRear, value);
    sprintf(memBuffer, "%0.1f%s", value, "%");
    gtk_widget_queue_draw(graphMem);

    gtk_label_set_text(memusage, memBuffer);

    return 1;
}

int threadedSendReceiveMem()
{
    //gdk_threads_enter();
    // strcpy(memBuffer, "free -m | grep Mem | awk \'{printf(\"%0.1f\",$3/$2*100)}\'");
    receiveMemUsage();
    return 1;
}

void *socketSetupThreadFunction()
{
    char cmd[] = "whoami | xargs top -b -n 1 -u | awk '{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}' | sort -k 1";
    receiveDataAndCreateTasks(cmd);
    // strcpy(cpuBuffer, "top -bn1 | grep \"Cpu(s)\" | sed \"s/.*, *\\([0-9.]*\\)%* id.*/\\1/\" | awk \'{printf(\"%0.1f\",100-$1);}\'");
    receiveCPUUsage();
    // strcpy(memBuffer, "free -m | grep Mem | awk \'{printf(\"%0.1f\",$3/$2*100)}\'");
    receiveMemUsage();

    g_timeout_add_seconds(refreshRate, threadedSendReceiveTasks, NULL);
    g_timeout_add_seconds(refreshRate, threadedSendReceiveCPU, NULL);
    g_timeout_add_seconds(refreshRate, threadedSendReceiveMem, NULL);
}

void startFunction()
{
    GtkWidget *content_area;

    GThread *startLoadingWindowThread, *startSetupSocketThread;
    GError *err1 = NULL;
    GError *err2 = NULL;

    if ((startSetupSocketThread = g_thread_create((GThreadFunc)socketSetupThreadFunction, NULL, TRUE, &err2)) == NULL)
    {
        printf("Thread create failed: %s\n", err2->message);
        g_error_free(err2);
    }

    g_thread_join(startSetupSocketThread);
}

void sortByCommand(){
    char cmd[512];
	if(currSort==1){
		currSort=-1;
		strcpy(cmd,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\' | sort -k 1 -r");
	}else{
		currSort=1;
		strcpy(cmd,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\' | sort -k 1");
	}
	receiveDataAndCreateTasks(cmd);
}

void sortByPID(){
    char cmd[512];
	if(currSort==2){
		currSort=-2;
		strcpy(cmd,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\' | sort -k 2 -r");
	}else{
		currSort=2;
		strcpy(cmd,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\' | sort -k 2");
	}
	receiveDataAndCreateTasks(cmd);
}

void sortByCPU(){
    char cmd[512];
	if(currSort==3){
		currSort=-3;
		strcpy(cmd,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\' | sort -k 3 -r");
	}else{
		currSort=3;
		strcpy(cmd,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\' | sort -k 3");
	}
	receiveDataAndCreateTasks(cmd);
}

void sortByMem(){
    char cmd[512];
	if(currSort==4){
		currSort=-4;
		strcpy(cmd,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\' | sort -k 4 -r");
	}else{
		currSort=4;
		strcpy(cmd,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\' | sort -k 4");
	}
	receiveDataAndCreateTasks(cmd);
}

void sortByUser(){
	char cmd[512];
	if(currSort==5){
		currSort=-5;
		strcpy(cmd,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\' | sort -k 5 -r");
	}else{
		currSort=5;
		strcpy(cmd,"whoami | xargs top -b -n 1 -u | awk \'{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}\' | sort -k 5");
	}
    receiveDataAndCreateTasks(cmd);
}

int main(int argc, char *argv[])
{
    int i = 0;
    sem_init(&mutexTask, 0, 1);
    gtk_init(&argc, &argv);

    //  * Construct a GtkBuilder instance and load our UI description
    builder = gtk_builder_new();
    if (gtk_builder_add_from_file(builder, "builder.ui", &error) == 0)
    {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    //  * Connect signal handlers to the constructed widgets.
    window = gtk_builder_get_object(builder, "window");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    //  * menu bar
    MenuBar = gtk_builder_get_object(builder, "MenuBar");
    //  * file button on menu bar
    menu_file = gtk_builder_get_object(builder, "menuitem_file");
    g_signal_connect(menu_file, "activate", G_CALLBACK(printMenuActivatedData), "Menu-File");
    //  * view button on menu bar
    menu_view = gtk_builder_get_object(builder, "menuitem_view");
    g_signal_connect(menu_view, "activate", G_CALLBACK(printMenuActivatedData), "Menu-View");
    //  * options button on menu bar
    menu_options = gtk_builder_get_object(builder, "menuitem_options");
    g_signal_connect(menu_options, "activate", G_CALLBACK(printMenuActivatedData), "Menu-Options");
    //  * help button on menu bar
    menu_help = gtk_builder_get_object(builder, "menuitem_help");
    g_signal_connect(menu_help, "activate", G_CALLBACK(printMenuActivatedData), "Menu-Help");

    // * label for cpu and memory usage
    cpuusage = gtk_builder_get_object(builder, "cpu-usage-label");
    memusage = gtk_builder_get_object(builder, "mem-usage-label");

    graphCPU = (gtk_builder_get_object(builder, "cpuGraphArea"));
    g_signal_connect(graphCPU, "draw", G_CALLBACK(drawCPUGraph), NULL);

    graphMem = (gtk_builder_get_object(builder, "memGraphArea"));
    g_signal_connect(graphMem, "draw", G_CALLBACK(drawMemGraph), NULL);

    gtk_widget_show_all(window);

    sortCommand=gtk_builder_get_object(builder,"process_name");
	g_signal_connect(sortCommand,"clicked",G_CALLBACK(sortByCommand),NULL);

	sortPID=gtk_builder_get_object(builder,"process_pid");
	g_signal_connect(sortPID,"clicked",G_CALLBACK(sortByPID),NULL);

	sortCPU=gtk_builder_get_object(builder,"process_cpu");
	g_signal_connect(sortCPU,"clicked",G_CALLBACK(sortByCPU),NULL);

	sortMem=gtk_builder_get_object(builder,"process_memory");
	g_signal_connect(sortMem,"clicked",G_CALLBACK(sortByMem),NULL);

	sortUser=gtk_builder_get_object(builder,"process_user");
	g_signal_connect(sortUser,"clicked",G_CALLBACK(sortByUser),NULL);

    vp = GTK_WIDGET(gtk_builder_get_object(builder, "process-add-box"));
    box = gtk_box_new(TRUE, 0);
    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);

    gtk_widget_set_size_request(scrolledWindow, 600, 490);
    gtk_widget_set_size_request(box, 580, 490);

    gtk_container_add(GTK_CONTAINER(scrolledWindow), box);
    gtk_container_add(GTK_CONTAINER(vp), scrolledWindow);

    for (i = 0; i < 200; i++)
    {
        tempOut[i] = gtk_box_new(FALSE, 0);
        gtk_widget_set_size_request(tempOut[i], 580, 35);

        but[i] = gtk_button_new();
        gtk_widget_set_size_request(but[i], 580, 35);
        gtk_button_set_relief(but[i], GTK_RELIEF_NONE);

        tempIn[i] = gtk_box_new(FALSE, 0);
        gtk_widget_set_size_request(tempIn[i], 580, 35);

        taskData[i][0] = gtk_label_new("");
        gtk_container_add(GTK_CONTAINER(tempIn[i]), taskData[i][0]);
        gtk_widget_set_size_request(taskData[i][0], 241, 35);
        gtk_label_set_justify(taskData[i][0], GTK_JUSTIFY_LEFT);

        taskData[i][1] = gtk_label_new("");
        gtk_container_add(GTK_CONTAINER(tempIn[i]), taskData[i][1]);
        gtk_widget_set_size_request(taskData[i][1], 76, 35);

        taskData[i][2] = gtk_label_new("");
        gtk_container_add(GTK_CONTAINER(tempIn[i]), taskData[i][2]);
        gtk_widget_set_size_request(taskData[i][2], 76, 35);

        taskData[i][3] = gtk_label_new("");
        gtk_container_add(GTK_CONTAINER(tempIn[i]), taskData[i][3]);
        gtk_widget_set_size_request(taskData[i][3], 76, 35);

        taskData[i][4] = gtk_label_new("");
        gtk_container_add(GTK_CONTAINER(tempIn[i]), taskData[i][4]);
        gtk_widget_set_size_request(taskData[i][4], 106, 35);

        gtk_widget_show(taskData[i][0]);
        gtk_widget_show(taskData[i][1]);
        gtk_widget_show(taskData[i][2]);
        gtk_widget_show(taskData[i][3]);
        gtk_widget_show(taskData[i][4]);

        gtk_container_add(GTK_CONTAINER(but[i]), tempIn[i]);
        gtk_widget_show(tempIn[i]);

        gtk_container_add(GTK_CONTAINER(tempOut[i]), but[i]);
        gtk_widget_show(but[i]);
        g_signal_connect(GTK_WIDGET(but[i]), "clicked", G_CALLBACK(killTask), NULL);

        gtk_container_add(GTK_CONTAINER(box), tempOut[i]);
    }

    gtk_widget_show(box);
    gtk_widget_show(scrolledWindow);

    // GThread *mainThread;
    // mainThread = g_thread_new(NULL, mainCallBack, NULL);
    // g_thread_join(mainThread);
    // gtk_main();
    // gdk_threads_leave();

    startFunction();
    GThread *mainThread;
    mainThread = g_thread_create(mainCallBack, NULL, TRUE, NULL);
    g_thread_join(mainThread);
    //gtk_main();
    gdk_threads_leave();

    return 0;
}