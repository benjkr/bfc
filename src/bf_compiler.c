#include "bf.c"
#include <stdio.h>
#include <stdlib.h>

#define TEMP_SRC_FILE "./.temp.c"
#define TEMP_OUT_FILE "./.temp"
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

void bf_compile(Interpreter *interpreter)
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
    cmd_append(&cmd, "cc", "-Wall", "-o", TEMP_OUT_FILE, "-O3", TEMP_SRC_FILE);
    if (!nob_cmd_run(&cmd)) abort();
    nob_delete_file(TEMP_SRC_FILE);

    compile_stop_time = nanos_since_unspecified_epoch();
}

void usage(FILE *stream)
{
    fprintf(stream, "Usage: %s [OPTIONS] <BF-FILE>\n", flag_program_name());
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);
}

int main(int argc, char *argv[])
{
    program_start_time = nanos_since_unspecified_epoch();
    bool *help = flag_bool("help", false, "Print this help to stdout and exit with 0");
    bool *show_metrics = flag_bool("metrics", false, "Show metrics");
    if (!flag_parse(argc, argv))
    {
        usage(stderr);
        flag_print_error(stderr);
        return 1;
    }
    argc = flag_rest_argc();
    argv = flag_rest_argv();

    if (argc != 1 || *help)
    {
        usage(stderr);
        return 1;
    }

    Interpreter interpreter = {0};
    if (!bf_init(argv[0], &interpreter, *show_metrics)) abort();

    bf_compile(&interpreter);
    bf_free(&interpreter);

    Cmd cmd = {0};
    cmd_append(&cmd, TEMP_OUT_FILE);

    run_start_time = nanos_since_unspecified_epoch();
    if (!nob_cmd_run(&cmd)) return 1;
    run_stop_time = nanos_since_unspecified_epoch();
    nob_delete_file(TEMP_OUT_FILE);

    program_stop_time = nanos_since_unspecified_epoch();
    if (*show_metrics) bf_print_metrics();
    return 0;
}