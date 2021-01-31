#include <gtk/gtk.h>

#include "dw_app.h"
#include "dw_app_window.h"
#include "dw_app_get_new_scope.c"
#include "dw_app_runner.c"
#include "dw_app_runner_simple.c"
#include <libgen.h>
#include <locale.h>
#include <assert.h>
#include <glib.h>
#include <glib/gstdio.h>

#define dw_config_dir "~/.config/deconwolf/"

typedef struct {
    DwAppWindow * window; // main window
    GtkWidget * file_tree;
    GtkWidget * channel_tree;
    GtkWidget * scope_tree;
    GtkWidget * cmd;
    GtkWidget * status;
} gconf;

// Columns for the scopes
enum
    {
     sNAME_COLUMN,
     sNA_COLUMN,
     sNI_COLUMN,
     sDX_COLUMN,
     sDZ_COLUMN,
     sSN_COLUMNS
    };

// Columns for the channels
enum
    {
     cALIAS_COLUMN,
     cNAME_COLUMN,
     cEMISSION_COLUMN,
     cNITER_COLUMN,
     cN_COLUMNS
    };

// Columns for files
enum
    {
     fFILE_COLUMN,
     fCHANNEL_COLUMN,
     fN_COLUMNS
    };

gconf config;

// File
typedef struct {
    char * name;
    char * channel;
} dwfile;


void dw_channel_free(DwChannel * chan)
{
    if(chan->name != NULL)
        free(chan->name);
    if(chan->alias != NULL)
        free(chan->alias);
}

DwChannel * dw_channel_new()
{
    DwChannel * chan = malloc(sizeof(DwChannel));
    chan->name = NULL;
    chan->alias = NULL;
    return chan;
}


// GtkTreeView
GtkWidget * create_file_tree()
    {

    /* Create tree-view for files */


    GtkTreeStore * file_store = gtk_tree_store_new (fN_COLUMNS,       /* Total number of columns */
                                                    G_TYPE_STRING,   /* File name */
                                                    G_TYPE_STRING);   /* Channel */


    GtkWidget * file_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (file_store));
    config.file_tree = file_tree;

GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer),
                 "foreground", "black",
                 NULL);


   /* Create a column, associating the "text" attribute of the
    * cell_renderer to the first column of the model */
   GtkTreeViewColumn * column = gtk_tree_view_column_new_with_attributes ("File", renderer,
                                                      "text", fFILE_COLUMN,
                                                      NULL);

   /* Add the column to the view. */
   gtk_tree_view_append_column (GTK_TREE_VIEW (file_tree), column);

   renderer = gtk_cell_renderer_text_new ();
   column = gtk_tree_view_column_new_with_attributes ("Channel",
                                                      renderer,
                                                      "text", fCHANNEL_COLUMN,
                                                      NULL);

   gtk_tree_view_append_column (GTK_TREE_VIEW (file_tree), column);

   return file_tree;
    }


int is_tif_file_name(char * fname)
{
    GRegex *regex;
    GMatchInfo *match_info;
    int match = 0;

    regex = g_regex_new ("\\.TIFF?$", G_REGEX_CASELESS, 0, NULL);
    g_regex_match (regex, fname, 0, &match_info);
    if (g_match_info_matches (match_info))
    {
        match = 1;
    }
    g_match_info_free (match_info);
    g_regex_unref (regex);

    return match;
}

char * get_psfname(char * dir, char * channel)
{
    int len = strlen(dir) + strlen(channel);
    char * name = malloc(len+20);
    sprintf(name, "%s/PSFBW/%s.tif", dir, channel);
    return name;
}

char * get_channel_name(char *fname0)
{
    char * fname = strdup(fname0);
    char * ret = NULL;

    for(int kk = 0 ; kk<strlen(fname); kk++)
        fname[kk] = toupper(fname[kk]);

    // Return channel name, i.e. /dir/dapi_001.tif -> DAPI

    GRegex *regex;
    GMatchInfo *match_info;

    regex = g_regex_new ("([A-Z0-9]*)\\_[0-9]*\\.TIFF?", 0, 0, NULL);
    g_regex_match (regex, fname, 0, &match_info);
    while (g_match_info_matches (match_info))
    {
        gchar *word = g_match_info_fetch (match_info, 1); // 1=match group only
        //g_print ("Found: %s\n", word);
        if(ret == NULL)
        {
            ret = strdup(word);
        } else {
            printf("Warning: duplicate channel match in %s\n", fname0);
        }
        g_free (word);
        g_match_info_next (match_info, NULL);
    }
    g_match_info_free (match_info);
    g_regex_unref (regex);
    free(fname);

    if(ret == NULL)
    {
        char * ret = malloc(5);
        sprintf(ret, "?");
    }
    return ret;
}

