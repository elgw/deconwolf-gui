#include "dw_scope.h"

void dw_scope_free(DwScope * scope)
{
    if(scope == NULL)
        return;

    if(scope->name != NULL)
        g_free(scope->name);
    g_free(scope);
}

DwScope * dw_scope_get_from_model(GtkTreeModel * model, GtkTreeIter * iter)
{
    DwScope * scope = g_malloc0(sizeof(DwScope));
    scope->name = NULL;

    gchar *name;
    gfloat NA, ni, xy_nm, z_nm;

    // Make sure you terminate calls to gtk_tree_model_get() with a “-1” value
    gtk_tree_model_get (model, iter,
                        sNAME_COLUMN, &name,
                        sNA_COLUMN, &NA,
                        sNI_COLUMN, &ni,
                        sDX_COLUMN, &xy_nm,
                        sDZ_COLUMN, &z_nm,
                        -1);

    scope->name = g_strdup(name);
    g_free(name);
    scope->NA = NA;
    scope->ni = ni;
    scope->xy_nm = xy_nm;
    scope->z_nm = z_nm;
    return scope;
}


DwScope ** dw_scopes_get_from_gtk_tree_view(GtkTreeView * tv)
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
    gint nscopes = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), NULL);

    //printf("There are %d channels\n", nchan); fflush(stdout);
    if(nscopes < 1)
    {
        return NULL;
    }

    DwScope ** slist = g_malloc0( (nscopes+1) * sizeof(DwScope*));
    slist[nscopes] = NULL; // A null terminates the list

    // Get all files and add to list.
    gint pos = 0;
    while (valid)
    {
        slist[pos++] = dw_scope_get_from_model(model, &iter);
        valid = gtk_tree_model_iter_next (model,
                                          &iter);
    }

    return slist;
}


DwScope * dwscope_get_selected_from_gtk_tree_view(GtkTreeView * tv)
{
    // Get the scope from the list of scopes

    GtkTreeModel * model = gtk_tree_view_get_model(tv);
    GtkTreeIter iter;
    GtkTreeSelection * selection = gtk_tree_view_get_selection(tv);

    if(gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        DwScope * scope = dw_scope_get_from_model(model, &iter);
        return scope;
    } else {
        return NULL;
    }
}


void dw_scope_to_key_file(DwScope * scope, GKeyFile * kf)
{
    char * name = scope->name;
    if(name == NULL)
    {
        printf("unable to save scope -- no name\n");
        return;
    }

    g_key_file_set_double(kf, name, "NA", (double) scope->NA);
    g_key_file_set_double(kf, name, "ni", (double) scope->ni);
    g_key_file_set_double(kf, name, "DX_NM", (double) scope->xy_nm);
    g_key_file_set_double(kf, name, "DZ_NM", (double) scope->z_nm);
    return;
}


