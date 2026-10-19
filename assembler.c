#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "assembler/asm.h"

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
    if (argc != 3) { fprintf(stderr, "Usage: %s <source.asm> <program.bin>\n", argv[0]); return 1; }
    if (!strcmp(argv[1],argv[2])) { fprintf(stderr, "Input and output paths must differ\n"); return 1; }
    FILE *input = fopen(argv[1], "r");
    if (!input) { perror("Cannot open assembly source"); return 1; }
    int alias = output_references_source(argv[1],argv[2]);
    if (alias) { if (alias>0) fprintf(stderr,"Input and output must not reference the same file\n"); fclose(input); return 1; }
    size_t capacity=4096, length=0;
    char *source=malloc(capacity);
    if (!source) { fclose(input); return 1; }
    int byte;
    while ((byte=fgetc(input)) != EOF) {
        if (length+1 >= capacity) {
            size_t next=capacity*2;
            if (next<=capacity) { free(source); fclose(input); return 1; }
            char *grown=realloc(source,next);
            if (!grown) { free(source); fclose(input); return 1; }
            source=grown; capacity=next;
        }
        source[length++]=(char)byte;
    }
    source[length]=0;
    int failed=ferror(input); if (fclose(input)) failed=1;
    if (failed) { free(source); perror("Cannot read assembly source"); return 1; }
    AsmResult result;
    int ok=assemble_text(source,&result); free(source);
    if (!ok) { fprintf(stderr,"%s:%zu: %s\n",argv[1],result.error_line,result.error); return 1; }
    FILE *output=fopen(argv[2],"wb");
    if (!output) { perror("Cannot create bytecode output"); return 1; }
    ok=fwrite(result.bytes,1,result.size,output)==result.size;
    if (fclose(output)) ok=0;
    if (!ok) { perror("Cannot write bytecode"); remove(argv[2]); return 1; }
    printf("Assembled %zu instruction(s) (%zu bytes)\n",result.size/3,result.size);
    return 0;
}
