#include <gtk/gtk.h>
#include "dw_app.h"

int main (int argc, char **argv)
{
    int status = g_application_run (G_APPLICATION (dw_app_new()), argc, argv);
    return status;
}
