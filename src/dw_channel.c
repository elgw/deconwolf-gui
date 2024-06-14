#include "dw_channel.h"


DwChannel * dw_channel_new()
{
    DwChannel * chan = g_malloc0(sizeof(DwChannel));
    chan->name = NULL;
    chan->alias = NULL;
    return chan;
}

void dw_channel_free(DwChannel * chan)
{
    if(chan == NULL)
        return;

    g_free(chan->name);
    g_free(chan->alias);
    g_free(chan);
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
    g_free(channels);
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

    DwChannel ** clist = g_malloc0( (nchan+1) * sizeof(DwChannel*));
    clist[nchan] = NULL; // A null terminates the list

    // Get all files and add to list.
    gint pos = 0;
    while (valid)
    {
        assert(pos < nchan);
        gchar *alias;
        gchar *name;
        gint niter;
        gchar *lambda;

        gtk_tree_model_get (model, &iter,
                            cALIAS_COLUMN, &alias,
                            cNAME_COLUMN, &name,
                            cNITER_COLUMN, &niter,
                            cEMISSION_COLUMN, &lambda,
                            -1);

        clist[pos] = g_malloc0(sizeof(DwChannel));
        clist[pos]->name = g_strdup(name);
        clist[pos]->alias = g_strdup(alias);
        clist[pos]->lambda = atof(lambda);
        clist[pos]->niter = (int) niter;
        if(0){
            printf("%s %s %f %d\n",
                   clist[pos]->name, clist[pos]->alias,
                   clist[pos]->lambda, clist[pos]->niter);
        }
        g_free(alias);
        g_free(name);
        g_free(lambda);

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
        if(strcmp(alias, channels[pos]->alias) == 0)
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

    DwChannel ** channels = g_malloc0((length+1)*sizeof(DwChannel*));
    channels[length] = NULL; // End of array

    for(gsize kk = 0; kk < length; kk++)
    {
        channels[kk] = dw_channel_new();
        DwChannel * chan = channels[kk];
        gchar * group = groups[kk]; // I.e. Alias
        chan->alias = g_strdup(group);

        // Read name
        gchar *val = g_key_file_get_string (key_file, group, "Name", &error);
        if (val == NULL)
        {
            val = g_strdup ("Give me a name");
        }
        chan->name = g_strdup(val);
        g_free(val);
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


typedef struct {
    double lambda;
    GtkWidget * widget;
} UserData1;


gboolean
color_draw_cb (
    GtkDrawingArea* drawing_area,
    cairo_t* cr,
    int width,
    int height,
    gpointer user_data)
{
    UserData1 * ud = (UserData1*) user_data;
    DwRGB * C = dw_RGB_new_from_lambda(ud->lambda);
    cairo_set_source_rgb (cr, C->R, C->G, C->B);
    g_free(C);
    cairo_paint (cr);
    return TRUE;
}

// https://docs.gtk.org/gtk4/signal.Editable.changed.html
/* Called when the lambda is changed */
gboolean elambda_event(GtkWidget * widget,
                       gpointer user_data)
{
    GtkEntry * entry = GTK_ENTRY(widget);
    UserData1 * u = (UserData1*) user_data;
    u->lambda = atof(gtk_editable_get_text(entry));
    // Force redraw of the box
    gtk_widget_queue_draw(u->widget);
    return FALSE; // pass on event
}


void channel_new()
{
    #if 0
DwChannel * channel = NULL;
int result = 0;
switch (result)
{
case GTK_RESPONSE_ACCEPT:
    channel = g_malloc0(sizeof(DwChannel));
    channel->name = g_strdup(gtk_entry_buffer_get_text(gtk_entry_get_buffer((GtkEntry *) eName)));
    channel->alias = g_strdup(gtk_entry_buffer_get_text(gtk_entry_get_buffer(eAlias)));
    channel->lambda = atof(gtk_entry_buffer_get_text(gtk_entry_get_buffer(eLambda)));
    channel->niter = atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer((eNiter))));

    break;
default:
    // do_nothing_since_dialog_was_cancelled ();
    break;

}

return channel;
#endif
}

bool cb_dw_channels_ok(GtkWidget * widget, gpointer * p)
{
    // g_object_set_data  to store stuff associated with the window?
    printf("TODO\n");
    GtkWindow * window = GTK_WINDOW(p);
    gtk_window_close(GTK_WINDOW(window));
    return true;
}

bool cb_dw_channels_close(GtkWidget * widget, gpointer * p)
{
    GtkWindow * window = GTK_WINDOW(p);
    gtk_window_close(GTK_WINDOW(window));
    return true;
}

// This used to be a blocking thing, but there is no reason that it has to be that.
// However if it isn't blocking it can't return a DwChannel
// https://discourse.gnome.org/t/how-should-i-replace-a-gtk-dialog-run-in-gtk-4/35015/
// Also we don't need to re-create it all the time. It could actually be constructed at
// program start and shown/hidden as needed with new data.
void
dw_channel_edit_dlg(GtkWindow *parent, DwChannel * old_channel)
{
    GtkWindow *dialog;
    GtkWidget *content_area;
    GtkDialogFlags flags;

    // TODO: Free this one when window close
    UserData1 * ud = calloc(1, sizeof(UserData1));

    char * msg = g_malloc0(1024);
    if(old_channel == NULL)
    {
        sprintf(msg, "Add a new channel");
    } else {
        sprintf(msg, "Edit an existing channel");
        ud->lambda = old_channel->lambda;
    }


    dialog = gtk_window_new();
    gtk_window_set_modal(dialog, true);
    gtk_window_set_title(dialog, msg);
    g_free(msg);


    GtkWidget * lAlias = gtk_label_new("Alias");
    GtkWidget * eAlias = gtk_entry_new();
    GtkWidget * lName = gtk_label_new("Full name");
    GtkWidget * eName = gtk_entry_new();
    GtkWidget * lLambda = gtk_label_new("Emission maxima [nm]");
    GtkWidget * eLambda = gtk_entry_new();
    GtkWidget * lNiter = gtk_label_new("Number of iterations");
    GtkWidget * eNiter = gtk_entry_new();
    GtkWidget * lColor = gtk_label_new("Color:");

    GtkWidget * eColor = gtk_drawing_area_new();
    ud->widget = eColor;
    gtk_drawing_area_set_content_width(eColor, 100);
    gtk_drawing_area_set_content_height(eColor, 50);


    gtk_drawing_area_set_draw_func(eColor, (GtkDrawingAreaDrawFunc) color_draw_cb, ud, NULL);

    g_signal_connect(GTK_EDITABLE(eLambda), "changed", G_CALLBACK(elambda_event), ud);

    if(old_channel != NULL)
    {
        char * buff = g_malloc0(1024);
        gtk_entry_set_text((GtkEntry*) eName, old_channel->name);
        gtk_entry_set_text((GtkEntry*) eAlias, old_channel->alias);

        sprintf(buff, "%d", old_channel->niter);
        gtk_entry_set_text((GtkEntry*) eNiter, buff);
        sprintf(buff, "%f", old_channel->lambda);
        gtk_entry_set_text((GtkEntry*) eLambda, buff);
        g_free(buff);
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

    gtk_widget_set_halign((GtkWidget*) grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget*) grid, GTK_ALIGN_CENTER);
    GtkWidget * hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget * im = gtk_image_new_from_resource("/images/FluorescentCells.jpg");
    gtk_widget_set_margin_bottom((GtkWidget*) im, 20);
    gtk_widget_set_margin_top((GtkWidget*) im, 20);

    gtk_box_append(hbox, grid);
    gtk_widget_set_size_request(im, 256, 256);
    gtk_box_append(hbox, im);

    gtk_widget_set_halign((GtkWidget*) hbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget*) hbox, GTK_ALIGN_CENTER);

    // Box with the main content in top
    // and Ok/Cancel buttons in the bottom
    GtkBox * vbox0 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_box_append(vbox0, hbox);

    GtkButton * btn_ok = gtk_button_new_with_label("Ok");
    g_signal_connect(btn_ok, "clicked", cb_dw_channels_ok, dialog);
    GtkButton * btn_cancel = gtk_button_new_with_label("Cancel");
    g_signal_connect(btn_cancel, "clicked", cb_dw_channels_close, dialog);
    GtkBox * box_btn = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_box_append(box_btn, btn_cancel);
    gtk_box_append(box_btn, btn_ok);
    gtk_box_append(vbox0, box_btn);
    gtk_window_set_child (dialog,  vbox0);
    gtk_widget_show(dialog);
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
