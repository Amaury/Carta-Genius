; Carta-Genius Installer script for Windows
; 2005 Jul 01 - v1.2.1
;
; How to use this script:
; 1. Download and install NSIS from http://nsis.sourceforge.net
; 2. Compile and configure Carta-Genius under Cygwin environment
;    (see documentation).
; 3. Copy the cygwin1.dll file in the 'lib/' directory of the
;    Carta-Genius source tree.
; 4. Copy XULRunner in the 'gui/xulrunner-windows/' directory of the
;    Carta-Genius source tree, and type 'make windows' in the 'gui/'
;    directory of the Carta-Genius source tree.
; 5. Compile this installer script using NSIS (directly, or using
;    HM NSIS Edit, an IDE for NSIS)

; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "Carta-Genius"
!define PRODUCT_VERSION "5.2.0"
!define PRODUCT_PUBLISHER "Pandocr�on"
!define PRODUCT_WEB_SITE "http://www.pandocreon.fr"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\carta-genius.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_FULL_PATH "${PRODUCT_PUBLISHER}\${PRODUCT_NAME} ${PRODUCT_VERSION}"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "COPYING.txt"
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
;!define MUI_FINISHPAGE_RUN "$SMPROGRAMS\${PRODUCT_FULL_PATH}\Carta-GUI.lnk"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "French"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "pandocreon_carta-genius-${PRODUCT_VERSION}-windows-install.exe"
;InstallDir "$PROGRAMFILES\Pandocr�on\Carta-Genius 4.0.0"
InstallDir "$PROGRAMFILES\${PRODUCT_FULL_PATH}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "Program files" SEC01
  ; Copy Carta-Genius program
    SetOutPath "$INSTDIR\bin"
    SetOverwrite ifnewer
    File "..\..\bin\carta-genius.exe"
  ; Copy XulRunner
    SetOutPath "$INSTDIR\gui"
    File /r "..\..\gui\xulrunner-windows\*.*"
  ; Copy Carta-GUI
    SetOutPath "$INSTDIR\gui\chrome"
    File /r "..\..\gui\carta-gui"
  ; Add shortcuts
    ;CreateDirectory "$SMPROGRAMS\Pandocr�on\Carta-Genius 4.0.0"
    CreateDirectory "$SMPROGRAMS\${PRODUCT_FULL_PATH}"
    SetOutPath "$INSTDIR\gui\chrome"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_FULL_PATH}\Carta-GUI.lnk" "$INSTDIR\gui\xulrunner.exe" "carta-gui\carta-gui.ini"
  ; write configuration file for Carta-GUI
    FileOpen $1 "$WINDIR\carta-gui.cfg" "w"
    FileWrite $1 "$INSTDIR"
    FileClose $1
  ; Copy Cygwin's DLL file
    SetOutPath "$WINDIR"
    ;SetOverwrite off
    File "..\..\lib\cygwin1.dll"
SectionEnd

Section "Documentation" SEC02
  ; Copy PDF documentation
    SetOutPath "$INSTDIR\doc"
    SetOverwrite ifnewer
    File "..\..\doc\francais\1.guide-utilisateur\carta-genius-fr.pdf"
  ; Copy HTML quick guide
    SetOutPath "$INSTDIR\doc\aide-memoire"
    File "..\..\doc\francais\4.aide-memoire\*.*"
  ; Add shortcuts
    CreateShortCut "$SMPROGRAMS\${PRODUCT_FULL_PATH}\Documentation.lnk" "$INSTDIR\doc\carta-genius-fr.pdf"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_FULL_PATH}\Aide-memoire.lnk" "$INSTDIR\doc\aide-memoire\index.html"
SectionEnd

Section "Examples" SEC03
  ; Copy examples
    SetOutPath "$INSTDIR\doc\exemples"
    File "..\..\doc\francais\2.exemples\*.*"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_FULL_PATH}\Exemples.lnk" "$INSTDIR\doc\exemples\"
SectionEnd


Section -AdditionalIcons
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\${PRODUCT_FULL_PATH}\D�sinstallation.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\bin\carta-gui.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\bin\carta-gui.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) a �t� d�sinstall� avec succ�s."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Voulez-vous vraiment effacer $(^Name) et tous ses composants ?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  RMDir /r /REBOOTOK "$SMPROGRAMS\${PRODUCT_FULL_PATH}"
  RMDir "$SMPROGRAMS\${PRODUCT_PUBLISHER}"

  RMDir /r "$INSTDIR"
  RMDir "$PROGRAMFILES\${PRODUCT_PUBLISHER}"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

