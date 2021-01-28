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


#endif
