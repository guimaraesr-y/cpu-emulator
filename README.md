# 🚀 CPU Emulator & Assembler

## 📝 About the Project

This project is a simple CPU emulator and assembler written in C. It demonstrates how to simulate a minimal CPU with basic instructions and how to assemble textual instructions into machine-readable bytecode. This is a great learning tool for understanding how assembly languages and CPU instruction sets work at a low level.

## ✨ Features

### 🖥️ CPU Emulator

✔️ Supports basic operations: `mov`, `add`, `sub`, `jmp`, `xor`, and `hlt`.<br>
✔️ Simulates **8 registers** (`R0` to `R7`).<br>
✔️ **1024 bytes** of memory.<br>
✔️ Executes instructions directly from memory.<br>

### ⚙️ Assembler

✔️ Converts textual instructions into **bytecode**.<br>
✔️ Supports the following commands: `mov`, `add`, `sub`, `jmp`, `xor`, `hlt`.<br>
✔️ Easy to extend with new instructions!<br>

---

## 📦 Prerequisites

- A C compiler (e.g., `gcc`).
- Basic knowledge of C programming and CPU architecture.

## 🚀 How to Use

### 🔹 Cloning the Repository

```bash
 git clone https://github.com/guimaraesr-y/cpu-emulator
 cd cpu-emulator
```

### 🔹 Compiling the Project

To compile the assembler and emulator:

```bash
gcc -o assembler assembler.c
gcc -o emulator emulator.c
```

or use Makefile:

```bash
make
```

### 🔹 Assembling an Assembly Program

Create a file `program.asm` with the following content:

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

### 🔹 Running the Emulator

Execute the emulator with the assembled bytecode:

```bash
./emulator program.bin
```

### 📌 Expected Output

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

## 🛠️ Instruction Set

| Instruction | Description                        | Example          |
|-------------|------------------------------------|------------------|
| `mov`       | Moves a value into a register      | `mov R0, 10`     |
| `add`       | Adds values of two registers       | `add R0, R1`     |
| `sub`       | Subtracts values of two registers  | `sub R1, R0`     |
| `jmp`       | Jumps to a specific memory address | `jmp 5`          |
| `xor`       | Performs a bitwise XOR operation   | `xor R0, R1`     |
| `hlt`       | Halts the program execution        | `hlt`            |

## 🤝 Contributing

Feel free to open **issues** or submit **pull requests** to improve the project. All contributions are welcome! 🛠️🚀

---

## 🙌 Acknowledgments

This project is inspired by the basics of **CPU architecture** and **assembly language**. Thanks to everyone who contributed ideas and feedback! 🎉
