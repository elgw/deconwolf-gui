#ifndef __dw_file_h
#define __dw_file_h

#include <gtk/gtk.h>
#include <assert.h>

// Columns for files
enum
{
    fFILE_COLUMN,
    fCHANNEL_COLUMN,
    fN_COLUMNS
};

// File
typedef struct {
    char * name;
    char * channel;
} DwFile;

DwFile * dw_file_new();
void dw_file_free(DwFile *);
void dw_files_free(DwFile **);

// Get a NULL-terminated array of files
DwFile ** dw_files_get_from_gtk_tree_view(GtkTreeView * tv);


#endif