gboolean add_channel(char * alias, char * name, float emission, int iter)
{
    GtkTreeStore * channel_store = (GtkTreeStore*) gtk_tree_view_get_model((GtkTreeView*) config.channel_tree);
    GtkTreeIter iter1;  /* Parent iter */

    gtk_tree_store_append (channel_store, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (channel_store, &iter1,
                        cALIAS_COLUMN, alias,
                        cNAME_COLUMN, name,
                        cNITER_COLUMN, iter,
                        cEMISSION_COLUMN, emission,
                        -1);
    return TRUE;
}

gboolean
new_channel_cb(GtkWidget *widget,
               gpointer user_data)
{
    add_channel("WLF", "Wolfram-X", 70.0, 100);
    return TRUE;
}


    GtkWidget * create_channel_tree()
    {
    /* Create tree-view for files */
    GtkTreeStore * channel_store = gtk_tree_store_new (cN_COLUMNS,       /* Total number of columns */
                                                       G_TYPE_STRING,
                                                       G_TYPE_STRING,
                                                       G_TYPE_FLOAT,
                                                    G_TYPE_INT);

    GtkWidget * channel_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (channel_store));
    config.channel_tree = channel_tree;

    GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer),
                 "foreground", "black",
                 NULL);

    g_object_set(G_OBJECT (renderer), "editable", FALSE, NULL);

   /* Create a column, associating the "text" attribute of the
    * cell_renderer to the first column of the model */
   GtkTreeViewColumn * column = gtk_tree_view_column_new_with_attributes ("Alias", renderer,
                                                      "text", cALIAS_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);


   column = gtk_tree_view_column_new_with_attributes ("Emission [nm]",
                                                      renderer,
                                                      "text", cEMISSION_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);

   column = gtk_tree_view_column_new_with_attributes ("Iterations",
                                                      renderer,
                                                      "text", cNITER_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);


   column = gtk_tree_view_column_new_with_attributes ("Name",
                                                      renderer,
                                                      "text", cNAME_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);


   GtkWidget * btnNew = gtk_button_new_from_icon_name("list-add",
                                                      GTK_ICON_SIZE_SMALL_TOOLBAR);
   gtk_widget_set_tooltip_text(btnNew, "Add another channel");

   GtkWidget * btnDel = gtk_button_new_from_icon_name("list-remove",
                                                      GTK_ICON_SIZE_SMALL_TOOLBAR);
   gtk_widget_set_tooltip_text(btnDel, "Remove selected channel");

   GtkWidget * btnEdit = gtk_button_new_from_icon_name("preferences-other",
                                                      GTK_ICON_SIZE_SMALL_TOOLBAR);
   gtk_widget_set_tooltip_text(btnEdit, "Edit selected channel");

   GtkWidget * btnSave = gtk_button_new_from_icon_name("document-save",
                                                       GTK_ICON_SIZE_SMALL_TOOLBAR);
   gtk_widget_set_tooltip_text(btnSave, "Save current list as default");

   g_signal_connect(btnNew, "clicked", G_CALLBACK (new_channel_cb), NULL);
   g_signal_connect(btnDel, "clicked", G_CALLBACK (del_channel_cb), NULL);
   g_signal_connect(btnSave, "clicked", G_CALLBACK (save_channel_cb), NULL);

   GtkWidget * Bar = gtk_action_bar_new();
   gtk_action_bar_pack_start((GtkActionBar*) Bar, btnNew);
   gtk_action_bar_pack_start((GtkActionBar*) Bar, btnDel);
   gtk_action_bar_pack_start((GtkActionBar*) Bar, btnEdit);
   gtk_action_bar_pack_start((GtkActionBar*) Bar, btnSave);


   GtkWidget * boxV = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
   gtk_box_pack_end((GtkBox*) boxV,
                    Bar,
                    FALSE, TRUE, 5);
   gtk_box_pack_start((GtkBox*) boxV,
                    channel_tree,
                    FALSE, TRUE, 5);

   return boxV;
   }




