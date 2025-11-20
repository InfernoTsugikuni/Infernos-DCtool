; NSIS Script for Inferno's DCTool
!define PRODUCT_NAME "Inferno's DCTool"
!define PRODUCT_VERSION "1.0.2"
!define PRODUCT_PUBLISHER "Inferno Tsugikuni"
!define PRODUCT_WEB_SITE "https://infernotsugikuni.work/multitool"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\Infernos_DCTool.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_USER_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

; UI
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"

!define MUI_ICON "InstallIcon.ico"

; Variables
Var IsAdminInstall
Var UserRegRoot
Var UserUninstKey
Var AdminInstallRadio
Var UserInstallRadio

; General
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "InfernosDCTool_Setup.exe"
InstallDir ""  ; 
ShowInstDetails show
ShowUnInstDetails show

; Compression settings for smaller installer
SetCompressor /SOLID lzma
SetCompressorDictSize 32

; Start with user privileges
RequestExecutionLevel user

; Interface Settings
!define MUI_ABORTWARNING

; Custom page for installation type selection
Page custom InstallTypePageCreate InstallTypePageLeave

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "English"

; Initialization function
Function .onInit
  ; Initialize variables
  StrCpy $IsAdminInstall "false"
  StrCpy $INSTDIR "$LOCALAPPDATA\${PRODUCT_NAME}"
  StrCpy $UserRegRoot "HKCU"
  StrCpy $UserUninstKey "${PRODUCT_USER_UNINST_KEY}"
  
  ; Check command line for admin flag
  ${GetParameters} $R0
  ${If} $R0 == "/ADMIN"
    ; We were restarted with admin privileges
    StrCpy $IsAdminInstall "true"
    StrCpy $INSTDIR "$PROGRAMFILES\${PRODUCT_NAME}"
    StrCpy $UserRegRoot "HKLM"
    StrCpy $UserUninstKey "${PRODUCT_UNINST_KEY}"
    
    ; Check if we actually have admin privileges
    UserInfo::GetAccountType
    Pop $0
    ${If} $0 != "Admin"
      MessageBox MB_ICONSTOP "Failed to obtain administrator privileges. Installation cancelled."
      Quit
    ${EndIf}
  ${EndIf}
FunctionEnd

; Installation Type Selection Page
Function InstallTypePageCreate
  ; Skip this page if we're running as admin (restarted)
  ${GetParameters} $R0
  ${If} $R0 == "/ADMIN"
    Abort ; Skip this page
  ${EndIf}
  
  nsDialogs::Create 1018
  Pop $0
  
  ${NSD_CreateLabel} 0 0 100% 20u "Choose installation type:"
  Pop $0
  
  ${NSD_CreateRadioButton} 20u 30u 100% 12u "Install for all users (requires administrator privileges)"
  Pop $AdminInstallRadio
  
  ${NSD_CreateRadioButton} 20u 50u 100% 12u "Install for current user only"
  Pop $UserInstallRadio
  ${NSD_Check} $UserInstallRadio ; Default to user install
  
  ${NSD_CreateLabel} 0 80u 100% 40u "Installing for all users will place the application in Program Files and make it available to all users on this computer. Installing for current user only will place it in your personal AppData folder."
  Pop $0
  
  nsDialogs::Show
FunctionEnd

Function InstallTypePageLeave
  ${NSD_GetState} $AdminInstallRadio $0
  ${If} $0 == ${BST_CHECKED}
    ; Admin install selected - need to restart with elevation
    MessageBox MB_YESNO "Administrator privileges are required for system-wide installation. The installer will now restart with elevated privileges.$\n$\nContinue?" IDYES +2
    Abort
    
    ; Restart with elevated privileges
    GetTempFileName $R0
    ExecShell "runas" "$EXEPATH" "/ADMIN"
    
    Quit
  ${Else}
    ; User install selected
    StrCpy $IsAdminInstall "false"
    StrCpy $INSTDIR "$LOCALAPPDATA\${PRODUCT_NAME}"
    StrCpy $UserRegRoot "HKCU"
    StrCpy $UserUninstKey "${PRODUCT_USER_UNINST_KEY}"
  ${EndIf}
