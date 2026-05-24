#include "bf.h"
#include "nob.h"
#include <stdio.h>

#define TEMP_SRC_FILE "./.temp.c"
#define BF_ARRAY_LEN "655360"
#define BF_ARRAY "MEMORY"
#define BF_INSTRUCTION_POINTER "ip"
#define BF_DATA_POINTER "dp"
#define BF_CURRENT BF_ARRAY "[" BF_DATA_POINTER "]"

#define BF_INC BF_CURRENT "+="
#define BF_DEC BF_CURRENT "-="
#define BF_FORWORD BF_DATA_POINTER "+="
#define BF_BACK BF_DATA_POINTER "-="
#define BF_IN BF_CURRENT "=getc(stdin);"
#define BF_OUT "putc(" BF_CURRENT ", stdout);"
#define BF_LOOP_START "while(" BF_CURRENT "){"
#define BF_LOOP_END "}"

const char *file_template = "#include <stdio.h>\n"
                            "#include <stdlib.h>\n"
                            "__uint8_t " BF_ARRAY "[" BF_ARRAY_LEN "];\n"
                            "size_t " BF_DATA_POINTER " = 0;\n"
                            "size_t " BF_INSTRUCTION_POINTER " = 0;\n"
                            "void bf();\n"
                            "int main(){ bf(); return 0; }"
                            "";

const char *INSTRUCTION_TO_SRC[] = {
    [TYPE_FORWORD] = BF_FORWORD,
    [TYPE_BACK] = BF_BACK,
    [TYPE_INC] = BF_INC,
    [TYPE_DEC] = BF_DEC,
    [TYPE_OUT] = BF_OUT,
    [TYPE_IN] = BF_IN,
    [TYPE_LOOP_START] = BF_LOOP_START,
    [TYPE_LOOP_END] = BF_LOOP_END,
};

void bf_compile(Interpreter *interpreter, char *out_file)
{
    compile_start_time = nanos_since_unspecified_epoch();
    String_Builder sb = {0};
    sb_append_cstr(&sb, file_template);

    sb_append_cstr(&sb, "void bf(){");
    for (interpreter->ip = 0; interpreter->ip < interpreter->instructions.count; interpreter->ip++)
    {
        Instruction ins = interpreter->instructions.items[interpreter->ip];
        switch (ins.t)
        {
        case TYPE_DEC:
        case TYPE_INC:
        case TYPE_FORWORD:
        case TYPE_BACK:
            sb_append_cstr(&sb, INSTRUCTION_TO_SRC[ins.t]);
            sb_append_cstr(&sb, temp_sprintf("%d", ins.d.repeats));
            sb_append_cstr(&sb, ";");
            temp_reset();
            break;
        default:
            sb_append_cstr(&sb, INSTRUCTION_TO_SRC[ins.t]);
            break;
        }
    }
    sb_append_cstr(&sb, "}");
    nob_write_entire_file(TEMP_SRC_FILE, sb.items, sb.count);

    Cmd cmd = {0};
    cmd_append(&cmd, "cc", "-Wall", "-o", out_file, "-O3", TEMP_SRC_FILE);
    if (!nob_cmd_run(&cmd)) abort();
    nob_delete_file(TEMP_SRC_FILE);

    compile_stop_time = nanos_since_unspecified_epoch();
}