gboolean add_scope(char * name, float na, float ni, float dx, float dz)
/* Add another microscope to the list of microscopes */
{
    GtkTreeStore * scope_store = (GtkTreeStore*) gtk_tree_view_get_model((GtkTreeView*) config.scope_tree);
    GtkTreeIter iter1;  /* Parent iter */
    gtk_tree_store_append (scope_store, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (scope_store, &iter1,
                        sNAME_COLUMN, name,
                        sNA_COLUMN, na,
                        sNI_COLUMN, ni,
                        sDX_COLUMN, dx,
                        sDZ_COLUMN, dz,
                        -1);
    return TRUE;
}



gboolean
new_scope_cb (GtkWidget *widget,
               gpointer   user_data)
{
    //    add_scope("Magic-scope1", 3.14, 1.4, 45, 45);

    DwScope * scope = dw_app_get_new_scope((GtkWindow*) config.window, NULL);
    if(scope != NULL)
    {
        add_scope(scope->name, scope->NA, scope->ni, scope->xy_nm, scope->z_nm);
        free(scope);
    }

    return TRUE;
}

    GtkWidget * create_microscope_tab()
    {
    /* Create tree-view for microscopes */
    GtkTreeStore * scope_store = gtk_tree_store_new (sSN_COLUMNS,       /* Total number of columns */
                                                       G_TYPE_STRING,
                                                       G_TYPE_FLOAT,// na
                                                       G_TYPE_FLOAT, // ni
                                                       G_TYPE_FLOAT, // dx
                                                     G_TYPE_FLOAT); // dz

    GtkWidget * scope_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (scope_store));
    config.scope_tree = scope_tree;

    GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer),
                 "foreground", "black",
                 NULL);
    g_object_set(G_OBJECT (renderer), "editable", FALSE, NULL);

   /* Create a column, associating the "text" attribute of the
    * cell_renderer to the first column of the model */
   GtkTreeViewColumn * column = gtk_tree_view_column_new_with_attributes ("Name", renderer,
                                                      "text", sNAME_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);


   column = gtk_tree_view_column_new_with_attributes ("NA",
                                                      renderer,
                                                      "text", sNA_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);

   column = gtk_tree_view_column_new_with_attributes ("ni",
                                                      renderer,
                                                      "text", sNI_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);


   column = gtk_tree_view_column_new_with_attributes ("dx [nm]",
                                                      renderer,
                                                      "text", sDX_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);


   column = gtk_tree_view_column_new_with_attributes ("dz [nm]",
                                                      renderer,
                                                      "text", sDZ_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);

   GtkWidget * btnNew = gtk_button_new_from_icon_name("list-add",
                                                         GTK_ICON_SIZE_SMALL_TOOLBAR);
   gtk_widget_set_tooltip_text(btnNew, "Add another microscope");
   GtkWidget * btnDel = gtk_button_new_from_icon_name("list-remove",
                                                      GTK_ICON_SIZE_SMALL_TOOLBAR);
   gtk_widget_set_tooltip_text(btnDel, "Remove selected microscope");
   GtkWidget * btnEdit = gtk_button_new_from_icon_name("preferences-other",
                                                       GTK_ICON_SIZE_SMALL_TOOLBAR);
   gtk_widget_set_tooltip_text(btnEdit, "Edit selected microscope");

   GtkWidget * Bar = gtk_action_bar_new();
   gtk_action_bar_pack_start ((GtkActionBar*) Bar, btnNew);
   gtk_action_bar_pack_start ((GtkActionBar*) Bar, btnDel);
   gtk_action_bar_pack_start ((GtkActionBar*) Bar, btnEdit);

   g_signal_connect (btnNew, "clicked", G_CALLBACK (new_scope_cb), NULL);
   g_signal_connect (btnDel, "clicked", G_CALLBACK (del_scope_cb), NULL);
   g_signal_connect (btnEdit, "clicked", G_CALLBACK (edit_scope_cb), NULL);
   // TODO: Add the missing callbacks

   GtkWidget * A = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

   gtk_box_pack_end ((GtkBox*) A,
                       Bar,
                       FALSE,
                       TRUE,
                       5);

   gtk_box_pack_start ((GtkBox*) A,
                     scope_tree,
                     TRUE,
                     TRUE,
                     5);

   return A;
   }


