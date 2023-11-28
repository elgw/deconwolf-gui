# CHANGELOG

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
