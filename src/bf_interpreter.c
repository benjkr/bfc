#include "bf.h"
#include "nob.h"
#include <stdio.h>

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
