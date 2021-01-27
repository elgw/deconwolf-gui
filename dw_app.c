#include <gtk/gtk.h>

#include "dw_app.h"
#include "dw_app_window.h"

struct _DwApp
{
  GtkApplication parent;
};

G_DEFINE_TYPE(DwApp, dw_app, GTK_TYPE_APPLICATION);

static void
dw_app_init (DwApp *app)
{
}

static void
dw_app_activate (GApplication *app)
{
  DwAppWindow *win;

  win = dw_app_window_new (DW_APP (app));
  gtk_window_present (GTK_WINDOW (win));
  // Add this to show the image
  gtk_widget_show_all(GTK_WIDGET (win));
}

static void
dw_app_open (GApplication  *app,
                  GFile        **files,
                  gint           n_files,
                  const gchar   *hint)
{
  GList *windows;
  DwAppWindow *win;
  int i;

  windows = gtk_application_get_windows (GTK_APPLICATION (app));
  if (windows)
    win = DW_APP_WINDOW (windows->data);
  else
    win = dw_app_window_new (DW_APP (app));

  for (i = 0; i < n_files; i++)
    dw_app_window_open (win, files[i]);

  gtk_window_present (GTK_WINDOW (win));
}

static void
dw_app_class_init (DwAppClass *class)
{
  G_APPLICATION_CLASS (class)->activate = dw_app_activate;
  G_APPLICATION_CLASS (class)->open = dw_app_open;
}

DwApp *
dw_app_new (void)
{
  return g_object_new (DW_APP_TYPE,
                       "application-id", "org.gtk.exampleapp",
                       "flags", G_APPLICATION_HANDLES_OPEN,
                       NULL);
}