void file_tree_append(const char * file)
{
    // Should complain if the file format isn't something like:
    // file:///home/erikw/Desktop/iEG701_25oilx_200928_009/max_x_024.tif\r\n
    if(strlen(file) < 6)
    {
        printf("Weird DND data\n");
        return;
    }
    if(strncmp(file, "file://", 7) != 0)
    {
        printf("Weird DND data\n");
        return;
    }

    char * fname0 = strdup(file);
    // Remove trailing CRCL
    char * fname = fname0 + 7;
    for(size_t kk = 0; kk < strlen(fname); kk++)
    {
        if(fname[kk] == '\n' || fname[kk] == '\r')
        {
            fname[kk] = '\0';
        }
    }

    GtkTreeStore * filetm = (GtkTreeStore*) gtk_tree_view_get_model((GtkTreeView*) config.file_tree);
    GtkTreeIter iter1;  /* Parent iter */

    if(!is_tif_file_name(fname))
    {
        goto done;
    }


    char * cname = get_channel_name(fname);
    gtk_tree_store_append (filetm, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (filetm, &iter1,
                        fFILE_COLUMN, fname,
                        fCHANNEL_COLUMN, cname,
                        -1);
    free(cname);
 done:
    free(fname0);
    return;
}

static  void
drag_data_cb(GtkWidget *wgt, GdkDragContext *context, int x, int y,
    GtkSelectionData *seldata, guint info, guint time,
    gpointer userdata)
{
    //  printf("Starting DnD callback\n"); fflush(stdout);
  if (!seldata || !gtk_selection_data_get_data(seldata))
  {
    printf("DND data is weird\n");
    goto done;
  }

  guint dformat = gtk_selection_data_get_format (seldata);

  if(dformat != 8)
  {
    printf("Don't recognize the data format %d of the DND\n", dformat);
    goto done;
  }

  //  printf("%d data items\n", gtk_selection_data_get_length (seldata));
  const guchar * data = gtk_selection_data_get_data(seldata);
  //printf("---\n%s---\n", data);
  //fflush(stdout);

  /* Append to file tree, need to split the data first */
  if(strlen( (char *) data) > 0)
  {
      char * dnd = strdup( (char *) data);
      char delim = '\n';
      char * file = strtok(dnd, &delim);
      if(file != NULL)
      {
          file_tree_append(file);
      }
      while( file != NULL)
      {
          file = strtok(NULL, &delim);
          if(file != NULL)
          {
              file_tree_append(file);
          }
      }
      if(dnd != NULL)
      {
          free(dnd);
      }
  }



done:
  gtk_drag_finish (context,
                 TRUE, // success,
                 FALSE, // delete original
                 time);

  //  printf("DnD handling done\n");
}



struct _DwAppWindow
{
    GtkApplicationWindow parent;
};

G_DEFINE_TYPE(DwAppWindow, dw_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void
dw_app_window_init (DwAppWindow *app)
{


}

static void
dw_app_window_class_init (DwAppWindowClass *class)
{
}

GtkWidget * dw_frame()
{
     return gtk_frame_new(NULL);
}

gboolean run_dw_cb_not_ready(GtkWidget * widget, gpointer user_data)
{
    // Run deconwolf
    dw_app_runner((GtkWindow*) config.window, "pause 1");
    return TRUE;
}

void runscript(char * name)
{
    GAppInfo *appinfo = NULL;
    gboolean ret = FALSE;

    appinfo = g_app_info_create_from_commandline(name,
                                                 NULL,
                                                 G_APP_INFO_CREATE_NEEDS_TERMINAL,
                                                 NULL);
    g_assert(appinfo != NULL); // TODO error handling is not implemented.

    ret = g_app_info_launch(appinfo, NULL, NULL, NULL);
    g_assert(ret == TRUE); // TODO error handling is not implemented.
}


gboolean save_dw_cb(GtkWidget * widget, gpointer user_data)
{
    char * filename = NULL;
    if(save_cmd((GtkWindow*) config.window, &filename))
    {
        free(filename);
    }
    return TRUE;
}


gboolean run_dw_cb(GtkWidget * widget, gpointer user_data)
{
    // Run deconwolf
    // 1/ Save the script to a file
    // 2/ Use g_app_info_create_from_commandline
    //    to execute it

    char * filename = NULL;
    if(save_cmd((GtkWindow*) config.window, &filename)) // in dw_app_runner_simple.c
    { // If we got a filename
        // Get text from
        // config.cmd
        // and save to that file.

        GtkTextIter start, end;
        GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (config.cmd));
        gtk_text_buffer_get_start_iter (buffer, &start);
        gtk_text_buffer_get_end_iter (buffer, &end);
        gchar * text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
        gtk_text_buffer_set_modified (buffer, FALSE);


        /* set the contents of the file to the text from the buffer */
        gboolean result;
        GError * err = NULL;
        if (filename != NULL)
        {
            result = g_file_set_contents (filename, text, -1, &err);

        }

        if (result == FALSE)
        {
            /* error saving file, show message to user */
            //error_message (err->message);
            g_error_free (err);

        } else {
            int chmod_ok = g_chmod(filename, S_IXUSR | S_IWUSR | S_IRUSR );
            g_assert(chmod_ok == 0);
        }

        g_free (text);


        // Run it
        printf("To run: %s\n", filename);
        runscript(filename);
        free(filename);
    }
        return TRUE;
}


GtkWidget * run_frame()
{

    GtkWidget * cmd = gtk_text_view_new();
    gtk_text_view_set_monospace( (GtkTextView *) cmd , TRUE);

    config.cmd = cmd;

    g_object_set(G_OBJECT(cmd), "editable", FALSE, NULL);

    GtkWidget * cmd_scroll = gtk_scrolled_window_new (NULL, NULL);

    gtk_container_add (GTK_CONTAINER (cmd_scroll),
                       cmd);

    GtkWidget * fcmd_scroll = gtk_frame_new("Commands");
    gtk_container_add(GTK_CONTAINER(fcmd_scroll), cmd_scroll);


    GtkWidget * frame = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER (frame), fcmd_scroll);

    GtkWidget * ButtonRun = gtk_button_new_from_icon_name("media-playback-start",
                                                          GTK_ICON_SIZE_SMALL_TOOLBAR);

    gtk_widget_set_tooltip_text(ButtonRun, "Save the script and run it");

    GtkWidget * ButtonSaveAs = gtk_button_new_from_icon_name("document-save-as",
                                                          GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(ButtonSaveAs, "Save the script to disk");

    GtkWidget * Bar = gtk_action_bar_new();
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, ButtonRun);
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, ButtonSaveAs);

    g_signal_connect (ButtonRun, "clicked", G_CALLBACK (run_dw_cb), NULL);
    g_signal_connect (ButtonRun, "clicked", G_CALLBACK (save_dw_cb), NULL);

    GtkWidget * A = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    gtk_box_pack_end ((GtkBox*) A,
                      Bar,
                      FALSE,
                      TRUE,
                      5);

    gtk_box_pack_start ((GtkBox*) A,
                        frame,
                        TRUE,
                        TRUE,
                        5);

    gtk_widget_show (cmd);
    return A;

    }

