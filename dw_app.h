#ifndef __DWAPP_H
#define __DWAPP_H

#include <gtk/gtk.h>


#define DW_APP_TYPE (dw_app_get_type ())
G_DECLARE_FINAL_TYPE (DwApp, dw_app, DW, APP, GtkApplication)


    DwApp     *dw_app_new         (void);


#endif /* __EXAMPLEAPP_H */
