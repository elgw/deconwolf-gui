#include "common.h"

void print_g_object_name(gpointer test)
{
    printf("G_VALUE_TYPE_NAME: %s\n", G_VALUE_TYPE_NAME(test));
}

void view_object(gpointer test_candidate)
{

    GObject *object = G_OBJECT(test_candidate);
    GType type = G_OBJECT_TYPE (object);
    const char *type_name = g_type_name(type);
    g_print ("\nThe name of the type is: %s\n", type_name);

    GObjectClass *object_class = G_OBJECT_GET_CLASS(object);
    GParamSpec **properties = g_object_class_list_properties(object_class, NULL);
    gint i, n = g_strv_length((gchar **)properties);

    for (i = 0; i < n; i++) {
        g_print("\nProperty %d: %s\n", i, g_param_spec_get_name(properties[i]));

        GValue value = G_VALUE_INIT;
        GValue target = G_VALUE_INIT;
        g_value_init(&target,G_TYPE_STRING);

        g_object_get_property(object,g_param_spec_get_name(properties[i]),&value);

        GType value_type = G_VALUE_TYPE(&value);
        const char *type_n = g_type_name(value_type);
        g_print("Property typ: %s\n", type_n);

        if (g_value_transform(&value, &target))
                printf("Property value: %s\n", g_value_get_string(&target));

        g_value_unset(&target);
        g_value_unset(&value);
    }

    g_free(properties);
}
