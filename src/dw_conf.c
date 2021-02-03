#include "dw_conf.h"

DwConf * dw_conf_new()
{
    DwConf * conf = malloc(sizeof(DwConf));
    conf->overwrite = FALSE;
    conf->nthreads = 4;
    conf->tilesize = 1024;
    return conf;
}


DwConf * dw_conf_new_from_file(char * file)
{
    DwConf * conf = dw_conf_new();

    if(file == NULL)
    {
        return conf;
    }

    GError * error = NULL;
 GKeyFile * key_file = g_key_file_new ();

 if (!g_key_file_load_from_file (key_file, file, G_KEY_FILE_NONE, &error))
 {
     if (!g_error_matches (error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
         g_warning ("Error loading key file: %s", error->message);
     g_error_free(error);
     g_key_file_free(key_file);
     return conf;
 }

 gsize length;

 gchar ** groups =
 g_key_file_get_groups (key_file,
                        &length);
 if(length != 1)
 {
     printf("Can't parse anything from %s\n", file);
     g_key_file_free(key_file);
     g_error_free(error);
     return conf;
 }


 gchar group[] = "deconwolf";

     gint nthreads = g_key_file_get_integer(key_file, group, "nthreads", &error);
     conf->nthreads = nthreads;
     gint tilesize = g_key_file_get_integer(key_file, group, "tilesize", &error);
     conf->tilesize = tilesize;
     gboolean overwrite = g_key_file_get_boolean(key_file, group, "overwrite", &error);
     conf->overwrite = overwrite;

 g_strfreev(groups);
 g_key_file_free(key_file);
 return conf;

}
void dw_conf_save_to_file(DwConf * conf, char * file)
{
    GKeyFile * key_file = g_key_file_new();
    GError * error = NULL;

    g_key_file_set_integer(key_file, "deconwolf", "nthreads", conf->nthreads);
    g_key_file_set_integer(key_file, "deconwolf", "tilesize", conf->tilesize);
    g_key_file_set_boolean(key_file, "deconwolf", "overwrite", conf->overwrite);

    if (!g_key_file_save_to_file (key_file, file, &error))
    {
        g_warning ("Error saving key file: %s", error->message);
        g_error_free(error);
    }
    g_key_file_free(key_file);
}
