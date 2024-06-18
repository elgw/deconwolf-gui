#include <assert.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <locale.h>

#include "dw_app.h"
#include "dw_app_window.h"
#include "common.h"

#define titlestr "BiCroLab deconwolf GUI, 2021-2024"

#define OUTSCRIPT_SH 0
#define OUTSCRIPT_BAT 1

// Global settings for this app
typedef struct {
    GApplication *app;
    DwAppWindow * window; // main window
    GtkWidget * file_tree;
    GtkWidget * channel_tree;
    GtkWidget * scope_tree;
    GtkWidget * cmd;
    GtkWidget * status;
    GtkNotebook * notebook; // There is just one notebook

    GtkAdjustment * dwc_nthreads;
    GtkAdjustment * dwc_tilesize;
    GtkCheckButton * dwc_outformat_uint16;
    GtkSwitch * dwc_overwrite;
    char * savefolder; // Suggested folder to save the script in
    gboolean has_dw;
    char * default_open_uri; // Where to open files
    char * regexp_channel; // Regular expression to identify channels

    GtkCheckButton* bq_best;
    GtkCheckButton* bq_good;
    GtkCheckButton* bq_bad;

    GtkCheckButton* hw_cpu;
    GtkCheckButton* hw_gpu;
    gint outscript; // sh or bat ?
} GlobConf;

GlobConf config;

gboolean has_dw()
{
    gchar* dw_path =
        g_find_program_in_path(
                               "dw"
                               );
    if (dw_path == NULL)
    {
        return false;
    }
    g_free(dw_path);
    return true;
#if 0
    printf("dw path: %s\n", dw_path);

    g_char * cmd -

        gchar * c_stdout = NULL;
    gchar * c_stderr = NULL;
    GError * error = NULL;
    gboolean result =
        g_spawn_command_line_sync(
                                  "dw",
                                  &c_stdout,
                                  &c_stderr,
                                  NULL, // wait_status
                                  &error
                                  );
    printf("stdout: %s\n", c_stdout);
    printf("stderr: %s\n", c_stderr);
    return result;
#endif
}

/* Forward declarations */
static gboolean
on_drop (GtkDropTarget *target,
         const GValue  *value,
         double         x,
         double         y,
         gpointer       data);



/** Build the name of a configuration file
 * Typical result: "channels" -> "/home/user/.config/deconwolf/dw_gui_channels"
 * Will create the folder if missing (and possible) at the first time it is run.
 */
char * get_configuration_file(const char * name)
{
    gchar * outfolder = g_build_filename(g_get_user_config_dir(), "deconwolf", NULL);

    if(g_mkdir_with_parents(outfolder, 0755) == -1)
    {
        printf("Unable to access %s\n", outfolder);
        g_free(outfolder);
        return NULL;
    }

    gchar * cfile0 = g_strjoin(NULL, "dw_gui_", name, NULL);
    gchar * cfile = g_build_filename(g_get_user_config_dir(), "deconwolf", cfile0, NULL);
    g_free(cfile0);
    return cfile;
}


