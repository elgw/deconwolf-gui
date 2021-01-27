#include <gtk/gtk.h>
#include "dw_app.h"

int
main (int    argc,
      char **argv)
{
    GtkApplication *app;
    int status;

    status = g_application_run (G_APPLICATION (dw_app_new()), argc, argv);

    return status;
}
