#include <gtk/gtk.h>
#include "dw_app.h"
#include "dw_app_window.h"
#include <libgen.h>
#include <locale.h>
#include <assert.h>
#include <glib.h>
#include <glib/gstdio.h>

// Global settings for this app
typedef struct {
    DwAppWindow * window; // main window
    GtkWidget * file_tree;
    GtkWidget * channel_tree;
    GtkWidget * scope_tree;
    GtkWidget * cmd;
    GtkWidget * status;
    GtkNotebook * notebook; // There is just one notebook

    GtkAdjustment * dwc_nthreads;
    GtkAdjustment * dwc_tilesize;
    GtkToggleButton * dwc_outformat_uint16;
    GtkSwitch * dwc_overwrite;
    char * savefolder; // Suggested folder to save the script in
    gboolean has_dw;
    char * default_open_uri; // Where to open files
    char * regexp_channel; // Regular expression to identify channels

    GtkToggleButton* bq_best;
    GtkToggleButton* bq_good;
    GtkToggleButton* bq_bad;

    GtkToggleButton* hw_cpu;
    GtkToggleButton* hw_gpu;

} GlobConf;

GlobConf config;

gboolean has_dw()
{
    int ret = system("dw --version > /dev/null 2>&1"); //The redirect to /dev/null ensures that your program does not produce the output of these commands.
    if (ret == 0) {
        //The executable was found.
        return TRUE;
    }
    return FALSE;
}

/* Forward declarations */
static  void
drag_data_cb(GtkWidget *wgt, GdkDragContext *context, int x, int y,
             GtkSelectionData *seldata, guint info, guint time,
             gpointer userdata);


char * get_configuration_file(char * name)
/* Return the name for the configuration file */
{

    // Set up the configuration folder
    char * cfile = malloc(1024);
    sprintf(cfile, "%s/%s/", g_get_user_config_dir(), "deconwolf");
    if(g_mkdir_with_parents(cfile, S_IXUSR | S_IWUSR | S_IRUSR) == -1)
    {
        printf("Unable to access %s\n", cfile);
        free(cfile);
        return NULL;
    }

    // Return the actual name for the configuration file
    sprintf(cfile, "%s/deconwolf/dw_gui_%s", g_get_user_config_dir(), name);
    return cfile;
}

DwConf * parse_dw_conf()
{
    DwConf *conf = dw_conf_new();
    conf->nthreads = (int) round(gtk_adjustment_get_value(config.dwc_nthreads));
    conf->tilesize = (int) round(gtk_adjustment_get_value(config.dwc_tilesize));
    conf->overwrite = gtk_switch_get_state(config.dwc_overwrite);
    if( gtk_toggle_button_get_active(config.dwc_outformat_uint16))
    {
        conf->outformat = DW_CONF_OUTFORMAT_UINT16;
    }
    else
    {
        conf->outformat = DW_CONF_OUTFORMAT_FLOAT32;
    }

    if(gtk_toggle_button_get_active(config.bq_best)){
        conf->border_quality = DW_CONF_BORDER_QUALITY_BEST;
    }
    if(gtk_toggle_button_get_active(config.bq_good)){
        conf->border_quality = DW_CONF_BORDER_QUALITY_GOOD;
    }
    if(gtk_toggle_button_get_active(config.bq_bad)){
        conf->border_quality = DW_CONF_BORDER_QUALITY_BAD;
    }

    if(gtk_toggle_button_get_active(config.hw_gpu)){
        conf->use_gpu = 1;
    } else {
        conf->use_gpu = 0;
    }

    return conf;
}

gboolean
save_dw_settings_cb (GtkWidget *widget,
                     gpointer   user_data)
{
    UNUSED(widget);
    UNUSED(user_data);
    DwConf * conf = parse_dw_conf();
    char * cfile = get_configuration_file("deconwolf");
    dw_conf_save_to_file(conf, cfile);
    free(cfile);

    return TRUE;
}

gboolean
next_page_cb (GtkWidget *widget,
              gpointer   user_data)
{
    UNUSED(widget);
    UNUSED(user_data);
    gtk_notebook_next_page (config.notebook);
    return TRUE;
}



