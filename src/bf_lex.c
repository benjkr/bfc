#include "bf.h"
#include "nob.h"

bool repeat_token(Token *last, Token_Type t)
{
    if (last->t == t && last->d.repeats < INT8_MAX)
    {
        last->d.repeats += 1;
        return true;
    }

    return false;
}

bool bf_init(const char *f, Lexer *lexer, bool show_metrics)
{
    lexer_start_time = nanos_since_unspecified_epoch();
    String_Builder sb = {0};
    if (!nob_read_entire_file(f, &sb))
    {
        return false;
    }

    lexer->dp = 0;
    lexer->ip = 0;
    lexer->tokens = (Tokens){0};

    IntStack stack = {0};
    for (size_t i = 0; i < sb.count; i++)
    {
        char c = sb.items[i];
        Token token = {0};

        Token *last = lexer->tokens.count ? &da_last(&lexer->tokens) : NULL;
        switch (c)
        {
        case '+':
            token.t = TYPE_INC;
            if (last && repeat_token(last, token.t)) continue;
            token.d.repeats = 1;
            break;
        case '-':
            token.t = TYPE_DEC;
            if (last && repeat_token(last, token.t)) continue;
            token.d.repeats = 1;
            break;
        case '>':
            token.t = TYPE_FORWORD;
            if (last && repeat_token(last, token.t)) continue;
            token.d.repeats = 1;
            break;
        case '<':
            token.t = TYPE_BACK;
            if (last && repeat_token(last, token.t)) continue;
            token.d.repeats = 1;
            break;
        case '.':
            token.t = TYPE_OUT;
            break;
        case ',':
            token.t = TYPE_IN;
            break;
        case '[':
            token.t = TYPE_LOOP_START;
            da_append(&stack, lexer->tokens.count);
            break;
        case ']':
            token.t = TYPE_LOOP_END;
            token.d.loop_start_ip = da_pop(&stack);
            lexer->tokens.items[token.d.loop_start_ip].d.loop_end_ip = lexer->tokens.count;
            break;
        default:
            continue;
        }
        da_append(&lexer->tokens, token);
    }
    if (stack.count > 0)
    {
        NOB_TODO("Unclosed '['");
    }

    sb_free(sb);
    da_free(stack);

    lexer_stop_time = nanos_since_unspecified_epoch();
    return true;
}

void bf_free(Lexer *lexer)
{
    da_free(lexer->tokens);
}

#define nanos_to_seconds_float(nanos) (float)(nanos) / (float)(1e+9)
void bf_print_metrics(void)
{
    printf("\n");
    printf("Metrics Summary:\n");
    printf("\tLexer Time: %f Seconds\n", nanos_to_seconds_float(lexer_stop_time - lexer_start_time));
    printf("\tCompile Time: %f Seconds\n", nanos_to_seconds_float(compile_stop_time - compile_start_time));
    printf("\tRun Time: %f Seconds\n", nanos_to_seconds_float(run_stop_time - run_start_time));
    printf("\tTotal Program Time: %f Seconds\n", nanos_to_seconds_float(program_stop_time - program_start_time));
}