DwChannel ** dw_channels_get_from_gui()
{
    // Get a list of all the channels.
    // 1, Count the number of channels
    // 2, Allocate the list
    // 3, Populate the list
    // Get Model

    GtkTreeModel * model =
        gtk_tree_view_get_model ( (GtkTreeView*) config.channel_tree);

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

dwfile ** dwfile_get(int * nfiles)
{
    // Get an array with all files

    GtkTreeModel * model =
        gtk_tree_view_get_model ( (GtkTreeView*) config.file_tree);

    GtkTreeIter iter;

    gboolean valid = gtk_tree_model_get_iter_first (model, &iter);
    if(valid == FALSE)
    {
        return NULL;
    }
    // Figure out how many rows there are
    gint nfiles_list = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), NULL);

    // printf("There are %d files\n", nfiles_list); fflush(stdout);
    if(nfiles_list < 1)
    {
        nfiles[0] = 0;
        return NULL;
    }
    nfiles[0] = nfiles_list;

    dwfile ** flist = malloc( nfiles_list * sizeof(dwfile*));

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
        flist[pos] = malloc(sizeof(dwfile));
        flist[pos]->name = strdup(file);
        flist[pos]->channel = strdup(channel);
        //        printf("%s %s\n", flist[pos]->name, flist[pos]->channel);

        g_free(file);
        g_free(channel);

        pos++;

        valid = gtk_tree_model_iter_next (model,
                                          &iter);

    }

    return flist;
}

