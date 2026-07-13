@echo off
chcp 65001 >nul 2>&1
setlocal enabledelayedexpansion

echo ========================================
echo   JQChat One-Click Package Script
echo ========================================
echo.

set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build"
set "PACKAGE_DIR=%PROJECT_DIR%package"
set "QT_BIN=D:\Software\Qt\6.9.3\msvc2022_64\bin"
set "INNO_COMPILER=D:\Software\Inno Setup 6\Inno Setup 6\ISCC.exe"

REM ---- Step 1: Check Qt ----
if not exist "%QT_BIN%\windeployqt.exe" (
    echo [ERROR] windeployqt.exe not found
    echo Please update QT_BIN path in this script
    pause
    exit /b 1
)

REM ---- Step 2: Build Release ----
echo [1/3] Building Release ...
cmake --build "%BUILD_DIR%" --config Release --parallel
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)
echo [1/3] Build OK
echo.

REM ---- Step 3: windeployqt ----
echo [2/3] Collecting Qt dependencies ...

set "EXE_FILE=%BUILD_DIR%\Release\JQChat.exe"
if not exist "%EXE_FILE%" (
    echo [ERROR] JQChat.exe not found
    pause
    exit /b 1
)

if exist "%PACKAGE_DIR%" rmdir /s /q "%PACKAGE_DIR%"
mkdir "%PACKAGE_DIR%"

copy "%EXE_FILE%" "%PACKAGE_DIR%\" >nul

"%QT_BIN%\windeployqt" --release --no-translations "%PACKAGE_DIR%\JQChat.exe"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] windeployqt failed!
    pause
    exit /b 1
)
echo [2/3] Dependencies OK
echo.

REM ---- Step 3.5: Copy VC++ Runtime ----
echo [2.5/3] Copying VC++ runtime DLLs ...

set "SYSTEM32=C:\Windows\System32"

if exist "%SYSTEM32%\vcruntime140.dll" (
    copy "%SYSTEM32%\vcruntime140.dll" "%PACKAGE_DIR%\" >nul
    copy "%SYSTEM32%\vcruntime140_1.dll" "%PACKAGE_DIR%\" >nul
    copy "%SYSTEM32%\msvcp140.dll" "%PACKAGE_DIR%\" >nul
    copy "%SYSTEM32%\msvcp140_1.dll" "%PACKAGE_DIR%\" >nul
    copy "%SYSTEM32%\msvcp140_2.dll" "%PACKAGE_DIR%\" >nul
    copy "%SYSTEM32%\msvcp140_atomic_wait.dll" "%PACKAGE_DIR%\" >nul
    copy "%SYSTEM32%\msvcp140_codecvt_ids.dll" "%PACKAGE_DIR%\" >nul
    copy "%SYSTEM32%\concrt140.dll" "%PACKAGE_DIR%\" >nul 2>&1
    echo [2.5/3] VC++ Runtime OK
) else (
    echo [WARN] VC++ Runtime DLLs not found
)
echo.

REM ---- Step 4: Inno Setup ----
echo [3/3] Creating installer ...

if not exist "%INNO_COMPILER%" (
    echo [SKIP] Inno Setup 6 not installed
    echo        Download: https://jrsoftware.org/isinfo.php
    goto :done
)

set "INSTALLER_DIR=%PROJECT_DIR%installer"
if not exist "%INSTALLER_DIR%" mkdir "%INSTALLER_DIR%"

set "ISS_FILE=%PROJECT_DIR%setup.iss"

(
echo [Setup]
echo AppName=JQChat
echo AppVersion=1.0.0
echo DefaultDirName={autopf}\JQChat
echo DefaultGroupName=JQChat
echo OutputDir=%INSTALLER_DIR%
echo OutputBaseFilename=JQChat_Setup_v1.0.0
echo Compression=lzma
echo SolidCompression=yes
echo UninstallDisplayName=JQChat
echo SetupIconFile=%PROJECT_DIR%resources\images\JQChat.ico
echo UninstallDisplayIcon={app}\JQChat.exe
echo.
echo [Files]
echo Source: "%PACKAGE_DIR%\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs
echo.
echo [Icons]
echo Name: "{group}\JQChat"; Filename: "{app}\JQChat.exe"
echo Name: "{commondesktop}\JQChat"; Filename: "{app}\JQChat.exe"
echo.
echo [Run]
echo Filename: "{app}\JQChat.exe"; Description: "Launch JQChat"; Flags: nowait postinstall
) > "%ISS_FILE%"

"%INNO_COMPILER%" "%ISS_FILE%"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Inno Setup compile failed!
    pause
    exit /b 1
)

del "%ISS_FILE%"
echo [3/3] Installer OK
echo.

:done
echo ========================================
echo   Done!
echo   Portable : %PACKAGE_DIR%
if exist "%INSTALLER_DIR%\JQChat_Setup_v1.0.0.exe" (
    echo   Installer: %INSTALLER_DIR%\JQChat_Setup_v1.0.0.exe
)
echo ========================================
pause
