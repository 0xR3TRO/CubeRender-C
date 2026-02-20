/*
 * ============================================================
 *  CubeRender — ASCII 3-D rotating cube renderer
 *  Version 2.0
 *
 *  Platforms : macOS, Linux, Windows (MinGW / MSVC)
 *
 *  Quick start (double-click a launcher):
 *    macOS / Linux  →  run.command
 *    Windows        →  run.bat
 *
 *  Manual build:
 *    macOS / Linux  →  make            (or: cc -O2 -o cube cube.c -lm)
 *    Windows        →  build.bat       (requires MinGW gcc in PATH)
 *
 *  Usage:
 *    ./cube [options]
 *
 *  Options:
 *    --help          Show this help and exit
 *    --no-color      Disable ANSI colors (plain ASCII output)
 *    --speed <n>     Rotation speed multiplier, default 1.0
 *    --size  <n>     Cube half-size, default 18
 *    --fps   <n>     Target frames per second, default 60
 *
 *  Controls:
 *    Ctrl+C          Quit
 * ============================================================
 */

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Platform compatibility ─────────────────────────────────── */
#ifdef _WIN32
#  include <windows.h>
#  define SLEEP_MS(ms)  Sleep(ms)
   static void enable_ansi(void) {
       HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
       DWORD  mode = 0;
       GetConsoleMode(h, &mode);
       /* Enable ANSI/VT escape sequences on Windows 10+ */
       SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
   }
#else
#  include <unistd.h>
#  define SLEEP_MS(ms)  usleep((unsigned int)((ms) * 1000u))
   static void enable_ansi(void) { /* natively supported */ }
#endif

/* ── Default configuration ───────────────────────────────────── */
#define WIDTH        160     /* character buffer width            */
#define HEIGHT        44     /* character buffer height           */
#define DISTANCE_CAM 100     /* projection distance               */
#define K1            40.0f  /* perspective constant              */
#define INCREMENT      0.4f  /* surface scan step                 */
#define DEFAULT_FPS   60
#define DEFAULT_SPEED  1.0f
#define DEFAULT_SIZE  18.0f

/* ── ANSI color codes (16-color, universal support) ─────────── */
#define CLR_RESET  "\033[0m"
#define CLR_FRONT  "\033[91m"   /* bright red     — front face  */
#define CLR_RIGHT  "\033[92m"   /* bright green   — right face  */
#define CLR_LEFT   "\033[93m"   /* bright yellow  — left face   */
#define CLR_BACK   "\033[94m"   /* bright blue    — back face   */
#define CLR_BOTTOM "\033[95m"   /* bright magenta — bottom face */
#define CLR_TOP    "\033[96m"   /* bright cyan    — top face    */

/* ── Face character / color mapping ─────────────────────────── */
static const char  FACE_CH[7]  = { 0, '@', '$', '~', '#', ';', '+' };
static const char *FACE_CLR[7] = {
    CLR_RESET,
    CLR_FRONT, CLR_RIGHT, CLR_LEFT,
    CLR_BACK,  CLR_BOTTOM, CLR_TOP
};

/* ── Render buffers ──────────────────────────────────────────── */
static float A, B, C;                /* rotation angles (radians)  */
static float zBuf[WIDTH * HEIGHT];
static char  cBuf[WIDTH * HEIGHT];   /* character per pixel        */
static int   kBuf[WIDTH * HEIGHT];   /* face index 1-6, 0 = empty  */

/* ── Runtime settings (set from CLI args) ───────────────────── */
static int   use_color = 1;
static float rot_speed = DEFAULT_SPEED;
static float cube_size = DEFAULT_SIZE;
static int   target_fps = DEFAULT_FPS;

static volatile int running = 1;

/* ── Signal handler ─────────────────────────────────────────── */
static void on_sigint(int s) { (void)s; running = 0; }

/* ── Help text ──────────────────────────────────────────────── */
static void print_help(const char *argv0) {
    printf(
        "CubeRender v2.0 — ASCII rotating cube\n"
        "\n"
        "Usage:\n"
        "  %s [options]\n"
        "\n"
        "Options:\n"
        "  --help          Show this help and exit\n"
        "  --no-color      Disable ANSI colors\n"
        "  --speed <n>     Rotation speed multiplier  (default: 1.0)\n"
        "  --size  <n>     Cube half-size in units     (default: 18)\n"
        "  --fps   <n>     Target frames per second    (default: 60)\n"
        "\n"
        "Controls:\n"
        "  Ctrl+C          Quit\n",
        argv0
    );
}

/* ── 3-axis rotation matrix ─────────────────────────────────── */
static float rotX(float i, float j, float k) {
    return j*sinf(A)*sinf(B)*cosf(C) - k*cosf(A)*sinf(B)*cosf(C)
         + j*cosf(A)*sinf(C)         + k*sinf(A)*sinf(C)
         + i*cosf(B)*cosf(C);
}
static float rotY(float i, float j, float k) {
    return j*cosf(A)*cosf(C) + k*sinf(A)*cosf(C)
         - j*sinf(A)*sinf(B)*sinf(C) + k*cosf(A)*sinf(B)*sinf(C)
         - i*cosf(B)*sinf(C);
}
static float rotZ(float i, float j, float k) {
    return k*cosf(A)*cosf(B) - j*sinf(A)*cosf(B) + i*sinf(B);
}

