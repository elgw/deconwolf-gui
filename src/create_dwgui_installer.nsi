Name "deconwolf-gui"

;Icon "todo.ico"

OutFile "deconwolf-gui_0_0_7_Setup.exe"

; The default installation directory
InstallDir $PROGRAMFILES\deconwolf-gui

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\deconwolf-gui" "Install_Dir"

RequestExecutionLevel admin

!include LogicLib.nsh

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "deconwolf"
 
  SectionIn RO
 
   ; Add the istall dir to the path
  EnVar::SetHKCU
EnVar::Check "Path" "$InstDir"
Pop $0
${If} $0 = 0
  DetailPrint "Already there"
${Else}
  EnVar::AddValue "Path" "$InstDir"
  Pop $0 ; 0 on success
${EndIf}
 
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there (you can add more File lines too)
  File /r "*"

  ; NOTE: Same list for uninstaller
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\deconwolf-gui "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\deconwolf-gui" "DisplayName" "deconwolf"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\deconwolf-gui" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\deconwolf-gui" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\deconwolf-gui" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts (required)"
  SectionIn RO

  CreateDirectory "$SMPROGRAMS\deconwolf-gui"
  CreateShortCut '$SMPROGRAMS\BiCroLab\deconwolf-gui.lnk' '$INSTDIR\bin\dw_gui.exe' "" '$INSTDIR\bin\dw_gui.ico' 0
  CreateShortcut "$SMPROGRAMS\BiCroLab\deconwolf-gui-Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0  
  
SectionEnd

Section "Desktop icon"
  SectionIn RO
  
  CreateShortCut '$desktop\deconwolf-gui.lnk' '$INSTDIR\bin\dw_gui.exe' "" '$INSTDIR\bin\dw_gui.ico' 0
  CreateShortcut "$SMPROGRAMS\BiCroLab\deconwolf-gui-Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0  
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\deconwolf-gui"
  DeleteRegKey HKLM SOFTWARE\deconwolf-gui
  
  RMDir /r "$INSTDIR"

  ; Remove from path
  EnVar::DeleteValue "Path" "$InstDir"
Pop $0

SectionEnd