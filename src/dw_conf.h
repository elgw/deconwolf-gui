#ifndef __dw_conf_h
#define __dw_conf_h

#include <gtk/gtk.h>

#define DW_CONF_OUTFORMAT_UINT16 0
#define DW_CONF_OUTFORMAT_FLOAT32 1

#define DW_CONF_BORDER_QUALITY_BEST 2
#define DW_CONF_BORDER_QUALITY_GOOD 1
#define DW_CONF_BORDER_QUALITY_BAD 0

typedef struct{
    gboolean overwrite;
    gint nthreads;
    gint tilesize;
    gint outformat; // DW_CONF_OUTFORMAT_ ...
    gint border_quality; // DW_CONF_BORDER_QUALITY_ ...
    gint use_gpu;

} DwConf;

DwConf * dw_conf_new();
void dw_conf_free(DwConf*);
DwConf * dw_conf_new_from_file(char * );
void dw_conf_save_to_file(DwConf *, char *);

#endif
