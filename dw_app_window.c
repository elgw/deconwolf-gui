#include <gtk/gtk.h>

#include "dw_app.h"
#include "dw_app_window.h"

struct _DwAppWindow
{
    GtkApplicationWindow parent;
};

G_DEFINE_TYPE(DwAppWindow, dw_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void
dw_app_window_init (DwAppWindow *app)
{


}

static void
dw_app_window_class_init (DwAppWindowClass *class)
{
}

DwAppWindow *
dw_app_window_new (DwApp *app)
{
    DwAppWindow * window = g_object_new (DW_APP_WINDOW_TYPE, "application", app, NULL);

    GtkWidget *image;
    /* gdk-pixbuf-csource --name=wolf1_pb wolf1.png > wolf1_pb.c */
    //GdkPixbuf * pixbuf = gdk_pixbuf_new_from_inline (-1, wolf1_pb, FALSE, NULL);
    //image = gtk_image_new_from_pixbuf(pixbuf);
    image = gtk_image_new_from_resource("/images/wolf1.png");
    //image = gtk_image_new_from_file ("wolf1.png");
    gtk_container_add (GTK_CONTAINER (window), image);

    return window;

}

void
dw_app_window_open (DwAppWindow *win,
                         GFile            *file)
{
}
