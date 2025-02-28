# 🚀 Emulador de CPU & Assembler

## 📝 Sobre o Projeto

Este projeto é um emulador de CPU e assembler simples escrito em C. Ele demonstra como simular uma CPU minimalista com instruções básicas e como montar instruções textuais em código de máquina legível. Esta é uma ferramenta de aprendizado excelente para entender como linguagens de montagem e conjuntos de instruções de CPU funcionam em baixo nível.

## ✨ Funcionalidades

### 🖥️ Emulador de CPU

✔️ Suporta operações básicas: `mov`, `add`, `sub`, `jmp`, `xor` e `hlt`.<br>
✔️ Simula **8 registradores** (`R0` a `R7`).<br>
✔️ **1024 bytes** de memória.<br>
✔️ Executa instruções diretamente da memória.<br>

### ⚙️ Assembler

✔️ Converte instruções textuais em **código de máquina**.<br>
✔️ Suporta os seguintes comandos: `mov`, `add`, `sub`, `jmp`, `xor`, `hlt`.<br>
✔️ Fácil de estender com novas instruções!<br>

---

## 📦 Pré-requisitos

- Um compilador C (e.g., `gcc`).
- Conhecimento básico de programação C e arquitetura de CPU.

## 🚀 Como Usar

### 🔹 Clonando o Repositório

```bash
 git clone https://github.com/guimaraesr-y/cpu-emulator
 cd cpu-emulator
```

### 🔹 Compilando o Projeto

Para compilar o assembler e o emulador:

```bash
gcc -o assembler assembler.c
gcc -o emulator emulator.c
```

ou use Makefile:

```bash
make
```

### 🔹 Montando um Programa Assembly

Crie um arquivo `program.asm` com o seguinte conteúdo:

```asm
mov R0, 10
mov R1, 20
add R0, R1
sub R1, R0
jmp 5
hlt
```

Execute o assembler para gerar o código de máquina:

```bash
./assembler program.asm program.bin
```

### 🔹 Executando o Emulador

Execute o emulador com o código de máquina gerado:

```bash
./emulator program.bin
```

### 📌 Saída Esperada

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

| Instruction | Description                                  | Example          |
|-------------|----------------------------------------------|------------------|
| `mov`       | Move um valor para um registrador            | `mov R0, 10`     |
| `add`       | Adiciona valores de dois registradores       | `add R0, R1`     |
| `sub`       | Subtrai valores de dois registradores        | `sub R1, R0`     |
| `jmp`       | Salta para um endereço de memória específico | `jmp 5`          |
| `xor`       | Executa uma operação XOR bit a bit           | `xor R0, R1`     |
| `hlt`       | Interrompe a execução do programa            | `hlt`            |

## 🤝 Contribuindo

Sinta-se à vontade para abrir **issues** ou enviar **pull requests** para melhorar o projeto. Todas as contribuições são bem-vindas! 🛠️🚀

---

## 🙌 Agradecimentos

Este projeto foi inspirado nos conceitos básicos de **arquitetura de CPU** e **linguagem de montagem**. Agradeço a todos que contribuíram com ideias e feedback! 🎉
