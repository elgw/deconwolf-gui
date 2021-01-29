#include <gtk/gtk.h>

// Function to open a dialog box with a message
void
dw_app_get_new_scope(GtkWindow *parent)
{
 GtkWidget *dialog, *label, *content_area;
 GtkDialogFlags flags;

 // Create the widgets
 flags = GTK_DIALOG_DESTROY_WITH_PARENT;

 dialog = gtk_dialog_new_with_buttons ("Message",
                                       parent,
                                       flags,
                                       "Cancel",
                                       GTK_RESPONSE_NONE,
                                       "Ok",
                                       GTK_RESPONSE_ACCEPT,
                                       NULL);
 content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
 label = gtk_label_new ("Here you will be able to add a new microscope. For the moment, please edit the provided ones by clicking the cells in the table.");


 gtk_container_add (GTK_CONTAINER (content_area), label);
 gtk_widget_show_all(content_area);

 int result = gtk_dialog_run (GTK_DIALOG (dialog));
 switch (result)
 {
 case GTK_RESPONSE_ACCEPT:
     printf("Parse before delete...\n");
     break;
 default:
     // do_nothing_since_dialog_was_cancelled ();
     break;
 }
 gtk_widget_destroy (dialog);


}
