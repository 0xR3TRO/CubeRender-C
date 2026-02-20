/*
 * CubeRender — ASCII rotating cubes renderer
 *
 * Obsługiwane platformy: macOS, Linux, Windows (MinGW/MSVC)
 *
 * Budowanie:
 *   macOS / Linux : make          (lub: cc -O2 -o cube cube.c -lm)
 *   Windows       : build.bat     (wymaga gcc z MinGW w PATH)
 *
 * Sterowanie:
 *   Ctrl+C  — wyjście
 */

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Kompatybilność platform ─────────────────────────────────── */
#ifdef _WIN32
#  include <windows.h>
#  define SLEEP_MS(ms)  Sleep(ms)
   /* Włącz przetwarzanie sekwencji ANSI w starszych konsolach Windows */
   static void enable_ansi(void) {
       HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
       DWORD  mode = 0;
       GetConsoleMode(h, &mode);
       SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
   }
#else
#  include <unistd.h>
#  define SLEEP_MS(ms)  usleep((ms) * 1000u)
   static void enable_ansi(void) { /* już obsługiwane */ }
#endif

/* ── Konfiguracja ────────────────────────────────────────────── */
#define WIDTH          160   /* szerokość bufora znakowego         */
#define HEIGHT          44   /* wysokość bufora znakowego          */
#define DISTANCE_CAM   100   /* odległość kamery od sceny          */
#define K1              40.0f
#define INCREMENT        0.5f
#define FRAME_MS        16   /* ~60 fps                            */
#define ROT_SPEED        0.012f

/* ── Kolory ANSI (standardowe 16-kolorowe, działają wszędzie) ── */
#define CLR_RESET  "\033[0m"
#define CLR_1      "\033[91m"   /* jasny czerwony    — @ */
#define CLR_2      "\033[92m"   /* jasny zielony     — $ */
#define CLR_3      "\033[93m"   /* jasny żółty       — ~ */
#define CLR_4      "\033[94m"   /* jasny niebieski   — # */
#define CLR_5      "\033[95m"   /* jasny magenta     — ; */
#define CLR_6      "\033[96m"   /* jasny cyjan       — + */

/* ── Globalne bufory ─────────────────────────────────────────── */
static float A, B, C;                        /* kąty obrotu       */
static float zBuf[WIDTH * HEIGHT];
static char  cBuf[WIDTH * HEIGHT];           /* znaki             */
static int   kBuf[WIDTH * HEIGHT];           /* kody kolorów 1-6  */

static volatile int running = 1;

/* ── Obsługa sygnału wyjścia ─────────────────────────────────── */
static void on_sigint(int s) { (void)s; running = 0; }

/* ── Macierz obrotu: 3 osie (A, B, C) ───────────────────────── */
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

/* ── Rzutowanie punktu na bufor ─────────────────────────────── */
static void project(float cx, float cy, float cz,
                    float ho, int ch, int color) {
    float rx  = rotX(cx, cy, cz);
    float ry  = rotY(cx, cy, cz);
    float z   = rotZ(cx, cy, cz) + DISTANCE_CAM;
    float ooz = 1.0f / z;
    int   xp  = (int)(WIDTH  / 2 + ho + K1 * ooz * rx * 2);
    int   yp  = (int)(HEIGHT / 2       + K1 * ooz * ry);
    int   idx = xp + yp * WIDTH;

    if (idx >= 0 && idx < WIDTH * HEIGHT && ooz > zBuf[idx]) {
        zBuf[idx] = ooz;
        cBuf[idx] = (char)ch;
        kBuf[idx] = color;
    }
}

/* ── Renderowanie jednego sześcianu ─────────────────────────── */
static void renderCube(float size, float ho) {
    for (float cx = -size; cx < size; cx += INCREMENT) {
        for (float cy = -size; cy < size; cy += INCREMENT) {
            project( cx,   cy,  -size, ho, '@', 1);  /* przód   */
            project( size, cy,   cx,   ho, '$', 2);  /* prawo   */
            project(-size, cy,  -cx,   ho, '~', 3);  /* lewo    */
            project(-cx,   cy,   size, ho, '#', 4);  /* tył     */
            project( cx,  -size,-cy,   ho, ';', 5);  /* dół     */
            project( cx,   size, cy,   ho, '+', 6);  /* góra    */
        }
    }
}

/* ── Wypisanie znaku z kolorem ───────────────────────────────── */
static void putColored(char ch, int color) {
    const char *c;
    switch (color) {
        case 1:  c = CLR_1; break;
        case 2:  c = CLR_2; break;
        case 3:  c = CLR_3; break;
        case 4:  c = CLR_4; break;
        case 5:  c = CLR_5; break;
        case 6:  c = CLR_6; break;
        default: c = CLR_RESET; break;
    }
    fputs(c, stdout);
    putchar(ch);
}

/* ── Główna pętla ────────────────────────────────────────────── */
int main(void) {
    enable_ansi();
    signal(SIGINT, on_sigint);

    printf("\033[?25l");   /* ukryj kursor */
    printf("\033[2J");     /* wyczyść ekran (jednorazowo) */

    while (running) {
        memset(cBuf, '.', sizeof(cBuf));
        memset(zBuf,   0, sizeof(zBuf));
        memset(kBuf,   0, sizeof(kBuf));

        /*
         * Układ trzech sześcianów obok siebie:
         *   duży (r=20) po lewej, średni (r=10) w centrum,
         *   mały (r=5)  po prawej.
         *
         * horizontalOffset = przesunięcie w osi X na ekranie.
         */
        renderCube(20.0f, -40.0f);
        renderCube(10.0f,  10.0f);
        renderCube( 5.0f,  40.0f);

        /* Powrót kursora na początek bez migotania */
        printf("\033[H");

        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            if (i % WIDTH == 0 && i != 0) {
                fputs(CLR_RESET "\n", stdout);
            } else if (kBuf[i]) {
                putColored(cBuf[i], kBuf[i]);
            } else {
                putchar(cBuf[i]);
            }
        }
        fputs(CLR_RESET "\n", stdout);
        fflush(stdout);

        /* Obrót sceny */
        A += ROT_SPEED;
        B += ROT_SPEED;
        C += ROT_SPEED * 0.5f;

        SLEEP_MS(FRAME_MS);
    }

    /* Przywróć terminal */
    printf("\033[?25h" CLR_RESET "\n");
    return 0;
}