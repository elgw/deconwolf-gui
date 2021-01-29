#include <gtk/gtk.h>

// Function to open a dialog box with a message
dwscope *
dw_app_get_new_scope(GtkWindow *parent, dwscope * old_scope)
{
 GtkWidget *dialog, *content_area;
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


 GtkWidget * lName = gtk_label_new("Name");
 GtkWidget * eName = gtk_entry_new();
 gtk_entry_set_text((GtkEntry*) eName, "Manufacturer, Floor, Room, etc");
 GtkWidget * lNA = gtk_label_new("Numerical aperture");
 GtkWidget * eNA = gtk_entry_new();
 GtkWidget * lni = gtk_label_new("Refractive index of immersion");
 GtkWidget * eni = gtk_entry_new();
 GtkWidget * ldx = gtk_label_new("Pixel size dx (=dy) [nm]");
 GtkWidget * edx = gtk_entry_new();
 GtkWidget * ldz = gtk_label_new("Distance between planes, dz [nm]");
 GtkWidget * edz = gtk_entry_new();

 if(old_scope != NULL)
 {
     char * buff = malloc(1024);
     gtk_entry_set_text((GtkEntry*) eName, old_scope->name);
     sprintf(buff, "%f", old_scope->NA);
     gtk_entry_set_text((GtkEntry*) eNA, buff);
     sprintf(buff, "%f", old_scope->ni);
     gtk_entry_set_text((GtkEntry*) eni, buff);
     sprintf(buff, "%f", old_scope->xy_nm);
     gtk_entry_set_text((GtkEntry*) edx, buff);
     sprintf(buff, "%f", old_scope->z_nm);
     gtk_entry_set_text((GtkEntry*) edz, buff);
     free(buff);
 }

GtkWidget * grid = gtk_grid_new();
gtk_grid_attach((GtkGrid*) grid, lName, 1, 1, 1, 1);
gtk_grid_attach((GtkGrid*) grid, eName, 2, 1, 1, 1);
gtk_grid_attach((GtkGrid*) grid, lNA, 1, 2, 1, 1);
gtk_grid_attach((GtkGrid*) grid, eNA, 2, 2, 1, 1);
gtk_grid_attach((GtkGrid*) grid, lni, 1, 3, 1, 1);
gtk_grid_attach((GtkGrid*) grid, eni, 2, 3, 1, 1);
gtk_grid_attach((GtkGrid*) grid, ldx, 1, 4, 1, 1);
gtk_grid_attach((GtkGrid*) grid, edx, 2, 4, 1, 1);
gtk_grid_attach((GtkGrid*) grid, ldz, 1, 5, 1, 1);
gtk_grid_attach((GtkGrid*) grid, edz, 2, 5, 1, 1);

GtkWidget * hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
GtkWidget * im = gtk_image_new_from_resource("/images/Compound_Microscope_(cropped).jpeg");

gtk_box_pack_end((GtkBox*) hbox, im, FALSE, TRUE, 5);
gtk_box_pack_start((GtkBox*) hbox, grid, FALSE, TRUE, 5);

 gtk_container_add (GTK_CONTAINER (content_area),  hbox);
 gtk_widget_show_all(content_area);

 int result = gtk_dialog_run (GTK_DIALOG (dialog));
 dwscope * scope = NULL;
 switch (result)
 {
 case GTK_RESPONSE_ACCEPT:
     scope = malloc(sizeof(scope));
     scope->name = strdup(gtk_entry_get_text((GtkEntry*) eName));
     scope->NA = atof(gtk_entry_get_text((GtkEntry*) eNA));
     scope->ni = atof(gtk_entry_get_text((GtkEntry*) eni));
     scope->xy_nm = atof(gtk_entry_get_text((GtkEntry*) edx));
     scope->z_nm = atof(gtk_entry_get_text((GtkEntry*) edz));
     break;
 default:
     // do_nothing_since_dialog_was_cancelled ();
     break;
 }
 gtk_widget_destroy (dialog);

 return scope;
}
