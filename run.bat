@echo off
:: ============================================================
::  CubeRender — Windows double-click launcher
::
::  Double-click this file in Explorer → cmd opens → cube runs.
::
::  Requirements: gcc (MinGW) in PATH
::  Install MinGW: winget install -e --id MSYS2.MSYS2
::    then in MSYS2:  pacman -S mingw-w64-x86_64-gcc
::    and add C:\msys64\mingw64\bin to your system PATH.
:: ============================================================

setlocal
cd /d "%~dp0"

set BINARY=cube.exe
set SOURCE=cube.c

echo ========================================
echo   CubeRender v2.0
echo ========================================
echo.

:: Verify gcc is available
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] gcc not found in PATH.
    echo.
    echo Install MinGW via:
    echo   winget install -e --id MSYS2.MSYS2
    echo Then in MSYS2 terminal run:
    echo   pacman -S mingw-w64-x86_64-gcc
    echo And add C:\msys64\mingw64\bin to your system PATH.
    echo.
    pause
    exit /b 1
)

:: Compile if binary is missing
if not exist %BINARY% (
    echo [Build] Compiling %SOURCE% ...
    gcc -O2 -Wall -std=c11 -o %BINARY% %SOURCE% -lm
    if %errorlevel% neq 0 (
        echo.
        echo [ERROR] Compilation failed.
        pause
        exit /b 1
    )
    echo [Build] Done.
    echo.
)

echo Starting CubeRender...   Quit: Ctrl+C
echo.
%BINARY%

echo.
echo CubeRender exited.
pause
endlocal