GtkWidget * create_deconwolf_frame()
{

    char * cfile = get_configuration_file("deconwolf");
    DwConf * dwconf = dw_conf_new_from_file(cfile);
    free(cfile);

    GtkAdjustment * adjThreads =
        gtk_adjustment_new (dwconf->nthreads, 1, 1024, 1, 1, 1);
    config.dwc_nthreads = adjThreads;

    GtkAdjustment * adjTile =
        gtk_adjustment_new (dwconf->tilesize, 100, 1024*1024, 1, 10, 1);
    config.dwc_tilesize = adjTile;

    GtkWidget * lOverwrite = gtk_label_new("Overwrite existing files?");
    GtkWidget * lThreads = gtk_label_new("Number of threads to use:");
    GtkWidget * lTile = gtk_label_new("Max side length of a tile [pixels]");

    GtkWidget * vOverwrite = gtk_switch_new();
    gtk_switch_set_state((GtkSwitch*) vOverwrite, dwconf->overwrite);
    config.dwc_overwrite = (GtkSwitch*) vOverwrite;
    gtk_widget_set_halign((GtkWidget*) config.dwc_overwrite, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget*) config.dwc_overwrite, GTK_ALIGN_CENTER);

    GtkWidget * vThreads = gtk_spin_button_new(adjThreads, 1, 0);
    GtkWidget * vTile = gtk_spin_button_new(adjTile, 10, 0);

    GtkWidget * lFormat = gtk_label_new("Output format:");
    GtkWidget * out_uint16 = gtk_radio_button_new_with_label(NULL, "unsigned 16-bit");
    GtkWidget * out_float32 = gtk_radio_button_new_with_label(NULL, "32 bit floating point");
    gtk_radio_button_join_group((GtkRadioButton*) out_float32, (GtkRadioButton*) out_uint16);
    config.dwc_outformat_uint16 = (GtkToggleButton*) out_uint16;
    if(dwconf->outformat == DW_CONF_OUTFORMAT_UINT16)
    {
        gtk_toggle_button_set_active( (GtkToggleButton*) out_uint16, TRUE);
    }
    else
    {
        gtk_toggle_button_set_active( (GtkToggleButton*) out_float32, TRUE);
    }

    GtkWidget * lBorder = gtk_label_new("Border quality");
    GtkWidget * bq_best = gtk_radio_button_new_with_label(NULL, "Best (Default)");
    GtkWidget * bq_good = gtk_radio_button_new_with_label(NULL, "Good");
    GtkWidget * bq_bad = gtk_radio_button_new_with_label(NULL, "Periodic (fastest)");
    config.bq_best = (GtkToggleButton*) bq_best;
    config.bq_good = (GtkToggleButton*) bq_good;
    config.bq_bad = (GtkToggleButton*) bq_bad;

    gtk_radio_button_join_group(
        (GtkRadioButton*) bq_best,
        (GtkRadioButton*) bq_good);
    gtk_radio_button_join_group(
        (GtkRadioButton*) bq_bad,
        (GtkRadioButton*) bq_good);

    switch(dwconf->border_quality)
    {
    case DW_CONF_BORDER_QUALITY_BEST:
        gtk_toggle_button_set_active( (GtkToggleButton*) bq_best, TRUE);
        break;
    case DW_CONF_BORDER_QUALITY_GOOD:
        gtk_toggle_button_set_active( (GtkToggleButton*) bq_good, TRUE);
        break;
    case DW_CONF_BORDER_QUALITY_BAD:
        gtk_toggle_button_set_active( (GtkToggleButton*) bq_bad, TRUE);
        break;
    }

    /* CPU / GPU */
    GtkWidget * lHardware = gtk_label_new("Hardware");
    GtkWidget * hw_cpu = gtk_radio_button_new_with_label(NULL, "CPU (Default)");
    GtkWidget * hw_gpu = gtk_radio_button_new_with_label(NULL, "GPU (read the docs!)");

    config.hw_cpu = (GtkToggleButton*) hw_cpu;
    config.hw_gpu = (GtkToggleButton*) hw_gpu;

    gtk_radio_button_join_group(
        (GtkRadioButton*) hw_cpu,
        (GtkRadioButton*) hw_gpu);

    if(dwconf->use_gpu)
    {
        gtk_toggle_button_set_active( (GtkToggleButton*) hw_gpu, TRUE);
    } else {
        gtk_toggle_button_set_active( (GtkToggleButton*) hw_cpu, TRUE);
    }

    GtkWidget * grid = gtk_grid_new();
    gtk_grid_set_row_spacing ((GtkGrid*) grid , 5);
    gtk_grid_set_column_spacing ((GtkGrid*) grid , 5);
    // x, y, w, h
    gtk_grid_attach((GtkGrid*) grid, lThreads, 1, 1, 1, 2);
    gtk_grid_attach((GtkGrid*) grid, vThreads, 2, 1, 2, 1);
    gtk_grid_attach((GtkGrid*) grid, lOverwrite, 1, 3, 1, 2);
    gtk_grid_attach((GtkGrid*) grid, vOverwrite, 2, 3, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, lTile, 1, 5, 1, 2);
    gtk_grid_attach((GtkGrid*) grid, vTile, 2, 5, 2, 2);
    // Output format
    gtk_grid_attach((GtkGrid*) grid, lFormat, 1, 9, 1, 2);
    gtk_grid_attach((GtkGrid*) grid, out_uint16, 2, 9, 2, 1);
    gtk_grid_attach((GtkGrid*) grid, out_float32, 2, 10, 2, 1);
    /* Border option */
    gtk_grid_attach((GtkGrid*) grid, lBorder, 1, 12, 1, 3);
    gtk_grid_attach((GtkGrid*) grid, bq_best, 2, 12, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, bq_good, 2, 13, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, bq_bad, 2, 14, 1, 1);
    /* Hardware option */
    gtk_grid_attach((GtkGrid*) grid, lHardware, 1, 16, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, hw_cpu, 2, 16, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, hw_gpu, 2, 17, 1, 1);

    gtk_widget_set_halign((GtkWidget*) grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget*) grid, GTK_ALIGN_CENTER);

    GtkWidget * btnSave = gtk_button_new_from_icon_name("document-save",
                                                        GTK_ICON_SIZE_SMALL_TOOLBAR);

    GtkWidget * btnNext = gtk_button_new_from_icon_name("go-next",
                                                        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(btnNext, "Next page");
    g_signal_connect (btnNext, "clicked", G_CALLBACK (next_page_cb), NULL);

    gtk_widget_set_tooltip_text(btnSave, "Set as defaults.");
    g_signal_connect (btnSave, "clicked", G_CALLBACK (save_dw_settings_cb), NULL);

    GtkWidget * Bar = gtk_action_bar_new();
    gtk_action_bar_pack_start ((GtkActionBar*) Bar, btnSave);
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, btnNext);
    GtkWidget * A = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    gtk_box_pack_end ((GtkBox*) A,
                      Bar,
                      FALSE,
                      TRUE,
                      5);

    gtk_box_pack_start ((GtkBox*) A,
                        grid,
                        TRUE,
                        TRUE,
                        5);
    free(dwconf);
    return A;

}