static char * load_setting_string(const char * file, const char * group, const char * name, const char * default_value)
{
    char * cfile = get_configuration_file(file);
    GError * error = NULL;
    GKeyFile * key_file = g_key_file_new ();

    if (!g_key_file_load_from_file (key_file, cfile, G_KEY_FILE_NONE, &error))
    {
        if (!g_error_matches (error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
        {
            g_warning ("Error loading key file: %s", error->message);
        }
        g_error_free(error);
        g_key_file_free(key_file);
        goto return_default;
    }

    char * value = g_key_file_get_string(key_file,
                                         group, name, &error);
    g_key_file_free(key_file);

    if(error == NULL)
    {
        return value;
    }

    g_clear_error(&error);

 return_default: ;
    return g_strdup(default_value);
}

void save_setting_string(const char * file, const char * group, const char * name, const char * value)
{
    char * cfile = get_configuration_file(file);
    GError * error = NULL;
    GKeyFile * key_file = g_key_file_new ();

    g_key_file_set_string(key_file,
                          group,
                          name,
                          value);

    if (!g_key_file_save_to_file (key_file, cfile, &error))
    {
        g_warning ("Error saving key file: %s", error->message);
        g_error_free(error);
    }

    g_key_file_free(key_file);

    return;
}



DwConf * parse_dw_conf()
{
    DwConf *conf = dw_conf_new();
    conf->nthreads = (int) round(gtk_adjustment_get_value(config.dwc_nthreads));
    conf->tilesize = (int) round(gtk_adjustment_get_value(config.dwc_tilesize));
    conf->overwrite = gtk_switch_get_state(config.dwc_overwrite);
    if( gtk_check_button_get_active(config.dwc_outformat_uint16))
    {
        conf->outformat = DW_CONF_OUTFORMAT_UINT16;
    }
    else
    {
        conf->outformat = DW_CONF_OUTFORMAT_FLOAT32;
    }

    if(gtk_check_button_get_active(config.bq_best)){
        conf->border_quality = DW_CONF_BORDER_QUALITY_BEST;
    }
    if(gtk_check_button_get_active(config.bq_good)){
        conf->border_quality = DW_CONF_BORDER_QUALITY_GOOD;
    }
    if(gtk_check_button_get_active(config.bq_bad)){
        conf->border_quality = DW_CONF_BORDER_QUALITY_BAD;
    }

    if(gtk_check_button_get_active(config.hw_gpu)){
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
    g_free(cfile);

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
    //printf("create_deconwolf_frame\n");
    char * cfile = get_configuration_file("deconwolf");
    DwConf * dwconf = dw_conf_new_from_file(cfile);
    g_free(cfile);

    GtkAdjustment * adjThreads =
        gtk_adjustment_new (dwconf->nthreads, 0, 1024, 1, 1, 1);
    config.dwc_nthreads = adjThreads;

    GtkAdjustment * adjTile =
        gtk_adjustment_new (dwconf->tilesize, 100, 1024*1024, 1, 10, 1);
    config.dwc_tilesize = adjTile;

    GtkWidget * lOverwrite = gtk_label_new("Overwrite existing files?");

    GtkWidget * lThreads = gtk_label_new("Number of threads to use:");
    gtk_widget_set_tooltip_text(lThreads, "0 means automatic. There is no point in using more threads than the CPU has cores");
    GtkWidget * lTile = gtk_label_new("Max side length of a tile [pixels]");

    GtkWidget * vOverwrite = gtk_switch_new();
    gtk_switch_set_state((GtkSwitch*) vOverwrite, dwconf->overwrite);
    config.dwc_overwrite = (GtkSwitch*) vOverwrite;
    gtk_widget_set_halign((GtkWidget*) config.dwc_overwrite, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget*) config.dwc_overwrite, GTK_ALIGN_CENTER);

    GtkWidget * vThreads = gtk_spin_button_new(adjThreads, 1, 0);
    GtkWidget * vTile = gtk_spin_button_new(adjTile, 10, 0);

    GtkWidget * lFormat = gtk_label_new("Output format:");
    gtk_widget_set_tooltip_text(lFormat, "Please note that the intensity of 16-bit images will be scaled");
    GtkWidget * out_uint16 = gtk_check_button_new_with_label("unsigned 16-bit");
    GtkWidget * out_float32 = gtk_check_button_new_with_label("32 bit floating point");
    gtk_check_button_set_group((GtkCheckButton*) out_float32, (GtkCheckButton*) out_uint16);
    config.dwc_outformat_uint16 = GTK_CHECK_BUTTON(out_uint16);
    if(dwconf->outformat == DW_CONF_OUTFORMAT_UINT16)
    {
        gtk_check_button_set_active( GTK_CHECK_BUTTON(out_uint16), TRUE);
    }
    else
    {
        gtk_check_button_set_active( GTK_CHECK_BUTTON(out_float32), TRUE);
    }

    GtkWidget * lBorder = gtk_label_new("Border quality");
    GtkWidget * bq_best = gtk_check_button_new_with_label("Best (Default)");
    GtkWidget * bq_good = gtk_check_button_new_with_label("Good");
    GtkWidget * bq_bad = gtk_check_button_new_with_label("Periodic (fastest)");
    config.bq_best = GTK_CHECK_BUTTON( bq_best );
    config.bq_good = GTK_CHECK_BUTTON( bq_good );
    config.bq_bad = GTK_CHECK_BUTTON( bq_bad );

    gtk_check_button_set_group(
                               (GtkCheckButton*) bq_best,
                               (GtkCheckButton*) bq_good);
    gtk_check_button_set_group(
                               (GtkCheckButton*) bq_bad,
                               (GtkCheckButton*) bq_good);


    switch(dwconf->border_quality)
    {
    case DW_CONF_BORDER_QUALITY_BEST:
        gtk_check_button_set_active( GTK_CHECK_BUTTON( bq_best ), TRUE);
        break;
    case DW_CONF_BORDER_QUALITY_GOOD:
        gtk_check_button_set_active( GTK_CHECK_BUTTON( bq_good ), TRUE);
        break;
    case DW_CONF_BORDER_QUALITY_BAD:
        gtk_check_button_set_active( GTK_CHECK_BUTTON( bq_bad ), TRUE);
        break;
    }

    /* CPU / GPU */
    GtkWidget * lHardware = gtk_label_new("Hardware");
    GtkWidget * hw_cpu = gtk_check_button_new_with_label("CPU (Default)");
    GtkWidget * hw_gpu = gtk_check_button_new_with_label("GPU (read the docs!)");

    config.hw_cpu =  GTK_CHECK_BUTTON(hw_cpu);
    config.hw_gpu =  GTK_CHECK_BUTTON(hw_gpu);

    gtk_check_button_set_group(
                               (GtkCheckButton*) hw_cpu,
                               (GtkCheckButton*) hw_gpu);

    if(dwconf->use_gpu)
    {
        gtk_check_button_set_active( GTK_CHECK_BUTTON(hw_gpu), TRUE);
    } else {
        gtk_check_button_set_active( GTK_CHECK_BUTTON(hw_cpu), TRUE);
    }

    GtkWidget * grid = gtk_grid_new();
    gtk_grid_set_row_spacing ((GtkGrid*) grid , 5);
    gtk_grid_set_column_spacing ((GtkGrid*) grid , 5);

    GtkWidget * sep;

    // x, y, w, h
    int y = 1;
    // Threads
    gtk_grid_attach((GtkGrid*) grid, lThreads, 1, y, 1, 2);
    gtk_grid_attach((GtkGrid*) grid, vThreads, 2, y, 2, 1);
    y++;

    sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), sep, 1, y, 5, 1);
    y++;

    // Overwrite
    gtk_grid_attach((GtkGrid*) grid, lOverwrite, 1, y, 1, 2);
    gtk_grid_attach((GtkGrid*) grid, vOverwrite, 2, y, 1, 1);
    y++;
    sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), sep, 1, y, 5, 1);
    y++;
    // Tile size
    gtk_grid_attach((GtkGrid*) grid, lTile, 1, y, 1, 2);
    gtk_grid_attach((GtkGrid*) grid, vTile, 2, y, 2, 2);
    y++; y++;
    sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), sep, 1, y, 5, 1);
    y++;
    // Output format
    gtk_grid_attach((GtkGrid*) grid, lFormat, 1, y, 1, 2);
    gtk_grid_attach((GtkGrid*) grid, out_uint16, 2, y, 2, 1);
    gtk_grid_attach((GtkGrid*) grid, out_float32, 2, y+1, 2, 1);
    y++; y++;
    sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), sep, 1, y, 5, 1);
    y++;
    /* Border option */
    gtk_grid_attach((GtkGrid*) grid, lBorder, 1, y, 1, 3);
    gtk_grid_attach((GtkGrid*) grid, bq_best, 2, y, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, bq_good, 2, y+1, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, bq_bad, 2, y+2, 1, 1);
    y++; y++; y++;
    sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), sep, 1, y, 5, 1);
    y++;
    /* Hardware option */
    gtk_grid_attach((GtkGrid*) grid, lHardware, 1, y, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, hw_cpu, 2, y, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, hw_gpu, 2, y+1, 1, 1);


    gtk_widget_set_halign((GtkWidget*) grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget*) grid, GTK_ALIGN_CENTER);

    GtkWidget * btnSave = gtk_button_new_from_icon_name("document-save");
    GtkWidget * btnNext = gtk_button_new_from_icon_name("go-next");

    gtk_widget_set_tooltip_text(btnNext, "Next page");
    g_signal_connect (btnNext, "clicked", G_CALLBACK (next_page_cb), NULL);

    gtk_widget_set_tooltip_text(btnSave, "Set as defaults.");
    g_signal_connect (btnSave, "clicked", G_CALLBACK (save_dw_settings_cb), NULL);

    GtkWidget * Bar = gtk_action_bar_new();
    gtk_action_bar_pack_start ((GtkActionBar*) Bar, btnSave);
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, btnNext);
    GtkWidget * A = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    gtk_box_append(GTK_BOX(A), Bar);
    gtk_box_append(GTK_BOX(A), grid);

    g_free(dwconf);
    dwconf=NULL;

    GtkWidget * scroller = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW(scroller), A);
    gtk_widget_set_vexpand(scroller, true);

    return scroller;
}