DwScope * dwscope_get()
{
    // Get the scope from the list of scopes
    GtkTreeView * view = (GtkTreeView*) config.scope_tree;
    GtkTreeModel * model = gtk_tree_view_get_model( (GtkTreeView*) view);
    GtkTreeIter iter;
    GtkTreeSelection * selection = gtk_tree_view_get_selection( (GtkTreeView*) view);

    if(gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        DwScope * scope = malloc(sizeof(DwScope));
        scope->name = NULL;

        gchar *name;
        gfloat NA, ni, xy_nm, z_nm;

        // Make sure you terminate calls to gtk_tree_model_get() with a “-1” value
        gtk_tree_model_get (model, &iter,
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
    } else {
        return NULL;
    }
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

void update_cmd(int ready)
{

    // Make sure that the radix symbol is '.'
    // Does GTK change this from "C"?
    setlocale(LC_ALL,"C");


    // https://developer.gnome.org/glib/stable/glib-Hash-Tables.html
    // Todo: use a g_hash_table for the channels

    // Get Model
    GtkTreeModel * model =
        gtk_tree_view_get_model ( (GtkTreeView*) config.file_tree);
    // Get selection

    GtkTreeIter iter;
    gint row_count = 0;
    gboolean valid = gtk_tree_model_get_iter_first (model, &iter);

    // Get all files and add to list.
    while (valid)
    {
        gchar *file_data;
        gchar *chan_data;

        // Make sure you terminate calls to gtk_tree_model_get() with a “-1” value
        gtk_tree_model_get (model, &iter,
                            0, &file_data,
                            1, &chan_data,
                            -1);

        // Do something with the data
        //g_print ("Row %d: (%s,%s)\n",
        //         row_count, file_data, chan_data);
        g_free (file_data);
        g_free(chan_data);

        valid = gtk_tree_model_iter_next (model,
                                          &iter);
        row_count++;
    }


    // Get all channels and add to list

    // Get scope
    DwScope * scope = dwscope_get();

    DwChannel ** channels = dw_channels_get_from_gui();
    int nfiles = 0;
    dwfile ** files = dwfile_get(&nfiles);

    GtkTextView * cmd = (GtkTextView*) config.cmd;
    GtkTextIter titer;
    GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_get_iter_at_offset(buffer, &titer, 0);

    if(scope == NULL)
    {
        gtk_text_buffer_insert(buffer, &titer, "# Please select a microscope!", -1);
        gtk_text_view_set_buffer(cmd, buffer);
        return;
    }

    char * buff = malloc(1024*1024);
    sprintf(buff, "# Microscope: %s\n", scope->name);
    gtk_text_buffer_insert(buffer, &titer, buff, -1);
    //sprintf(buff, "# %d channels available\n", nchan);
    //gtk_text_buffer_insert(buffer, &titer, buff, -1);
    sprintf(buff, "# %d files available\n", nfiles);
    gtk_text_buffer_insert(buffer, &titer, buff, -1);

    int nthreads = 8;
    int tilesize = 2048;
    // Generate PSFs -- only for used channels
    for(int kk = 0 ; kk < nfiles; kk++)
    {
        DwChannel * ch = dw_channels_get_by_alias(channels, files[kk]->channel);
        if(ch != NULL)
        {
            char * fdir = strdup(files[kk]->name);
            fdir = dirname(fdir);
            char * psf = get_psfname(fdir, files[kk]->channel);
            sprintf(buff, "mkdir %s/PSFBW/\n", fdir);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);
            sprintf(buff, "dw_bw --lambda %f --NA %f --ni %f --threads %d --resxy %f --resz %f %s\n",
                    ch->lambda, scope->NA, scope->ni, nthreads, scope->xy_nm, scope->z_nm,
                    psf);
            //        printf("%s", buff);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);
            sprintf(buff, "dw --tilesize %d --iter %d --threads %d %s %s\n",
                    tilesize, ch->niter, nthreads,
                    files[kk]->name, psf);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);

            free(fdir);
            free(psf);
        } else {
            sprintf(buff, "# Missing channel for: %s\n", files[kk]->name);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);
        }
    }

    gtk_text_view_set_buffer (cmd,
                              buffer);
    free(scope);
    free(buff);
    return;
}

void update_status()
{

    update_cmd(1);
}

gboolean
tab_change_cb(GtkNotebook *notebook,
              GtkWidget   *page,
              guint        page_num,
              gpointer     user_data)
{
    if(page_num == 5)
    {
        update_status();
    }
    return TRUE;
}




void del_selected_file()
{
    GtkTreeModel * model =
        gtk_tree_view_get_model ( (GtkTreeView*) config.file_tree);
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( (GtkTreeView*) config.file_tree);
    GtkTreeIter iter;

    // Remove the first selected item
    // Note: has to be modified to handle multiple selected
    if(gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
    }
}

void del_selected_scope()
{
    GtkTreeModel * model =
        gtk_tree_view_get_model ( (GtkTreeView*) config.scope_tree);
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( (GtkTreeView*) config.scope_tree);
    GtkTreeIter iter;

    // Remove the first selected item
    // Note: has to be modified to handle multiple selected
    if(gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
    }
}

