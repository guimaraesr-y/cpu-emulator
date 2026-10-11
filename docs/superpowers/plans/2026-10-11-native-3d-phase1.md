# Native 3D CPU laboratory implementation plan

> **For agentic workers:** Use native execution in this session, as authorized by the user. Track tasks with checkboxes.

**Goal:** Implement issue #2 Phase 1: native C desktop scene and deterministic instruction-by-instruction visualization.
**Architecture:** `playback` captures the existing CPU before/after a single `cpu_step`. Scene and GUI read a visible snapshot; playback timing never executes instructions. Reset cancels the animation and reloads the program.
**Tech Stack:** C99, raylib 5.5, raymath, raygui 4.0, Make/CMake.
**Spec:** https://github.com/guimaraesr-y/cpu-emulator/issues/2

## Global constraints
- Eight uint8 registers, 1024-byte RAM, fixed 3-byte ISA v1; no data load/store operations.
- Preserve CLI behavior and assembler semantics.
- Fetch/decode/execute/writeback are presentation phases, not physical cycles.

## Review focus
- Invalid register/opcode/jump must not cause unsafe trace formatting or animate a successful operation.
- PC beyond loaded code must not read three bytes out of bounds.
- Repeated step requests during animation must not execute another instruction.
- Reset during animation must cancel it and restore zeroed registers.
- A large frame delta must complete presentation without consuming additional instructions.

## Tasks
- [x] Add `visualizer/playback.h/.c`: `lab_load`, `lab_reset`, `lab_step`, `lab_update`, `lab_visible`; snapshots plus changed-register mask. Verify `tests/test_playback.c` covers the review focus, all six opcodes, wraparound and 16-bit jumps.
- [x] Add `scene.h/.c`: CPU board, RAM/PC/control/ALU/register geometry, instruction-specific bus pulses and mouse orbit/zoom/focus. Add `ui.h/.c`: raygui step/reset, bytes, decimal/hex/binary registers, status and explanations. Add `main.c`: bounded binary load, default assembled example and window lifecycle.
- [x] Add pinned dependency setup, optional graphics Make target and CMake option. Generate demo with existing assembler; retain graphics-free CLI tests.
- [x] Run strict build and ISA/playback tests, render native smoke captures for arithmetic and halted state, inspect captures, document build and controls.

## Verification results
- Windows: strict Make build; 11 ISA tests and playback checks passed.
- Windows: CMake graphical Release build and both CTest suites passed.
- Native GL: default demo and XOR halted correctly; invalid register, missing HLT, truncated and oversized programs displayed faults and exported captures without crashing.
- Linux: CMake Release and UBSan builds; both CTest suites passed.
- Visually inspected arithmetic and halted-state captures. Full Linux graphical execution and macOS remain unverified locally.
- Fixed Windows assembler file-identity comparison discovered by the existing output-replacement test (CRT inode values are not unique on Windows).