GtkWidget * create_file_frame()
{
    GtkTreeStore * file_store = gtk_tree_store_new (fN_COLUMNS,       /* Total number of columns */
                                                    G_TYPE_STRING,   /* File name */
                                                    G_TYPE_STRING);   /* Channel */

    GtkWidget * file_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (file_store));



    config.file_tree = file_tree;

    GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();

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



    GtkWidget * btnNew = gtk_button_new_from_icon_name("list-add");

    gtk_widget_set_tooltip_text(btnNew, "Add file(s)");

    GtkWidget * btnDel = gtk_button_new_from_icon_name("list-remove");
    gtk_widget_set_tooltip_text(btnDel, "Remove selected file");

    GtkWidget * btnClear = gtk_button_new_from_icon_name("edit-delete");
    gtk_widget_set_tooltip_text(btnClear, "Clear the list of files");
    g_signal_connect(btnClear, "clicked", G_CALLBACK (clear_files_cb), NULL);
    g_signal_connect(btnNew, "clicked", G_CALLBACK (add_files_cb), NULL);
    g_signal_connect(btnDel, "clicked", G_CALLBACK (del_file_cb), NULL);


    GtkWidget * btnNext = gtk_button_new_from_icon_name("go-next");
    gtk_widget_set_tooltip_text(btnNext, "Next page");
    g_signal_connect (btnNext, "clicked", G_CALLBACK (next_page_cb), NULL);


    GtkWidget * Bar = gtk_action_bar_new();
    gtk_action_bar_pack_start((GtkActionBar*) Bar, btnNew);
    gtk_action_bar_pack_start((GtkActionBar*) Bar, btnDel);
    gtk_action_bar_pack_start((GtkActionBar*) Bar, btnClear);
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, btnNext);

    GtkWidget * file_tree_scroller = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_child(
                                  GTK_SCROLLED_WINDOW(file_tree_scroller),
                                  file_tree);
    gtk_widget_set_vexpand(file_tree_scroller, true);
    GtkWidget * boxV = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    gtk_box_append(GTK_BOX(boxV), Bar);
    gtk_box_append(GTK_BOX(boxV), file_tree_scroller);


    GtkWidget * file_frame = gtk_frame_new(NULL);
    gtk_frame_set_child (GTK_FRAME(file_frame), boxV);


    GtkEventController * ctrl = gtk_event_controller_key_new();
    g_signal_connect_object (ctrl, "key-pressed",
                             G_CALLBACK (file_tree_keypress),
                             file_tree, G_CONNECT_SWAPPED);
    gtk_widget_add_controller(GTK_WIDGET(file_tree), GTK_EVENT_CONTROLLER(ctrl));



    /* Set up Drag and Drop */

    // 1. Set up a GtkDropTarget
    // 2. Handle GtkDropTarget::drop
    GtkDropTarget *target =
        gtk_drop_target_new (G_TYPE_INVALID, GDK_ACTION_COPY);

    // Note: Does only work if when specified in this order, else a
    // list of files is detected as a single file.
    gtk_drop_target_set_gtypes (target, (GType [2]) {
            GDK_TYPE_FILE_LIST, G_TYPE_FILE,
        }, 2);

    g_signal_connect (target, "drop", G_CALLBACK (on_drop), file_frame);
    gtk_widget_add_controller (GTK_WIDGET (file_frame),
                               GTK_EVENT_CONTROLLER (target));

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

