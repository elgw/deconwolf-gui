#include "common.h"
void gtk_entry_set_text(GtkEntry *E, const char * text)
{
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(E), text, -1);
}

const char * gtk_entry_get_text(GtkEntry *E)
{
    return gtk_entry_buffer_get_text(gtk_entry_get_buffer(E));
}

void print_g_object_name(gpointer test)
{
    printf("G_VALUE_TYPE_NAME: %s\n", G_VALUE_TYPE_NAME(test));
}
