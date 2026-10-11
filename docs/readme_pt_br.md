# 🚀 Emulador de CPU e Assembler

[English README](../README.md) · [Especificação da ISA v1](./isa-v1.md)

Um emulador **educacional** de CPU e um assembler em C. Implementa oito registradores de 8 bits, 1.024 bytes de memória, saltos por endereço de byte e seis instruções (`MOV`, `ADD`, `SUB`, `JMP`, `XOR`, `HLT`). É um modelo funcional de instruções, sem simular circuitos físicos ou uma CPU comercial real.

A [especificação ISA v1](./isa-v1.md) é a referência para opcodes, tamanho das instruções, encoding, overflow e validações.

## Compilar e executar

Pré-requisitos: compilador C99 (GCC/Clang), Make e Python 3 para testes.

```sh
make
./bin/assembler example_programs/program1.asm program.bin
./bin/emulator program.bin
make test
```

O Makefile gera `bin/assembler` e `bin/emulator`, evitando o conflito com o diretório `assembler/`. `make clean` remove os executáveis gerados.

## Programa de exemplo

```asm
mov R0, 10
mov R1, 20
add R0, R1
sub R1, R0
jmp fim
mov R2, 99     ; instrução ignorada pelo salto
fim: hlt
```

O arquivo `example_programs/program1.asm` usa o rótulo `end` no lugar de `fim`, com a mesma lógica. O resultado é **R0 = 30**, **R1 = 246** e os demais registradores zerados. Como os registradores são **unsigned de 8 bits**, `20 - 30` resulta em `246` por overflow modular de 256. Valores negativos no `MOV` são codificados em complemento de dois: `mov R0, -1` armazena `255` (`0xFF`).

O exemplo `example_programs/xor_program.asm` calcula `3 XOR 5 = 6` em `R0`.

## Conjunto de instruções

Cada instrução ocupa **exatamente 3 bytes**, incluindo `JMP` e `HLT`. `JMP` utiliza endereço de **byte**, codificado em 16 bits little-endian (aceita endereços acima de 255). Um rótulo como `fim:` é resolvido pelo assembler, inclusive em saltos para a frente.

| Instrução | Efeito |
| --- | --- |
| `mov R0, 10` | Armazena um imediato de 8 bits |
| `add R0, R1` | Soma módulo 256 |
| `sub R0, R1` | Subtração módulo 256 |
| `xor R0, R1` | Operação XOR bit a bit |
| `jmp fim` | Salta para rótulo ou endereço válido de instrução |
| `hlt` | Encerra a execução |

O assembler rejeita sintaxe, registradores e endereços inválidos; o emulador rejeita bytecode incompleto, opcodes inválidos, saltos fora do programa e acessos indevidos, retornando código diferente de zero. A execução via CLI possui limite de 100.000 instruções para detectar loops infinitos. Os testes em `make test` cobrem fluxo completo (ASM → bytes → CPU), exemplos e casos de erro.

## Laboratório 3D nativo — Fase 2

O laboratório desktop usa este núcleo real, com raylib/raygui em C. Oferece play/pause/passo/reset/velocidade, editor Assembly com o mesmo parser da CLI, abertura e salvamento de arquivos, RAM paginada, câmera orbital, registradores em decimal/hex/binário e animações didáticas de FETCH/DECODE/EXECUTE/WRITEBACK.

```sh
cmake -S . -B build -DBUILD_VISUALIZER=ON
cmake --build build --config Release --parallel
./build/bin/cpu-lab  # Windows: build\bin\cpu-lab.exe
```

Consulte [instalação, controles e verificação](native-lab.md). O build padrão da CLI continua independente das bibliotecas gráficas.
