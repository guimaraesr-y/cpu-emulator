# CPU Emulator and Assembler

## Overview

This project is a simple CPU emulator and assembler written in C. It demonstrates how to simulate a minimal CPU with basic instructions and how to assemble textual instructions into machine-readable bytecode. This is a great learning tool for understanding how assembly languages and CPU instruction sets work at a low level.

## Features

- **CPU Emulator:**
  - Supports a set of basic operations: `mov`, `add`, `sub`, `jmp`, and `hlt`.
  - Executes instructions stored in memory.
  - Simulates 8 general-purpose registers (`R0` to `R7`) and a memory space of 1024 bytes.
- **Assembler:**
  - Converts textual instructions into binary bytecode.
  - Supports instructions like `mov`, `add`, `sub`, `jmp`, and `hlt`.
- Modular design for easy extension of the instruction set.

## Prerequisites

- A C compiler (e.g., `gcc`).
- Basic knowledge of C programming and CPU architecture.

## Getting Started

### Cloning the Repository

```bash
git clone https://github.com/guimaraesr-y/cpu-emulator
cd cpu-emulator
```

### Building the Project

To compile the assembler and emulator:

```bash
gcc -o assembler assembler.c
gcc -o emulator emulator.c
```

or use Makefile:

```bash
make
```

### Running the Project

#### Assembling Instructions

Create a text file (e.g., `program.asm`) with the following instructions:

```asm
mov R0, 10
mov R1, 20
add R0, R1
sub R1, R0
jmp 5
hlt
```

Run the assembler to generate the bytecode:

```bash
./assembler program.asm program.bin
```

#### Running the Emulator

Execute the emulator with the assembled bytecode:

```bash
./emulator program.bin
```

### Expected Output

After running the emulator, the state of the registers will be displayed:

```plaintext
R0: 30
R1: -10
R2: 0
R3: 0
R4: 0
R5: 0
R6: 0
R7: 0
```

## Instruction Set

| Instruction | Description                        | Example          |
|-------------|------------------------------------|------------------|
| `mov`       | Moves a value into a register      | `mov R0, 10`     |
| `add`       | Adds values of two registers       | `add R0, R1`     |
| `sub`       | Subtracts values of two registers  | `sub R1, R0`     |
| `jmp`       | Jumps to a specific memory address | `jmp 5`          |
| `xor`       | Performs a bitwise XOR operation   | `xor R0, R1`     |
| `hlt`       | Halts the program execution        | `hlt`            |

## Contributing

Contributions are welcome! Feel free to open issues or submit pull requests to improve the project.

## Acknowledgments

This project is inspired by basic CPU architecture and assembly language principles. Special thanks to anyone who provided feedback or suggestions during its development :)