void dw_scopes_to_disk(DwScope ** scopes, char * file)
{
    printf("Saving scopes to %s\n", file);
    GKeyFile * key_file = g_key_file_new();
    int pos = 0;
    while(scopes[pos] != NULL)
    {
        dw_scope_to_key_file(scopes[pos], key_file);
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

void dw_scopes_free(DwScope ** scopes)
{
    if(scopes == NULL)
    {
        return;
    }

    int pos = 0;
    while(scopes[pos] != NULL)
    {
        dw_scope_free(scopes[pos]);
        pos++;
    }
    g_free(scopes);
    return;
}



DwScope * dw_scope_new()
{
    DwScope * scope = g_malloc0(sizeof(DwScope));
    scope->name = NULL;
    scope->NA = 0;
    scope->ni=0;
    scope->xy_nm=0;
    scope->z_nm = 0;
    return scope;
}

DwScope ** dw_scopes_from_disk(char * fname)
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

    DwScope ** scopes = g_malloc0((length+1)*sizeof(DwScope*));
    scopes[length] = NULL; // End of array

    for(gsize kk = 0; kk<length; kk++)
    {
        scopes[kk] = dw_scope_new();
        DwScope * scope = scopes[kk];
        gchar * group = groups[kk]; // I.e. Alias
        scope->name = g_strdup(group);

        gdouble NA = g_key_file_get_double(key_file, group, "NA", &error);
        scope->NA = NA;
        gdouble ni = g_key_file_get_double(key_file, group, "ni", &error);
        scope->ni = ni;
        gdouble dx = g_key_file_get_double(key_file, group, "DX_NM", &error);
        scope->xy_nm = dx;
        gdouble dz = g_key_file_get_double(key_file, group, "DZ_NM", &error);
        scope->z_nm = dz;

    }
    g_assert(scopes[length] == NULL);
    g_strfreev(groups);
    g_key_file_free(key_file);
    return scopes;
}

void dw_scope_to_gtk_tree_store(DwScope * scope, GtkTreeStore* model, GtkTreeIter * iter)
{
    gtk_tree_store_set((GtkTreeStore*) model, iter,
                       sNAME_COLUMN, scope->name,
                       sNA_COLUMN, scope->NA,
                       sNI_COLUMN, scope->ni,
                       sDX_COLUMN, scope->xy_nm,
                       sDZ_COLUMN, scope->z_nm,
                       -1);
    return;
}


/*
 * GUI
 */

typedef struct {
    GtkWindow * window; // Dialog Window
    GtkWidget * eNA;
    GtkWidget * eni;
    GtkWidget * eName;
    GtkWidget * edx;
    GtkWidget * edz;
    void (*callback) (DwScope*);
} DwScopeState;

static DwScopeState * state = NULL;


bool cb_scope_close(GtkWidget * widget, gpointer * p)
{
    UNUSED(widget);
    UNUSED(p);
    gtk_widget_set_visible(GTK_WIDGET(state->window), false);
    return true;
}

bool cb_scope_ok(GtkWidget * widget, gpointer * p)
{

    DwScope * scope = g_malloc0(sizeof(DwScope));

    scope->name = g_strdup(gtk_editable_get_text(
                               GTK_EDITABLE( state->eName )));
    scope->NA = atof(gtk_editable_get_text(GTK_EDITABLE( state->eNA )));
    scope->ni = atof(gtk_editable_get_text(GTK_EDITABLE( state->eni)));
    scope->xy_nm = atof(gtk_editable_get_text(GTK_EDITABLE( state->edx )));
    scope->z_nm = atof(gtk_editable_get_text(GTK_EDITABLE( state->edz)));

    if(state->callback != NULL)
    {
        state->callback(scope);
    }

    dw_scope_free(scope);

    gtk_widget_set_visible(GTK_WIDGET(state->window), false);
    return NULL;
}

void dw_scope_edit_set_callback( void  (*callback) (DwScope *))
{
    state->callback = callback;
}

void dw_scope_edit_set(DwScope * old_scope)
{
    gtk_window_set_title(GTK_WINDOW(state->window), "Edit existing microscope");
    char * buff = g_malloc0(1024);
    gtk_editable_set_text( GTK_EDITABLE(state->eName), old_scope->name);
    sprintf(buff, "%f", old_scope->NA);
    gtk_editable_set_text( GTK_EDITABLE(state->eNA), buff);
    sprintf(buff, "%f", old_scope->ni);
    gtk_editable_set_text( GTK_EDITABLE(state->eni), buff);
    sprintf(buff, "%f", old_scope->xy_nm);
    gtk_editable_set_text( GTK_EDITABLE(state->edx), buff);
    sprintf(buff, "%f", old_scope->z_nm);
    gtk_editable_set_text( GTK_EDITABLE(state->edz), buff);
    g_free(buff);
}

void dw_scope_edit_reset(void)
{
    gtk_window_set_title(GTK_WINDOW(state->window), "Edit new microscope");
    gtk_editable_set_text( GTK_EDITABLE(state->eName), "BS2@100X");
    gtk_editable_set_text( GTK_EDITABLE(state->eNA), "1.45");
    gtk_editable_set_text( GTK_EDITABLE(state->eni), "1.52");
    gtk_editable_set_text( GTK_EDITABLE(state->edx), "65");
    gtk_editable_set_text( GTK_EDITABLE(state->edz), "200");
}

void dw_scope_edit_show()
{
    gtk_window_present(state->window);
}

void
dw_scope_edit_init()
{
    // Create the widgets
    GtkWidget * window = gtk_window_new();

    gtk_window_set_modal(GTK_WINDOW(window), true);

    state = (DwScopeState*) g_malloc0(sizeof(DwScopeState));
    state->window = GTK_WINDOW(window);

    GtkWidget * lName = gtk_label_new("Name");
    GtkWidget * eName = gtk_entry_new();
    //gtk_editable_set_text( GTK_EDITABLE(eName), "Manufacturer, Floor, Room, etc");
    GtkWidget * lNA = gtk_label_new("Numerical aperture");
    GtkWidget * eNA = gtk_entry_new();
    GtkWidget * lni = gtk_label_new("Refractive index of immersion");
    GtkWidget * eni = gtk_entry_new();
    GtkWidget * ldx = gtk_label_new("Pixel size dx (=dy) [nm]");
    GtkWidget * edx = gtk_entry_new();
    GtkWidget * ldz = gtk_label_new("Distance between planes, dz [nm]");
    GtkWidget * edz = gtk_entry_new();
    state->eNA = eNA;
    state->eni = eni;
    state->eName = eName;
    state->edx = edx;
    state->edz = edz;

    GtkWidget * grid = gtk_grid_new();
    gtk_grid_set_row_spacing ((GtkGrid*) grid , 5);
    gtk_grid_set_column_spacing ((GtkGrid*) grid , 5);

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

    gtk_widget_set_margin_bottom((GtkWidget*) im, 20);
    gtk_widget_set_margin_top((GtkWidget*) im, 20);
    gtk_widget_set_size_request(im, 256, 256);
    gtk_box_append((GtkBox*) hbox, im);
    gtk_box_append((GtkBox*) hbox, grid);

    gtk_widget_set_halign((GtkWidget*) grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget*) grid, GTK_ALIGN_CENTER);
    gtk_widget_set_halign((GtkWidget*) hbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget*) hbox, GTK_ALIGN_CENTER);

    GtkBox * vbox0 = (GtkBox *) gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_box_append(vbox0, hbox);

    GtkButton * btn_ok = (GtkButton *) gtk_button_new_with_label("Ok");
    g_signal_connect(btn_ok, "clicked", G_CALLBACK(cb_scope_ok), window);
    GtkButton * btn_cancel = (GtkButton *) gtk_button_new_with_label("Cancel");
    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(cb_scope_close), window);
    GtkBox * box_btn = (GtkBox *) gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_box_append(box_btn, GTK_WIDGET(btn_cancel));
    gtk_box_append(box_btn, GTK_WIDGET(btn_ok));
    gtk_box_append(vbox0, GTK_WIDGET(box_btn));
    gtk_window_set_child (GTK_WINDOW(window),  GTK_WIDGET(vbox0));

    g_signal_connect(G_OBJECT(window),
                     "close-request", G_CALLBACK(cb_scope_close), NULL);
}
