# CPU Emulator — ISA v1

This is a **functional educational CPU model**, not an electrical or cycle-accurate hardware simulation. Its only executable format is described below. The assembler and emulator share the constants in `opcodes.h` and `variables.h`.

## Machine and state

- **Registers:** eight unsigned **8-bit** registers, `R0`–`R7`, initialized to `0`. Arithmetic and XOR operate on these eight bits.
- **RAM:** 1,024 bytes, indexed from `0` through `1023`. At load time the program is copied into RAM starting at byte `0`; all remaining bytes are zeroed. There are no data load/store instructions in v1.
- **PC:** a byte address starting at `0`. It points to the opcode byte of the next instruction. Every normal instruction advances it by **3 bytes**; `JMP` sets it to its target. `HLT` advances PC by 3 and stops.
- **Program:** a nonempty series of complete 3-byte instructions, at most **1,023 bytes / 341 instructions** long. The final RAM byte remains unused by this encoding. The loader rejects incomplete and oversized files.
- **Integer semantics:** `MOV` uses an 8-bit immediate. Assembly literals are decimal or hexadecimal (`0x`); accepted range is `-128` to `255`. Negative literals encode their low 8 bits in two's complement, e.g. `mov R0, -1` stores **255** (`0xFF`). Registers display as **unsigned** values. `ADD` and `SUB` wrap modulo **256** (`250 + 10 = 4`, `20 - 30 = 246`). There are no flags or signed arithmetic in v1.

## Encoding

Every instruction occupies **exactly 3 bytes**: `opcode`, `operand 1`, `operand 2`. Addresses are **byte offsets**, not Assembly line numbers. `JMP` stores its target in **little-endian**, with the low byte before the high byte.

| Instruction | Opcode | Byte 1 | Byte 2 | Behavior |
| --- | --- | --- | --- | --- |
| `mov Rd, imm8` | `0x01` | destination register `0..7` | immediate bits `0..255` | `Rd = imm8` |
| `add Rd, Rs` | `0x02` | destination register | source register | `Rd = (Rd + Rs) mod 256` |
| `sub Rd, Rs` | `0x03` | destination register | source register | `Rd = (Rd - Rs) mod 256` |
| `jmp target` | `0x04` | address bits `7..0` | address bits `15..8` | `PC = target` |
| `xor Rd, Rs` | `0x05` | destination register | source register | `Rd = Rd XOR Rs` |
| `hlt` | `0xFF` | `0x00` | `0x00` | advance PC, stop |

The assembler accepts labels (`loop:`), references (`jmp loop`), or numeric byte addresses (`jmp 18`). Labels can appear on their own line or before an instruction. Labels are case-sensitive; instructions and register names are case-insensitive. `;` and `#` start comments. Extra operands or trailing tokens are errors.

For example:

```asm
mov R0, 10   ; address 0, bytes 01 00 0A
mov R1, 20   ; address 3, bytes 01 01 14
add R0, R1   ; address 6, bytes 02 00 01
jmp done     ; address 9, bytes 04 0F 00
mov R0, 99   ; address 12, skipped
 done: hlt   ; address 15, bytes FF 00 00
```

`JMP` accepts **only instruction-aligned byte addresses** (`target % 3 == 0`) within the **loaded program** (`0 <= target < program_size`). Its two-byte encoding can reach every valid instruction in the 1,024-byte RAM. Jumping to memory beyond the program, even if zero-filled, is an error. An aligned target containing an invalid opcode will fail when fetched.

## Execution errors

The assembler fails with a nonzero exit code and does not create an output file for unknown opcodes, invalid syntax, duplicate/unknown labels, invalid registers or values, invalid jumps, oversize programs and I/O failures. It does not perform static control-flow analysis or require a reachable `HLT`.

The emulator fails with a nonzero exit code for malformed program length, unknown opcodes, invalid registers, illegal jump targets, nonzero `HLT` padding, PC beyond loaded code, and I/O failures. On a fault, the PC remains at the faulting instruction. A CLI safety limit of **100,000 executed instructions** catches infinite loops. Neither error nor step limit prints a successful final register state.

The `cpu/cpu.h` API provides a zeroed reset, bounded bytecode load, and deterministic single-instruction `cpu_step`; no console output is embedded in the CPU core. The [native 3D laboratory](native-lab.md) captures before/after snapshots through a separate playback adapter and renders them in C with raylib/raygui.
