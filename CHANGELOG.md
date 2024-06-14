# CHANGELOG

## v0.0.7

UserData1 -> DwChannelState
ud -> state

Upgrade to Gtk4
### Before switch:
- [x] disable drag and drop
- GtkRadioButton -> Grouped checkbuttons gtk_check_button_set_group
- gtk_button_new_from_icon_name -- no size selection
- gtk_scrolled_window_new(NULL, NULL) -> gtk_scrolled_window_new
- GTK_ICON_SIZE_SMALL_TOOLBAR does not exist
- GtkFileChooser -> GtkFileDialogue
- gtk_container -> gtk_grid
- gtk_container_add -> gtk_grid_attach
- gtk_entry_get_text -> gtk_entry_buffer_get_text

``` shell
    dialog = gtk_file_dialog_new ("Save File",
                                          parent_window,
                                          action,
                                          "_Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "_Save",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);

                                          ->
    dialog = gtk_file_dialog_new ();
    gtk_file_dialog_set_title(dialog, "Save File");
    gtk_file_dialog_set_modal(dialog, true);
```

## After switch:
- [ ]

## v0.0.6, 2023-11-28
 - Cleaned up to not have any compilation warnings.
 - Compiles without gnu extensions and with clang.
 - Simplified makefile.
 - Fixed a few minor memory leaks.

## v0.0.5
 - Added the option to select GPU for processing (adding the flag `--method shbcl2`).

## v0.0.4
  - Updated the deb creation script (deb_make.sh).

## v0.0.3
   - File names can now contain single quotes, using **g_shell_quote**
   on file names when writing out the bash script.
