$ErrorActionPreference = "Stop"

$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir   = Join-Path $ProjectDir "build"
$PackageDir = Join-Path $ProjectDir "package"
$QtBin      = "D:\Software\Qt\6.9.3\msvc2022_64\bin"
$InnoSetup  = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
$InstallerDir = Join-Path $ProjectDir "installer"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  JQChat One-Click Package Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ---- Check Qt ----
if (-not (Test-Path "$QtBin\windeployqt.exe")) {
    Write-Host "[ERROR] windeployqt.exe not found at: $QtBin" -ForegroundColor Red
    Write-Host "Please update `$QtBin in this script." -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

# ---- Step 1: Build Release ----
Write-Host "[1/3] Building Release..." -ForegroundColor Green
cmake --build $BuildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Build failed!" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}
Write-Host "[1/3] Build OK" -ForegroundColor Green
Write-Host ""

# ---- Step 2: windeployqt ----
Write-Host "[2/3] Collecting Qt dependencies..." -ForegroundColor Green

$ExeFile = Join-Path $BuildDir "Release\JQChat.exe"
if (-not (Test-Path $ExeFile)) {
    Write-Host "[ERROR] JQChat.exe not found at: $ExeFile" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

if (Test-Path $PackageDir) { Remove-Item -Recurse -Force $PackageDir }
New-Item -ItemType Directory -Force -Path $PackageDir | Out-Null

Copy-Item $ExeFile $PackageDir

& "$QtBin\windeployqt" --release --no-translations "$PackageDir\JQChat.exe"
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] windeployqt failed!" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}
Write-Host "[2/3] Dependencies OK" -ForegroundColor Green
Write-Host ""

# ---- Step 3: Inno Setup installer ----
Write-Host "[3/3] Creating installer..." -ForegroundColor Green

if (-not (Test-Path $InnoSetup)) {
    Write-Host "[SKIP] Inno Setup 6 not installed." -ForegroundColor Yellow
    Write-Host "       Download: https://jrsoftware.org/isinfo.php" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "  Done! Portable package: $PackageDir" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Read-Host "Press Enter to exit"
    exit 0
}

# Generate .iss file
if (-not (Test-Path $InstallerDir)) { New-Item -ItemType Directory -Force -Path $InstallerDir | Out-Null }

$IssContent = @"
[Setup]
AppName=JQChat
AppVersion=1.0.0
DefaultDirName={autopf}\JQChat
DefaultGroupName=JQChat
OutputDir=$($InstallerDir -replace '\\','\\')
OutputBaseFilename=JQChat_Setup_v1.0.0
Compression=lzma
SolidCompression=yes
UninstallDisplayName=JQChat

[Files]
Source: "$($PackageDir -replace '\\','\\')\\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\JQChat"; Filename: "{app}\JQChat.exe"
Name: "{commondesktop}\JQChat"; Filename: "{app}\JQChat.exe"

[Run]
Filename: "{app}\JQChat.exe"; Description: "Launch JQChat"; Flags: nowait postinstall
"@

$IssFile = Join-Path $ProjectDir "setup.iss"
$IssContent | Out-File -FilePath $IssFile -Encoding Default

& $InnoSetup $IssFile
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Inno Setup compile failed!" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

Remove-Item $IssFile
Write-Host "[3/3] Installer OK" -ForegroundColor Green
Write-Host ""

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Done!" -ForegroundColor Cyan
Write-Host "  Portable : $PackageDir" -ForegroundColor Cyan
Write-Host "  Installer: $InstallerDir\JQChat_Setup_v1.0.0.exe" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Read-Host "Press Enter to exit"
