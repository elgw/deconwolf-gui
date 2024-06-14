#pragma once

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>


void gtk_entry_set_text(GtkEntry *E, const char * text);

const char * gtk_entry_get_text(GtkEntry *E);

/* Print the class type of an object */
void print_g_object_name(gpointer test);

void view_object(gpointer test_candidate);
