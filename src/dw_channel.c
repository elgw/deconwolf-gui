#include "dw_channel.h"

typedef struct {
    GtkWindow * window; // Dialog Window
    double lambda;
    GtkWidget * eLambda;
    GtkWidget * eAlias;
    GtkWidget * eName;
    GtkWidget * eNiter;
    GtkWidget * eColor;
    void (*callback) (DwChannel*);
} DwChannelState;

static DwChannelState * state = NULL;

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


/*
 * GUI
 */

gboolean
color_draw_cb (
    GtkDrawingArea* drawing_area,
    cairo_t* cr,
    int width,
    int height,
    gpointer user_data)
{
    DwRGB * C = dw_RGB_new_from_lambda(state->lambda);
    cairo_set_source_rgb (cr, C->R, C->G, C->B);
    g_free(C);
    cairo_paint (cr);
    return TRUE;
}

/* Called when the value of lambda is changed
 * to update the colorful box below */
gboolean elambda_event(GtkWidget * widget,
                       gpointer user_data)
{
    UNUSED(user_data);
    GtkEntry * entry = GTK_ENTRY(widget);
    state->lambda = atof(gtk_editable_get_text( GTK_EDITABLE(entry)));
    // Force redraw of the box
    gtk_widget_queue_draw(state->eColor);
    return FALSE; // pass on event
}

/* When the user clicks ok, propagate information in the window
 * to the callback function and hide.
 * TODO: Add validation here
 */
bool cb_dw_channels_ok(GtkWidget * widget, gpointer * p)
{
    if(state->callback == NULL)
    {
        printf("No callback!\n");
        return true;
    }

    DwChannel * chan = g_malloc0(sizeof(DwChannel));

    chan->name = g_strdup(gtk_editable_get_text( GTK_EDITABLE(state->eName)));
    chan->alias = g_strdup(gtk_editable_get_text( GTK_EDITABLE(state->eAlias)));
    chan->lambda = atof(gtk_editable_get_text( GTK_EDITABLE(state->eLambda)));
    chan->niter = atoi(gtk_editable_get_text( GTK_EDITABLE(state->eNiter)));

    state->callback(chan);
    dw_channel_free(chan);
    gtk_widget_set_visible(GTK_WIDGET(state->window), false);
    //gtk_widget_hide(GTK_WIDGET(state->window));
    return true;
}

/* Overrides the close action to hide the window instead */
bool cb_dw_channels_close(GtkWidget * widget, gpointer * p)
{
    UNUSED(widget);
    UNUSED(p);
    //gtk_widget_hide(GTK_WIDGET(state->window));
    gtk_widget_set_visible(GTK_WIDGET(state->window), false);
    return true;
}

/* Set the data in the window */
void dw_channel_edit_set(DwChannel * channel)
{
    gtk_window_set_title(GTK_WINDOW(state->window), "Edit an existing channel");
    gtk_editable_set_text( GTK_EDITABLE(state->eName), channel->name);
    gtk_editable_set_text( GTK_EDITABLE(state->eAlias), channel->alias);
    char * buff = g_malloc0(1024);
    sprintf(buff, "%d", channel->niter);
    gtk_editable_set_text( GTK_EDITABLE(state->eNiter), buff);
    sprintf(buff, "%f", channel->lambda);
    gtk_editable_set_text( GTK_EDITABLE(state->eLambda), buff);
    g_free(buff);
}

/* Reset the data in the window */
void dw_channel_edit_reset()
{
    assert(state != NULL);
    assert(state->window != NULL);
    gtk_window_set_title( GTK_WINDOW(state->window), "Add a new channel");
    gtk_editable_set_text( GTK_EDITABLE(state->eName), "-");
    gtk_editable_set_text( GTK_EDITABLE(state->eAlias), "-");
    gtk_editable_set_text( GTK_EDITABLE(state->eNiter), "50");
    gtk_editable_set_text( GTK_EDITABLE(state->eLambda), "500");
}

