gboolean saveandrun(GtkWindow * parent_window, char ** savename)
{

    gboolean saved = FALSE;

    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new ("Save File",
                                      parent_window,
                                      action,
                                      "_Cancel",
                                      GTK_RESPONSE_CANCEL,
                                      "_Save",
                                      GTK_RESPONSE_ACCEPT,
                                      NULL);
chooser = GTK_FILE_CHOOSER (dialog);

gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);


char * suggname = malloc(1024);
sprintf(suggname, "dwcommands.sh");

gtk_file_chooser_set_current_name (chooser,
                                     suggname);


res = gtk_dialog_run (GTK_DIALOG (dialog));
if (res == GTK_RESPONSE_ACCEPT)
  {


    char * filename = gtk_file_chooser_get_filename (chooser);
    char * sname = strdup(filename);
    savename[0] = sname;

    // save_to_file (filename);
    g_free (filename);
    saved = TRUE;
  }

gtk_widget_destroy (dialog);
return saved;
}
