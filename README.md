# 🚀 CPU Emulator & Assembler

[Leia em português](./docs/readme_pt_br.md) · [ISA v1 specification](./docs/isa-v1.md)

A small, **educational** CPU and assembler written in C. It implements a deterministic instruction set with eight 8-bit registers, 1,024 bytes of RAM, byte-addressed jumps, and six instructions (`MOV`, `ADD`, `SUB`, `JMP`, `XOR`, `HLT`). It models instruction behavior, not physical circuits or a real commercial processor.

The **ISA v1 specification** defines every opcode, encoding, arithmetic rule, and error condition: [docs/isa-v1.md](./docs/isa-v1.md).

## Build and run

Requires a C99 compiler (`cc`/GCC/Clang), Make, and Python 3 for tests.

```sh
make
./bin/assembler example_programs/program1.asm program.bin
./bin/emulator program.bin
make test
```

`make` builds executables into `bin/`, avoiding the name conflict with the `assembler/` source directory. `make clean` removes generated executables. Use `make CFLAGS='-std=c99 -O2 -Wall -Wextra -Wpedantic'` to customize compiler flags.

## Example program

```asm
mov R0, 10
mov R1, 20
add R0, R1
sub R1, R0
jmp end       ; label resolves to the HLT byte address
mov R2, 99    ; skipped
end: hlt
```

`example_programs/program1.asm` outputs:

```text
HLT after 6 instruction(s)
Final State of Registers:
R0: 30
R1: 246
R2: 0
R3: 0
R4: 0
R5: 0
R6: 0
R7: 0
```

The registers are **unsigned 8-bit** values. `R1` wraps to **246** because `20 - 30` is `-10` modulo 256. Use `mov R0, -1` to store the bit pattern `0xFF` (displayed as `255`). The second example, `example_programs/xor_program.asm`, computes `3 XOR 5 = 6` in `R0`.

## Instruction set

All instructions have a fixed **3-byte encoding** (opcode and two operand bytes). `JMP` uses a 16-bit little-endian **byte address**, so its target can be above 255; labels avoid hand-counting bytes.

| Syntax | Effect |
| --- | --- |
| `mov R0, 10` | Put an immediate 8-bit value into a register |
| `add R0, R1` | Unsigned addition modulo 256 |
| `sub R0, R1` | Unsigned subtraction modulo 256 |
| `xor R0, R1` | Bitwise XOR |
| `jmp end` | Jump to a label or an aligned byte address |
| `hlt` | Stop execution |

The assembler rejects invalid operands and labels; the emulator rejects malformed or illegal bytecode with a nonzero exit code. The CLI also limits execution to 100,000 instructions to catch infinite loops. Automated end-to-end tests cover all six opcodes, jumps beyond the first 256 bytes, wrapping, and failure cases.

## Native 3D laboratory

The Phase 2 desktop laboratory is written in C with raylib/raygui. It visualizes the real CPU state through fetch/decode/execute/writeback presentation phases, with play/pause/step/reset/speed controls, a native Assembly editor using the CLI's parser, paged RAM inspection, orbit/zoom camera and decimal/hex/binary registers.

```sh
cmake -S . -B build -DBUILD_VISUALIZER=ON
cmake --build build --config Release --parallel
./build/bin/cpu-lab  # Windows: build\bin\cpu-lab.exe
```

See [native laboratory setup, controls and verification](docs/native-lab.md) for platform dependencies and the Make workflow. Contributions and reviews are welcome.
