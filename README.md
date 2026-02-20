# CubeRender-C

A terminal-based ASCII 3-D rotating cube renderer written in pure C.  
No dependencies beyond the C standard library and `libm`.

```
  @@@@@@@@@@@@@@@
  @@@@@@@@@@@@@@@@#
  @@@     @@@@    ##
  @@   @   @@@  #  #
  @@  @@@  @@@ ##  #
  @@ @@@@@ @@####  #
  @@@@@@@@@@@  ####
   ###########  ###
    ##########   ##
     ##########  #
      ##########
```

---

## Features

- Real-time ASCII rendering with a full 3-axis rotation matrix (X, Y, Z)
- Per-face ANSI color highlighting (6 colors, one per cube face)
- **Automatic terminal resize** — window is set to the correct size on launch
- Cross-platform: **macOS**, **Linux**, **Windows** (MinGW / MSVC)
- CLI flags for runtime control (`--speed`, `--size`, `--fps`, `--no-color`)
- Clean exit on `Ctrl+C` with cursor/color state fully restored
- Double-click launchers — no terminal knowledge required

---

## Quick Start

### macOS / Linux — double-click launch

1. Download and unzip the repository.
2. **Double-click `run.command`** in Finder / file manager.  
   _(First time on macOS: right-click → **Open** → **Open**)_
3. A Terminal window opens, compiles the source automatically, and starts the animation.

### Windows — double-click launch

1. Download and unzip the repository.
2. **Double-click `run.bat`** in Explorer.
3. A `cmd` window opens, compiles the source automatically, and starts the animation.

> **Windows requirement:** `gcc` from [MinGW](https://www.msys2.org/) must be in your `PATH`.  
> Quick install:
>
> ```bat
> winget install -e --id MSYS2.MSYS2
> ```
>
> Then in the MSYS2 terminal:
>
> ```bash
> pacman -S mingw-w64-x86_64-gcc
> ```
>
> Add `C:\msys64\mingw64\bin` to your system `PATH`.

---

## Manual Build

### macOS / Linux

```bash
make          # compile
make run      # compile + run
make clean    # remove binary
```

Or without Make:

```bash
cc -O2 -Wall -std=c11 -o cube cube.c -lm
./cube
```

### Windows (MinGW)

```bat
build.bat         # compile only
build.bat run     # compile + run
```

Or manually:

```bat
gcc -O2 -Wall -std=c11 -o cube.exe cube.c -lm
cube.exe
```

---

## CLI Options

```
Usage:
  ./cube [options]

Options:
  --help          Show help and exit
  --no-color      Disable ANSI colors (plain ASCII output)
  --speed <n>     Rotation speed multiplier   (default: 1.0)
  --size  <n>     Cube half-size in units      (default: 18)
  --fps   <n>     Target frames per second     (default: 60)

Controls:
  Ctrl+C          Quit
```

Examples:

```bash
./cube --speed 2.5              # spin faster
./cube --size 12 --fps 30       # smaller cube, lower framerate
./cube --no-color               # plain ASCII, no colors
```

---

## How It Works

### Rotation

Each point on the cube surface is transformed by a combined 3-axis rotation matrix using three angles **A**, **B**, **C** (in radians) that increment every frame:

$$
\begin{pmatrix} x' \\ y' \\ z' \end{pmatrix}
=
R_z(C) \cdot R_y(B) \cdot R_x(A)
\cdot
\begin{pmatrix} x \\ y \\ z \end{pmatrix}
$$

### Projection

After rotation the point is projected onto the 2-D character buffer using perspective division:

$$
x_p = \frac{W}{2} + K_1 \cdot \frac{x'}{z' + d}, \qquad
y_p = \frac{H}{2} + K_1 \cdot \frac{y'}{z' + d}
$$

where $d$ = camera distance, $K_1$ = perspective constant, $W \times H$ = buffer size (160 × 44).

### Z-Buffer

A floating-point z-buffer (`1/z` per pixel) resolves face occlusion — only the closest surface point per character cell is drawn.

### Characters per face

| Face   | Char | Color          |
| ------ | ---- | -------------- |
| Front  | `@`  | Bright Red     |
| Right  | `$`  | Bright Green   |
| Left   | `~`  | Bright Yellow  |
| Back   | `#`  | Bright Blue    |
| Bottom | `;`  | Bright Magenta |
| Top    | `+`  | Bright Cyan    |

---

## Project Structure

```
CubeRender-C/
  cube.c          Main source — renderer, CLI parser, platform layer
  run.command     macOS / Linux double-click launcher (auto-compile + run)
  run.bat         Windows double-click launcher       (auto-compile + run)
  Makefile        macOS / Linux build system
  build.bat       Windows build script (MinGW)
  .gitignore      Excludes binaries and OS metadata
  LICENSE         MIT License
  README.md       This file
```

---

## License

This project is licensed under the [MIT License](LICENSE).  
© 2026 [0xRETRO](https://github.com/0xR3TRO)