GtkWidget * create_file_frame()
{

    GtkTreeStore * file_store = gtk_tree_store_new (fN_COLUMNS,       /* Total number of columns */
                                                    G_TYPE_STRING,   /* File name */
                                                    G_TYPE_STRING);   /* Channel */

    GtkWidget * file_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (file_store));
    //GtkWidget * file_tree = (GtkWidget*) gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (file_store));
    //gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (file_tree),
    //                                      fFILE_COLUMN, GTK_SORT_ASCENDING);
    // Fixes performance issue
    //    gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW (file_tree), TRUE);

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
    gtk_tree_view_column_set_sort_column_id(column, fFILE_COLUMN);

    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);

    /* Add the column to the view. */
    gtk_tree_view_append_column (GTK_TREE_VIEW (file_tree), column);

    //    gtk_tree_model_sort_append_column(GTK_TREE_MODEL_SORT (file_tree), column);
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Channel",
                                                       renderer,
                                                       "text", fCHANNEL_COLUMN,
                                                       NULL);
    gtk_tree_view_column_set_sort_column_id(column, fCHANNEL_COLUMN);
    gtk_tree_view_column_set_sizing (column,
                                     GTK_TREE_VIEW_COLUMN_FIXED);

    gtk_tree_view_append_column (GTK_TREE_VIEW (file_tree), column);


    GtkWidget * btnNew = gtk_button_new_from_icon_name("list-add",
                                                       GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(btnNew, "Add file(s)");

    GtkWidget * btnDel = gtk_button_new_from_icon_name("list-remove",
                                                       GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(btnDel, "Remove selected file");

    GtkWidget * btnClear = gtk_button_new_from_icon_name("edit-delete",
                                                         GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(btnClear, "Clear the list of files");
    g_signal_connect(btnClear, "clicked", G_CALLBACK (clear_files_cb), NULL);
    g_signal_connect(btnNew, "clicked", G_CALLBACK (add_files_cb), NULL);
    g_signal_connect(btnDel, "clicked", G_CALLBACK (del_file_cb), NULL);


    GtkWidget * btnNext = gtk_button_new_from_icon_name("go-next",
                                                        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(btnNext, "Next page");
    g_signal_connect (btnNext, "clicked", G_CALLBACK (next_page_cb), NULL);


    GtkWidget * Bar = gtk_action_bar_new();
    gtk_action_bar_pack_start((GtkActionBar*) Bar, btnNew);
    gtk_action_bar_pack_start((GtkActionBar*) Bar, btnDel);
    gtk_action_bar_pack_start((GtkActionBar*) Bar, btnClear);
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, btnNext);

    GtkWidget * file_tree_scroller = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (file_tree_scroller),
                       file_tree);

    GtkWidget * boxV = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start((GtkBox*) boxV,
                       file_tree_scroller,
                       TRUE, TRUE, 0);
    gtk_box_pack_end((GtkBox*) boxV,
                     Bar,
                     FALSE, TRUE, 0);

    GtkWidget * file_frame = gtk_frame_new(NULL);
    gtk_container_add (GTK_CONTAINER (file_frame),
                       boxV);

    g_signal_connect (G_OBJECT (file_tree), "key_press_event",
                      G_CALLBACK (file_tree_keypress), NULL);

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
    gtk_drag_dest_set(file_frame, GTK_DEST_DEFAULT_ALL, targetentries, 3,
                      GDK_ACTION_COPY );
    g_signal_connect(file_frame, "drag_data_received",
                     G_CALLBACK(drag_data_cb), NULL);

    return file_frame;
}


int is_tif_file_name(const char * fname)
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

char * get_channel_name_regexp(const char *fname0)
{
    char * fname = strdup(fname0);
    char * ret = NULL;

    for(size_t kk = 0 ; kk<strlen(fname); kk++)
        fname[kk] = toupper(fname[kk]);

    // Return channel name, i.e. /dir/dapi_001.tif -> DAPI

    GRegex *regex;
    GMatchInfo *match_info;

    assert(config.regexp_channel != NULL);
    assert(strlen(config.regexp_channel) > 0);

    regex = g_regex_new (config.regexp_channel, 0, 0, NULL);
    if(regex == NULL)
    {
        printf("Can't create a regular expression out of '%s'\n", config.regexp_channel);
        return NULL;
    }
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

char * get_channel_name_alias(const char * fname0)
{
    if(fname0 == NULL)
    {
        return NULL;
    }
    if(strlen(fname0) == 0)
    {
        return NULL;
    }

    char * fname = strdup(fname0);
    for(size_t kk = 0; kk<strlen(fname); kk++)
    {
        fname[kk] = toupper(fname[kk]);
    }

    char * channel = NULL;
    // Compare the file name to the aliases and return the first match
    DwChannel ** channels = dw_channels_get_from_gtk_tree_view((GtkTreeView*) config.channel_tree);
    DwChannel ** channelsp = channels;
    for( ; *channelsp && channel == NULL; channelsp++)
    {
        char * ualias = strdup(channelsp[0]->alias);
        for(size_t kk = 0; kk<strlen(ualias); kk++)
        {
            ualias[kk] = toupper(ualias[kk]);
        }
        if(strstr(fname, ualias) != NULL)
        {

            // printf("Matches %s\n", ualias);
            channel = strdup(channelsp[0]->alias);
        } else {
            // printf("%s != %s\n", fname, ualias );
        }
        free(ualias);

    }

    dw_channels_free(channels);
    return channel;
}

char * get_channel_name(const char *fname)
{
    char * cname_reg = get_channel_name_regexp(fname);
    if(cname_reg != NULL)
    {
        return cname_reg;
    }

    char * cname_alias = get_channel_name_alias(fname);
    if(cname_alias != NULL)
    {
        return cname_alias;
    }

    return NULL;
}

gboolean add_channel(char * alias, char * name, float emission, int iter)
{
    GtkTreeStore * channel_store = (GtkTreeStore*) gtk_tree_view_get_model((GtkTreeView*) config.channel_tree);
    GtkTreeIter iter1;  /* Parent iter */

    char * emission_str = malloc(1024);
    sprintf(emission_str, "%.2f", emission);
    gtk_tree_store_append (channel_store, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (channel_store, &iter1,
                        cALIAS_COLUMN, alias,
                        cNAME_COLUMN, name,
                        cNITER_COLUMN, iter,
                        cEMISSION_COLUMN, emission_str,
                        -1);
    free(emission_str);
    return TRUE;
}

gboolean
new_channel_cb(GtkWidget *widget,
               gpointer user_data)
{
    UNUSED(widget);
    UNUSED(user_data);
    DwChannel * chan = dw_channel_edit_dlg((GtkWindow*) config.window, NULL);
    if(chan != NULL)
    {
        add_channel(chan->alias, chan->name, chan->lambda, chan->niter);
        dw_channel_free(chan);
    }
    return TRUE;
}


GtkWidget * create_channel_tree()
{
    /* Create tree-view for files */
    GtkTreeStore * channel_store = gtk_tree_store_new (cN_COLUMNS,       /* Total number of columns */
                                                       G_TYPE_STRING,
                                                       G_TYPE_STRING,
                                                       G_TYPE_STRING,
                                                       G_TYPE_INT);

    GtkWidget * channel_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (channel_store));
    //gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW (channel_tree), TRUE);
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
    gtk_tree_view_column_set_sort_column_id(column, 0);

    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Emission [nm]",
                                                       renderer,
                                                       "text", cEMISSION_COLUMN,
                                                       NULL);
    gtk_tree_view_column_set_sort_column_id(column, cEMISSION_COLUMN);
    g_object_set (renderer, "xalign", 0.5, NULL);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);

    column = gtk_tree_view_column_new_with_attributes ("Iterations",
                                                       renderer,
                                                       "text", cNITER_COLUMN,
                                                       NULL);
    gtk_tree_view_column_set_sort_column_id(column, cNITER_COLUMN);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (channel_tree), column);


    column = gtk_tree_view_column_new_with_attributes ("Name",
                                                       renderer,
                                                       "text", cNAME_COLUMN,
                                                       NULL);
    gtk_tree_view_column_set_sort_column_id(column, cNAME_COLUMN);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
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
    GtkWidget * btnNext = gtk_button_new_from_icon_name("go-next",
                                                        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(btnNext, "Next page");
    g_signal_connect (btnNext, "clicked", G_CALLBACK (next_page_cb), NULL);

    g_signal_connect(btnNew, "clicked", G_CALLBACK (new_channel_cb), NULL);
    g_signal_connect(btnDel, "clicked", G_CALLBACK (del_channel_cb), NULL);
    g_signal_connect(btnSave, "clicked", G_CALLBACK (save_channels_cb), NULL);
    g_signal_connect(btnEdit, "clicked", G_CALLBACK (edit_channel_cb), NULL);

    GtkWidget * Bar = gtk_action_bar_new();
    gtk_action_bar_pack_start((GtkActionBar*) Bar, btnNew);
    gtk_action_bar_pack_start((GtkActionBar*) Bar, btnDel);
    gtk_action_bar_pack_start((GtkActionBar*) Bar, btnEdit);
    gtk_action_bar_pack_start((GtkActionBar*) Bar, btnSave);
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, btnNext);

    GtkWidget * boxV = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_end((GtkBox*) boxV,
                     Bar,
                     FALSE, TRUE, 5);

    // Make the list of channels scrollable
    GtkWidget * channel_tree_scroll = gtk_scrolled_window_new (NULL, NULL);

    gtk_container_add (GTK_CONTAINER (channel_tree_scroll),
                       channel_tree);

    gtk_box_pack_start((GtkBox*) boxV,
                       channel_tree_scroll,
                       TRUE, TRUE, 5);

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
    UNUSED(widget);
    UNUSED(user_data);

    DwScope * scope = dw_scope_edit_dlg((GtkWindow*) config.window, NULL);
    if(scope != NULL)
    {
        add_scope(scope->name, scope->NA, scope->ni, scope->xy_nm, scope->z_nm);
        free(scope);
    }

    return TRUE;
}

