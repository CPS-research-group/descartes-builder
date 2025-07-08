[Setup]
AppName=DescartesBuilder
AppVersion=0.1
DefaultDirName={autopf}\DescartesBuilder
DefaultGroupName=DescartesBuilder
OutputBaseFilename=DescartesBuilderInstaller
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64os
ArchitecturesInstallIn64BitMode=x64os

[Files]
Source: "..\app\build_win\bin\*"; DestDir: "{app}\bin"; Flags: recursesubdirs createallsubdirs
Source: ".\python_win\*"; DestDir: "{app}\python_win"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\examples\*"; DestDir: "{app}\examples"; Flags: recursesubdirs createallsubdirs

[Icons]
Name: "{group}\DescartesBuilder"; Filename: "{app}\bin\DescartesBuilder.exe"
