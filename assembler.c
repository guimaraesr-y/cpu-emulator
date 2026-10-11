#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "opcodes.h"
#include "variables.h"

#define MAX_INSTRUCTIONS (MEMORY_SIZE / INSTRUCTION_SIZE)
#define MAX_LABELS MAX_INSTRUCTIONS
#define MAX_NAME 64
#define MAX_LINE 512

typedef struct {
    char name[MAX_NAME];
    uint16_t address;
} Label;

typedef struct {
    uint8_t opcode;
    uint8_t arg1;
    uint8_t arg2;
    char target[MAX_NAME];
    int target_is_label;
    size_t line;
} Instruction;

typedef struct {
    Instruction instructions[MAX_INSTRUCTIONS];
    Label labels[MAX_LABELS];
    size_t instruction_count;
    size_t label_count;
    const char *source;
} Assembly;

static int error_at(const Assembly *assembly, size_t line, const char *format, ...) {
    fprintf(stderr, "%s:%zu: ", assembly->source, line);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    return 0;
}

static char *trim(char *text) {
    while (isspace((unsigned char)*text)) {
        text++;
    }
    size_t length = strlen(text);
    while (length > 0 && isspace((unsigned char)text[length - 1])) {
        text[--length] = '\0';
    }
    return text;
}

static int is_identifier(const char *text) {
    if (!isalpha((unsigned char)*text) && *text != '_') {
        return 0;
    }
    for (text++; *text; text++) {
        if (!isalnum((unsigned char)*text) && *text != '_') {
            return 0;
        }
    }
    return 1;
}

static int parse_number(const char *text, long *value) {
    if (*text == '\0') {
        return 0;
    }
    const char *digits = (*text == '-' || *text == '+') ? text + 1 : text;
    const int base = digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X') ? 0 : 10;
    char *end;
    errno = 0;
    *value = strtol(text, &end, base);
    return errno == 0 && end != text && *end == '\0';
}

static int parse_register(const char *text, uint8_t *reg) {
    if (strlen(text) != 2 || (text[0] != 'R' && text[0] != 'r') ||
        text[1] < '0' || text[1] >= '0' + NUM_REGISTERS) {
        return 0;
    }
    *reg = (uint8_t)(text[1] - '0');
    return 1;
}

static int find_label(const Assembly *assembly, const char *name) {
    for (size_t i = 0; i < assembly->label_count; i++) {
        if (strcmp(assembly->labels[i].name, name) == 0) {
            return (int)assembly->labels[i].address;
        }
    }
    return -1;
}

static int add_label(Assembly *assembly, const char *name, size_t line) {
    if (!is_identifier(name) || strlen(name) >= MAX_NAME) {
        return error_at(assembly, line, "invalid label: '%s'", name);
    }
    if (find_label(assembly, name) >= 0) {
        return error_at(assembly, line, "duplicate label: '%s'", name);
    }
    if (assembly->label_count >= MAX_LABELS) {
        return error_at(assembly, line, "too many labels");
    }
    Label *label = &assembly->labels[assembly->label_count++];
    strcpy(label->name, name);
    label->address = (uint16_t)(assembly->instruction_count * INSTRUCTION_SIZE);
    return 1;
}