gboolean
save_scopes_cb (GtkWidget *widget,
                gpointer   user_data)
{
    UNUSED(widget);
    UNUSED(user_data);
    // Save list of microscopes to ini file.

    // First, figure out where:
    char * cfile = get_configuration_file("microscopes");
    DwScope ** scopes = dw_scopes_get_from_gtk_tree_view((GtkTreeView*) config.scope_tree);
    dw_scopes_to_disk(scopes, cfile);
    dw_scopes_free(scopes);
    free(cfile);
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
    //    gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW (scope_tree), TRUE);
    config.scope_tree = scope_tree;

    GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();

    g_object_set(G_OBJECT (renderer), "editable", FALSE, NULL);

    /* Create a column, associating the "text" attribute of the
     * cell_renderer to the first column of the model */
    GtkTreeViewColumn * column = gtk_tree_view_column_new_with_attributes ("Name", renderer,
                                                                           "text", sNAME_COLUMN,
                                                                           NULL);
    gtk_tree_view_column_set_sort_column_id(column, sNAME_COLUMN);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);


    column = gtk_tree_view_column_new_with_attributes ("NA",
                                                       renderer,
                                                       "text", sNA_COLUMN,
                                                       NULL);
    gtk_tree_view_column_set_sort_column_id(column, sNA_COLUMN);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);

    column = gtk_tree_view_column_new_with_attributes ("ni",
                                                       renderer,
                                                       "text", sNI_COLUMN,
                                                       NULL);
    gtk_tree_view_column_set_sort_column_id(column, sNI_COLUMN);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);


    column = gtk_tree_view_column_new_with_attributes ("dx [nm]",
                                                       renderer,
                                                       "text", sDX_COLUMN,
                                                       NULL);
    gtk_tree_view_column_set_sort_column_id(column, sDX_COLUMN);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (scope_tree), column);

    column = gtk_tree_view_column_new_with_attributes ("dz [nm]",
                                                       renderer,
                                                       "text", sDZ_COLUMN,
                                                       NULL);
    gtk_tree_view_column_set_sort_column_id(column, sDZ_COLUMN);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
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

    GtkWidget * btnSave = gtk_button_new_from_icon_name("document-save",
                                                        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(btnSave, "Save current list as default");
    GtkWidget * btnNext = gtk_button_new_from_icon_name("go-next",
                                                        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(btnNext, "Next page");
    g_signal_connect (btnNext, "clicked", G_CALLBACK (next_page_cb), NULL);

    GtkWidget * Bar = gtk_action_bar_new();
    gtk_action_bar_pack_start ((GtkActionBar*) Bar, btnNew);
    gtk_action_bar_pack_start ((GtkActionBar*) Bar, btnDel);
    gtk_action_bar_pack_start ((GtkActionBar*) Bar, btnEdit);
    gtk_action_bar_pack_start ((GtkActionBar*) Bar, btnSave);
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, btnNext);

    g_signal_connect (btnNew, "clicked", G_CALLBACK (new_scope_cb), NULL);
    g_signal_connect (btnDel, "clicked", G_CALLBACK (del_scope_cb), NULL);
    g_signal_connect (btnEdit, "clicked", G_CALLBACK (edit_scope_cb), NULL);
    g_signal_connect(btnSave, "clicked", G_CALLBACK (save_scopes_cb), NULL);


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


void file_tree_append_dnd_file(const char * file)
{
    // Should complain if the file format isn't something like:
    // file:///home/erikw/Desktop/iEG701_25oilx_200928_009/max_x_024.tif\r\n
    // Replace '%20' by ' ' etc

    GError * err = NULL;
    char * fname = g_filename_from_uri(file, NULL, &err);
    if(err != NULL || fname == NULL)
    {
        return;
    }
    if(strlen(fname) == 0)
    {
        return;
    }

    size_t lastpos = strlen(fname)-1;
    if(fname[lastpos] == '\n' || fname[lastpos] == '\r') {
        fname[lastpos] = '\0';
    }
    lastpos --;
    if(strlen(fname) > 1)
    {
        if(fname[lastpos] == '\n' || fname[lastpos] == '\r') {
            fname[lastpos] = '\0';
        }
    }

    //printf("fname: \n>%s<\n", fname);
    file_tree_append(fname);
    free(fname);
    return;
}

void file_tree_append(const char * fname)
    {
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
    return;
}



static  void
drag_data_cb(GtkWidget *wgt, GdkDragContext *context, int x, int y,
             GtkSelectionData *seldata, guint info, guint time,
             gpointer userdata)
{
    UNUSED(wgt);
    UNUSED(x);
    UNUSED(y);
    UNUSED(info);
    UNUSED(userdata);

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


    gchar ** uris =  gtk_selection_data_get_uris(seldata);
    for (gchar **uris_iter = uris; uris_iter && *uris_iter; ++uris_iter)
        {
            printf("--%s--\n", *uris_iter);
        }
    g_strfreev(uris);
    // Use g_uri_to_string() or similar here

    //printf("---\n%s---\n", data);
    //fflush(stdout);
    const guchar * data = gtk_selection_data_get_data(seldata);
    /* Append to file tree, need to split the data first */
    if(strlen( (char *) data) > 0)
    {
        char * dnd = strdup( (char *) data);
        char delim = '\n';
        char * file = strtok(dnd, &delim);

        if(file != NULL)
        {
                file_tree_append_dnd_file(file);
        }
        while( file != NULL)
        {
            file = strtok(NULL, &delim);
            if(file != NULL)
            {
                file_tree_append_dnd_file(file);
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
    UNUSED(app);
}

static void
dw_app_window_class_init (DwAppWindowClass *class)
{
    UNUSED(class);
}


#ifdef __APPLE__

void runscript(const char * name_in)
{
    gchar * name = g_shell_quote(name_in);

    char * cmd = malloc(strlen(name) + 100);
    sprintf(cmd, "open -a terminal.app %s", name);
    int ret = system(cmd);
    printf("Return value from system(): %d\n", ret);
    g_free(name);
}

#else
void runscript(const char * name_in)
{
    GAppInfo *appinfo = NULL;
    gboolean ret = FALSE;

    // Quote the command to run,
    // it might contain white spaces
    gchar * name = g_shell_quote(name_in);

    //printf("Trying to run ->%s<-\n", name);
    GError *err = NULL;
    // TODO: appinfo always returns NULL on osx
    appinfo = g_app_info_create_from_commandline(name, // command line
                                                 NULL, // To use command line
                                                 G_APP_INFO_CREATE_NEEDS_TERMINAL,
                                                 &err);
    if(err != NULL)
    {
        fprintf(stderr, "Unable to run %s. Error: %s\n", name, err->message);
        goto exit1;
    }

    if(appinfo == NULL)
    {
        fprintf(stderr, "g_app_info_create_from_commandline returned NULL, unable to launch dw\n");
        goto exit1;
    }

    ret = g_app_info_launch(appinfo,
                            NULL,
                            NULL,
                            &err);

    if(ret == FALSE)
    {
        fprintf(stderr, "g_app_info_launch returned FALSE\n");
    }

    if(err != NULL)
    {
        fprintf(stderr, "g_app_info_launch failed. Error: %s\n", err->message);
        goto exit2;
    }

 exit2:
    g_object_unref (appinfo);

 exit1:
    g_free(name);
}
#endif

gboolean save_cmd(GtkWindow * parent_window, char ** savename)
/* Save the command-queue to disk. Returns true on success and sets savename
   to the filename that was used.
*/
{

    gboolean saved = FALSE;

    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new ("Save File",
                                          parent_window,
                                          action,
                                          "_Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "_Save",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    chooser = GTK_FILE_CHOOSER (dialog);

    // Todo: handle "confirm-overwrite" signal to
    //       add the option "append".
    // see: https://developer.gnome.org/gtk3/stable/GtkFileChooser.html#GtkFileChooser-confirm-overwrite

    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

    // use folder of first tif file if available
    if(config.savefolder != NULL)
    {
        gtk_file_chooser_set_current_folder (chooser, config.savefolder);
    }

    char * suggname = malloc(1024);
    sprintf(suggname, "dw_script");
    gtk_file_chooser_set_current_name (chooser, suggname);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char * filename = gtk_file_chooser_get_filename (chooser);
        char * sname = strdup(filename);
        savename[0] = sname;

        // save_to_file (filename);
        g_free (filename);
        saved = TRUE;
    }

    gtk_widget_destroy (dialog);
    return saved;
}

gboolean save_dw_cb(GtkWidget * widget, gpointer user_data)
{
    UNUSED(widget);
    UNUSED(user_data);
    // This should by factored to remove out duplicate code in
    // gboolean run_dw_cb(GtkWidget * widget, gpointer user_data)
    //
    char * filename = NULL;
    if(save_cmd((GtkWindow*) config.window, &filename))
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
        gboolean result = TRUE;;
        GError * err = NULL;
        if (filename != NULL)
        {
            result = g_file_set_contents (filename, text, -1, &err);
            if (result == FALSE)
            {
                /* error saving file, show message to user */
                //error_message (err->message);
                g_error_free (err);

            } else {
                int chmod_ok = g_chmod(filename, S_IXUSR | S_IRGRP | S_IXGRP );
                g_assert(chmod_ok == 0);
            }
        }
        g_free (text);
        free(filename);
    }
    return TRUE;
}


gboolean run_dw_cb(GtkWidget * widget, gpointer user_data)
{
    UNUSED(widget);
    UNUSED(user_data);
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

            if (result == FALSE)
            {
                /* error saving file, show message to user */
                //error_message (err->message);
                g_error_free (err);

            } else {
                int chmod_ok = g_chmod(filename, S_IXUSR | S_IWUSR | S_IRUSR );
                g_assert(chmod_ok == 0);
            }
        }
        g_free (text);


        // Run it
        //printf("To run: %s\n", filename);
        runscript(filename);
        free(filename);
    }
    return TRUE;
}


GtkWidget * create_run_frame()
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
    g_signal_connect (ButtonSaveAs, "clicked", G_CALLBACK (save_dw_cb), NULL);

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

void update_cmd()
{

    // Parse data from the GUI components
    DwScope * scope = dwscope_get_selected_from_gtk_tree_view((GtkTreeView*) config.scope_tree);
    DwChannel ** channels = dw_channels_get_from_gtk_tree_view((GtkTreeView*) config.channel_tree);
    DwFile ** files = dw_files_get_from_gtk_tree_view((GtkTreeView*) config.file_tree);
    DwConf * dwconf = parse_dw_conf();
    //printf("dwconf->outformat %d\n", dwconf->outformat);

    // Update the suggested folder to save the script to
    if(config.savefolder != NULL)
    {
        free(config.savefolder);
    }

    if(files != NULL)
    {
        config.savefolder = g_path_get_dirname(files[0]->name);
    }

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
    sprintf(buff, "# Microscope: '%s'\n", scope->name);
    gtk_text_buffer_insert(buffer, &titer, buff, -1);
    //sprintf(buff, "# %d channels available\n", nchan);
    //gtk_text_buffer_insert(buffer, &titer, buff, -1);
    //sprintf(buff, "# %d files available\n", nfiles);
    //gtk_text_buffer_insert(buffer, &titer, buff, -1);

    int nthreads = dwconf->nthreads;
    int tilesize = dwconf->tilesize;

    char * ostring = malloc(1024);
    ostring[0] = '\0';
    if(dwconf->overwrite)
    {
        sprintf(ostring, " --overwrite ");
    }

    char * fstring = malloc(1024);
    fstring[0] = '\0';
    if(dwconf->outformat == DW_CONF_OUTFORMAT_FLOAT32)
    {
        sprintf(fstring, " --float");
    }

    switch(dwconf->border_quality)
    {
    case DW_CONF_BORDER_QUALITY_BAD:
        sprintf(fstring + strlen(fstring), " --bq 0");
        break;
    case DW_CONF_BORDER_QUALITY_GOOD:
        sprintf(fstring + strlen(fstring), " --bq 1");
        break;
    case DW_CONF_BORDER_QUALITY_BEST:
        sprintf(fstring + strlen(fstring), " --bq 2");
        break;
    }

    if(dwconf->use_gpu == 1)
    {
        sprintf(fstring + strlen(fstring), " --method shbcl2");
    }

    /* Generate the list of commands to run */
    if(files == NULL)
    {
        goto nofiles;
    }
    int kk = 0;
    while(files[kk] != NULL)
    {
        DwChannel * ch = dw_channels_get_by_alias(channels, files[kk]->channel);
        if(ch != NULL)
        {
            gchar * fdir = g_path_get_dirname(files[kk]->name);
            char * psf = get_psfname(fdir, files[kk]->channel);

            /* Shell quoted version of the file names, allows files to contain
               single quotes in their names and avoids "accidental" mixing file
               names with commands :)
            */
            char * outdir = malloc(strlen(fdir) + 100);
            sprintf(outdir, "%s/PSFBW/", fdir);
            gchar * q_outdir = g_shell_quote(outdir);
            gchar * q_psfname = g_shell_quote(psf);
            gchar * q_filename = g_shell_quote(files[kk]->name);


            sprintf(buff, "mkdir %s\n", q_outdir);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);
            sprintf(buff, "dw_bw %s --lambda %f --NA %.3f --ni %.3f --threads %d"
                    " --resxy %.1f --resz %.1f %s\n",
                    ostring,
                    ch->lambda, scope->NA, scope->ni, nthreads,
                    scope->xy_nm, scope->z_nm,
                    q_psfname);

            //        printf("%s", buff);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);
            sprintf(buff, "dw %s %s --tilesize %d --iter %d --threads %d %s %s\n",
                    ostring,
                    fstring,
                    tilesize, ch->niter, nthreads,
                    q_filename, q_psfname);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);

            g_free(q_filename);
            g_free(q_psfname);
            g_free(q_outdir);
            free(outdir);

            g_free(fdir);
            free(psf);
        } else {
            sprintf(buff, "# Missing channel for: %s\n", files[kk]->name);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);
        }
        kk++;
    }

 nofiles:
    gtk_text_view_set_buffer (cmd,
                              buffer);

    free(buff);
    free(ostring);
    free(fstring);

    dw_conf_free(dwconf);
    dw_files_free(files);
    dw_scope_free(scope);
    dw_channels_free(channels);

    return;
}


