[Setup]
AppName=Mozart
; icon: \bin\oz.exe
AppCopyright=Open-Source system, see www.mozart-oz.org
AppVerName=Mozart 1.3.1
AppPublisher=SICS, DFKI, and other parties
DefaultDirName={pf}\Mozart
DefaultGroupName=Mozart
AllowNoIcons=yes
LicenseFile=LICENSE.rtf
;InfoBeforeFile=README
OutputBaseFilename=Mozart-1.3.1
Compression=lzma
SolidCompression=yes
OutputDir=.
ChangesAssociations=yes
ChangesEnvironment=yes
WizardImageFile=MozartBitmap.bmp
WizardSmallImageFile=logo\mozart-55x24.gif
WizardImageBackcolor=$fffff0
WizardImageStretch=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "slovak"; MessagesFile: "compiler:Languages\Slovak.isl"
Name: "slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"

[Icons]
Name: "{group}\{cm:UninstallProgram,Mozart}"; Filename: "{uninstallexe}"

[Components]
Name: "base"; Description: "Base System"; Types: full compact custom; Flags: fixed checkablealone
Name: "base\progint"; Description: "Programming Interface"; Types: full custom; Flags: checkablealone
Name: "docs"; Description: "Documentation"; Types: full custom
Name: "docs\chm"; Description: "Documentation"; Types: full custom; Flags: checkablealone
Name: "docs\chm\applets"; Description: "Demo Applets"; Types: full custom; Flags: checkablealone
Name: "docs\examples"; Description: "Examples"; Types: full custom; Flags: checkablealone
Name: "contrib"; Description: "Contributions"; Types: full custom; Flags: checkablealone
Name: "contrib\os"; Description: "OS"; Types: full custom; Flags: checkablealone
Name: "contrib\gdbm"; Description: "GDBM"; Types: full custom; Flags: checkablealone
Name: "contrib\regex"; Description: "Regular Expressions"; Types: full custom; Flags: checkablealone
Name: "contrib\ap"; Description: "Application Programming"; Types: full custom; Flags: checkablealone
Name: "contrib\compat"; Description: "Backwards Compatibility"; Types: full custom; Flags: checkablealone
Name: "contrib\doc_c"; Description: "Documentation Processing"; Types: full custom; Flags: checkablealone
Name: "contrib\tk"; Description: "Additional Widgets"; Types: full custom; Flags: checkablealone
Name: "contrib\micq"; Description: "Mozart Instant Messenger"; Types: full custom; Flags: checkablealone
Name: "contrib\directory"; Description: "Directory Service"; Types: full custom; Flags: checkablealone

[Files]
; applets_images
Source: "cache\x-oz\doc\demo\applets\images\*.*"; DestDir: "{app}\cache\x-oz\doc\demo\applets\images" ; Flags: recursesubdirs; Components: docs\chm
; base
Source: "*.*"; Excludes: "*.iss,*.exe,*~"; DestDir: "{app}"; Components: base
; bin
; bin_oz
; bin_ozengine
; bin_ozenginew
Source: "bin\*.*"; DestDir: "{app}\bin"; Components: base
; cache_xoz_system
Source: "cache\x-oz\system\*.ozf"; DestDir: "{app}\cache\x-oz\system"; Flags: recursesubdirs; Components: base
Source: "cache\x-oz\system\images\*.xbm"; DestDir:  "{app}\cache\x-oz\system\images"; Flags: recursesubdirs; Components: base
Source: "cache\x-oz\system\images\*.jpg"; DestDir:  "{app}\cache\x-oz\system\images"; Flags: recursesubdirs; Components: base
; cache_xozlib
Source: "cache\x-ozlib\*"; DestDir: "{app}\cache\x-ozlib"; Flags: recursesubdirs skipifsourcedoesntexist; Components: base
; contrib_*
Source: "cache\x-oz\contrib\ap\*.*"; DestDir: "{app}\cache\x-oz\contrib\ap"; Flags: skipifsourcedoesntexist recursesubdirs; Components: contrib\ap
Source: "cache\x-oz\contrib\compat\*.ozf"; DestDir: "{app}\cache\x-oz\contrib\compat"; Flags: skipifsourcedoesntexist recursesubdirs; Components: contrib\compat
Source: "cache\x-oz\contrib\directory\*.*"; DestDir: "{app}\cache\x-oz\contrib\directory"; Flags: skipifsourcedoesntexist recursesubdirs; Components: contrib\directory
Source: "cache\x-oz\contrib\doc\*.*"; DestDir: "{app}\cache\x-oz\contrib\doc"; Flags: skipifsourcedoesntexist recursesubdirs; Components: contrib\doc_c
Source: "cache\x-oz\contrib\gdbm.*"; DestDir: "{app}\cache\x-oz\contrib"; Flags: skipifsourcedoesntexist recursesubdirs; Components: contrib\gdbm
Source: "cache\x-oz\contrib\micq\*.*"; DestDir: "{app}\cache\x-oz\contrib\micq"; Flags: skipifsourcedoesntexist recursesubdirs; Components: contrib\micq
Source: "cache\x-oz\contrib\os\*.*"; DestDir: "{app}\cache\x-oz\contrib\os"; Flags: skipifsourcedoesntexist recursesubdirs; Components: contrib\os
Source: "cache\x-oz\contrib\regex.*"; DestDir: "{app}\cache\x-oz\contrib"; Flags: skipifsourcedoesntexist recursesubdirs; Components: contrib\regex
Source: "cache\x-oz\contrib\tk\*.*"; DestDir: "{app}\cache\x-oz\contrib\tk"; Flags: skipifsourcedoesntexist recursesubdirs; Components: contrib\tk
; convertTextPickle
Source: "bin\convertTextPickle"; DestDir: "{app}\bin"; DestName: "convertTextPickle.exe"; Components: base; Flags: skipifsourcedoesntexist
; doc
Source: "doc\Mozart.chm"; DestDir: "{app}\doc"; Components: docs\chm; Flags: skipifsourcedoesntexist
; doc_addons
Source: "doc\add-ons\README.html"; DestDir: "{app}\doc\add-ons"; Components: docs\chm; Flags:skipifsourcedoesntexist
; doc_demo_applets
Source: "doc\demo\applets\*.oza"; DestDir: "{app}\doc\demo\applets"; Components: docs\chm\applets
; doc_duchierozmake
;
; doc_mozartstdlib
;
; doc_mt10
Source: "doc\system\MT10.oz"; DestDir: "{app}\doc\system"; Flags: skipifsourcedoesntexist; Components: docs\chm
; doc_system
Source: "doc\system\MT10.ozf"; DestDir: "{app}\doc\system"; Flags: skipifsourcedoesntexist; Components: docs\chm
; examples
Source: "examples\*.*"; DestDir: "{app}\examples"; Flags: recursesubdirs; Components: docs\examples
; include
Source: "install\*.*"; DestDir: "{app}\include"; Flags: skipifsourcedoesntexist recursesubdirs; Components: base
; platform_win32i486
Source: "platform\win32-i486\*.*"; DestDir: "{app}\platform\win32-i486"; Flags: recursesubdirs; Components: base
; share
Source: "share\*.*"; DestDir: "{app}\share"; Components: base
; share_doc
Source: "share\doc\*.*"; DestDir: "{app}\share\doc"; Components: base
; share_elisp
Source: "share\elisp\*.*"; DestDir: "{app}\share\elisp"; Components: base\progint