void edit_selected_scope()
{
    printf("edit_selected_scope()\n");
    GtkTreeModel * model =
        gtk_tree_view_get_model ( (GtkTreeView*) config.scope_tree);
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( (GtkTreeView*) config.scope_tree);
    GtkTreeIter iter;

    // Remove the first selected item
    // Note: has to be modified to handle multiple selected
    if(gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        // Get what we need from the mode
        gchar * sname;
        gfloat sNA;
        gfloat sNI;
        gfloat sDX;
        gfloat sDZ;

        gtk_tree_model_get(model, &iter,
                           sNAME_COLUMN, &sname,
                           sNA_COLUMN, &sNA,
                           sNI_COLUMN, &sNI,
                           sDX_COLUMN, &sDX,
                           sDZ_COLUMN, &sDZ,
                           -1);
        DwScope * current_scope = malloc(sizeof(DwScope));
        current_scope->name = strdup(sname);
        current_scope->NA = sNA;
        current_scope->ni = sNI;
        current_scope->xy_nm = sDX;
        current_scope->z_nm = sDZ;
        DwScope * scope = dw_app_get_new_scope((GtkWindow*) config.window, current_scope);
        if(scope != NULL)
        {
            gtk_tree_store_set((GtkTreeStore*) model, &iter,
                               sNAME_COLUMN, scope->name,
                               sNA_COLUMN, scope->NA,
                               sNI_COLUMN, scope->ni,
                               sDX_COLUMN, scope->xy_nm,
                               sDZ_COLUMN, scope->z_nm,
                               -1);

            free(scope);
        }
        free(current_scope);
    }

}



void del_selected_channel()
{
    GtkTreeModel * model =
        gtk_tree_view_get_model ( (GtkTreeView*) config.channel_tree);
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( (GtkTreeView*) config.channel_tree);
    GtkTreeIter iter;

    // Remove the first selected item
    // Note: has to be modified to handle multiple selected
    if(gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
    }
}

gboolean edit_scope_cb(GtkWidget * w, gpointer p)
{
    edit_selected_scope();
    return TRUE;
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

void dw_channels_free(DwChannel ** channels)
{
    int pos = 0;
    while(channels[pos] != NULL)
    {
        dw_channel_free(channels[pos]);
        pos++;
    }
    free(channels);
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

    for(int kk = 0; kk<length; kk++)
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

gboolean save_channel_cb(GtkWidget * w, gpointer p)
{
    // Save list of channels to ini file.

    // First, figure out where:
    char * cfile = malloc(1024);
    sprintf(cfile, "%s/deconwolf/", g_get_user_config_dir());
    if(g_mkdir_with_parents(cfile, S_IXUSR | S_IWUSR | S_IRUSR) == -1)
        {
            printf("Unable to access %s\n", cfile);
            free(cfile);
            return FALSE;
        }

    // Grab information from gui and save
    sprintf(cfile, "%s/deconwolf/dw_gui_channels", g_get_user_config_dir());
    DwChannel ** channels = dw_channels_get_from_gui();
    dw_channels_to_disk(channels, cfile);
    dw_channels_free(channels);
    free(cfile);
    return TRUE;
}

gboolean del_channel_cb(GtkWidget * w, gpointer p)
{
    del_selected_channel();
    return TRUE;
}
gboolean del_scope_cb(GtkWidget * w, gpointer p)
{
    del_selected_scope();
    return TRUE;
}

gboolean file_tree_keypress (GtkWidget *tree_view, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Delete){
        del_selected_file();
    }
    return FALSE;
}
gboolean channel_tree_keypress (GtkWidget *tree_view, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Delete){
        del_selected_channel();
    }
    return FALSE;
}
gboolean microscope_tree_keypress (GtkWidget *tree_view, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Delete){
        del_selected_scope();
    }
    return FALSE;
}


