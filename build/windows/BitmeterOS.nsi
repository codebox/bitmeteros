; BitMeterOS.nsi
;
;--------------------------------

!include LogicLib.nsh
!include servicelib.nsh
!include EnvVarUpdate.nsh

; The name of the installer
Name "BitMeter OS"

; The file to write
OutFile "BitMeterOSInstaller.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Codebox\BitMeterOS

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\BitMeterOS" "Install_Dir"

LicenseData licence.txt

RequestExecutionLevel admin
;--------------------------------

; Pages

Page license
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------
!macro CreateInternetShortcut FILENAME URL ICONFILE ICONINDEX
  WriteINIStr "${FILENAME}.url" "InternetShortcut" "URL" "${URL}"
  WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconFile" "${ICONFILE}"
  WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconIndex" "${ICONINDEX}"
!macroend
;--------------------------------

; The stuff to install
Var pathLen
Var instDirLen
Var totalLen
Section "Main"

  SectionIn RO
  
  FindProcDLL::FindProc "bmclient.exe"
  ${If} $R0 == "1"
    MessageBox MB_OK|MB_ICONSTOP "The BitMeter OS client is currently running, please exit the client and try again." /SD IDOK
    Abort
  ${EndIf}

  FindProcDLL::FindProc "bmdb.exe"
  ${If} $R0 == "1"
    MessageBox MB_OK|MB_ICONSTOP "The BitMeter OS Database Admin utility (bmdb.exe) is currently running, please exit the utility and try again." /SD IDOK
    Abort
  ${EndIf}

  !insertmacro SERVICE "running" "BitMeterCaptureService" ""
  Pop $R0
  ${If} $R0 == "true"
  !insertmacro SERVICE "stop" "BitMeterCaptureService" ""
  Sleep 3000  
  ${EndIf}

  !insertmacro SERVICE "running" "BitMeterWebService" ""
  Pop $R0
  ${If} $R0 == "true"
  !insertmacro SERVICE "stop" "BitMeterWebService" ""
  Sleep 3000  
  ${EndIf}
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  SetOverwrite on
  File ".\BitMeterCaptureService.exe"
  File ".\BitMeterCapture.exe"
  File ".\BitMeterWebService.exe"
  File ".\BitMeterWeb.exe"
  File ".\bmclient.exe"
  File ".\bmdb.exe"
  File ".\bmsync.exe"
  
  ; Check the PATH length before adding a new entry (see http://sourceforge.net/tracker/?func=detail&aid=3176678&group_id=200024&atid=971845)
  ReadEnvStr $0 "PATH"
  StrLen $pathLen $0
  StrLen $instDirLen "$INSTDIR"
  IntOp $totalLen $pathLen + $instDirLen
  
  ${If} $pathLen = 0
    MessageBox MB_ICONEXCLAMATION  "The installer is unable to read your System PATH, possibly due to its length - to avoid any risk of corruption this installer will not update its value. If you intend to use the command-line tools you may wish to edit your PATH manually to include the BitMeter installation folder: $INSTDIR"  
  ${ElseIf} $totalLen > 1024
	MessageBox MB_ICONEXCLAMATION  "Your System PATH is quite long, to avoid any risk of corruption this installer will not update its value. If you intend to use the command-line tools you may wish to edit your PATH manually to include the BitMeter installation folder: $INSTDIR"
  ${Else}
    ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR"
  ${EndIf}
  
  SetShellVarContext all
  IfFileExists "$APPDATA\BitMeterOS\bitmeter.db" upgradeDb blankDb
  
  upgradeDb:
    DetailPrint "upgradeDb"
    ExecWait '"$INSTDIR\bmdb.exe" upgrade 7'
    Goto afterDb
  
  blankDb:
    SetOutPath $APPDATA\BitMeterOS
    SetOverwrite off
    File "..\bitmeter.db"
    Goto afterDb
  
  afterDb:
    SetOverwrite on
    SetOutPath $APPDATA\BitMeterOS\web
    File /r "..\..\webserver\web\*.ico"
    File /r "..\..\webserver\web\*.html"
    File /r "..\..\webserver\web\*.xml"
    SetOutPath $APPDATA\BitMeterOS\web\js
    File /r "..\..\webserver\web\js\*.js"
    SetOutPath $APPDATA\BitMeterOS\web\css
    File /r "..\..\webserver\web\css\*.css"
    SetOutPath $APPDATA\BitMeterOS\web\css\images
    File /r "..\..\webserver\web\css\images\*.gif"
    File /r "..\..\webserver\web\css\images\*.png"
    SetOutPath $APPDATA\BitMeterOS\web\m
	File /r "..\..\webserver\web\m\*.xml"
	SetOutPath $APPDATA\BitMeterOS\web\m\js
	File /r "..\..\webserver\web\m\js\*.js"
	SetOutPath $APPDATA\BitMeterOS\web\m\css
	File /r "..\..\webserver\web\m\css\*.css"

  SetOutPath $SYSDIR
  SetOverwrite on
  File ".\sqlite3.dll"

  ; Create a desktop shortcut to the web interface
  !insertmacro CreateInternetShortcut "$DESKTOP\BitMeter OS" "http://localhost:2605" "$APPDATA\BitMeterOS\web\favicon.ico" "0"

  ; Create a Start Menu shortcut to the web interface
  CreateDirectory "$SMPROGRAMS\BitMeter OS"
  !insertmacro CreateInternetShortcut "$SMPROGRAMS\BitMeter OS\BitMeter OS Web Interface" "http://localhost:2605" "$APPDATA\BitMeterOS\web\favicon.ico" "0"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\BitMeterOS "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BitMeterOS" "DisplayName" "BitMeter OS"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BitMeterOS" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BitMeterOS" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BitMeterOS" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
  !insertmacro SERVICE "installed" "BitMeterCaptureService" "action=delete"
  !insertmacro SERVICE "create"    "BitMeterCaptureService" "path=$INSTDIR\BitMeterCaptureService.exe;autostart=1;interact=0;depend=;display=BitMeter Capture Service;description=BitMeter Capture Service;"
  !insertmacro SERVICE "start"     "BitMeterCaptureService" ""

  !insertmacro SERVICE "installed" "BitMeterWebService" "action=delete"
  !insertmacro SERVICE "create"    "BitMeterWebService" "path=$INSTDIR\BitMeterWebService.exe;autostart=1;interact=0;depend=;display=BitMeter Web Service;description=BitMeter Web Interface Service;"
  !insertmacro SERVICE "start"     "BitMeterWebService" ""

  ExecShell "open" "http://codebox.org.uk/bitmeteros/installed?version=0.7.6&platform=windows"
SectionEnd


;--------------------------------

; Uninstaller

Section "Uninstall"
  
  FindProcDLL::FindProc "bmclient.exe"
  ${If} $R0 == "1"
    MessageBox MB_OK|MB_ICONSTOP "The BitMeter OS client is currently running, please exit the client and try again." /SD IDOK
    Abort
  ${EndIf}
  
  FindProcDLL::FindProc "bmdb.exe"
  ${If} $R0 == "1"
    MessageBox MB_OK|MB_ICONSTOP "The BitMeter OS Database Admin utility (bmdb.exe) is currently running, please exit the utility and try again." /SD IDOK
    Abort
  ${EndIf}
  
  !undef UN
  !define UN "un."


  !insertmacro SERVICE "running" "BitMeterCaptureService" ""
  Pop $R0
  ${If} $R0 == "true"
  !insertmacro SERVICE "stop" "BitMeterCaptureService" ""
  ${EndIf}
  !insertmacro SERVICE "delete" "BitMeterCaptureService" ""
  Sleep 3000
  
  !insertmacro SERVICE "running" "BitMeterWebService" ""
  Pop $R0
  ${If} $R0 == "true"
  !insertmacro SERVICE "stop" "BitMeterWebService" ""
  ${EndIf}
  !insertmacro SERVICE "delete" "BitMeterWebService" ""
  Sleep 3000

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BitMeterOS"
  DeleteRegKey HKLM SOFTWARE\BitMeterOS

  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR"
    
  ; Remove files and uninstaller
  Delete $INSTDIR\*.*
  
  SetShellVarContext all
  RMDir /r "$APPDATA\BitmeterOS"
  Delete "$DESKTOP\BitMeter OS.url"
  RmDir /r "$SMPROGRAMS\BitMeter OS"
  
  ; Remove directories used
  RMDir "$INSTDIR"
  
SectionEnd

Function .onInit
    ; ### Check that the user is an Administrator
    UserInfo::GetName
    Pop $0
    UserInfo::GetAccountType
    Pop $1  
    UserInfo::GetOriginalAccountType
    Pop $2
    StrCmp $1 "Admin" +3 0
        MessageBox MB_OK|MB_ICONSTOP "You must be a member of the Administrators group on this computer to install BitMeter. The installer will now abort." /SD IDOK
        Abort
  
FunctionEnd

Function un.onInit
    ; ### Check that the user is an Administrator
    UserInfo::GetName
    Pop $0
    UserInfo::GetAccountType
    Pop $1  
    UserInfo::GetOriginalAccountType
    Pop $2
    StrCmp $1 "Admin" +3 0
        MessageBox MB_OK|MB_ICONSTOP "You must be a member of the Administrators group on this computer to uninstall BitMeter. The uninstaller will now abort." /SD IDOK
        Abort
FunctionEnd

Function .onUserAbort
    MessageBox MB_YESNO|MB_ICONQUESTION "Abort install?" IDYES NoCancelAbort
        Abort
    NoCancelAbort:
FunctionEnd


