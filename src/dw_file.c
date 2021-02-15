#include "dw_file.h"

DwFile * dw_file_new()
{
    DwFile * file = malloc(sizeof(DwFile));
    file->name = NULL;
    file->channel = NULL;
    return file;
}

void dw_file_free(DwFile * file)
{
    if(file == NULL)
        return;

    if(file->name != NULL)
    {
        free(file->name);
    }
    if(file->channel != NULL)
    {
        free(file->channel);
    }
    free(file);
}

void dw_files_free(DwFile ** files)
{
    if(files == NULL)
    {
        return;
    }

    int pos = 0;
    while(files[pos] != NULL)
    {
        dw_file_free(files[pos]);
        pos++;
    }
    free(files);
}

DwFile ** dw_files_get_from_gtk_tree_view(GtkTreeView * tv)
{
    // Get a NULL-terminated array with all files
    // Returns NULL if no files

    GtkTreeModel * model =
        gtk_tree_view_get_model ( tv );

    GtkTreeIter iter;

    gboolean valid = gtk_tree_model_get_iter_first (model, &iter);
    if(valid == FALSE)
    {
        return NULL;
    }
    // Figure out how many rows there are
    gint nfiles_list = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), NULL);

    DwFile ** flist = malloc( (nfiles_list+1) * sizeof(DwFile*));
    flist[nfiles_list] = NULL;

    // Get all files and add to list.
    gint pos = 0;
    while (valid)
    {
        assert(pos < nfiles_list);
        gchar *file;
        gchar *channel;

        // Make sure you terminate calls to gtk_tree_model_get() with a “-1” value
        gtk_tree_model_get (model, &iter,
                            fFILE_COLUMN, &file,
                            fCHANNEL_COLUMN, &channel,
                            -1);
        flist[pos] = dw_file_new();
        flist[pos]->name = strdup(file);
        if(channel == NULL)
        {
            flist[pos]->channel = strdup("_UNKNOWN");
        }
        else {
        flist[pos]->channel = strdup(channel);
        }
        //        printf("%s %s\n", flist[pos]->name, flist[pos]->channel);

        g_free(file);
        g_free(channel);

        pos++;

        valid = gtk_tree_model_iter_next (model,
                                          &iter);
    }
    return flist;
}