char * get_channel_name_regexp(const char *fname0)
{
    char * fname = g_strdup(fname0);
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
            ret = g_strdup(word);
        } else {
            printf("Warning: duplicate channel match in %s\n", fname0);
        }
        g_free (word);
        word = NULL;
        g_match_info_next (match_info, NULL);
    }
    g_match_info_free (match_info);
    g_regex_unref (regex);
    g_free(fname);

    if(ret == NULL)
    {
        char * ret = g_malloc0(5);
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

    char * fname = g_strdup(fname0);
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
        char * ualias = g_strdup(channelsp[0]->alias);
        for(size_t kk = 0; kk<strlen(ualias); kk++)
        {
            ualias[kk] = toupper(ualias[kk]);
        }
        if(strstr(fname, ualias) != NULL)
        {

            // printf("Matches %s\n", ualias);
            channel = g_strdup(channelsp[0]->alias);
        } else {
            // printf("%s != %s\n", fname, ualias );
        }
        g_free(ualias);

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


// TODO: Change to add or update
// then we can use this also as the end point for "edit"
gboolean add_channel(char * alias, char * name, float emission, int iter)
{
    GtkTreeStore * channel_store = (GtkTreeStore*) gtk_tree_view_get_model((GtkTreeView*) config.channel_tree);
    GtkTreeIter iter1;  /* Parent iter */

    char * emission_str = g_malloc0(1024);
    sprintf(emission_str, "%.2f", emission);
    gtk_tree_store_append (channel_store, &iter1, NULL);
    gtk_tree_store_set (channel_store, &iter1,
                        cALIAS_COLUMN, alias,
                        cNAME_COLUMN, name,
                        cNITER_COLUMN, iter,
                        cEMISSION_COLUMN, emission_str,
                        -1);
    g_free(emission_str);
    return TRUE;
}

void add_channel_DwChannel(DwChannel * chan)
{
    add_channel(chan->alias, chan->name, chan->lambda, chan->niter);
}



gboolean
new_channel_cb(GtkWidget *widget,
               gpointer user_data)
{
    UNUSED(widget);
    UNUSED(user_data);
    dw_channel_edit_reset();
    dw_channel_edit_show();
    return TRUE;
}


/* returns a box */
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
#if 0
    g_object_set (G_OBJECT (renderer),
                  "foreground", "black",
                  NULL);
#endif

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


    GtkWidget * btnNew = gtk_button_new_from_icon_name("list-add");
    gtk_widget_set_tooltip_text(btnNew, "Add another channel");

    GtkWidget * btnDel = gtk_button_new_from_icon_name("list-remove");
    gtk_widget_set_tooltip_text(btnDel, "Remove selected channel");

    GtkWidget * btnEdit = gtk_button_new_from_icon_name("preferences-other");
    gtk_widget_set_tooltip_text(btnEdit, "Edit selected channel");

    GtkWidget * btnSave = gtk_button_new_from_icon_name("document-save");
    gtk_widget_set_tooltip_text(btnSave, "Save current list as default");

    GtkWidget * btnNext = gtk_button_new_from_icon_name("go-next");
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

    gtk_box_append(GTK_BOX(boxV), Bar);

    // Make the list of channels scrollable
    GtkWidget * channel_tree_scroll = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW(channel_tree_scroll), channel_tree);
    gtk_widget_set_vexpand(channel_tree_scroll, true);
    gtk_box_append(GTK_BOX(boxV), channel_tree_scroll);

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

void add_scope_DwScope(DwScope * scope)
{
    add_scope(scope->name, scope->NA, scope->ni, scope->xy_nm, scope->z_nm);
}


gboolean
new_scope_cb (GtkWidget *widget,
              gpointer   user_data)
{
    UNUSED(widget);
    UNUSED(user_data);

    dw_scope_edit_reset();
    dw_scope_edit_show();

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
    g_free(cfile);
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

    GtkWidget * btnNew = gtk_button_new_from_icon_name("list-add");
    gtk_widget_set_tooltip_text(btnNew, "Add another microscope");
    GtkWidget * btnDel = gtk_button_new_from_icon_name("list-remove");
    gtk_widget_set_tooltip_text(btnDel, "Remove selected microscope");
    GtkWidget * btnEdit = gtk_button_new_from_icon_name("preferences-other");
    gtk_widget_set_tooltip_text(btnEdit, "Edit selected microscope");

    GtkWidget * btnSave = gtk_button_new_from_icon_name("document-save");
    gtk_widget_set_tooltip_text(btnSave, "Save current list as default");
    GtkWidget * btnNext = gtk_button_new_from_icon_name("go-next");
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

    gtk_box_append(GTK_BOX(A), Bar);
    gtk_box_append(GTK_BOX(A), scope_tree);

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
    g_free(fname);
    return;
}

void file_tree_append(const char * fname)
{
    GtkTreeStore * filetm = (GtkTreeStore*) gtk_tree_view_get_model((GtkTreeView*) config.file_tree);
    GtkTreeIter iter1;  /* Parent iter */

    if(!is_tif_file_name(fname))
    {
        printf("Not a tif file!\n");
        goto done;
    }

    char * cname = get_channel_name(fname);
    gtk_tree_store_append (filetm, &iter1, NULL);  /* Acquire a top-level iterator */
    gtk_tree_store_set (filetm, &iter1,
                        fFILE_COLUMN, fname,
                        fCHANNEL_COLUMN, cname,
                        -1);

    g_free(cname);

 done:
    return;
}


// https://developer.gnome.org/documentation/tutorials/drag-and-drop.html
static gboolean
on_drop (GtkDropTarget *target,
         const GValue  *value,
         double         x,
         double         y,
         gpointer       data)
{
    // Call the appropriate setter depending on the type of data
    // that we received
    //print_g_object_name(value);

    if(G_VALUE_HOLDS(value, GDK_TYPE_FILE_LIST))
    {
        GdkFileList *file_list = g_value_get_boxed (value);
        GSList *list = gdk_file_list_get_files (file_list);
        for (GSList *l = list; l != NULL; l = l->next)
        {
            GFile* file = l->data;
            g_print ("+ %s\n", g_file_get_path (file));
            file_tree_append(g_file_get_path(file));
        }
        return true;
    } else if (G_VALUE_HOLDS (value, G_TYPE_FILE))
    {
        GFile * file = g_value_get_object (value);
        char * t = g_file_get_path(file);

        file_tree_append(g_file_get_path(file));
        g_free(t);
        //my_widget_set_file (self, g_value_get_object (value));
        return true;
    } else {
        return false;
    }
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

    char * cmd = g_malloc0(strlen(name) + 100);
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

void save_cmd_to_file(  GObject* source_object,
                        GAsyncResult* res,
                        gpointer data
                        )
{

    GError * error = NULL;
    GFile * file =
        gtk_file_dialog_save_finish (
                                     (GtkFileDialog*) source_object,
                                     res,
                                     &error
                                     );
    if(error != NULL)
    {
        // We could use the domain/code/message ...
        printf("No files selected or an error occured\n");
        g_error_free(error);
        return;
    }

    char * outname = g_file_get_path(file);
    if(outname == NULL)
    {
        printf("Could not get the file name");
        return;
    }
    printf("Want to save to %s\n", outname);

    GtkTextIter start, end;
    GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (config.cmd));
    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);
    char * text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
    gtk_text_buffer_set_modified (buffer, FALSE);

    GError * err = NULL;
    gboolean result = g_file_set_contents (outname, text, -1, &err);
    g_free(text);

    if (result == FALSE)
    {
        /* error saving file, show message to user */
        //error_message (err->message);
        g_error_free(err);
        goto done;
    } else {
        // TODO: Only add S_IXUSR to the defaults
        int chmod_ok = g_chmod(outname, 0755 );
        g_assert(chmod_ok == 0);
    }

 done: ;

    g_free(outname);
    return;

