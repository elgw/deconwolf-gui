#include <gtk/gtk.h>


typedef struct{
    double pf1; // progressbar fraction 1
    double pf2; // progressbar fraction 2
    char * cmd;
    GtkProgressBar * pb1;
    GtkProgressBar * pb2;
    GtkTextView * status;
    gboolean active;
} JobData;

JobData * job_data_new(char * cmd,
                       GtkProgressBar * pb1, GtkProgressBar * pb2,
                       GtkTextView * status)
{
    JobData * j = g_malloc0(sizeof(JobData));
    j->cmd = cmd;
    j->pb1 = pb1;
    j->pb2 = pb2;
    j->pf1 = 0;
    j->pf2 = 0;
    j->status = status;
    j->active = TRUE;
    return j;
}

gboolean setpb(gpointer data)
{
    JobData * j = (JobData*) data;
    if(j->active == TRUE)
    {
        gtk_progress_bar_set_fraction(j->pb1, j->pf1);
        gtk_progress_bar_set_fraction(j->pb2, j->pf2);
    }
    return FALSE; // Not to be called again
}

gpointer run_commands_th(gpointer j0)
{
/* TODO
 - Check if cancel is pressed
 - Redirect stdout from commands to TextView
 - ...
 */
    JobData * j = (JobData*) j0;
    for(int a = 0; a<10; a++)
    {
        //gtk_progress_bar_set_fraction(j->pb1, (a+1.0)/10);
        for(int b = 0; b<10; b++)
        {
            j->pf1 = (a+1.0)/10;
            j->pf2 = (b+1.0)/10;
            g_idle_add (setpb, j);
            g_usleep(100000);
            if(j->active == FALSE)
            {
                return NULL;
            }
        }
    }
    return NULL;
}

// Run commands
void
dw_app_runner(GtkWindow *parent, char * commands)
{
 GtkWidget *dialog, *content_area;
 GtkDialogFlags flags;

 // Create the widgets
 flags = GTK_DIALOG_DESTROY_WITH_PARENT;

 dialog = gtk_window_new();
#ifdef GTK3
 "Running ...",
                                       parent,
                                       flags,
                                       "Cancel",
                                       GTK_RESPONSE_ACCEPT,
                                       NULL);
content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
#endif
content_area = dialog;


 GtkWidget * status = gtk_text_view_new();
 g_object_set(G_OBJECT(status), "editable", FALSE, NULL);

 GtkWidget * status_sb = gtk_scrolled_window_new();
gtk_grid_attach((GtkGrid *) status_sb, status, 0,0,1, 1);

 GtkWidget * pb1 = gtk_progress_bar_new(); // Command number
 GtkWidget * pb2 = gtk_progress_bar_new(); // If deconwolf, update on the current iteration

 GtkWidget * box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
 #ifdef GTK3
 gtk_box_pack_start((GtkBox*) box, status_sb, TRUE, TRUE, 5);
 gtk_box_pack_end((GtkBox*) box, pb2, FALSE, FALSE, 5);
 gtk_box_pack_end((GtkBox*) box, pb1, FALSE, FALSE, 5);
#endif

gtk_grid_attach ((GtkGrid*) content_area, box, 0, 0, 1, 1);

 JobData * job_data = job_data_new(commands, (GtkProgressBar*) pb1, (GtkProgressBar*) pb2, (GtkTextView*) status);;
 GThread * runthread = g_thread_new("run-dw-thread", run_commands_th, job_data);

 #ifdef GTK3
 int result = gtk_dialog_run (GTK_DIALOG (dialog));
 #else
 int result = 0;
 #endif
 job_data->active = FALSE;
 switch (result)
 {
 case GTK_RESPONSE_ACCEPT:
     printf("Cancel pressed...\n");
     break;
 default:
     // do_nothing_since_dialog_was_cancelled ();
     break;
 }
g_thread_join(runthread);


#ifdef GTK3
 gtk_widget_destroy (dialog);
 #endif

 g_free(job_data); // Something with reference counting goes bananas with this.

}