static int parse_line(Assembly *assembly, char *line, size_t line_number) {
    /* The assembler accepts ';' and '#' as end-of-line comments. */
    char *comment = strpbrk(line, ";#");
    if (comment != NULL) {
        *comment = '\0';
    }
    char *cursor = trim(line);
    if (*cursor == '\0') {
        return 1;
    }

    /* Accept labels on their own line or before an instruction. */
    while (*cursor != '\0') {
        char *colon = strchr(cursor, ':');
        char *space = strpbrk(cursor, " \t");
        if (colon == NULL || (space != NULL && space < colon)) {
            break;
        }
        *colon = '\0';
        if (!add_label(assembly, cursor, line_number)) {
            return 0;
        }
        cursor = trim(colon + 1);
    }
    if (*cursor == '\0') {
        return 1;
    }
    if (assembly->instruction_count >= MAX_INSTRUCTIONS) {
        return error_at(assembly, line_number, "program exceeds %d bytes of memory", MEMORY_SIZE);
    }

    char *space = strpbrk(cursor, " \t");
    char *operands = space != NULL ? trim(space + 1) : cursor + strlen(cursor);
    if (space != NULL) {
        *space = '\0';
    }
    for (char *c = cursor; *c; c++) {
        *c = (char)tolower((unsigned char)*c);
    }

    Instruction instruction = {0};
    instruction.line = line_number;
    if (strcmp(cursor, "mov") == 0) instruction.opcode = OPCODE_MOV;
    else if (strcmp(cursor, "add") == 0) instruction.opcode = OPCODE_ADD;
    else if (strcmp(cursor, "sub") == 0) instruction.opcode = OPCODE_SUB;
    else if (strcmp(cursor, "xor") == 0) instruction.opcode = OPCODE_XOR;
    else if (strcmp(cursor, "jmp") == 0) instruction.opcode = OPCODE_JMP;
    else if (strcmp(cursor, "hlt") == 0) instruction.opcode = OPCODE_HLT;
    else return error_at(assembly, line_number, "unknown instruction '%s'", cursor);

    if (instruction.opcode == OPCODE_HLT) {
        if (*operands != '\0') {
            return error_at(assembly, line_number, "HLT takes no operands");
        }
    } else if (instruction.opcode == OPCODE_JMP) {
        if (*operands == '\0' || strchr(operands, ' ') || strchr(operands, '\t') ||
            strchr(operands, ',')) {
            return error_at(assembly, line_number, "JMP expects one byte address or label");
        }
        long address;
        if (parse_number(operands, &address)) {
            if (address < 0 || address >= MEMORY_SIZE || address % INSTRUCTION_SIZE != 0) {
                return error_at(assembly, line_number, "invalid JMP byte address '%s'", operands);
            }
            instruction.arg1 = (uint8_t)(address & 0xff);
            instruction.arg2 = (uint8_t)((address >> 8) & 0xff);
        } else {
            if (!is_identifier(operands) || strlen(operands) >= MAX_NAME) {
                return error_at(assembly, line_number, "invalid JMP target '%s'", operands);
            }
            strcpy(instruction.target, operands);
            instruction.target_is_label = 1;
        }
    } else {
        char *comma = strchr(operands, ',');
        if (comma == NULL) {
            return error_at(assembly, line_number, "instruction requires two comma-separated operands");
        }
        *comma = '\0';
        char *lhs = trim(operands);
        char *rhs = trim(comma + 1);
        if (*lhs == '\0' || *rhs == '\0' || strchr(rhs, ',') ||
            !parse_register(lhs, &instruction.arg1)) {
            return error_at(assembly, line_number, "invalid operands/register");
        }
        if (instruction.opcode == OPCODE_MOV) {
            long immediate;
            if (!parse_number(rhs, &immediate) || immediate < -128 || immediate > 255) {
                return error_at(assembly, line_number, "MOV immediate must be -128..255");
            }
            instruction.arg2 = (uint8_t)immediate;
        } else if (!parse_register(rhs, &instruction.arg2)) {
            return error_at(assembly, line_number, "second operand must be R0..R7");
        }
    }

    assembly->instructions[assembly->instruction_count++] = instruction;
    return 1;
}

static int resolve_jumps(Assembly *assembly) {
    const size_t program_size = assembly->instruction_count * INSTRUCTION_SIZE;
    for (size_t i = 0; i < assembly->instruction_count; i++) {
        Instruction *instruction = &assembly->instructions[i];
        if (instruction->opcode != OPCODE_JMP) {
            continue;
        }
        int target = (int)((unsigned)instruction->arg1 | ((unsigned)instruction->arg2 << 8));
        if (instruction->target_is_label) {
            target = find_label(assembly, instruction->target);
            if (target < 0) {
                return error_at(assembly, instruction->line, "unknown label '%s'", instruction->target);
            }
            instruction->arg1 = (uint8_t)(target & 0xff);
            instruction->arg2 = (uint8_t)((target >> 8) & 0xff);
        }
        if ((size_t)target >= program_size || target % INSTRUCTION_SIZE != 0) {
            return error_at(assembly, instruction->line, "JMP target %d is not a loaded instruction", target);
        }
    }
    return 1;
}

/*
 * Resolve existing paths before opening the output for writing.
 * stat() follows symlinks, so this also catches hardlinks and path aliases.
 */