FunctionEnd

; Installer Sections
Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  
  ; Main executable
  File "build\Infernos_DCTool.exe"
  
  ; Qt Core Libraries
  File "build\Qt6Core.dll"
  File "build\Qt6Gui.dll"
  File "build\Qt6Network.dll"
  File "build\Qt6Svg.dll"
  File "build\Qt6Widgets.dll"
  
  ; Runtime Libraries
  File "build\libc++.dll"
  File "build\libunwind.dll"
  File "build\libwinpthread-1.dll"
  
  ; DirectX and OpenGL
  File "build\D3Dcompiler_47.dll"
  File "build\opengl32sw.dll"
  
  ; Qt Plugins and Resources
  SetOutPath "$INSTDIR\assets"
  File /r "build\assets\*.*"
  
  SetOutPath "$INSTDIR\generic"
  File /r "build\generic\*.*"
  
  SetOutPath "$INSTDIR\iconengines"
  File /r "build\iconengines\*.*"
  
  SetOutPath "$INSTDIR\imageformats"
  File /r "build\imageformats\*.*"
  
  SetOutPath "$INSTDIR\networkinformation"
  File /r "build\networkinformation\*.*"
  
  SetOutPath "$INSTDIR\platforms"
  File /r "build\platforms\*.*"
  
  SetOutPath "$INSTDIR\styles"
  File /r "build\styles\*.*"
  
  SetOutPath "$INSTDIR\tls"
  File /r "build\tls\*.*"
  
  SetOutPath "$INSTDIR\translations"
  File /r "build\translations\*.*"
  
  SetOutPath "$INSTDIR"
SectionEnd

