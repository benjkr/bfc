#include "bf.c"
#include <stdio.h>
#include <stdlib.h>

#define MEMORY_LEN 655360
__uint8_t MEMORY[MEMORY_LEN];

void bf_run(Interpreter *interpreter)
{
    run_start_time = nanos_since_unspecified_epoch();
    for (interpreter->ip = 0; interpreter->ip < interpreter->instructions.count; interpreter->ip++)
    {
        Instruction ins = interpreter->instructions.items[interpreter->ip];
        switch (ins.t)
        {
        case TYPE_BACK:
            interpreter->dp -= ins.d.repeats;
            break;
        case TYPE_FORWORD:
            interpreter->dp += ins.d.repeats;
            break;
        case TYPE_INC:
            MEMORY[interpreter->dp] += ins.d.repeats;
            break;
        case TYPE_DEC:
            MEMORY[interpreter->dp] -= ins.d.repeats;
            break;
        case TYPE_IN:
            MEMORY[interpreter->dp] = getchar();
            break;
        case TYPE_OUT:
            putchar(MEMORY[interpreter->dp]);
            break;
        case TYPE_LOOP_START:
            if (MEMORY[interpreter->dp] == 0) interpreter->ip = ins.d.loop_end_ip;
            break;
        case TYPE_LOOP_END:
            if (MEMORY[interpreter->dp] != 0) interpreter->ip = ins.d.loop_start_ip;
            break;
        default:
            break;
        }
    }

    run_stop_time = nanos_since_unspecified_epoch();
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

    bf_run(&interpreter);
    bf_free(&interpreter);

    program_stop_time = nanos_since_unspecified_epoch();
    if (*show_metrics) bf_print_metrics();
    return 0;
}