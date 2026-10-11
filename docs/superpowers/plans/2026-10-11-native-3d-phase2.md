# Native interactive laboratory implementation plan

**Goal:** Deliver Phase 2 of issue #2 after merging PR #4.
**Architecture:** Extract existing parser/encoder into a graphics-free assembler library shared by CLI and application. A separate application controller owns playback, editable/compiled source, source mapping, file I/O and playback limits. Scene continues to read CPU snapshots.
**Stack:** C99, raylib 5.5, raygui 4.0, Make/CMake.

- [x] Extract `assembler/asm.h/.c` without duplicating ISA behavior; keep all existing CLI tests passing. Return structured diagnostics and source-line map.
- [x] Add `visualizer/app.h/.c`: pause freezes animation, resume uses the same trace, manual step completes a paused instruction, speed adjusts presentation, reset cancels playback and automatic playback has a 100,000-step limit.
- [x] Add native source editor, load/save `.asm` and load `.bin`, examples, dirty-source guard, line highlighting and all-RAM inspection.
- [x] Test source maps, CRLF, syntax errors, file behavior, pause/resume/reset, dirty-source guard and speed. Strict build, native GL smoke tests and Linux UBSan regression tests.

**Review focus:** no stale source execution after failed open/compile; no duplicate instruction at pause/resume; no source mapping when dirty or binary-only; bounded file reads and editor capacity; loading/reset clears in-flight presentation.

**Scope limit:** editor is basic (ASCII keyboard entry, clipboard, whole-source selection; no undo), source pane clips long lines. Shader/camera polish remains Phase 3.
