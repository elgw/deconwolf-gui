#include <gtk/gtk.h>

#include "dw_app.h"
#include "dw_app_window.h"
#include "dw_app_get_new_scope.c"
#include <libgen.h>
#include <locale.h>
#include <assert.h>

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

// Channel
typedef struct {
    char * name;
    char * alias;
    float lambda;
    int niter;
} dwchannel;

// Microscope
typedef struct {
    char * name;
    float NA;
    float ni;
    float xy_nm;
    float z_nm;
} dwscope;

// deconwolf
typedef struct {
    int nthreads;
    int tilesize;
} dwconf;


// GtkTreeView
GtkWidget * create_file_tree()
    {

    /* Create tree-view for files */


    GtkTreeStore * file_store = gtk_tree_store_new (fN_COLUMNS,       /* Total number of columns */
                                                    G_TYPE_STRING,   /* File name */
                                                    G_TYPE_STRING);   /* Channel */


    GtkWidget * file_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (file_store));

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

    GtkWidget * create_channel_tree()
    {

    /* Create tree-view for files */


    GtkTreeStore * channel_store = gtk_tree_store_new (cN_COLUMNS,       /* Total number of columns */
                                                       G_TYPE_STRING,
                                                       G_TYPE_STRING,
                                                       G_TYPE_FLOAT,
                                                    G_TYPE_INT);

    GtkTreeIter iter1;  /* Parent iter */

    gtk_tree_store_append (channel_store, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (channel_store, &iter1,
                        cALIAS_COLUMN, "DAPI",
                        cNAME_COLUMN, "4′,6-diamidino-2-phenylindole",
                        cNITER_COLUMN, 50,
                        cEMISSION_COLUMN, 466.0,
                        -1);
    gtk_tree_store_append (channel_store, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (channel_store, &iter1,
                        cALIAS_COLUMN, "A594",
                        cNAME_COLUMN, "Alexa Fluor 594",
                        cEMISSION_COLUMN, 617.0,
                        cNITER_COLUMN, 100,
                        -1);

    GtkWidget * channel_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (channel_store));
    GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer),
                 "foreground", "black",
                 NULL);

    g_object_set(G_OBJECT (renderer), "editable", TRUE, NULL);

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


   return channel_tree;
   }

gboolean
new_scope_cb (GtkWidget *widget,
               gpointer   user_data)
{
    dw_app_get_new_scope((GtkWindow*) config.window);
    return TRUE;
}

    GtkWidget * create_microscope_tab()
    {

    /* Create tree-view for files */


    GtkTreeStore * scope_store = gtk_tree_store_new (sSN_COLUMNS,       /* Total number of columns */
                                                       G_TYPE_STRING,
                                                       G_TYPE_FLOAT,// na
                                                       G_TYPE_FLOAT, // ni
                                                       G_TYPE_FLOAT, // dx
                                                     G_TYPE_FLOAT); // dz

    GtkTreeIter iter1;  /* Parent iter */

    gtk_tree_store_append (scope_store, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (scope_store, &iter1,
                        sNAME_COLUMN, "Bicroscope 1, 100X",
                        sNA_COLUMN, 1.45,
                        sNI_COLUMN, 1.515,
                        sDX_COLUMN, 130.0,
                        sDZ_COLUMN, (float) 300.0,
                        -1);
    gtk_tree_store_append (scope_store, &iter1, NULL);
    gtk_tree_store_set (scope_store, &iter1,
                        sNAME_COLUMN, "Bicroscope 1, 60X",
                        sNA_COLUMN, 1.40,
                        sNI_COLUMN, 1.515,
                        sDX_COLUMN, 216.0,
                        sDZ_COLUMN, (float) 300.0,
                        -1);
    gtk_tree_store_append (scope_store, &iter1, NULL);
    gtk_tree_store_set (scope_store, &iter1,
                        sNAME_COLUMN, "Bicroscope 2, 100X",
                        sNA_COLUMN, 1.40,
                        sNI_COLUMN, 1.512,
                        sDX_COLUMN, 65.0,
                        sDZ_COLUMN, (float) 250.0,
                        -1);

    GtkWidget * scope_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (scope_store));
    config.scope_tree = scope_tree;
    GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer),
                 "foreground", "black",
                 NULL);
    g_object_set(G_OBJECT (renderer), "editable", TRUE, NULL);

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

   // TODO: put in action bar
   GtkWidget * ButtonNew = gtk_button_new_from_icon_name("list-add",
                                                         GTK_ICON_SIZE_SMALL_TOOLBAR);
   GtkWidget * Bar = gtk_action_bar_new();
   gtk_action_bar_pack_start ((GtkActionBar*) Bar, ButtonNew);

   //gtk_button_set_label(ButtonNew, "Add");

   g_signal_connect (ButtonNew, "clicked", G_CALLBACK (new_scope_cb), NULL);


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

    GtkWidget * ButtonRun = gtk_button_new_from_icon_name("system-run",
                                                          GTK_ICON_SIZE_SMALL_TOOLBAR);
    GtkWidget * ButtonSaveAs = gtk_button_new_from_icon_name("document-save-as",
                                                          GTK_ICON_SIZE_SMALL_TOOLBAR);
    GtkWidget * Bar = gtk_action_bar_new();
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, ButtonRun);
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, ButtonSaveAs);

    // g_signal_connect (ButtonNew, "clicked", G_CALLBACK (new_scope_cb), NULL);

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

