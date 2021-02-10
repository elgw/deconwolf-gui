#include "dw_channel.h"

DwChannel * dw_channel_new()
{
    DwChannel * chan = malloc(sizeof(DwChannel));
    chan->name = NULL;
    chan->alias = NULL;
    return chan;
}

void dw_channel_free(DwChannel * chan)
{
    if(chan == NULL)
        return;

    if(chan->name != NULL)
        free(chan->name);
    if(chan->alias != NULL)
        free(chan->alias);
    free(chan);
}

void dw_channels_free(DwChannel ** channels)
{
    if(channels == NULL)
    {
        return;
    }
    int pos = 0;
    while(channels[pos] != NULL)
    {
        dw_channel_free(channels[pos]);
        pos++;
    }
    free(channels);
}


DwChannel ** dw_channels_get_from_gtk_tree_view(GtkTreeView * tv)
    {
     // Get a list of all the channels.
     // 1, Count the number of channels
     // 2, Allocate the list
     // 3, Populate the list
     // Get Model

     GtkTreeModel * model = gtk_tree_view_get_model (tv);

     GtkTreeIter iter;

     gboolean valid = gtk_tree_model_get_iter_first (model, &iter);
     if(valid == FALSE)
     {
         return NULL;
     }
     // Figure out how many rows there are
     gint nchan = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), NULL);

     //printf("There are %d channels\n", nchan); fflush(stdout);
     if(nchan < 1)
     {
         return NULL;
     }

     DwChannel ** clist = malloc( (nchan+1) * sizeof(DwChannel*));
     clist[nchan] = NULL; // A null terminates the list

     // Get all files and add to list.
     gint pos = 0;
     while (valid)
     {
         assert(pos < nchan);
         gchar *alias;
         gchar *name;
         gint niter;
         gfloat lambda;

         gtk_tree_model_get (model, &iter,
                             cALIAS_COLUMN, &alias,
                             cNAME_COLUMN, &name,
                             cNITER_COLUMN, &niter,
                             cEMISSION_COLUMN, &lambda,
                             -1);

         clist[pos] = malloc(sizeof(DwChannel));
         clist[pos]->name = strdup(name);
         clist[pos]->alias = strdup(alias);
         clist[pos]->lambda = (float) lambda;
         clist[pos]->niter = (int) niter;
         if(0){
             printf("%s %s %f %d\n",
                    clist[pos]->name, clist[pos]->alias,
                    clist[pos]->lambda, clist[pos]->niter);
         }
         g_free(alias);
         g_free(name);

         pos++;

         valid = gtk_tree_model_iter_next (model,
                                           &iter);

     }

     return clist;
    }


DwChannel * dw_channels_get_by_alias(DwChannel ** channels, char * alias)
{
    int pos = 0;
    while(channels[pos] != NULL)
    {
        if(strcasecmp(alias, channels[pos]->alias) == 0)
        {
            return channels[pos];
        }
        pos++;
    }
    return NULL;
}


void dw_chan_to_key_file(DwChannel * chan, GKeyFile * kf)
{
    char * alias = chan->alias;
    if(alias == NULL)
    {
        printf("unable to save channel -- no alias\n");
        return;
    }
    g_key_file_set_string (kf, alias, "Name", chan->name);
    g_key_file_set_double(kf, alias, "lambda", (double) chan->lambda);
    g_key_file_set_integer(kf, alias, "iter", chan->niter);
    return;
}


