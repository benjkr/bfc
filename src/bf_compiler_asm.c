#include "bf.h"
#include "nob.h"
#include <stdio.h>

#define TEMP_SRC_FILE "./.temp.s"
#define BF_ARRAY_LEN "655360"
#define BF_ARRAY "MEMORY"
#define BF_DATA_POINTER "rcx"
#define BF_CURRENT "BYTE [" BF_DATA_POINTER "]"
#define BF_LOAD_MEM_ADDRESS "lea " BF_DATA_POINTER ",[rel " BF_ARRAY "]"

#define BF_INC "add " BF_CURRENT ","
#define BF_DEC "sub " BF_CURRENT ","
#define BF_FORWORD "add " BF_DATA_POINTER ","
#define BF_BACK "sub " BF_DATA_POINTER ","
#define BF_IN                                                                                                          \
    "\tpush " BF_DATA_POINTER "\n"                                                                                     \
    "\tcall getchar\n"                                                                                                 \
    "\tpop " BF_DATA_POINTER "\n"                                                                                      \
    "\tmov " BF_CURRENT ", al\n"
#define BF_OUT                                                                                                         \
    "\tpush " BF_DATA_POINTER "\n"                                                                                     \
    "\tmov dil, " BF_CURRENT "\n"                                                                                      \
    "\tcall putchar\n"                                                                                                 \
    "\tpop " BF_DATA_POINTER "\n"
#define BF_LOOP_CHECK "cmp " BF_CURRENT ",0"

static const char *file_template = "extern putchar\n"
                                   "extern getchar\n"
                                   "section .bss\n"
                                   "    " BF_ARRAY " resb " BF_ARRAY_LEN "\n"
                                   "\n"
                                   "section .text\n"
                                   "    global main\n"
                                   "main:\n"
                                   "\t" BF_LOAD_MEM_ADDRESS "\n";
static const char *file_template_end = "\tmov rax, 60\n"
                                       "\txor rdi, rdi\n"
                                       "\tsyscall\n";

static const char *INSTRUCTION_TO_SRC[] = {
    [TYPE_FORWORD] = BF_FORWORD, [TYPE_BACK] = BF_BACK, [TYPE_INC] = BF_INC,           [TYPE_DEC] = BF_DEC,
    [TYPE_OUT] = BF_OUT,         [TYPE_IN] = BF_IN,     [TYPE_LOOP_START] = "<ERROR>", [TYPE_LOOP_END] = "<ERROR>",
};

void bf_compile_asm(Lexer *lexer, char *out_file)
{
    compile_start_time = nanos_since_unspecified_epoch();
    String_Builder sb = {0};
    sb_append_cstr(&sb, file_template);

    for (lexer->ip = 0; lexer->ip < lexer->tokens.count; lexer->ip++)
    {
        Token token = lexer->tokens.items[lexer->ip];
        switch (token.t)
        {
        case TYPE_DEC:
        case TYPE_INC:
        case TYPE_FORWORD:
        case TYPE_BACK:
            sb_append_cstr(&sb, temp_sprintf("\t%s%d\n", INSTRUCTION_TO_SRC[token.t], token.d.repeats));
            break;
        case TYPE_LOOP_START:
            sb_append_cstr(&sb, temp_sprintf("\tjmp while%ld\n", token.d.loop_end_ip));
            sb_append_cstr(&sb, temp_sprintf("while%zu:\n", lexer->ip));
            break;
        case TYPE_LOOP_END:
            sb_append_cstr(&sb, temp_sprintf("while%zu:\n", lexer->ip));
            sb_append_cstr(&sb, temp_sprintf("\t%s\n", BF_LOOP_CHECK));
            sb_append_cstr(&sb, temp_sprintf("\tjnz while%ld\n", token.d.loop_start_ip));
            break;
        default:
            sb_append_cstr(&sb, INSTRUCTION_TO_SRC[token.t]);
            break;
        }
        temp_reset();
    }
    sb_append_cstr(&sb, file_template_end);
    nob_write_entire_file(TEMP_SRC_FILE, sb.items, sb.count);

    Cmd cmd = {0};
    cmd_append(&cmd, "nasm", "-f", "elf64", "-o", TEMP_SRC_FILE ".o", "-O3", TEMP_SRC_FILE);
    if (!nob_cmd_run(&cmd)) abort();
    cmd_append(&cmd, "gcc", TEMP_SRC_FILE ".o", "-O3", "-o", out_file, "-no-pie");
    if (!nob_cmd_run(&cmd)) abort();
    nob_delete_file(TEMP_SRC_FILE);
    nob_delete_file(TEMP_SRC_FILE ".o");

    compile_stop_time = nanos_since_unspecified_epoch();
}