/* Display the window */
void dw_channel_edit_show()
{
    gtk_window_present(state->window);
}

/* Intitialize the window with all GUI components
 * also initialize the state object
 * The window remains hidden until displayed
 */
void
dw_channel_edit_init()
{
    GtkWindow *dialog;

    state = g_malloc0(sizeof(DwChannelState));
    dialog = (GtkWindow*) gtk_window_new();
    state->window = dialog;
    gtk_window_set_modal(dialog, true);

    GtkWidget * lAlias = (GtkWidget*) gtk_label_new("Alias");
    GtkWidget * eAlias = (GtkWidget*) gtk_entry_new();
    state->eAlias = eAlias;
    GtkWidget * lName = (GtkWidget*) gtk_label_new("Full name");
    GtkWidget * eName = (GtkWidget*) gtk_entry_new();
    state->eName = eName;
    GtkWidget * lLambda = (GtkWidget*) gtk_label_new("Emission maxima [nm]");
    GtkWidget * eLambda = (GtkWidget*)  gtk_entry_new();
    state->eLambda = eLambda;
    GtkWidget * lNiter = (GtkWidget*) gtk_label_new("Number of iterations");
    GtkWidget * eNiter = (GtkWidget*) gtk_entry_new();
    state->eNiter = eNiter;
    GtkWidget * lColor = (GtkWidget*) gtk_label_new("Color:");

    GtkWidget * eColor = (GtkWidget*) gtk_drawing_area_new();
    state->eColor = eColor;

    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(eColor), 100);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(eColor), 50);

    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(eColor),
                                   (GtkDrawingAreaDrawFunc) color_draw_cb, NULL, NULL);

    g_signal_connect(GTK_EDITABLE(eLambda), "changed", G_CALLBACK(elambda_event), NULL);

    GtkWidget * grid = (GtkWidget*) gtk_grid_new();
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
    GtkWidget * hbox = (GtkWidget*) gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget * im = (GtkWidget*) gtk_image_new_from_resource("/images/FluorescentCells.jpg");
    gtk_widget_set_margin_bottom((GtkWidget*) im, 20);
    gtk_widget_set_margin_top((GtkWidget*) im, 20);


    gtk_widget_set_size_request(im, 200, 200);
    g_object_set ( im,
                   "margin-start", 50,
                   "margin-end", 50,
                   NULL);
    gtk_box_append(GTK_BOX(hbox), im);
    gtk_box_append(GTK_BOX(hbox), grid);

    gtk_widget_set_halign(hbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(hbox, GTK_ALIGN_CENTER);

    // Box with the main content in top
    // and Ok/Cancel buttons in the bottom
    GtkBox * vbox0 = (GtkBox *) gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_box_append(vbox0, hbox);

    GtkButton * btn_ok = (GtkButton *) gtk_button_new_with_label("Ok");
    g_signal_connect(btn_ok, "clicked", G_CALLBACK(cb_dw_channels_ok), dialog);
    GtkButton * btn_cancel = (GtkButton *) gtk_button_new_with_label("Cancel");
    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(cb_dw_channels_close), dialog);
    GtkBox * box_btn = (GtkBox *) gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_box_append(box_btn, GTK_WIDGET(btn_cancel));
    gtk_box_append(box_btn, GTK_WIDGET(btn_ok));
    gtk_box_append(vbox0, GTK_WIDGET(box_btn));
    gtk_window_set_child (GTK_WINDOW(dialog),  GTK_WIDGET(vbox0));

    g_signal_connect(G_OBJECT(dialog),
                     "close-request", G_CALLBACK(cb_dw_channels_close), NULL);
}

/* Set the callback, i.e. where to send the information when the user
 * clicks ok
 */
void dw_channel_edit_set_callback( void  (*callback) (DwChannel*))
{
    state->callback = callback;
}
