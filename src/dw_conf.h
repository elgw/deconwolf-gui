#ifndef __dw_conf_h
#define __dw_conf_h

#include <gtk/gtk.h>

typedef struct{
    gboolean overwrite;
    gint nthreads;
    gint tilesize;
} DwConf;

DwConf * dw_conf_new();
DwConf * dw_conf_new_from_file(char * );
void dw_conf_save_to_file(DwConf *, char *);

#endif
