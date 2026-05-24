#include "bf.h"
#include "nob.h"
#include <stdio.h>

void bf_run(Lexer *lexer)
{
    run_start_time = nanos_since_unspecified_epoch();
    for (lexer->ip = 0; lexer->ip < lexer->tokens.count; lexer->ip++)
    {
        Token token = lexer->tokens.items[lexer->ip];
        switch (token.t)
        {
        case TYPE_BACK:
            lexer->dp -= token.d.repeats;
            break;
        case TYPE_FORWORD:
            lexer->dp += token.d.repeats;
            break;
        case TYPE_INC:
            MEMORY[lexer->dp] += token.d.repeats;
            break;
        case TYPE_DEC:
            MEMORY[lexer->dp] -= token.d.repeats;
            break;
        case TYPE_IN:
            MEMORY[lexer->dp] = getchar();
            break;
        case TYPE_OUT:
            putchar(MEMORY[lexer->dp]);
            break;
        case TYPE_LOOP_START:
            if (MEMORY[lexer->dp] == 0) lexer->ip = token.d.loop_end_ip;
            break;
        case TYPE_LOOP_END:
            if (MEMORY[lexer->dp] != 0) lexer->ip = token.d.loop_start_ip;
            break;
        default:
            break;
        }
    }

    run_stop_time = nanos_since_unspecified_epoch();
}