Section -AdditionalIcons
  ; Create shortcuts based on installation type
  ${If} $IsAdminInstall == "true"
    ; Admin install - create shortcuts for all users
    WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
    CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\Infernos_DCTool.exe"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\Infernos_DCTool.exe"
  ${Else}
    ; User install - create shortcuts for current user only
    WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
    CreateDirectory "$STARTMENU\Programs\${PRODUCT_NAME}"
    CreateShortCut "$STARTMENU\Programs\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\Infernos_DCTool.exe"
    CreateShortCut "$STARTMENU\Programs\${PRODUCT_NAME}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
    CreateShortCut "$STARTMENU\Programs\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\Infernos_DCTool.exe"
  ${EndIf}
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  
  ; Write registry entries based on installation type
  ${If} $IsAdminInstall == "true"
    ; Admin install - write to HKLM
    WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\Infernos_DCTool.exe"
    WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
    WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
    WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Infernos_DCTool.exe"
    WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
    WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "NoModify" 1
    WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "NoRepair" 1
    ; Store installation type for uninstaller
    WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallType" "Admin"
  ${Else}
    ; User install - write to HKCU
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\App Paths\Infernos_DCTool.exe" "" "$INSTDIR\Infernos_DCTool.exe"
    WriteRegStr HKCU "${PRODUCT_USER_UNINST_KEY}" "DisplayName" "$(^Name)"
    WriteRegStr HKCU "${PRODUCT_USER_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
    WriteRegStr HKCU "${PRODUCT_USER_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Infernos_DCTool.exe"
    WriteRegStr HKCU "${PRODUCT_USER_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr HKCU "${PRODUCT_USER_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
    WriteRegStr HKCU "${PRODUCT_USER_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegStr HKCU "${PRODUCT_USER_UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegDWORD HKCU "${PRODUCT_USER_UNINST_KEY}" "NoModify" 1
    WriteRegDWORD HKCU "${PRODUCT_USER_UNINST_KEY}" "NoRepair" 1
    ; Store installation type for uninstaller
    WriteRegStr HKCU "${PRODUCT_USER_UNINST_KEY}" "InstallType" "User"
  ${EndIf}
SectionEnd

; Finish page function - Ask user if they want to start the application
Function .onInstSuccess
  MessageBox MB_YESNO "Installation completed successfully. Would you like to start Inferno's DCTool now?" IDNO NoLaunch
    Exec "$INSTDIR\Infernos_DCTool.exe"
  NoLaunch:
FunctionEnd

; Uninstaller functions
Function un.onInit
  ; Check if this was an admin or user install by checking registry
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "InstallType"
  ${If} $0 == "Admin"
    ; This was an admin install - check if we have admin privileges
    UserInfo::GetAccountType
    Pop $1
    ${If} $1 != "Admin"
      ; We need admin privileges but don't have them
      MessageBox MB_YESNO "Administrator privileges are required to uninstall this application. Restart uninstaller with elevated privileges?" IDNO +3
      
      ; Re-execute ourselves with the runas verb
      ExecShell "runas" "$INSTDIR\uninst.exe"
      
      Quit
    ${EndIf}
    
    StrCpy $IsAdminInstall "true"
    StrCpy $UserRegRoot "HKLM"
    StrCpy $UserUninstKey "${PRODUCT_UNINST_KEY}"
  ${Else}
    ; Check HKCU if not found in HKLM
    ReadRegStr $0 HKCU "${PRODUCT_USER_UNINST_KEY}" "InstallType"
    ${If} $0 == "User"
      ; This was a user install
      StrCpy $IsAdminInstall "false"
      StrCpy $UserRegRoot "HKCU"
      StrCpy $UserUninstKey "${PRODUCT_USER_UNINST_KEY}"
    ${Else}
      ; Unknown install type - default to user
      StrCpy $IsAdminInstall "false"
      StrCpy $UserRegRoot "HKCU"
      StrCpy $UserUninstKey "${PRODUCT_USER_UNINST_KEY}"
    ${EndIf}
  ${EndIf}
  
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer." 
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\Infernos_DCTool.exe"
  Delete "$INSTDIR\Qt6Core.dll"
  Delete "$INSTDIR\Qt6Gui.dll"
  Delete "$INSTDIR\Qt6Network.dll"
  Delete "$INSTDIR\Qt6Pdf.dll"
  Delete "$INSTDIR\Qt6Svg.dll"
  Delete "$INSTDIR\Qt6Widgets.dll"
  Delete "$INSTDIR\libc++.dll"
  Delete "$INSTDIR\libunwind.dll"
  Delete "$INSTDIR\libwinpthread-1.dll"

  Delete "$INSTDIR\D3Dcompiler_47.dll"
  Delete "$INSTDIR\opengl32sw.dll"
  
  RMDir /r "$INSTDIR\assets"
  RMDir /r "$INSTDIR\generic"
  RMDir /r "$INSTDIR\iconengines"
  RMDir /r "$INSTDIR\imageformats"
  RMDir /r "$INSTDIR\networkinformation"
  RMDir /r "$INSTDIR\platforms"
  RMDir /r "$INSTDIR\styles"
  RMDir /r "$INSTDIR\tls"
  RMDir /r "$INSTDIR\translations"

  ; Remove shortcuts based on installation type
  ${If} $IsAdminInstall == "true"
    ; Admin install shortcuts
    Delete "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk"
    Delete "$SMPROGRAMS\${PRODUCT_NAME}\Website.lnk"
    Delete "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"
    RMDir "$SMPROGRAMS\${PRODUCT_NAME}"
  ${Else}
    ; User install shortcuts
    Delete "$STARTMENU\Programs\${PRODUCT_NAME}\Uninstall.lnk"
    Delete "$STARTMENU\Programs\${PRODUCT_NAME}\Website.lnk"
    Delete "$STARTMENU\Programs\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"
    RMDir "$STARTMENU\Programs\${PRODUCT_NAME}"
  ${EndIf}
  
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  RMDir "$INSTDIR"

  ; Remove registry entries based on installation type
  ${If} $IsAdminInstall == "true"
    DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
    DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  ${Else}
    DeleteRegKey HKCU "${PRODUCT_USER_UNINST_KEY}"
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\App Paths\Infernos_DCTool.exe"
  ${EndIf}
  
  SetAutoClose true
SectionEnd
