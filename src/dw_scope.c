#include "dw_scope.h"

void dw_scope_free(DwScope * scope)
{
    if(scope == NULL)
        return;

    if(scope->name != NULL)
        free(scope->name);
    free(scope);
}

DwScope * dw_scope_get_from_model(GtkTreeModel * model, GtkTreeIter * iter)
{
    DwScope * scope = malloc(sizeof(DwScope));
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

    scope->name = strdup(name);
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

    DwScope ** slist = malloc( (nscopes+1) * sizeof(DwScope*));
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
 int pos = 0;
 while(scopes[pos] != NULL)
 {
     dw_scope_free(scopes[pos]);
     pos++;
 }
 free(scopes);
}



DwScope * dw_scope_new()
{
    DwScope * scope = malloc(sizeof(DwScope));
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

 DwScope ** scopes = malloc((length+1)*sizeof(DwScope*));
 scopes[length] = NULL; // End of array

 for(int kk = 0; kk<length; kk++)
 {
     scopes[kk] = dw_scope_new();
     DwScope * scope = scopes[kk];
     gchar * group = groups[kk]; // I.e. Alias
     scope->name = strdup(group);

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


DwScope *
dw_scope_edit_dlg(GtkWindow *parent, DwScope * old_scope)
{
 GtkWidget *dialog, *content_area;
 GtkDialogFlags flags;


 char * msg = malloc(1024);
 if(old_scope == NULL)
 {
     sprintf(msg, "Add a new microscope");
 } else {
     sprintf(msg, "Edit an existing microscope");
 }


 // Create the widgets
 flags = GTK_DIALOG_DESTROY_WITH_PARENT;
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

gtk_box_pack_end((GtkBox*) hbox, im, FALSE, TRUE, 5);
gtk_box_pack_start((GtkBox*) hbox, grid, FALSE, TRUE, 5);

 gtk_container_add (GTK_CONTAINER (content_area),  hbox);


 gtk_widget_show_all(content_area);

 int result = gtk_dialog_run (GTK_DIALOG (dialog));
 DwScope * scope = NULL;
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
