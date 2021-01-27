#include <gtk/gtk.h>

#include "dw_app.h"
#include "dw_app_window.h"




typedef struct {
    GtkWidget * file_tree;
    GtkWidget * channel_tree;
    GtkWidget * scope_tree;
    GtkWidget * cmd;
    GtkWidget * status;

} gconf;

gconf config;

// File
typedef struct {
    char * name;
} dwfile;

// Channel
typedef struct {
    char * name;
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

    GtkWidget * create_file_tree()
    {

    /* Create tree-view for files */
        enum
        {
         FILE_COLUMN,
         CHANNEL_COLUMN,
         N_COLUMNS
        };

    GtkTreeStore * file_store = gtk_tree_store_new (N_COLUMNS,       /* Total number of columns */
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
                                                      "text", FILE_COLUMN,
                                                      NULL);

   /* Add the column to the view. */
   gtk_tree_view_append_column (GTK_TREE_VIEW (file_tree), column);

   renderer = gtk_cell_renderer_text_new ();
   column = gtk_tree_view_column_new_with_attributes ("Channel",
                                                      renderer,
                                                      "text", CHANNEL_COLUMN,
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
    enum
    {
     ALIAS_COLUMN,
     CHANNEL_COLUMN,
     EMISSION_COLUMN,
     NITER_COLUMN,
     N_COLUMNS
    };

    GtkTreeStore * channel_store = gtk_tree_store_new (N_COLUMNS,       /* Total number of columns */
                                                       G_TYPE_STRING,
                                                       G_TYPE_STRING,
                                                       G_TYPE_FLOAT,
                                                    G_TYPE_INT);

    GtkTreeIter iter1;  /* Parent iter */

    gtk_tree_store_append (channel_store, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (channel_store, &iter1,
                        ALIAS_COLUMN, "DAPI",
                        CHANNEL_COLUMN, "4′,6-diamidino-2-phenylindole",
                        NITER_COLUMN, 50,
                        EMISSION_COLUMN, 466.0,
                        -1);
    gtk_tree_store_append (channel_store, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (channel_store, &iter1,
                        ALIAS_COLUMN, "A594",
                        CHANNEL_COLUMN, "Alexa Fluor 594",
                        EMISSION_COLUMN, 617.0,
                        NITER_COLUMN, 100,
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
                                                      "text", ALIAS_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);


   column = gtk_tree_view_column_new_with_attributes ("Emission [nm]",
                                                      renderer,
                                                      "text", EMISSION_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);

   column = gtk_tree_view_column_new_with_attributes ("Iterations",
                                                      renderer,
                                                      "text", NITER_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);


   column = gtk_tree_view_column_new_with_attributes ("Name",
                                                      renderer,
                                                      "text", CHANNEL_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);


   return channel_tree;
   }

    GtkWidget * create_microscope_tree()
    {

    /* Create tree-view for files */
    enum
    {
     NAME_COLUMN,
     NA_COLUMN,
     NI_COLUMN,
     DX_COLUMN,
     DZ_COLUMN,
     SN_COLUMNS
    };

    GtkTreeStore * scope_store = gtk_tree_store_new (SN_COLUMNS,       /* Total number of columns */
                                                       G_TYPE_STRING,
                                                       G_TYPE_FLOAT,// na
                                                       G_TYPE_FLOAT, // ni
                                                       G_TYPE_FLOAT, // dx
                                                     G_TYPE_FLOAT); // dz

    GtkTreeIter iter1;  /* Parent iter */

    gtk_tree_store_append (scope_store, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (scope_store, &iter1,
                        NAME_COLUMN, "Bicroscope1 100X",
                        NA_COLUMN, 1.45,
                        NI_COLUMN, 1.512,
                        DX_COLUMN, 130.0,
                        DZ_COLUMN, (float) 300.0
                        -1);


    GtkWidget * scope_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (scope_store));
    GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer),
                 "foreground", "black",
                 NULL);
    g_object_set(G_OBJECT (renderer), "editable", TRUE, NULL);

   /* Create a column, associating the "text" attribute of the
    * cell_renderer to the first column of the model */
   GtkTreeViewColumn * column = gtk_tree_view_column_new_with_attributes ("Name", renderer,
                                                      "text", NAME_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);


   column = gtk_tree_view_column_new_with_attributes ("NA",
                                                      renderer,
                                                      "text", NA_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);

   column = gtk_tree_view_column_new_with_attributes ("ni",
                                                      renderer,
                                                      "text", NI_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);


   column = gtk_tree_view_column_new_with_attributes ("dx [nm]",
                                                      renderer,
                                                      "text", DX_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);


   column = gtk_tree_view_column_new_with_attributes ("dz [nm]",
                                                      renderer,
                                                      "text", DZ_COLUMN,
                                                      NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);

   return scope_tree;
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

    enum
    {
     FILE_COLUMN,
     CHANNEL_COLUMN,
     N_COLUMNS
    };

    char * cname = get_channel_name(fname);
    gtk_tree_store_append (filetm, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (filetm, &iter1,
                        FILE_COLUMN, fname,
                        CHANNEL_COLUMN, cname,

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
  printf("---\n%s---\n", data);
  fflush(stdout);

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
    //    return gtk_frame_new(NULL);

    GtkWidget * status = gtk_text_view_new();
    GtkWidget * cmd = gtk_text_view_new();

    config.cmd = cmd;
    config.status = status;

    g_object_set(G_OBJECT(status), "editable", FALSE, NULL);
    g_object_set(G_OBJECT(cmd), "editable", FALSE, NULL);

    GtkWidget * fcmd = gtk_frame_new("Commands");
    gtk_container_add(GTK_CONTAINER(fcmd), cmd);

    GtkWidget * fstatus = gtk_frame_new("Status");
    gtk_container_add(GTK_CONTAINER(fstatus), status);

    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start ((GtkBox*) box,
                        fstatus,
                        TRUE,
                        TRUE,
                        5);

    gtk_box_pack_end ((GtkBox*) box,
                        fcmd,
                        TRUE,
                        TRUE,
                        5);

    GtkWidget * frame = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER (frame), box);
    gtk_widget_show (status);
    gtk_widget_show (cmd);

    return frame;
}


gboolean
tab_change_cb(GtkNotebook *notebook,
              GtkWidget   *page,
              guint        page_num,
              gpointer     user_data)
{

    if(page_num == 4)
    {
        printf("Todo: update status and cmd\n");
    }
    return TRUE;
}

DwAppWindow *
dw_app_window_new (DwApp *app)
{

    GtkWidget * frame_drop = gtk_frame_new (NULL);
    GtkWidget * frame_files = gtk_frame_new (NULL);
    GtkWidget * frame_channels = gtk_frame_new (NULL);
    GtkWidget * frame_scope = gtk_frame_new (NULL);
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

    g_signal_connect(notebook, "switch-page",
                     G_CALLBACK(tab_change_cb), NULL);

    GtkWidget *image = gtk_image_new_from_resource("/images/wolf1.png");

    GtkWidget * file_tree = create_file_tree();
    GtkWidget * channel_tree = create_channel_tree();
    GtkWidget * scope_tree = create_microscope_tree();

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
    gtk_container_add (GTK_CONTAINER (frame_scope), scope_tree);

    gtk_container_add (GTK_CONTAINER (window), notebook);

    config.file_tree = file_tree;
    config.channel_tree = channel_tree;
    config.scope_tree = scope_tree;


    return window;

}

void
dw_app_window_open (DwAppWindow *win,
                         GFile            *file)
{
}
