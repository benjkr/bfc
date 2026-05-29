#include "bf.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"
#define NOB_IMPLEMENTATION
#define NOB_NO_ECHO
#include "nob.h"

uint64_t program_start_time = 0;
uint64_t lexer_start_time = 0;
uint64_t compile_start_time = 0;
uint64_t run_start_time = 0;
uint64_t program_stop_time = 0;
uint64_t lexer_stop_time = 0;
uint64_t compile_stop_time = 0;
uint64_t run_stop_time = 0;
uint8_t MEMORY[MEMORY_LEN] = {0};

void usage(FILE *stream)
{
    fprintf(stream, "Usage: %s [OPTIONS] <BF-FILE>\n", flag_program_name());
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);
    fprintf(stream, "Use one of -c -nasm -jit -interpret\n");
}

int main(int argc, char *argv[])
{
    program_start_time = nanos_since_unspecified_epoch();
    bool *help = flag_bool("help", false, "Print this help to stdout and exit with 0");
    bool *show_metrics = flag_bool("metrics", false, "Show metrics");
    bool *run = flag_bool("run", false, "COMPILER ONLY: Run the output executable");
    char **out_file = flag_str("o", "a.out", "COMPILER ONLY: Output executable filename.");
    bool *nasm_backend = flag_bool("nasm", false, "ASM backend");
    bool *c_backend = flag_bool("c", false, "C backend");
    bool *jit = flag_bool("jit", false, "Just-In-Time compile and run the program");
    bool *interpret = flag_bool("interpret", false, "Interpret and run the program");
    if (!flag_parse(argc, argv))
    {
        usage(stderr);
        flag_print_error(stderr);
        return 1;
    }
    argc = flag_rest_argc();
    argv = flag_rest_argv();

    int sum_of_engines = *jit + *interpret + *nasm_backend + *c_backend;
    if (argc != 1 || *help || sum_of_engines != 1)
    {
        usage(stderr);
        return 1;
    }

    Lexer lexer = {0};
    if (!bf_init(argv[0], &lexer, *show_metrics)) abort();

    if (*jit) bf_jit(&lexer);
    else if (*interpret) bf_run(&lexer);
    else if (*c_backend) bf_compile(&lexer, *out_file);
    else if (*nasm_backend) bf_compile_asm(&lexer, *out_file);

    if (*run && (*c_backend || *nasm_backend))
    {
        Cmd cmd = {0};
        if (*out_file[0] == '/') cmd_append(&cmd, *out_file);
        else cmd_append(&cmd, temp_sprintf("./%s", *out_file));

        run_start_time = nanos_since_unspecified_epoch();
        if (!nob_cmd_run(&cmd)) return 1;
        run_stop_time = nanos_since_unspecified_epoch();
        temp_reset();
    }

    bf_free(&lexer);
    program_stop_time = nanos_since_unspecified_epoch();
    if (*show_metrics) bf_print_metrics();
    return 0;
}