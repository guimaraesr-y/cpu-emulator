# Native 3D CPU laboratory (Phase 2)

The laboratory runs locally in C99 using **raylib 5.5**, its raymath module, and **raygui 4.0**. It loads bytecode produced by the existing assembler. The default program is `example_programs/program1.asm`, assembled into `demo.bin` alongside the executable at build time.

## CMake (Windows/Linux)

Requires CMake 3.16+, Git, and a C99 compiler. Windows: use a recent MinGW-w64 distribution (for example w64devkit) or a Visual Studio developer shell. Linux: install the OpenGL/X11 development packages raylib needs:

```sh
sudo apt-get install build-essential cmake git libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
cmake -S . -B build -DBUILD_VISUALIZER=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build --output-on-failure -C Release
./build/bin/cpu-lab
```

On Windows run `build\bin\cpu-lab.exe`. CMake downloads dependencies pinned to the release commit IDs, not floating branches. Configure with `BUILD_VISUALIZER=OFF` (the default) to build/test the CLI without graphics or dependency downloads. The first graphical configure requires network access; subsequent builds use the local cache.

## Make

Windows (run inside a MinGW-w64/w64devkit environment with `gcc`, `make`, `sh` and Git on PATH):

```powershell
powershell -ExecutionPolicy Bypass -File scripts/setup-graphics.ps1
make CC=gcc PYTHON=python test
make CC=gcc visualizer
.\bin\cpu-lab.exe
```

Linux:

```sh
git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git .deps/raylib
git clone --depth 1 --branch 4.0 https://github.com/raysan5/raygui.git .deps/raygui
make test visualizer
./bin/cpu-lab
```

`make` continues to build only the CLI tools. `make visualizer` additionally builds raylib and the laboratory. If switching architectures/toolchains, use a fresh dependency build or clean raylib's generated object files first.

## Controls and programs

| Action | Control |
| --- | --- |
| Execute one instruction with presentation phases | Step button / Space |
| Play or pause, freezing the current presentation phase | Play/pause button / P |
| Change presentation speed | Speed slider (0.25x–8x) |
| Assemble edited source | Assemble button / Ctrl+Enter |
| Cancel current presentation and reload the program | Reset button / R |
| Orbit | Right mouse drag over the scene |
| Zoom | Mouse wheel over the scene |
| Focus RAM / control / ALU / registers | 1 / 2 / 3 / 4 over the scene |
| Restore overview | Home over the scene |
| Quit | Escape / close window |

For another program:

```sh
./bin/assembler example_programs/xor_program.asm bin/xor.bin
./bin/cpu-lab bin/xor.bin
```

RAM bytes and PC use **byte addresses**, registers show decimal/hex/binary, and arithmetic wraps modulo 256. The example halts after six instructions with `PC = 21`, `R0 = 30`, `R1 = 246`, `R2 = 0`. Invalid bytecode produces a visible fault rather than a simulated successful operation. File opening/reading failures report to stderr before opening a window.

## State and animation

`visualizer/playback.c` captures the CPU immediately before and after exactly one `cpu_step`. The same core serves the CLI. Rendering reads the before snapshot during FETCH/DECODE/EXECUTE and the after snapshot during WRITEBACK/idle. Frame time advances presentation only. Repeated step input while busy is ignored; reset cancels playback. The trace retains the bytes, memory/state snapshots and changed-register mask. RAM is never animated as receiving a data store, since the ISA has no load/store instructions.

The four phases are a **didactic presentation of an atomic instruction**, not an electrical/cycle-accurate CPU. No invented CPU flags or pipeline behavior is shown. Cinematic shaders and advanced camera polish remain Phase 3 work.

## Interactive workspace

Without a filename the application opens the Addition Assembly example. Pick Addition/XOR/Loop to load an editable example. The editor accepts ASCII keyboard input, newlines, arrows, Home/End, Backspace/Delete, Tab and clipboard paste. Ctrl+A selects the entire source; Ctrl+C copies the source; Ctrl+X cuts it when selected. Mouse wheel scrolls the editor. It is a basic editor without undo or partial text selection; long lines are clipped to the pane width.

Edit the path field and use **Open .asm/.bin** to load a file, or **Save .asm** to write the editor contents. Dropping an `.asm` or `.bin` file onto the window is also supported. Existing source files are replaced when saved; loading another file/example replaces the editor contents. Windows CRLF is normalized for display. Binary files have no Assembly source map; loading them clears the source pane.

Assembly compilation uses `assembler/asm.c`, the same parser/encoder as the CLI. Errors are shown with line numbers. Editing stops playback and disables execution until a successful assembly; failed assembly preserves the previously loaded program, while opening an invalid source file invalidates the loaded program to prevent running stale code. Source mapping highlights the loaded instruction only while source matches the compiled snapshot.

Pause freezes presentation without executing again. Step during a paused instruction completes its current presentation; the next Step starts the next instruction. Reset reloads the compiled program without discarding source edits. Automatic playback stops after 100,000 instruction attempts and requires reset. The RAM tab pages through all 1024 bytes (128 per page), distinguishing program bytes, unused RAM and the fetched instruction.

## Verification

`make test` includes the existing end-to-end ISA checks and a graphics-independent playback test covering snapshots, repeated step, reset during animation, large frame deltas, unsigned wrapping, all six opcodes, 16-bit jumps and faults.

```sh
./bin/cpu-lab --smoke-test lab-smoke.png
python3 scripts/check-native.py ./bin/cpu-lab
```

The smoke mode opens a hidden native GL window, advances the assembled example, writes an execution-phase image on the third instruction and `lab-smoke.png.final.png`, prints the final CPU state and exits. If a short or faulting program never reaches that phase, the requested image captures its final state instead. Success requires the requested image to be captured and both exports to succeed. The filename may include an existing directory path. It returns nonzero on a CPU fault, failure to export images, or failure to halt within 100 instruction attempts. On headless Linux use `xvfb-run -a`.

The native check script opens hidden GL windows for the default example, XOR, short HLT programs, Assembly with CRLF/labels, Assembly errors and invalid-register/missing-HLT/truncated/oversized programs, checks exit status and final state, and verifies capture export to absolute paths and export failures. Application tests additionally exercise pause/resume, stepping a paused phase, speed, dirty-source blocking, source maps, file opening/saving and the loop limit.

Locally verified: strict Windows MinGW-w64 build, Windows CMake graphical build, 11 ISA tests plus playback tests on Windows, graphics smoke and fault renders on Windows, and CMake Release/UBSan CLI/playback tests on Linux. Linux graphical build is covered by the added CI job; macOS has not been validated.