/* ── Project one surface point into the frame buffer ────────── */
static void project(float cx, float cy, float cz,
                    float ho, int face) {
    float rx  = rotX(cx, cy, cz);
    float ry  = rotY(cx, cy, cz);
    float z   = rotZ(cx, cy, cz) + (float)DISTANCE_CAM;
    if (z <= 0.0f) return;                 /* behind camera        */
    float ooz = 1.0f / z;
    int   xp  = (int)(WIDTH  / 2 + ho + K1 * ooz * rx * 2);
    int   yp  = (int)(HEIGHT / 2       + K1 * ooz * ry);
    int   idx = xp + yp * WIDTH;

    if (xp < 0 || xp >= WIDTH || yp < 0 || yp >= HEIGHT) return;
    if (ooz > zBuf[idx]) {
        zBuf[idx] = ooz;
        cBuf[idx] = FACE_CH[face];
        kBuf[idx] = face;
    }
}

/* ── Render a cube of given half-size centered at offset ho ─── */
static void render_cube(float size, float ho) {
    for (float cx = -size; cx < size; cx += INCREMENT) {
        for (float cy = -size; cy < size; cy += INCREMENT) {
            project( cx,   cy,  -size, ho, 1);   /* front  */
            project( size, cy,   cx,   ho, 2);   /* right  */
            project(-size, cy,  -cx,   ho, 3);   /* left   */
            project(-cx,   cy,   size, ho, 4);   /* back   */
            project( cx,  -size,-cy,   ho, 5);   /* bottom */
            project( cx,   size, cy,   ho, 6);   /* top    */
        }
    }
}

/* ── Flush one frame to stdout ──────────────────────────────── */
static void flush_frame(void) {
    printf("\033[H");   /* move cursor to top-left without clearing */

    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        if (i > 0 && i % WIDTH == 0) {
            if (use_color) fputs(CLR_RESET, stdout);
            putchar('\n');
        }
        if (kBuf[i]) {
            if (use_color) fputs(FACE_CLR[kBuf[i]], stdout);
            putchar(cBuf[i]);
        } else {
            putchar(' ');   /* empty space — cleaner than filling with '.' */
        }
    }

    if (use_color) fputs(CLR_RESET, stdout);
    putchar('\n');
    fflush(stdout);
}

/* ── Parse CLI arguments ────────────────────────────────────── */
static int parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--no-color") == 0) {
            use_color = 0;
        } else if (strcmp(argv[i], "--speed") == 0 && i + 1 < argc) {
            rot_speed = (float)atof(argv[++i]);
            if (rot_speed <= 0.0f) rot_speed = DEFAULT_SPEED;
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            cube_size = (float)atof(argv[++i]);
            if (cube_size <= 0.0f) cube_size = DEFAULT_SIZE;
        } else if (strcmp(argv[i], "--fps") == 0 && i + 1 < argc) {
            target_fps = atoi(argv[++i]);
            if (target_fps <= 0) target_fps = DEFAULT_FPS;
        } else {
            fprintf(stderr, "Unknown option: %s\nRun with --help for usage.\n",
                    argv[i]);
            return 0;
        }
    }
    return 1;
}

/* ── Entry point ─────────────────────────────────────────────── */
int main(int argc, char **argv) {
    if (!parse_args(argc, argv)) return 0;

    enable_ansi();
    signal(SIGINT, on_sigint);

    const float base_speed = 0.012f;
    const int   frame_ms   = 1000 / target_fps;

    printf("\033[?25l");  /* hide cursor                           */
    printf("\033[2J");    /* clear screen once                     */

    /* Banner — shown above the cube */
    printf("\033[1;1H");
    if (use_color) fputs("\033[1;97m", stdout);
    printf(" CubeRender v2.0  |  size=%.0f  speed=%.1fx  fps=%d"
           "  |  Ctrl+C to quit\n",
           cube_size, rot_speed, target_fps);
    if (use_color) fputs(CLR_RESET, stdout);

    while (running) {
        /* Clear buffers */
        memset(cBuf, 0, sizeof(cBuf));
        memset(zBuf, 0, sizeof(zBuf));
        memset(kBuf, 0, sizeof(kBuf));

        /* Render a single centered cube */
        render_cube(cube_size, 0.0f);

        flush_frame();

        /* Advance rotation angles */
        A += base_speed * rot_speed;
        B += base_speed * rot_speed;
        C += base_speed * rot_speed * 0.5f;

        SLEEP_MS(frame_ms);
    }

    /* Restore terminal state */
    printf("\033[?25h");   /* show cursor */
    if (use_color) fputs(CLR_RESET, stdout);
    printf("\n");
    return 0;
}