; Shoes definitions
!define APP_NAME "#{APPNAME}"
!define NSIS_NAME "#{NSIS_NAME}"
!define APP_STARTMENU_NAME "#{STARTMENU_NAME}"
!define APP_VERSION "#{WINVERSION}"
!define APP_PUBLISHER "#{PUBLISHER}"
!define APP_WEB_SITE "#{WEBSITE}"
!define APP_INST_KEY "Software\#{HKEY_ORG}\${APP_NAME}"
!define APP_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
!define APP_UNINST_ROOT_KEY "HKLM"

SetCompressor /SOLID lzma

!include "MUI.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "FileAssociation.nsh"
!include "EnvVarUpdate.nsh"

Var StartMenuFolder
Var UninstallerHasFinished

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON setup.ico
!define MUI_UNICON setup.ico
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEFINISHPAGE_BITMAP installer-1.bmp
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP installer-2.bmp
!define MUI_COMPONENTSPAGE_NODESC

; MUI Pages
!insertmacro MUI_PAGE_WELCOME
!define MUI_PAGE_HEADER_TEXT "This program use some MIT Licensed software"
!define MUI_PAGE_HEADER_SUBTEXT "You are free to do as you please with that part of the code."
!define MUI_LICENSEPAGE_TEXT_TOP "When you are ready to continue with Setup, click I Agree."
!define MUI_LICENSEPAGE_TEXT_BOTTOM " "
!insertmacro MUI_PAGE_LICENSE "..\COPYING.txt"

; MUI Start Menu Folder Page Configuration
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${APP_STARTMENU_NAME}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${APP_INST_KEY}" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_FINISHPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_NAME}.exe"
!insertmacro MUI_PAGE_FINISH

; MUI uninstaller pages
!insertmacro MUI_UNPAGE_DIRECTORY
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

; NSIS Output information
Name "${NSIS_NAME}"
OutFile "${APP_NAME}-${APP_VERSION} Setup.exe"
InstallDir "$PROGRAMFILES\${APP_NAME}"
InstallDirRegKey HKLM "${APP_INST_KEY}" ""
RequestExecutionLevel admin

ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
   SetOutPath "$INSTDIR"
   SetOverwrite ifnewer

   !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
      CreateDirectory "$SMPROGRAMS\${APP_STARTMENU_NAME}"
      CreateShortCut "$SMPROGRAMS\${APP_STARTMENU_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_NAME}.exe"
      ;CreateShortCut "$SMPROGRAMS\${APP_NAME}\Manual.lnk" "$INSTDIR\${APP_NAME}.exe" "--manual"
      ;CreateShortCut "$SMPROGRAMS\${APP_NAME}\Packager.lnk" "$INSTDIR\${APP_NAME}.exe" "--package"
   !insertmacro MUI_STARTMENU_WRITE_END
   
   CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_NAME}.exe"
   
   File /r /x nsis ..\*.*
   
   ${EnvVarUpdate} $0 "PATH" "A" HKLM $INSTDIR
   ${registerExtension} "$INSTDIR\${APP_NAME}.exe" ".shy" "Shoes Application"
SectionEnd

Section -AdditionalIcons
   CreateShortCut "$SMPROGRAMS\${APP_STARTMENU_NAME}\Uninstall ${APP_NAME}.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section -Post
   WriteUninstaller "$INSTDIR\Uninstall.exe"
   WriteRegStr HKLM "${APP_INST_KEY}" "" "$INSTDIR\${APP_NAME}.exe"
   ;WriteRegStr HKLM "${APP_INST_KEY}" "base" "$INSTDIR"
   WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "DisplayName" "$(^Name)"
   WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
   WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${APP_NAME}.exe"
   WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "DisplayVersion" "${APP_VERSION}"
   WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "URLInfoAbout" "${APP_WEB_SITE}"
   WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "Publisher" "${APP_PUBLISHER}"
  
   ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
   IntFmt $0 "0x%08X" $0
   WriteRegDWORD HKLM "${APP_UNINST_KEY}" "EstimatedSize" "$0"
SectionEnd

Section Uninstall
   !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

   Delete "$DESKTOP\${APP_NAME}.lnk"
   Delete "$SMPROGRAMS\${APP_STARTMENU_NAME}\Manual.lnk"
   Delete "$SMPROGRAMS\${APP_STARTMENU_NAME}\Packager.lnk"
   Delete "$SMPROGRAMS\${APP_STARTMENU_NAME}\${APP_NAME}.lnk"
   Delete "$SMPROGRAMS\${APP_STARTMENU_NAME}\Uninstall ${APP_NAME}.lnk"

   RMDir "$SMPROGRAMS\${APP_STARTMENU_NAME}"
   RMDir /r "$INSTDIR\*.*"
   RMDir "$INSTDIR"
   
   ${unregisterExtension} ".shy" "Shoes Application"
   ${un.EnvVarUpdate} $0 "PATH" "R" HKLM $INSTDIR

   DeleteRegKey ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}"
   DeleteRegKey HKLM "${APP_INST_KEY}"
   SetAutoClose true
   StrCpy $UninstallerHasFinished true
SectionEnd

Function un.onInit
   MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Uninstall $(^Name)?" IDYES +2
FunctionEnd

Function un.onUninstSuccess
   ${If} $UninstallerHasFinished == true
      HideWindow
      MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) has been uninstalled."
   ${EndIf}
FunctionEnd
