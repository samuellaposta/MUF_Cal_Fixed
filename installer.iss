[Setup]
AppName=MUFCalc
AppVersion=1.0
DefaultDirName={autopf}\MUFCalc
DefaultGroupName=MUFCalc
OutputBaseFilename=MUFCalcSetup
SetupIconFile=MUFCalc\resources\MUFCalc.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Files]
Source: "deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\MUFCalc"; Filename: "{app}\MUFCalc.exe"
Name: "{commondesktop}\MUFCalc"; Filename: "{app}\MUFCalc.exe"

[Run]
Filename: "{app}\MUFCalc.exe"; Description: "Launch MUFCalc"; Flags: nowait postinstall skipifsilent
