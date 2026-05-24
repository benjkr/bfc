#include "stdbool.h"
#include "stdlib.h"
#define NOB_IMPLEMENTATION
#define NOB_NO_ECHO
#include "nob.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"

uint64_t program_start_time = 0;
uint64_t interpreter_start_time = 0;
uint64_t compile_start_time = 0;
uint64_t run_start_time = 0;

uint64_t program_stop_time = 0;
uint64_t interpreter_stop_time = 0;
uint64_t compile_stop_time = 0;
uint64_t run_stop_time = 0;

typedef enum
{
    TYPE_FORWORD,
    TYPE_BACK,
    TYPE_INC,
    TYPE_DEC,
    TYPE_OUT,
    TYPE_IN,
    TYPE_LOOP_START,
    TYPE_LOOP_END,
} Instruction_Type;

typedef union {
    size_t loop_start_ip;
    size_t loop_end_ip;
    int8_t repeats;
} Instruction_Data;

typedef struct
{
    Instruction_Type t;
    Instruction_Data d;
} Instruction;

typedef struct
{
    Instruction *items;
    size_t count;
    size_t capacity;
} Instructions;

typedef struct
{
    int *items;
    size_t count;
    size_t capacity;
} IntStack;

typedef struct
{
    size_t ip;
    size_t dp;
    Instructions instructions;
} Interpreter;

bool repeat_instruction(Instruction *last, Instruction_Type t)
{
    if (last->t == t && last->d.repeats < INT8_MAX)
    {
        last->d.repeats += 1;
        return true;
    }

    return false;
}

bool bf_init(const char *f, Interpreter *interpreter, bool show_metrics)
{
    interpreter_start_time = nanos_since_unspecified_epoch();
    String_Builder sb = {0};
    if (!nob_read_entire_file(f, &sb))
    {
        return false;
    }

    interpreter->dp = 0;
    interpreter->ip = 0;
    interpreter->instructions = (Instructions){0};

    IntStack stack = {0};
    for (size_t i = 0; i < sb.count; i++)
    {
        char c = sb.items[i];
        Instruction ins = {0};

        Instruction *last = interpreter->instructions.count ? &da_last(&interpreter->instructions) : NULL;
        switch (c)
        {
        case '+':
            ins.t = TYPE_INC;
            if (last && repeat_instruction(last, ins.t)) continue;
            ins.d.repeats = 1;
            break;
        case '-':
            ins.t = TYPE_DEC;
            if (last && repeat_instruction(last, ins.t)) continue;
            ins.d.repeats = 1;
            break;
        case '>':
            ins.t = TYPE_FORWORD;
            if (last && repeat_instruction(last, ins.t)) continue;
            ins.d.repeats = 1;
            break;
        case '<':
            ins.t = TYPE_BACK;
            if (last && repeat_instruction(last, ins.t)) continue;
            ins.d.repeats = 1;
            break;
        case '.':
            ins.t = TYPE_OUT;
            break;
        case ',':
            ins.t = TYPE_IN;
            break;
        case '[':
            ins.t = TYPE_LOOP_START;
            da_append(&stack, interpreter->instructions.count);
            break;
        case ']':
            ins.t = TYPE_LOOP_END;
            ins.d.loop_start_ip = da_pop(&stack);
            interpreter->instructions.items[ins.d.loop_start_ip].d.loop_end_ip = interpreter->instructions.count;
            break;
        default:
            continue;
        }
        da_append(&interpreter->instructions, ins);
    }
    if (stack.count > 0)
    {
        NOB_TODO("Unclosed '['");
    }

    sb_free(sb);
    da_free(stack);

    interpreter_stop_time = nanos_since_unspecified_epoch();
    return true;
}

void bf_free(Interpreter *interpreter)
{
    da_free(interpreter->instructions);
}

#define nanos_to_seconds_float(nanos) (float)(nanos) / (float)(1e+9)
void bf_print_metrics(void)
{
    printf("\n");
    printf("Metrics Summary:\n");
    printf("\tInterpreter Time: %f Seconds\n", nanos_to_seconds_float(interpreter_stop_time - interpreter_start_time));
    printf("\tCompile Time: %f Seconds\n", nanos_to_seconds_float(compile_stop_time - compile_start_time));
    printf("\tRun Time: %f Seconds\n", nanos_to_seconds_float(run_stop_time - run_start_time));
    printf("\tTotal Program Time: %f Seconds\n", nanos_to_seconds_float(program_stop_time - program_start_time));
}