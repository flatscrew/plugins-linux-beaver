[Setup]
AppName=LinuxBeaverGEGL
AppVersion={#Version}
AppPublisher=Flatscrew
DefaultGroupName=RasterFlow
DefaultDirName={localappdata}\gegl-0.4\plug-ins
Compression=lzma
SolidCompression=yes
OutputBaseFilename=LinuxBeaverGEGL-{#Version}-Setup-win64
OutputDir=.
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
DisableProgramGroupPage=yes
ChangesEnvironment=yes

[Files]
Source: "..\build\payload\msys64\mingw64\lib\gegl-0.4\*.dll"; \
DestDir: "{localappdata}\gegl-0.4\plug-ins"; \
Flags: ignoreversion createallsubdirs recursesubdirs
