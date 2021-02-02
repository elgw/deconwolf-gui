#ifndef __DWAPPWIN_H
#define __DWAPPWIN_H

#include <gtk/gtk.h>
#include <ctype.h>
#include <math.h>
#include "dw_app.h"
#include "dw_channel.h"
#include "dw_scope.h"
#include "dw_conf.h"


#define DW_APP_WINDOW_TYPE (dw_app_window_get_type ())
G_DECLARE_FINAL_TYPE (DwAppWindow, dw_app_window, DW, APP_WINDOW, GtkApplicationWindow)

DwAppWindow *dw_app_window_new (DwApp *app);
void dw_app_window_open (DwAppWindow *win, GFile *file);


// Create the widget for the "Files" tab
GtkWidget * create_file_tree();
GtkWidget * create_deconwolf_tab();

void edit_selected_channel();

gboolean file_tree_keypress (GtkWidget *tree_view, GdkEventKey *event, gpointer data);
void del_selected_file();

// Remove selected items from the treeviews:
void del_selected_channel();
// Callback from buttons in channel tab
gboolean del_channel_cb(GtkWidget * w, gpointer p);
gboolean save_channels_cb(GtkWidget * w, gpointer p);
gboolean edit_channel_cb(GtkWidget * w, gpointer p);

gboolean clear_files_cb(GtkWidget * w, gpointer p);

void edit_selected_scope();
void del_selected_scope();

// deconwolf
typedef struct {
    int nthreads;
    int tilesize;
    gboolean overwrite;
} dwconf;

#endif