static int output_references_source(const char *source, const char *output) {
#ifdef _WIN32
    /* Windows CRT stat() does not provide a unique inode. Compare file IDs. */
    HANDLE input = CreateFileA(source, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (input == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Cannot inspect assembly source (Windows error %lu)\n", GetLastError());
        return -1;
    }
    HANDLE target = CreateFileA(output, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (target == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        CloseHandle(input);
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) return 0;
        fprintf(stderr, "Cannot inspect bytecode output (Windows error %lu)\n", error);
        return -1;
    }
    BY_HANDLE_FILE_INFORMATION a, b;
    int valid = GetFileInformationByHandle(input, &a) && GetFileInformationByHandle(target, &b);
    CloseHandle(input);
    CloseHandle(target);
    if (!valid) { fprintf(stderr, "Cannot inspect Windows file identity\n"); return -1; }
    return a.dwVolumeSerialNumber == b.dwVolumeSerialNumber &&
           a.nFileIndexHigh == b.nFileIndexHigh && a.nFileIndexLow == b.nFileIndexLow;
#else
    struct stat source_info;
    struct stat output_info;

    if (stat(source, &source_info) != 0) {
        perror("Cannot inspect assembly source");
        return -1;
    }
    if (stat(output, &output_info) != 0) {
        if (errno == ENOENT) {
            return 0;
        }
        perror("Cannot inspect bytecode output");
        return -1;
    }
    return source_info.st_dev == output_info.st_dev &&
           source_info.st_ino == output_info.st_ino;
#endif
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source.asm> <program.bin>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], argv[2]) == 0) {
        fprintf(stderr, "Input and output paths must differ\n");
        return EXIT_FAILURE;
    }
    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("Cannot open assembly source");
        return EXIT_FAILURE;
    }
    const int aliases_source = output_references_source(argv[1], argv[2]);
    if (aliases_source != 0) {
        if (aliases_source > 0) {
            fprintf(stderr, "Input and output must not reference the same file\n");
        }
        fclose(input);
        return EXIT_FAILURE;
    }

    Assembly assembly = {0};
    assembly.source = argv[1];
    char line[MAX_LINE];
    size_t line_number = 0;
    int ok = 1;
    while (fgets(line, sizeof(line), input) != NULL) {
        line_number++;
        const size_t length = strlen(line);
        if (length == sizeof(line) - 1 && line[length - 1] != '\n') {
            ok = error_at(&assembly, line_number, "source line is too long");
            break;
        }
        if (!parse_line(&assembly, line, line_number)) {
            ok = 0;
            break;
        }
    }
    if (ferror(input)) {
        perror("Cannot read assembly source");
        ok = 0;
    }
    if (fclose(input) != 0) {
        perror("Cannot close assembly source");
        ok = 0;
    }
    if (!ok) {
        return EXIT_FAILURE;
    }
    if (assembly.instruction_count == 0) {
        error_at(&assembly, 1, "program has no instructions");
        return EXIT_FAILURE;
    }
    if (!resolve_jumps(&assembly)) {
        return EXIT_FAILURE;
    }

    uint8_t bytecode[MEMORY_SIZE];
    for (size_t i = 0; i < assembly.instruction_count; i++) {
        const Instruction *inst = &assembly.instructions[i];
        bytecode[i * INSTRUCTION_SIZE] = inst->opcode;
        bytecode[i * INSTRUCTION_SIZE + 1] = inst->arg1;
        bytecode[i * INSTRUCTION_SIZE + 2] = inst->arg2;
    }

    FILE *output = fopen(argv[2], "wb");
    if (output == NULL) {
        perror("Cannot create bytecode output");
        return EXIT_FAILURE;
    }
    const size_t size = assembly.instruction_count * INSTRUCTION_SIZE;
    ok = fwrite(bytecode, 1, size, output) == size;
    if (fclose(output) != 0) {
        ok = 0;
    }
    if (!ok) {
        perror("Cannot write bytecode");
        remove(argv[2]);
        return EXIT_FAILURE;
    }
    printf("Assembled %zu instruction(s) (%zu bytes)\n", assembly.instruction_count, size);
    return EXIT_SUCCESS;
}
