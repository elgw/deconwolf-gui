#pragma once

#include <gtk/gtk.h>
#include "common.h"

#define DW_APP_TYPE (dw_app_get_type ())
G_DECLARE_FINAL_TYPE (DwApp, dw_app, DW, APP, GtkApplication)
#define UNUSED(x) (void)(x)

DwApp * dw_app_new(void);