dwchannel ** dwchannel_get(int * nchannels)
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
        nchannels[0] = 0;
        return NULL;
    }
    nchannels[0] = nchan;

    dwchannel ** clist = malloc( nchan * sizeof(dwchannel*));

    // Get all files and add to list.
    gint pos = 0;
    while (valid)
    {
        assert(pos < nchan);
        gchar *alias;
        gchar *name;
        gint niter;
        gfloat lambda;

        // Make sure you terminate calls to gtk_tree_model_get() with a “-1” value
        gtk_tree_model_get (model, &iter,
                            cALIAS_COLUMN, &alias,
                            cNAME_COLUMN, &name,
                            cNITER_COLUMN, &niter,
                            cEMISSION_COLUMN, &lambda,
                            -1);
        clist[pos] = malloc(sizeof(dwchannel));
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

dwscope * dwscope_get()
{
    // Get the scope from the list of scopes
    GtkTreeView * view = (GtkTreeView*) config.scope_tree;
    GtkTreeModel * model = gtk_tree_view_get_model( (GtkTreeView*) view);
    GtkTreeIter iter;
    GtkTreeSelection * selection = gtk_tree_view_get_selection( (GtkTreeView*) view);

    if(gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        dwscope * scope = malloc(sizeof(dwscope));
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

dwchannel * getchan(dwchannel ** channels, char * alias, int nchan)
    {
        for(int kk = 0; kk<nchan; kk++)
     {
      if(strcmp(alias, channels[kk]->alias) == 0)
      {
       return channels[kk];
      }
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
    dwscope * scope = dwscope_get();
    int nchan = 0;
    dwchannel ** channels = dwchannel_get(&nchan);
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
    sprintf(buff, "# %d channels available\n", nchan);
    gtk_text_buffer_insert(buffer, &titer, buff, -1);
    sprintf(buff, "# %d files available\n", nfiles);
    gtk_text_buffer_insert(buffer, &titer, buff, -1);

    int nthreads = 8;
    int tilesize = 2048;
    // Generate PSFs -- only for used channels
    for(int kk = 0 ; kk < nfiles; kk++)
    {
        dwchannel * ch = getchan(channels, files[kk]->channel, nchan);
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

gboolean file_tree_keypress (GtkWidget *tree_view, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Delete){

        // Get Model
        GtkTreeModel * model =
            gtk_tree_view_get_model ( (GtkTreeView*) tree_view);
        // Get selection
        GtkTreeSelection * selection = gtk_tree_view_get_selection ( (GtkTreeView*) tree_view);

        GtkTreeIter iter;

        // Remove the first selected item
        // Note: has to be modified to handle multiple selected
        if(gtk_tree_selection_get_selected (selection, &model, &iter))
        {
         gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
        }

        return TRUE;
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
    g_signal_connect (G_OBJECT (file_tree), "key_press_event", G_CALLBACK (file_tree_keypress), NULL);

    GtkWidget * channel_tree = create_channel_tree();
    //GtkWidget * channel_tree = gtk_label_new("testing");
    GtkWidget * scope_tab = create_microscope_tab();
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
    gtk_window_set_title (GTK_WINDOW (window), "BiCroLab deconwolf");

    GtkWidget * file_tree_scroller = gtk_scrolled_window_new (NULL, NULL);

    gtk_container_add (GTK_CONTAINER (file_tree_scroller),
                       file_tree);

    /* Pack components */
    gtk_container_add (GTK_CONTAINER (frame_drop), image);

    gtk_container_add (GTK_CONTAINER (frame_files), file_tree_scroller);
    gtk_container_add (GTK_CONTAINER (frame_channels), channel_tree);
    gtk_container_add (GTK_CONTAINER (frame_scope), scope_tab);

    gtk_container_add (GTK_CONTAINER (window), notebook);

    config.file_tree = file_tree;
    config.channel_tree = channel_tree;
    config.window = window;


    return window;

}

void
dw_app_window_open (DwAppWindow *win,
                         GFile            *file)
{
}
