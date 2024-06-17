#pragma once

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

char* shell_quote(const char* name);


/* Print the class type of an object */
void print_g_object_name(gpointer test);

void view_object(gpointer test_candidate);
