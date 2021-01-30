#ifndef __DWAPPWIN_H
#define __DWAPPWIN_H

#include <gtk/gtk.h>
#include <ctype.h>
#include "dw_app.h"


//#include "resources.c"

#define DW_APP_WINDOW_TYPE (dw_app_window_get_type ())
G_DECLARE_FINAL_TYPE (DwAppWindow, dw_app_window, DW, APP_WINDOW, GtkApplicationWindow)


    DwAppWindow       *dw_app_window_new          (DwApp *app);
void                    dw_app_window_open         (DwAppWindow *win,
                                                         GFile            *file);

// Callback from buttons
gboolean del_channel_cb(GtkWidget * w, gpointer p);
gboolean del_scope_cb(GtkWidget * w, gpointer p);
gboolean edit_scope_cb(GtkWidget * w, gpointer p);


// Remove selected items from the treeviews:
void del_selected_channel();
void del_selected_scope();
void del_selected_file();

gboolean file_tree_keypress (GtkWidget *tree_view, GdkEventKey *event, gpointer data);

// Channel
typedef struct {
    char * name;
    char * alias;
    float lambda;
    int niter;
} DwChannel;

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
    gboolean overwrite;
} dwconf;


#endif