#ifdef GTK3
    // Run it
    //printf("To run: %s\n", filename);
    runscript(filename);
    g_free(filename);
#endif
}



/* Save the command-queue to disk. Returns true on success and sets savename
   to the filename that was used.
*/
gboolean save_dw_cb(GtkWidget * widget, gpointer user_data)
{
    /* TODO: Crashes on windows before shown
       https://discourse.gnome.org/t/gtk4-c-windows-cannot-save-using-gtk-file-dialog-save/13072/21
       The “context” is:

       the file selection dialog has settings, which require
       installing the GTK schemas alongside your application’s own
       settings

       If you don’t install the org.gtk.Settings.* XML schemas
       alongside your application’s settings schemas, and you don’t
       compile all of them with glib-compile-schemas, then GTK won’t
       find the schemas, and assume that the installation is
       broken—which it is—and abort.

       https://manpages.ubuntu.com/manpages/trusty/en/man1/glib-compile-schemas.1.html
    */
    // TODO: default folder based on some image in the list
    GtkFileDialog * dialog = gtk_file_dialog_new ();
    gtk_file_dialog_set_title(dialog, "Save File");
    gtk_file_dialog_set_modal(dialog, true);

    gtk_file_dialog_set_initial_name(dialog, "dw_script.sh");

    GCancellable* canc = g_cancellable_new();
    gtk_file_dialog_save(GTK_FILE_DIALOG (dialog),
                         NULL,
                         canc,
                         save_cmd_to_file,
                         NULL);
}


/* Save and run afterwards
 * TODO: flag that it should be run ...
 */
gboolean run_dw_cb(GtkWidget * widget, gpointer user_data)
{
    save_dw_cb(widget, user_data);
    return true;
}


GtkWidget * create_run_frame()
{

    GtkWidget * cmd = gtk_text_view_new();
    gtk_text_view_set_monospace( (GtkTextView *) cmd , TRUE);

    config.cmd = cmd;

    g_object_set(G_OBJECT(cmd), "editable", FALSE, NULL);

    GtkWidget * cmd_scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(cmd_scroll, true);

    gtk_scrolled_window_set_child( GTK_SCROLLED_WINDOW(cmd_scroll), cmd);

    GtkWidget * fcmd_scroll = gtk_frame_new("Commands");
    gtk_frame_set_child( GTK_FRAME(fcmd_scroll), cmd_scroll);


    GtkWidget * frame = gtk_frame_new(NULL);
    gtk_frame_set_child(GTK_FRAME(frame), fcmd_scroll);

    //GtkWidget * ButtonRun = gtk_button_new_from_icon_name("media-playback-start");
    //gtk_widget_set_tooltip_text(ButtonRun, "Save the script and run it");

    GtkWidget * ButtonSaveAs = gtk_button_new_from_icon_name("document-save-as");
    gtk_widget_set_tooltip_text(ButtonSaveAs, "Save the script to disk");

    GtkWidget * Bar = gtk_action_bar_new();
    //gtk_action_bar_pack_end ((GtkActionBar*) Bar, ButtonRun);
    gtk_action_bar_pack_end ((GtkActionBar*) Bar, ButtonSaveAs);

    //g_signal_connect (ButtonRun, "clicked", G_CALLBACK (run_dw_cb), NULL);
    g_signal_connect (ButtonSaveAs, "clicked", G_CALLBACK (save_dw_cb), NULL);

    GtkWidget * A = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);


    gtk_box_append(GTK_BOX(A), Bar);
    gtk_box_append(GTK_BOX(A), frame);

    gtk_widget_set_visible(cmd, true);
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
        g_free(config.savefolder);
        config.savefolder = NULL;
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
        if(config.outscript == OUTSCRIPT_SH)
        {
            gtk_text_buffer_insert(buffer, &titer, "# Please select a microscope!", -1);
        } else {
            gtk_text_buffer_insert(buffer, &titer, "REM Please select a microscope!", -1);
        }
        gtk_text_view_set_buffer(cmd, buffer);
        return;
    }

    char comment[] = "REM";
    if(config.outscript == OUTSCRIPT_SH)
    {
        sprintf(comment, "#");
    }

    char * buff = g_malloc0(1024*1024);

    sprintf(buff, "%s script generated by deconwolf-gui %s\n\n", comment, DW_GUI_VERSION);
    gtk_text_buffer_insert(buffer, &titer, buff, -1);

    sprintf(buff, "%s Microscope name: '%s'\n", comment, scope->name);
    gtk_text_buffer_insert(buffer, &titer, buff, -1);

    //sprintf(buff, "# %d channels available\n", nchan);
    //gtk_text_buffer_insert(buffer, &titer, buff, -1);
    //sprintf(buff, "# %d files available\n", nfiles);
    //gtk_text_buffer_insert(buffer, &titer, buff, -1);

    int nthreads = dwconf->nthreads;
    int tilesize = dwconf->tilesize;

    char * ostring = g_malloc0(1024);
    ostring[0] = '\0';
    if(dwconf->overwrite)
    {
        sprintf(ostring, " --overwrite ");
    }

    char * fstring = g_malloc0(1024);
    fstring[0] = '\0';
    if(dwconf->outformat == DW_CONF_OUTFORMAT_FLOAT32)
    {
        sprintf(fstring, " --float");
    }

    char * tstring = g_malloc0(1024);
    fstring[0] = '\0';
    if(nthreads > 0)
    {
        sprintf(tstring, "--threads %d", nthreads);
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
        // Default: Not needed
        // sprintf(fstring + strlen(fstring), " --bq 2");
        break;
    }

    if(dwconf->use_gpu == 1)
    {
        sprintf(fstring + strlen(fstring), " --gpu");
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
            gchar * psffile = g_strjoin(NULL, files[kk]->channel, ".tif", NULL);
            gchar * psf = g_build_filename(fdir, "PSFBW", psffile, NULL);
            g_free(psffile);

            /* Shell quoted version of the file names, allows files to contain
               single quotes in their names and avoids "accidental" mixing file
               names with commands :)
            */
            gchar * outdir = g_build_filename(fdir, "PSFBW", NULL);

            gchar * q_outdir = shell_quote(outdir);
            gchar * q_psfname = shell_quote(psf);
            gchar * q_filename = shell_quote(files[kk]->name);


            sprintf(buff, "mkdir %s\n", q_outdir);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);
            sprintf(buff, "dw_bw %s --lambda %f --NA %.3f --ni %.3f"
                    " --resxy %.1f --resz %.1f %s %s\n",
                    ostring,
                    ch->lambda, scope->NA, scope->ni,
                    scope->xy_nm, scope->z_nm, tstring, q_psfname);



            //        printf("%s", buff);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);
            sprintf(buff, "dw %s %s --tilesize %d --iter %d %s %s %s\n",
                    ostring,
                    fstring,
                    tilesize, ch->niter, tstring,
                    q_filename, q_psfname);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);

            g_free(q_filename);
            g_free(q_psfname);
            g_free(q_outdir);
            g_free(outdir);

            g_free(fdir);
            g_free(psf);
        } else {
            sprintf(buff, "%s Missing channel for: %s\n", comment, files[kk]->name);
            gtk_text_buffer_insert(buffer, &titer, buff, -1);
        }
        kk++;
    }

 nofiles:
    gtk_text_view_set_buffer (cmd,
                              buffer);

    g_free(buff);
    g_free(ostring);
    g_free(fstring);
    g_free(tstring);

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
        DwScope * current_scope = g_malloc0(sizeof(DwScope));
        if(sname != NULL)
        {
            current_scope->name = g_strdup(sname);
        }
        current_scope->NA = sNA;
        current_scope->ni = sNI;
        current_scope->xy_nm = sDX;
        current_scope->z_nm = sDZ;
        g_free(sname);

        dw_scope_edit_set(current_scope);
        dw_scope_edit_show();
        dw_scope_free(current_scope);
    }
    return;
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
        DwChannel * curr = g_malloc0(sizeof(DwChannel));
        assert(cname != NULL);
        curr->name = g_strdup(cname);
        assert(calias != NULL);
        curr->alias = g_strdup(calias);
        curr->lambda = atof(clambda);
        curr->niter = cniter;

        /* Free return values from gtk_tree_model_get */
        g_free(calias);
        g_free(cname);
        g_free(clambda);

        dw_channel_edit_set(curr);
        dw_channel_edit_show();
        dw_channel_free(curr);
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
    g_free(cfile);
    return TRUE;
}

