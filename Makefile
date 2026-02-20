# CubeRender — Makefile (macOS / Linux)
# Użycie:
#   make          — kompilacja
#   make run      — kompilacja + uruchomienie
#   make clean    — usunięcie pliku wykonywalnego

CC      = cc
CFLAGS  = -O2 -Wall -Wextra -std=c11
LIBS    = -lm
TARGET  = cube
SRC     = cube.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
	@echo "✓ Zbudowano: ./$(TARGET)"

run: all
	@echo "Uruchamianie... (Wyjście: Ctrl+C)"
	./$(TARGET)

clean:
	rm -f $(TARGET)
	@echo "✓ Wyczyszczono"