gboolean
tab_change_cb(GtkNotebook *notebook,
              GtkWidget   *page,
              guint        page_num,
              gpointer     user_data)
{
    UNUSED(notebook);
    UNUSED(page);
    UNUSED(user_data);

    if(page_num == 5)
    {
        update_cmd();
    }
    return TRUE;
}

gboolean del_file_cb(GtkWidget * w, gpointer p)
{
    UNUSED(w);
    UNUSED(p);
    del_selected_file();
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
        DwScope * scope = dw_scope_edit_dlg((GtkWindow*) config.window, current_scope);
        if(scope != NULL)
        {
            dw_scope_to_gtk_tree_store(scope, (GtkTreeStore*) model, &iter);
            free(scope);
        }
        free(current_scope);
    }
}


void edit_selected_channel()
{

    GtkTreeModel * model =
        gtk_tree_view_get_model ( (GtkTreeView*) config.channel_tree);
    GtkTreeSelection * selection = gtk_tree_view_get_selection ( (GtkTreeView*) config.channel_tree);
    GtkTreeIter iter;

    // Remove the first selected item
    // Note: has to be modified to handle multiple selected
    if(gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        // Get what we need from the mode
        gchar * cname;
        gchar * calias;
        gchar * clambda;
        gint cniter;

        gtk_tree_model_get(model, &iter,
                           cALIAS_COLUMN, &calias,
                           cNAME_COLUMN, &cname,
                           cEMISSION_COLUMN, &clambda,
                           cNITER_COLUMN, &cniter,
                           -1);
        DwChannel * curr = malloc(sizeof(DwChannel));
        curr->name = strdup(cname);
        curr->alias = strdup(calias);
        curr->lambda = atof(clambda);
        curr->niter = cniter;

        DwChannel * new = dw_channel_edit_dlg((GtkWindow*) config.window, curr);
        if(new != NULL)
        {
            char * lambdastr = malloc(1024);
            sprintf(lambdastr, "%.2f", new->lambda);
            gtk_tree_store_set((GtkTreeStore*) model, &iter,
                               cALIAS_COLUMN, new->alias,
                               cNAME_COLUMN, new->name,
                               cEMISSION_COLUMN, lambdastr,
                               cNITER_COLUMN, new->niter,
                               -1);
            free(lambdastr);
            free(new);
        }
        free(curr);
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
    UNUSED(w);
    UNUSED(p);
    edit_selected_scope();
    return TRUE;
}

gboolean edit_channel_cb(GtkWidget * w, gpointer p)
{
    UNUSED(w);
    UNUSED(p);
    edit_selected_channel();
    return TRUE;
}

gboolean save_channels_cb(GtkWidget * w, gpointer p)
{
    UNUSED(w);
    UNUSED(p);
    // Save list of channels to ini file.

    // First, figure out where:
    char * cfile = get_configuration_file("channels");
    DwChannel ** channels = dw_channels_get_from_gtk_tree_view((GtkTreeView*) config.channel_tree);
    dw_channels_to_disk(channels, cfile);
    dw_channels_free(channels);
    free(cfile);
    return TRUE;
}

gboolean add_files_cb(GtkWidget * w, gpointer p)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;


    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          (GtkWindow*) config.window,
                                          action,
                                          "_Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "_Open",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_file_chooser_set_select_multiple((GtkFileChooser *) dialog, TRUE);

    if(config.default_open_uri != NULL)
    {
        gtk_file_chooser_set_current_folder_uri( (GtkFileChooser *) dialog, config.default_open_uri);
    }



    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {

        if(config.default_open_uri != NULL)
        {
            free(config.default_open_uri);
        }
        config.default_open_uri = gtk_file_chooser_get_current_folder_uri( (GtkFileChooser *) dialog );

        GSList * filenames=NULL, *iter=NULL;;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filenames = gtk_file_chooser_get_filenames(chooser);
        for(iter = filenames; iter; iter = iter->next)
        {
            //printf("File: %s\n", (char *) iter->data);
            file_tree_append((char *) iter->data);
        }
        g_slist_free(filenames);
    }

    gtk_widget_destroy (dialog);



    return TRUE;
}