[Icons]
Name: "{group}\Oz Programming Interface"; Filename: "{app}\bin\oz.exe"; Comment: "An integrated development environment for Oz"; Components: base\progint
Name: "{group}\Documentation"; Filename: "{app}\doc\Mozart.chm"; Flags:runmaximized; Comment: "The Mozart online documentation."; Components: docs\chm

[Registry]
Root: HKCR; Subkey: ".oz"; ValueType: string; ValueName: ""; ValueData: "OzSource"; Flags: uninsdeletevalue; Components: base\progint
Root: HKCR; Subkey: "OzSource"; ValueType: string; ValueName: ""; ValueData: "Oz Source"; Flags: uninsdeletekey; Components: base\progint
Root: HKCR; Subkey: "OzSource\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\oz.exe,0"; Components: base\progint
Root: HKCR; Subkey: "OzSource\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\oz.exe"" ""%1"""; Components: base\progint
Root: HKCR; Subkey: ".ozg"; ValueType: string; ValueName: ""; ValueData: "OzGSource"; Flags: uninsdeletevalue; Components: base\progint
Root: HKCR; Subkey: "OzGSource"; ValueType: string; ValueName: ""; ValueData: "Oz Gump Source"; Flags: uninsdeletekey; Components: base\progint
Root: HKCR; Subkey: "OzGSource\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\oz.exe,0"; Components: base\progint
Root: HKCR; Subkey: "OzGSource\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\oz.exe"" ""%1"""; Components: base\progint
Root: HKCR; Subkey: ".oza"; ValueType: string; ValueName: ""; ValueData: "OzApplication"; Flags: uninsdeletevalue; Components: base\progint
Root: HKCR; Subkey: "OzApplication"; ValueType: string; ValueName: ""; ValueData: "Oz Application"; Flags: uninsdeletekey; Components: base\progint
Root: HKCR; Subkey: "OzApplication\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\ozenginew.exe,0"; Components: base\progint
Root: HKCR; Subkey: "OzApplication\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\ozenginew.exe"" ""%1"""; Components: base\progint

[Code]
#include "ModifyPath.iss"

procedure CurStepChanged(CurStep: TSetupStep);
begin
  case CurStep of
    ssPostInstall:
      begin
        if ExpandConstant('{userdocs}') = ExpandConstant('{commondocs}') then
          ModifyPath('{app}\bin', pmAddToEnd+pmAddAllways, psAllUsers)
        else
          ModifyPath('{app}\bin', pmAddToEnd+pmAddAllways, psCurrentUser)
      end
  end
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  case CurUninstallStep of
    usPostUninstall:
      begin
        if ExpandConstant('{userdocs}') = ExpandConstant('{commondocs}') then
          ModifyPath('{app}\bin', pmRemove, psAllUsers)
        else
          ModifyPath('{app}\bin', pmRemove, psCurrentUser)
      end
  end
end;