DwChannel ** dw_channels_from_disk(char * fname)
{
 GError * error = NULL;
 GKeyFile * key_file = g_key_file_new ();

 if (!g_key_file_load_from_file (key_file, fname, G_KEY_FILE_NONE, &error))
 {
     if (!g_error_matches (error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
         g_warning ("Error loading key file: %s", error->message);
     g_error_free(error);
     g_key_file_free(key_file);
     return NULL;
 }

 gsize length;
 gchar ** groups =
 g_key_file_get_groups (key_file,
                        &length);
 if(length == 0)
 {
     printf("Can't parse anything from %s\n", fname);
     g_key_file_free(key_file);
     g_error_free(error);
     return NULL;
 }

 DwChannel ** channels = malloc((length+1)*sizeof(DwChannel*));
 channels[length] = NULL; // End of array

 for(gsize kk = 0; kk < length; kk++)
 {
     channels[kk] = dw_channel_new();
     DwChannel * chan = channels[kk];
     gchar * group = groups[kk]; // I.e. Alias
     chan->alias = strdup(group);

     // Read name
     gchar *val = g_key_file_get_string (key_file, group, "Name", &error);
     if (val == NULL)
     {
         val = g_strdup ("Give me a name");
     }
     chan->name = strdup(val);
     free(val);
     gint niter = g_key_file_get_integer(key_file, group, "iter", &error);
     chan->niter = niter;

     gdouble lambda = g_key_file_get_double(key_file, group, "lambda", &error);
     chan->lambda = lambda;
 }
 g_assert(channels[length] == NULL);
 g_strfreev(groups);
 g_key_file_free(key_file);
 return channels;
}

gboolean
color_draw_cb (GtkWidget    *widget,
               cairo_t  * cr,
               gpointer      user_data)
{
    UNUSED(widget);

    double * lambda = (double*) user_data;
    DwRGB * C = dw_RGB_new_from_lambda(lambda[0]);
    cairo_set_source_rgb (cr, C->R, C->G, C->B);
    free(C);
    cairo_paint (cr);
    return TRUE;
}

typedef struct {
    double * lambda;
    GtkWidget * widget;
} UserData1;

gboolean elambda_event(GtkWidget * widget,
                       GdkEvent  *event,
                       gpointer user_data)
{
    UNUSED(event);
    UserData1 * u = (UserData1*) user_data;
    u->lambda[0] = atof(gtk_entry_get_text((GtkEntry*) widget));
    // printf("Got lambda = %f\n", u->lambda[0]);
    // Force redraw of the box
    gtk_widget_queue_draw(u->widget);
    return FALSE; // pass on event

}

// Function to open a dialog box with a message
DwChannel *
dw_channel_edit_dlg(GtkWindow *parent, DwChannel * old_channel)
{
 GtkWidget *dialog, *content_area;
 GtkDialogFlags flags;

 // Create the widgets
 flags = GTK_DIALOG_DESTROY_WITH_PARENT;
 double lambda = 0;

 char * msg = malloc(1024);
 if(old_channel == NULL)
 {
     sprintf(msg, "Add a new channel");
 } else {
     sprintf(msg, "Edit an existing channel");
     lambda = old_channel->lambda;
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
 GtkWidget * lColor = gtk_label_new("Color:");
 GtkWidget * eColor = gtk_event_box_new();
 g_signal_connect(eColor, "draw",
                  G_CALLBACK(color_draw_cb), &lambda);
 UserData1 ud;
 ud.lambda = &lambda;
 ud.widget = eColor;

 g_signal_connect(eLambda, "key-release-event",
                  G_CALLBACK(elambda_event), &ud);

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
gtk_grid_attach((GtkGrid*) grid, lColor, 1, 4, 1, 1);
gtk_grid_attach((GtkGrid*) grid, eColor, 2, 4, 1, 1);
gtk_grid_attach((GtkGrid*) grid, lNiter, 1, 5, 1, 1);
gtk_grid_attach((GtkGrid*) grid, eNiter, 2, 5, 1, 1);

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


void dw_channels_to_disk(DwChannel ** channels, char * file)
{
    printf("Saving channels to %s\n", file);
    GKeyFile * key_file = g_key_file_new();
    int pos = 0;
    while(channels[pos] != NULL)
    {
        dw_chan_to_key_file(channels[pos], key_file);
        pos++;
    }
    GError * error = NULL;
    if (!g_key_file_save_to_file (key_file, file, &error))
    {
        g_warning ("Error saving key file: %s", error->message);
        g_error_free(error);
    }
    g_key_file_free(key_file);

}