/** Callback from Open File dialogue
 * TODO: Check if valid files
 * TODO: Exclude folders
 */
void
got_files_from_dialog (  GObject* source_object,
                         GAsyncResult* res,
                         gpointer data
                         )
{
    UNUSED(data);

    GError * error = NULL;
    GListModel * files =
        gtk_file_dialog_open_multiple_finish (
                                              (GtkFileDialog*) source_object,
                                              res,
                                              &error
                                              );
    if(error != NULL)
    {
        // We could use the domain/code/message ...
        printf("No files selected or an error occured\n");
        g_error_free(error);
        return;
    }

    // Loop over GListModel ...
    // Should be a list of Gfile

    for(int i = 0; ; i++){

        GFile * file = g_list_model_get_item(files, i);
        if(file == NULL)
        {
            break;
        }
        char * t = g_file_get_path(file);
        file_tree_append(t);
        //printf("%s\n", t);
        g_free(t);
    }

    // TODO: Need to free files?
}

gboolean add_files_cb(GtkWidget * w, gpointer p)
{
    UNUSED(p);

    GtkWidget * dialog = (GtkWidget*) gtk_file_dialog_new ();
    gtk_file_dialog_set_title(GTK_FILE_DIALOG(dialog), "Open File");
    gtk_file_dialog_set_modal(GTK_FILE_DIALOG(dialog), true);

    if(config.default_open_uri != NULL)
    {
        GFile * path = g_file_new_for_path(config.default_open_uri);
        gtk_file_dialog_set_initial_folder( (GtkFileDialog *) dialog,
                                            path);
        g_object_unref(path);
    }

    GCancellable* canc = g_cancellable_new();
    gtk_file_dialog_open_multiple(GTK_FILE_DIALOG(dialog), // self
                                  GTK_WINDOW(config.window), // parent
                                  canc, // cancellable
                                  got_files_from_dialog, // callback
                                  NULL); // user_data
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

gboolean
file_tree_keypress (GtkEventController  *key,
                    guint                keyval,
                    guint                keycode,
                    GdkModifierType      state,
                    gpointer user_data)
{
    if (keyval == GDK_KEY_Delete){
        del_selected_file();
    }
    return FALSE;
}

gboolean channel_tree_keypress (GtkEventController  *key,
                                guint                keyval,
                                guint                keycode,
                                GdkModifierType      state,
                                gpointer user_data)
{
    if (keyval == GDK_KEY_Delete){
        del_selected_channel();
    }
    if (keyval == GDK_KEY_Return){
        edit_selected_channel();
    }
    return FALSE;
}

gboolean microscope_tree_keypress (GtkEventController  *key,
                                   guint                keyval,
                                   guint                keycode,
                                   GdkModifierType      state,
                                   gpointer user_data)
{
    if (keyval == GDK_KEY_Delete){
        del_selected_scope();
    }
    if (keyval == GDK_KEY_Return){
        edit_selected_scope();
    }
    return FALSE;
}

// GtkGestureClick::released
void channel_tree_buttonpress(GtkGestureClick* self,
                                  gint n_press,
                                  gdouble x,
                                  gdouble y,
                                  gpointer user_data)
{
    // TODO: Hard to generate a double click
    if(n_press >= 2) {
        edit_selected_channel();
    }

    return;
}


void microscope_tree_buttonpress(GtkGestureClick* self,
                                  gint n_press,
                                  gdouble x,
                                  gdouble y,
                                  gpointer user_data)
{
    if(n_press >= 2) {
        edit_selected_scope();
    }

    return;
}


void populate_channels()
{
    DwChannel ** channels = NULL;
    // Set up channels from configuration file or use defaults
    char * cfile = get_configuration_file("channels");
    if(cfile != NULL)
    {
        channels = dw_channels_from_disk(cfile);
        g_free(cfile);
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
    g_free(cfile);

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
    GtkWidget * frame_drop = gtk_frame_new(NULL);
    GtkWidget * overlay = gtk_overlay_new();

    GtkWidget * image = gtk_image_new_from_resource("/images/wolf1.png");
    gtk_widget_set_size_request(image, 400, 400);

    GtkDropTarget *target =
        gtk_drop_target_new (G_TYPE_INVALID, GDK_ACTION_COPY);

    gtk_drop_target_set_gtypes (target, (GType [2]) {
            GDK_TYPE_FILE_LIST, G_TYPE_FILE,
        }, 2);

    g_signal_connect (target, "drop", G_CALLBACK (on_drop), frame_drop);
    gtk_widget_add_controller (GTK_WIDGET (frame_drop),
                               GTK_EVENT_CONTROLLER (target));


    GtkWidget * label = gtk_label_new("Drag and Drop images here");

    gtk_widget_set_halign((GtkWidget* ) label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget* ) label, GTK_ALIGN_CENTER);

    gchar * markup =
        g_strdup_printf("<span foreground='#ffffff'><b>Drag and Drop images here</b></span>");

    gtk_label_set_markup(GTK_LABEL(label), markup);
    g_free(markup);
    gtk_overlay_set_child (GTK_OVERLAY(overlay), GTK_WIDGET(image));
    gtk_overlay_add_overlay((GtkOverlay*) overlay, label);
    gtk_frame_set_child (GTK_FRAME(frame_drop), GTK_WIDGET(overlay));
    return frame_drop;
}

void
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
    gtk_window_present( GTK_WINDOW(about));
}

