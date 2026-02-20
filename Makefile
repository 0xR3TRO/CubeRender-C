# ============================================================
#  CubeRender — Makefile (macOS / Linux)
#
#  Targets:
#    make           Build the binary
#    make run       Build and run
#    make clean     Remove compiled files
# ============================================================

CC      = cc
CFLAGS  = -O2 -Wall -Wextra -std=c11
LIBS    = -lm
TARGET  = cube
SRC     = cube.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
	@echo "Build successful: ./$(TARGET)"
	@echo "Run with: ./$(TARGET) --help"

run: all
	@echo "Starting CubeRender... (Quit: Ctrl+C)"
	./$(TARGET)

clean:
	rm -f $(TARGET)
	@echo "Cleaned up."