gboolean clear_files_cb(GtkWidget * w, gpointer p)
{
    UNUSED(w);
    UNUSED(p);
    GtkTreeModel * model = gtk_tree_view_get_model((GtkTreeView*) config.file_tree);
    GtkTreeSelection * sel = gtk_tree_view_get_selection((GtkTreeView*) config.file_tree);
    GtkTreeIter iter;

    while(gtk_tree_selection_get_selected(sel, &model, &iter))
    {
        gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
    }
    return TRUE;
}

gboolean del_channel_cb(GtkWidget * w, gpointer p)
{
    UNUSED(w);
    UNUSED(p);
    del_selected_channel();
    return TRUE;
}
gboolean del_scope_cb(GtkWidget * w, gpointer p)
{
    UNUSED(w);
    UNUSED(p);
    del_selected_scope();
    return TRUE;
}

gboolean file_tree_keypress (GtkWidget *tree_view, GdkEventKey *event, gpointer p)
{
    UNUSED(tree_view);
    UNUSED(p);
    if (event->keyval == GDK_KEY_Delete){
        del_selected_file();
    }
    return FALSE;
}

gboolean channel_tree_keypress (GtkWidget *tree_view, GdkEventKey *event, gpointer p)
{
    UNUSED(tree_view);
    UNUSED(p);
    if (event->keyval == GDK_KEY_Delete){
        del_selected_channel();
    }
    if (event->keyval == GDK_KEY_Return){
        edit_selected_channel();
    }
    return FALSE;
}

