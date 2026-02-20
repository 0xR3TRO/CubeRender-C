@echo off
:: CubeRender — skrypt budowania dla Windows (wymaga MinGW gcc w PATH)
:: Uruchomienie: build.bat
:: Aby zainstalowac MinGW: winget install -e --id MSYS2.MSYS2
::   a nastepnie w terminalu MSYS2: pacman -S mingw-w64-x86_64-gcc

setlocal

set TARGET=cube.exe
set SRC=cube.c
set CC=gcc
set CFLAGS=-O2 -Wall -Wextra -std=c11
set LIBS=-lm

echo [CubeRender] Budowanie %TARGET%...

%CC% %CFLAGS% -o %TARGET% %SRC% %LIBS%

if %errorlevel% neq 0 (
    echo [BLAD] Kompilacja nie powiodla sie.
    echo Upewnij sie, ze gcc (MinGW) jest zainstalowany i dostepny w PATH.
    pause
    exit /b 1
)

echo [OK] Plik %TARGET% zostal zbudowany.
echo Uruchomienie:  %TARGET%
echo Wyjscie:       Ctrl+C
echo.

set /p RUN="Uruchomic teraz? [T/n]: "
if /i "%RUN%"=="n" exit /b 0
%TARGET%

endlocal