DwAppWindow *
dw_app_window_new (DwApp *app)
{

    GtkWidget * frame_drop = gtk_frame_new (NULL);
    GtkWidget * frame_files = gtk_frame_new (NULL);
    GtkWidget * frame_channels = gtk_frame_new (NULL);
    GtkWidget * frame_scope = gtk_frame_new (NULL);
    GtkWidget * frame_run = run_frame();
    GtkWidget * frame_dw = dw_frame();

    gtk_frame_set_shadow_type (GTK_FRAME (frame_drop), GTK_SHADOW_IN);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_files), GTK_SHADOW_IN);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_channels), GTK_SHADOW_IN);

    GtkWidget * notebook = gtk_notebook_new();
    gtk_notebook_append_page ((GtkNotebook*) notebook,
                              frame_drop, gtk_label_new("Drop area"));

    gtk_notebook_append_page ((GtkNotebook*) notebook,
                              frame_files, gtk_label_new("Files"));

    gtk_notebook_append_page ((GtkNotebook*) notebook,
                              frame_channels, gtk_label_new("Channels"));

    gtk_notebook_append_page ((GtkNotebook*) notebook,
                              frame_scope, gtk_label_new("Microscope"));

    gtk_notebook_append_page ((GtkNotebook*) notebook,
                              frame_dw, gtk_label_new("Deconwolf"));

    gtk_notebook_append_page ((GtkNotebook*) notebook,
                              frame_run, gtk_label_new("Run"));

    g_signal_connect(notebook, "switch-page",
                     G_CALLBACK(tab_change_cb), NULL);

    GtkWidget *image = gtk_image_new_from_resource("/images/wolf1.png");

    GtkWidget * file_tree = create_file_tree();
    g_signal_connect (G_OBJECT (file_tree), "key_press_event",
                      G_CALLBACK (file_tree_keypress), NULL);

    GtkWidget * file_tree_scroller = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (file_tree_scroller),
                       file_tree);

    GtkWidget * channel_tree = create_channel_tree();
    g_signal_connect (G_OBJECT (config.channel_tree), "key_press_event",
                      G_CALLBACK (channel_tree_keypress), NULL);
    //GtkWidget * channel_tree = gtk_label_new("testing");
    GtkWidget * scope_tab = create_microscope_tab();
    g_signal_connect (G_OBJECT (config.scope_tree), "key_press_event",
                      G_CALLBACK (microscope_tree_keypress), NULL);
    //GtkWidget * scope_tab = gtk_label_new("Bug free, but boring, scope_tab replacement");

    /* Set up Drag and Drop */
    enum
    {
     TARGET_STRING,
     TARGET_URL
    };

    static GtkTargetEntry targetentries[] =
        {
         { "STRING",        0, TARGET_STRING },
         { "text/plain",    0, TARGET_STRING },
         { "text/uri-list", 0, TARGET_URL },
        };

    gtk_drag_dest_set(image, GTK_DEST_DEFAULT_ALL, targetentries, 3,
                      GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
    g_signal_connect(image, "drag_data_received",
                     G_CALLBACK(drag_data_cb), NULL);

    /* Create the window */
    DwAppWindow * window = g_object_new (DW_APP_WINDOW_TYPE, "application", app, NULL);
    config.window = window;
    gtk_window_set_title (GTK_WINDOW (window), "BiCroLab deconwolf GUI, 2021");



    /* Pack components */
    gtk_container_add (GTK_CONTAINER (frame_drop), image);
    gtk_container_add (GTK_CONTAINER (frame_files), file_tree_scroller);
    gtk_container_add (GTK_CONTAINER (frame_channels), channel_tree);
    gtk_container_add (GTK_CONTAINER (frame_scope), scope_tab);
    gtk_container_add (GTK_CONTAINER (window), notebook);

    char * cfile = malloc(1024*sizeof(char));
    sprintf(cfile, "%s/deconwolf/dw_gui_channels", g_get_user_config_dir());
    DwChannel ** channels = dw_channels_from_disk(cfile);
    free(cfile);
    int pos = 0;
    if(channels != NULL)
    {
    while(channels[pos] != NULL)
    {
        DwChannel * chan = channels[pos++];
        add_channel(chan->alias, chan->name, chan->lambda, chan->niter);
    }
    dw_channels_free(channels);
    } else {
    /* Until we read and write configuration files, add some defaults */
    add_channel("DAPI", "4′,6-diamidino-2-phenylindole", 466.0, 50);
    add_channel("A594", "Alexa Fluor 594", 617.0, 100);
    add_channel("CY5", "Alexa Fluor 647", 664.0, 100);
    add_channel("TMR", "Tetramethylrhodamine", 562.0, 100);
    }

    add_scope("Bicroscope-1, 100X", 1.45, 1.515, 130, 250);
    add_scope("Bicroscope-1, 60X", 1.40, 1.515, 216, 350);
    add_scope("Bicroscope-2, 100X", 1.40, 1.515, 65, 250);

    return window;

}

void
dw_app_window_open (DwAppWindow *win,
                         GFile            *file)
{
}
