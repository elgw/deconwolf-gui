#include <gtk/gtk.h>
#include "dw_app.h"

int main (int argc, char **argv)
{
    // https://discourse.gnome.org/t/having-trouble-getting-my-schema-to-work-in-gtk4-tutorial-example/8541/5
    g_setenv("GSETTINGS_SCHEMA_DIR", ".", FALSE);

    int status = g_application_run (G_APPLICATION (dw_app_new()), argc, argv);
    return status;
}
