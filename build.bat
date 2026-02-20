@echo off
:: ============================================================
::  CubeRender — Windows build script (MinGW gcc required)
::
::  How to install MinGW:
::    winget install -e --id MSYS2.MSYS2
::    Then in MSYS2 terminal: pacman -S mingw-w64-x86_64-gcc
::
::  Usage:
::    build.bat              Build only
::    build.bat run          Build and run
:: ============================================================

setlocal

set TARGET=cube.exe
set SRC=cube.c
set CC=gcc
set CFLAGS=-O2 -Wall -Wextra -std=c11
set LIBS=-lm

echo [CubeRender] Building %TARGET% ...

%CC% %CFLAGS% -o %TARGET% %SRC% %LIBS%

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Build failed.
    echo Make sure gcc (MinGW) is installed and available in PATH.
    echo Install: winget install -e --id MSYS2.MSYS2
    pause
    exit /b 1
)

echo [OK] %TARGET% built successfully.

if /i "%1"=="run" (
    echo Starting CubeRender...  Quit: Ctrl+C
    %TARGET%
)

endlocal