typedef struct {
    GtkWindow * window;
    GtkWidget * eRegexp;
    GtkWidget * eOutscript;
} global_conf_t;

static void
cb_config_ok (GtkButton *button,
              gpointer user_data)
{
    global_conf_t * conf = (global_conf_t *) user_data;
    // TODO Validation etc ...
    sprintf(config.regexp_channel, "%s",
            gtk_editable_get_text( GTK_EDITABLE(conf->eRegexp)));
    config.outscript = gtk_drop_down_get_selected(GTK_DROP_DOWN( conf->eOutscript ));

    save_setting_string("general", "general", "channel_regexp", config.regexp_channel);

    gtk_window_destroy(conf->window);
}

static void
cb_config_cancel (GtkButton *button,
                  gpointer user_data)
{
    global_conf_t * conf = (global_conf_t *) user_data;
    gtk_window_destroy(conf->window);
}


void
edit_global_config(void)
{
    GtkWindow * dialog = (GtkWindow *) gtk_window_new();
    gtk_window_set_modal(dialog, true);
    gtk_window_set_title(dialog, "Edit Global Settings");
    gtk_window_set_resizable(dialog, false);
    gtk_window_set_destroy_with_parent(dialog, true);

    GtkWidget * lRegexp = gtk_label_new("Channel regexp");
    GtkWidget * eRegexp = gtk_entry_new();
    gtk_editable_set_text( GTK_EDITABLE(eRegexp), config.regexp_channel);

    GtkWidget * grid = gtk_grid_new();
    gtk_grid_set_row_spacing ((GtkGrid*) grid , 5);
    gtk_grid_set_column_spacing ((GtkGrid*) grid , 5);

#if 0
    GtkWidget * lRegexp_extra =
        gtk_label_new(
                      "Set the regular expression used to identify channel \n"
                      "names from the file names. For example, if the \n"
                      "channel name is at the end, \n"
                      "try '[A-Z0-9]*\\_([A-Z0-9]*)\\.TIFF?'\n");

    gtk_label_set_selectable((GtkLabel*) lRegexp_extra, TRUE);

    gtk_grid_attach((GtkGrid*) grid, lRegexp_extra, 1, 1, 3, 2);
#endif
    gtk_grid_attach((GtkGrid*) grid, lRegexp, 1, 3, 1, 1);
    gtk_grid_attach((GtkGrid*) grid, eRegexp, 2, 3, 2, 1);

    GtkWidget * lOutscript = gtk_label_new("Script format");
    const char * options[] = {"sh (linux)", "bat (windows)", NULL};
    GtkWidget * eOutscript = gtk_drop_down_new_from_strings(options);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(eOutscript), config.outscript);

    gtk_grid_attach( GTK_GRID(grid), lOutscript, 1, 4, 1, 1);
    gtk_grid_attach( GTK_GRID(grid), eOutscript, 2, 4, 1, 1);

    gtk_widget_set_halign((GtkWidget*) grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((GtkWidget*) grid, GTK_ALIGN_CENTER);


    GtkBox * vbox0 = (GtkBox *) gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_box_append(vbox0, grid);

    global_conf_t * data = g_malloc0(sizeof(global_conf_t));
    data->window = dialog;
    data->eRegexp = eRegexp;
    data->eOutscript = eOutscript;

    // Ok and Cancel buttons in box_btn
    GtkButton * btn_ok = (GtkButton *) gtk_button_new_with_label("Ok");
    g_signal_connect(btn_ok, "clicked", G_CALLBACK(cb_config_ok), data);
    GtkButton * btn_cancel = (GtkButton *) gtk_button_new_with_label("Cancel");
    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(cb_config_cancel), data);
    GtkBox * box_btn = (GtkBox *) gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_box_append(box_btn, GTK_WIDGET(btn_cancel));
    gtk_box_append(box_btn, GTK_WIDGET(btn_ok));
    gtk_box_append(vbox0, GTK_WIDGET(box_btn));

    gtk_window_set_child (GTK_WINDOW(dialog),  GTK_WIDGET(vbox0));

    gtk_widget_set_visible(GTK_WIDGET(dialog), true);
    return;
}


