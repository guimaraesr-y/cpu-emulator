CC = cc
CFLAGS = -std=c99 -O2 -Wall -Wextra -Wpedantic

BIN_DIR = bin
ASSEMBLER = $(BIN_DIR)/assembler
EMULATOR = $(BIN_DIR)/emulator
VISUALIZER = $(BIN_DIR)/cpu-lab
RAYLIB_DIR ?= .deps/raylib
RAYGUI_DIR ?= .deps/raygui
PYTHON ?= python3
LAB_SOURCES = visualizer/main.c visualizer/playback.c visualizer/app.c assembler/asm.c visualizer/scene.c visualizer/editor.c visualizer/ui.c cpu/cpuFunctions.c
ifeq ($(OS),Windows_NT)
    GRAPHICS_LIBS = -lopengl32 -lgdi32 -lwinmm
else
    GRAPHICS_LIBS = -lm -lpthread -ldl -lrt -lX11
endif

.PHONY: all clean test visualizer test-playback
all: $(ASSEMBLER) $(EMULATOR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(ASSEMBLER): assembler.c assembler/asm.c assembler/asm.h opcodes.h variables.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ assembler.c assembler/asm.c

$(EMULATOR): main.c cpu/cpuFunctions.c cpu/cpu.h opcodes.h variables.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ main.c cpu/cpuFunctions.c

test: all test-playback test-app
	$(PYTHON) -m unittest discover -s tests -v

$(BIN_DIR)/test-playback: tests/test_playback.c visualizer/playback.c visualizer/playback.h cpu/cpuFunctions.c cpu/cpu.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_playback.c visualizer/playback.c cpu/cpuFunctions.c

test-playback: $(BIN_DIR)/test-playback
	./$(BIN_DIR)/test-playback

$(BIN_DIR)/test-app: tests/test_app.c visualizer/app.c visualizer/app.h visualizer/playback.c visualizer/playback.h assembler/asm.c assembler/asm.h cpu/cpuFunctions.c cpu/cpu.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_app.c visualizer/app.c visualizer/playback.c assembler/asm.c cpu/cpuFunctions.c

.PHONY: test-app
test-app: $(BIN_DIR)/test-app
	./$(BIN_DIR)/test-app

$(BIN_DIR)/demo.bin: $(ASSEMBLER) example_programs/program1.asm
	./$(ASSEMBLER) example_programs/program1.asm $@

# Get the pinned dependencies with scripts/setup-graphics.ps1 (or the documented git commands).
$(RAYLIB_DIR)/src/libraylib.a:
	$(MAKE) -C $(RAYLIB_DIR)/src PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC CC=$(CC)

$(VISUALIZER): $(LAB_SOURCES) visualizer/playback.h visualizer/app.h visualizer/editor.h assembler/asm.h visualizer/scene.h visualizer/ui.h $(RAYLIB_DIR)/src/libraylib.a $(RAYGUI_DIR)/src/raygui.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(RAYLIB_DIR)/src -I$(RAYGUI_DIR)/src -o $@ $(LAB_SOURCES) $(RAYLIB_DIR)/src/libraylib.a $(GRAPHICS_LIBS)

visualizer: $(VISUALIZER) $(BIN_DIR)/demo.bin

clean:
	rm -rf $(BIN_DIR)
