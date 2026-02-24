[Setup]
AppName=DescartesBuilder
AppVersion=0.2.2
SetupIconFile=..\..\app\resources\tool_logo.ico
UninstallDisplayName=DescartesBuilder
DefaultDirName={autopf}\DescartesBuilder
DefaultGroupName=DescartesBuilder
OutputBaseFilename=DescartesBuilderInstaller
Compression=zip
SolidCompression=no
ArchitecturesAllowed=x64os
ArchitecturesInstallIn64BitMode=x64os
UninstallDisplayIcon={app}\bin\DescartesBuilder.exe

[Files]
Source: "..\..\app\build_win\bin\Release\*"; DestDir: "{app}\bin"; Flags: recursesubdirs createallsubdirs
Source: ".\python_win\python.tar"; DestDir: "{app}"; Flags: ignoreversion
; Source: "..\..\examples\pipe*"; DestDir: "{app}\examples"; Flags: recursesubdirs createallsubdirs

[Run]
Filename: "{cmd}"; Parameters: "/C tar -xf ""{app}\python.tar"" -C ""{app}"" && del ""{app}\python.tar"""; Flags: runhidden waituntilterminated

[UninstallDelete]
; Remove the entire Python folder that was extracted at runtime
Type: filesandordirs; Name: "{app}\python"
Type: filesandordirs; Name: "{app}\bin"
; Remove examples folder if installed
; Type: filesandordirs; Name: "{app}\examples"

[Icons]
Name: "{group}\DescartesBuilder"; Filename: "{app}\bin\DescartesBuilder.exe"