gboolean microscope_tree_keypress (GtkWidget *tree_view, GdkEventKey *event, gpointer p)
{
    UNUSED(tree_view);
    UNUSED(p);
    if (event->keyval == GDK_KEY_Delete){
        del_selected_scope();
    }
    if (event->keyval == GDK_KEY_Return){
        edit_selected_scope();
    }
    return FALSE;
}

gboolean channel_tree_buttonpress(GtkWidget *tree_view,
                                     GdkEventButton * event,
                                     gpointer p)
{
    UNUSED(tree_view);
    UNUSED(p);
    if(event->type == GDK_DOUBLE_BUTTON_PRESS)
    {
        edit_selected_channel();
    }
    return FALSE;
}

gboolean microscope_tree_buttonpress(GtkWidget *tree_view,
                                     GdkEventButton * event,
                                     gpointer p)
{
    UNUSED(tree_view);
    UNUSED(p);
    if(event->type == GDK_DOUBLE_BUTTON_PRESS)
    {
        edit_selected_scope();
    }
    return FALSE;
}

void populate_channels()
{
    DwChannel ** channels = NULL;
    // Set up channels from configuration file or use defaults
    char * cfile = get_configuration_file("channels");
    if(cfile != NULL)
    {
        channels = dw_channels_from_disk(cfile);
        free(cfile);
    }

    int pos = 0;
    if(channels != NULL)
    {
        while(channels[pos] != NULL)
        {
            DwChannel * chan = channels[pos++];
            add_channel(chan->alias, chan->name, chan->lambda, chan->niter);
        }
        dw_channels_free(channels);
    }
    else
    {
        /* Until we read and write configuration files, add some defaults */
        add_channel("DAPI", "4′,6-diamidino-2-phenylindole", 466.0, 50);
        add_channel("A594", "Alexa Fluor 594", 617.0, 100);
        add_channel("CY5", "Alexa Fluor 647", 664.0, 100);
        add_channel("TMR", "Tetramethylrhodamine", 562.0, 100);
    }
    return;
}

void populate_microscopes()
{
    char * cfile = get_configuration_file("microscopes");
    DwScope ** scopes = dw_scopes_from_disk(cfile);
    free(cfile);

    int pos = 0;
    if(scopes != NULL)
    {
        while(scopes[pos] != NULL)
        {
            DwScope * scope = scopes[pos++];
            add_scope(scope->name, scope->NA, scope->ni, scope->xy_nm, scope->z_nm);
        }
        dw_scopes_free(scopes);
    }
    else
    {
        add_scope("Bicroscope-1, 100X", 1.45, 1.515, 130, 250);
        add_scope("Bicroscope-1, 60X", 1.40, 1.515, 216, 350);
        add_scope("Bicroscope-2, 100X", 1.40, 1.515, 65, 250);
    }
    return;
}


GtkWidget * create_drop_frame()
{

    GtkWidget * frame_drop = gtk_frame_new (NULL);
    GtkWidget * overlay = gtk_overlay_new();

    GtkWidget * image = gtk_image_new_from_resource("/images/wolf1.png");

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

    gtk_drag_dest_set(frame_drop, GTK_DEST_DEFAULT_ALL, targetentries, 3,
                      GDK_ACTION_COPY);
    g_signal_connect(frame_drop, "drag_data_received",
                     G_CALLBACK(drag_data_cb), NULL);
    //g_signal_connect(frame_drop, "drag-motion",
    //                 G_CALLBACK(drag_motion_cb), NULL);

#ifdef __APPLE__
    GtkWidget * label = gtk_label_new("Drag and Drop"
    "does not work on OSX at the moment.\n"
    "Please add files from the 'Files' tab.");
#else
    GtkWidget * label = gtk_label_new("Drag and Drop images here");
#endif
    gtk_widget_set_halign((GtkWidget* ) label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget* ) label, GTK_ALIGN_CENTER);
    gtk_widget_show(label);
    gtk_container_add (GTK_CONTAINER (overlay), image);
    gtk_overlay_add_overlay((GtkOverlay*) overlay, label);
    gtk_container_add (GTK_CONTAINER (frame_drop), overlay);
    return frame_drop;
}

static void
about_activated(GSimpleAction *simple,
               GVariant      *parameter,
               gpointer       p)
{
    UNUSED(simple);
    UNUSED(parameter);
    UNUSED(p);
    GtkWidget * about = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name((GtkAboutDialog*) about, "deconwolf GUI");
    gtk_about_dialog_set_version( (GtkAboutDialog*) about, DW_GUI_VERSION);
    gtk_about_dialog_set_website((GtkAboutDialog*) about, "https://github.com/elgw/dw_gui/");

    //    gtk_about_dialog_set_authors((GtkAboutDialog*) about, &author);
    gtk_window_set_title((GtkWindow*) about, "About ...");


    gtk_widget_show((GtkWidget*) about);
}