void
configuration_activated(GSimpleAction *simple,
                        GVariant      *parameter,
                        gpointer       p)
{
    UNUSED(simple);
    UNUSED(parameter);
    UNUSED(p);
    edit_global_config();
    return;
}

void warn_no_dw(GtkWindow * parent)
{
    GtkAlertDialog * dialog =
        gtk_alert_dialog_new(
                             "Could not locate deconwolf (i.e, the command 'dw'). "
                             "You will not be able to run anything from this GUI!"
                             );
    gtk_alert_dialog_show(dialog, parent);
}


DwAppWindow *
dw_app_window_new (DwApp *app)
{
    setlocale(LC_ALL,"C");

    config.app = G_APPLICATION(app);
    config.default_open_uri = NULL;
    config.savefolder = NULL;
    config.has_dw = has_dw();

#ifdef WINDOWS
    config.outscript = OUTSCRIPT_BAT;
#endif

    config.regexp_channel =
        load_setting_string("general", "general",
                            "channel_regexp", "([A-Z0-9]*)\\_[0-9]*\\.TIFF?");

    // Set up a fallback icon
    GError * error = NULL;
    GdkPixbuf * im = gdk_pixbuf_new_from_resource("/images/wolf1.png", &error);
    int width = gdk_pixbuf_get_width(im);
    int height = gdk_pixbuf_get_height(im);
    int new_height = round(100.0 / ( (double) width) * (double) height );

    //printf("Creating frames\n");
    GtkWidget * frame_drop = create_drop_frame ();
    GtkWidget * frame_dw = create_deconwolf_frame();
    GtkWidget * frame_files = create_file_frame();
    GtkWidget * frame_channels = gtk_frame_new (NULL);
    GtkWidget * frame_scope = gtk_frame_new (NULL);
    GtkWidget * frame_run = create_run_frame();

    //printf("Notebook\n");
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

    GtkWidget * channel_tree = create_channel_tree(); /* The box containing it */

    GtkEventController * key = gtk_event_controller_key_new();
    g_signal_connect_object (key, "key-pressed",
                             G_CALLBACK (channel_tree_keypress),
                             config.channel_tree, G_CONNECT_SWAPPED);
    gtk_widget_add_controller(GTK_WIDGET(config.channel_tree), GTK_EVENT_CONTROLLER(key));

    GtkGesture * gesture = gtk_gesture_click_new();
    g_signal_connect_object (gesture, "released",
                             G_CALLBACK (channel_tree_buttonpress),
                             config.channel_tree, G_CONNECT_SWAPPED);
    gtk_widget_add_controller(GTK_WIDGET(config.channel_tree), GTK_EVENT_CONTROLLER(gesture));


    GtkWidget * scope_tab = create_microscope_tab();


    GtkEventController * key2 = gtk_event_controller_key_new();
    g_signal_connect_object (key2, "key-pressed",
                             G_CALLBACK (microscope_tree_keypress),
                             config.scope_tree, G_CONNECT_SWAPPED);
    gtk_widget_add_controller(GTK_WIDGET(config.scope_tree), GTK_EVENT_CONTROLLER(key2));


    GtkGesture * gesture2 = gtk_gesture_click_new();
    g_signal_connect_object (gesture2, "released",
                             G_CALLBACK (microscope_tree_buttonpress),
                             gesture2, G_CONNECT_SWAPPED);
    gtk_widget_add_controller(GTK_WIDGET(config.scope_tree), GTK_EVENT_CONTROLLER(gesture2));


    /* Create the window */
    DwAppWindow * window = g_object_new (DW_APP_WINDOW_TYPE, "application", app, NULL);
    config.window = window;

    //printf("Packing into main window\n");
    /* Pack components */
    gtk_frame_set_child((GtkFrame*) frame_channels, channel_tree);
    gtk_frame_set_child((GtkFrame*) frame_scope, scope_tab);

    gtk_window_set_child (GTK_WINDOW (window), notebook);
    gtk_window_set_title( GTK_WINDOW(window), titlestr);
    //gtk_grid_attach (window, notebook, 0, 0, 1, 1);

    /* Parse saved presets */
    populate_channels();
    populate_microscopes();

    // Replace the stock menu bar with a new one
    // that has a menu
    GMenu * menu = g_menu_new();
    g_menu_insert(menu, 1, "About", "app.about");
    g_menu_insert(menu, 2, "Config", "app.configuration");

    GtkWidget * mbtn = gtk_menu_button_new();
    gtk_menu_button_set_menu_model((GtkMenuButton*) mbtn, (GMenuModel*) menu);
    GtkWidget * hbar = gtk_header_bar_new();

    gtk_header_bar_pack_end((GtkHeaderBar*) hbar, mbtn);
    gtk_window_set_titlebar((GtkWindow*) window, hbar);

    // See https://stackoverflow.com/questions/22582768/connecting-a-function-to-a-gtkaction


    static GActionEntry main_menu_actions[] =
        {
            { "about", about_activated, NULL, NULL, NULL, {0,0,0} },
            { "configuration", configuration_activated, NULL, NULL, NULL, {0,0,0} }
        };


    g_action_map_add_action_entries (G_ACTION_MAP (config.app),
                                     main_menu_actions,
                                     G_N_ELEMENTS (main_menu_actions),
                                     NULL);


    if(!config.has_dw) {
        warn_no_dw((GtkWindow*) window);
    }

    dw_channel_edit_init();
    dw_channel_edit_set_callback(add_channel_DwChannel);

    dw_scope_edit_init();
    dw_scope_edit_set_callback(add_scope_DwScope);

    return window;
}

void
dw_app_window_open (DwAppWindow *win,
                    GFile            *file)
{
    UNUSED(win);
    UNUSED(file);
    return;
}
