#include <gtk/gtk.h>

// Function to open a dialog box with a message
DwChannel *
dw_app_get_new_channel(GtkWindow *parent, DwChannel * old_channel)
{
 GtkWidget *dialog, *content_area;
 GtkDialogFlags flags;

 // Create the widgets
 flags = GTK_DIALOG_DESTROY_WITH_PARENT;

 char * msg = malloc(1024);
 if(old_channel == NULL)
 {
     sprintf(msg, "Add a new channel");
 } else {
     sprintf(msg, "Edit an existing channel");
 }

 dialog = gtk_dialog_new_with_buttons (msg,
                                       parent,
                                       flags,
                                       "Cancel",
                                       GTK_RESPONSE_NONE,
                                       "Ok",
                                       GTK_RESPONSE_ACCEPT,
                                       NULL);
 free(msg);
 content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));


 GtkWidget * lAlias = gtk_label_new("Alias");
 GtkWidget * eAlias = gtk_entry_new();

 GtkWidget * lName = gtk_label_new("Full name");
 GtkWidget * eName = gtk_entry_new();
 GtkWidget * lLambda = gtk_label_new("Emission maxima [nm]");
 GtkWidget * eLambda = gtk_entry_new();
 GtkWidget * lNiter = gtk_label_new("Number of iterations");
 GtkWidget * eNiter = gtk_entry_new();

 if(old_channel != NULL)
 {
     char * buff = malloc(1024);
     gtk_entry_set_text((GtkEntry*) eName, old_channel->name);
     gtk_entry_set_text((GtkEntry*) eAlias, old_channel->alias);

     sprintf(buff, "%d", old_channel->niter);
     gtk_entry_set_text((GtkEntry*) eNiter, buff);
     sprintf(buff, "%f", old_channel->lambda);
     gtk_entry_set_text((GtkEntry*) eLambda, buff);
     free(buff);
 }

GtkWidget * grid = gtk_grid_new();
gtk_grid_set_row_spacing ((GtkGrid*) grid , 5);
gtk_grid_set_column_spacing ((GtkGrid*) grid , 5);

gtk_grid_attach((GtkGrid*) grid, lAlias, 1, 1, 1, 1);
gtk_grid_attach((GtkGrid*) grid, eAlias, 2, 1, 1, 1);
gtk_grid_attach((GtkGrid*) grid, lName, 1, 2, 1, 1);
gtk_grid_attach((GtkGrid*) grid, eName, 2, 2, 1, 1);
gtk_grid_attach((GtkGrid*) grid, lLambda, 1, 3, 1, 1);
gtk_grid_attach((GtkGrid*) grid, eLambda, 2, 3, 1, 1);
gtk_grid_attach((GtkGrid*) grid, lNiter, 1, 4, 1, 1);
gtk_grid_attach((GtkGrid*) grid, eNiter, 2, 4, 1, 1);

GtkWidget * hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
GtkWidget * im = gtk_image_new_from_resource("/images/FluorescentCells.jpg");

gtk_box_pack_end((GtkBox*) hbox, im, FALSE, TRUE, 5);
gtk_box_pack_start((GtkBox*) hbox, grid, FALSE, TRUE, 5);

 gtk_container_add (GTK_CONTAINER (content_area),  hbox);
 gtk_widget_show_all(content_area);

 int result = gtk_dialog_run (GTK_DIALOG (dialog));
 DwChannel * channel = NULL;
 switch (result)
 {
 case GTK_RESPONSE_ACCEPT:
     channel = malloc(sizeof(DwChannel));
     channel->name = strdup(gtk_entry_get_text((GtkEntry*) eName));
     channel->alias = strdup(gtk_entry_get_text((GtkEntry*) eAlias));
     channel->lambda = atof(gtk_entry_get_text((GtkEntry*) eLambda));
     channel->niter = atoi(gtk_entry_get_text((GtkEntry*) eNiter));
     break;
 default:
     // do_nothing_since_dialog_was_cancelled ();
     break;
 }
 gtk_widget_destroy (dialog);

 return channel;
}