static void
edit_global_config(void)
{

    GtkWidget * dialog, *content_area;
    GtkDialogFlags flags;

    // Create the widgets
    flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    dialog = gtk_dialog_new_with_buttons ("Edit global settings",
                                          (GtkWindow*) config.window,
                                          flags,
                                          "Cancel",
                                          GTK_RESPONSE_NONE,
                                          "Ok",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    GtkWidget * lRegexp = gtk_label_new("Regular expression");
    GtkWidget * eRegexp = gtk_entry_new();
    gtk_entry_set_text((GtkEntry*) eRegexp, config.regexp_channel);

    GtkWidget * grid = gtk_grid_new();
    gtk_grid_set_row_spacing ((GtkGrid*) grid , 5);
    gtk_grid_set_column_spacing ((GtkGrid*) grid , 5);

    GtkWidget * lRegexp_extra = gtk_label_new("Set the regular expression used to identify channel \nnames from the file names. For example, if the \nchannel name is at the end, \ntry '[A-Z0-9]*\\_([A-Z0-9]*)\\.TIFF?'\n");
    gtk_label_set_line_wrap((GtkLabel*) lRegexp_extra, TRUE);
    gtk_label_set_selectable((GtkLabel*) lRegexp_extra, TRUE);

    gtk_grid_attach((GtkGrid*) grid, lRegexp_extra, 1, 1, 3, 2);
    gtk_grid_attach((GtkGrid*) grid, lRegexp, 1, 3, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, eRegexp, 2, 3, 2, 1);

    gtk_widget_set_halign((GtkWidget*) grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget*) grid, GTK_ALIGN_CENTER);

    gtk_container_add (GTK_CONTAINER (content_area),  grid);
    gtk_widget_show_all(content_area);
    int result = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (result)
    {
    case GTK_RESPONSE_ACCEPT:
        sprintf(config.regexp_channel, "%s", gtk_entry_get_text((GtkEntry*) eRegexp));
        break;
    default:
        // do_nothing_since_dialog_was_cancelled ();
        break;
    }
    gtk_widget_destroy (dialog);

    return;

}

static void
 configuration_activated(GSimpleAction *simple,
                      GVariant      *parameter,
                      gpointer       p)
{
    UNUSED(simple);
    UNUSED(parameter);
    UNUSED(p);
    edit_global_config();
    //printf("configuration...\n");

}

static GActionEntry main_menu_actions[] =
    {
     { "about", about_activated, NULL, NULL, NULL },
     { "configuration", configuration_activated, NULL, NULL, NULL }
    };

void warn_no_dw(GtkWindow * parent)
{
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget * dialog = gtk_message_dialog_new (parent,
                                     flags,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                     "Could not locate deconwolf (i.e, the command 'dw'). You will not be able to run anything from this GUI!"
                                     );
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

DwAppWindow *
dw_app_window_new (DwApp *app)
{
    setlocale(LC_ALL,"C");

    config.default_open_uri = NULL;
    config.savefolder = NULL;
    config.has_dw = has_dw();
    config.regexp_channel = malloc(1024);
    sprintf(config.regexp_channel, "([A-Z0-9]*)\\_[0-9]*\\.TIFF?");

    // Set up a fallback icon
    GError * error = NULL;
    GdkPixbuf * im = gdk_pixbuf_new_from_resource("/images/wolf1.png", &error);
    int width = gdk_pixbuf_get_width(im);
    int height = gdk_pixbuf_get_height(im);
    int new_height = round(100.0 / ( (double) width) * (double) height );
    GdkPixbuf * icon = gdk_pixbuf_scale_simple(im, 100, new_height, GDK_INTERP_BILINEAR);
    g_object_unref(im);
    gtk_window_set_default_icon(icon);
    g_object_unref(icon);

    GtkWidget * frame_drop = create_drop_frame ();
    GtkWidget * frame_dw = create_deconwolf_frame();
    GtkWidget * frame_files = create_file_frame();
    GtkWidget * frame_channels = gtk_frame_new (NULL);
    GtkWidget * frame_scope = gtk_frame_new (NULL);
    GtkWidget * frame_run = create_run_frame();

    gtk_frame_set_shadow_type (GTK_FRAME (frame_drop), GTK_SHADOW_IN);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_files), GTK_SHADOW_IN);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_channels), GTK_SHADOW_IN);

    GtkWidget * notebook = gtk_notebook_new();
    config.notebook = (GtkNotebook*) notebook;
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

    GtkWidget * channel_tree = create_channel_tree();
    g_signal_connect (G_OBJECT (config.channel_tree), "key_press_event",
                      G_CALLBACK (channel_tree_keypress), NULL);
    g_signal_connect (G_OBJECT (config.channel_tree), "button_press_event",
                      G_CALLBACK (channel_tree_buttonpress), NULL);

    GtkWidget * scope_tab = create_microscope_tab();
    g_signal_connect (G_OBJECT (config.scope_tree), "key_press_event",
                      G_CALLBACK (microscope_tree_keypress), NULL);
    g_signal_connect (G_OBJECT (config.scope_tree), "button_press_event",
                      G_CALLBACK (microscope_tree_buttonpress), NULL);


    /* Create the window */
    DwAppWindow * window = g_object_new (DW_APP_WINDOW_TYPE, "application", app, NULL);
    config.window = window;
    gtk_window_set_title (GTK_WINDOW (window), "BiCroLab deconwolf GUI, 2021 v0.1");

    /* Pack components */
    gtk_container_add (GTK_CONTAINER (frame_channels), channel_tree);
    gtk_container_add (GTK_CONTAINER (frame_scope), scope_tab);
    gtk_container_add (GTK_CONTAINER (window), notebook);

    /* Parse saved presets */
    populate_channels();
    populate_microscopes();

    // Replace the stock menu bar with a new one
    // that has a menu
    GMenu * menu = g_menu_new();
    g_menu_insert(menu, 1, "About", "menu1.about");
    g_menu_insert(menu, 2, "Config", "menu1.configuration");

    GtkWidget * mbtn = gtk_menu_button_new();
    gtk_menu_button_set_menu_model((GtkMenuButton*) mbtn, (GMenuModel*) menu);
    GtkWidget * hbar = gtk_header_bar_new();
    gtk_header_bar_set_title((GtkHeaderBar*) hbar, "BiCroLab deconwolf GUI, 2021");
    gtk_header_bar_set_show_close_button((GtkHeaderBar*) hbar, TRUE);
    gtk_header_bar_pack_end((GtkHeaderBar*) hbar, mbtn);
    gtk_window_set_titlebar((GtkWindow*) window, hbar);

    // See https://stackoverflow.com/questions/22582768/connecting-a-function-to-a-gtkaction

    GSimpleActionGroup * group = g_simple_action_group_new ();
    g_action_map_add_action_entries (G_ACTION_MAP (group),
                                     main_menu_actions, G_N_ELEMENTS (main_menu_actions),
                                     NULL);
    gtk_widget_insert_action_group((GtkWidget*) hbar, "menu1", (GActionGroup*) group);


    if(config.has_dw)
    {
        // g_printf("Deconwolf found!\n");
    } else {
        warn_no_dw((GtkWindow*) window);
    }

    return window;

}

void
dw_app_window_open (DwAppWindow *win,
                    GFile            *file)
{
    UNUSED(win);
    UNUSED(file);
}
