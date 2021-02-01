#ifndef __dw_scope_h
#define __dw_scope_h

#include <gtk/gtk.h>


// Microscope
typedef struct {
    char * name;
    float NA;
    float ni;
    float xy_nm;
    float z_nm;
} DwScope;

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

DwScope * dw_scope_new();
void dw_scope_free(DwScope *);
void dw_scopes_free(DwScope ** );
// Get NULL-terminated list of microscopes from the GUI
//DwScope ** dw_scopes_get_from_gui();
DwScope ** dw_scopes_get_from_gtk_tree_view(GtkTreeView * tv);
void dw_scope_to_gtk_tree_store(DwScope *, GtkTreeStore*, GtkTreeIter * iter);

//
DwScope * dw_scope_get_from_model(GtkTreeModel * model, GtkTreeIter * iter);

// Edit button in scope view
gboolean edit_scope_cb(GtkWidget * w, gpointer p);
// Delete selected scope
gboolean del_scope_cb(GtkWidget * w, gpointer p);
gboolean save_scopes_cb(GtkWidget * w, gpointer p);
/* Read channels from file */
DwScope ** dw_scopes_from_disk(char * fname);

void dw_scopes_to_disk(DwScope **, char *);
void dw_scope_to_key_file(DwScope *, GKeyFile *);
DwScope * dwscope_get_selected_from_gtk_tree_view(GtkTreeView * tv);

void dw_scope_to_key_file(DwScope * scope, GKeyFile * kf);

// Function to open a dialog box with a message
DwScope *
dw_scope_edit_dlg(GtkWindow *parent, DwScope * old_scope);


#endif
