#ifndef __dw_chan_h
#define __dw_chan_h

#include <gtk/gtk.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "dw_colors.h"

// Channel
typedef struct {
    char * name;
    char * alias;
    float lambda;
    int niter;
} DwChannel;

#define cALIAS_COLUMN 0
#define cNAME_COLUMN 1
#define cEMISSION_COLUMN 2
#define cNITER_COLUMN 3
#define cN_COLUMNS 4

DwChannel * dw_channel_new();
void dw_channel_free(DwChannel*);
void dw_channels_free(DwChannel **);

/* Write down NULL-terminated array of channels to a ini file */
void dw_channels_to_disk(DwChannel ** , char *);
/* Look trough an array of channels and return the first where alias matches  */
DwChannel * dw_channels_get_by_alias(DwChannel ** , char * alias);
/* Get all channels from gui as a null-terminated array */
DwChannel ** dw_channels_get_from_gtk_tree_view(GtkTreeView *);

/* Read channels from file */
DwChannel ** dw_channels_from_disk(char * fname);
void dw_chan_to_key_file(DwChannel * , GKeyFile *);

/* Open a dialog box that blocks parent and get/edit a dw_channel */
DwChannel *
dw_channel_edit_dlg(GtkWindow *parent, DwChannel * old_channel);

#endif
