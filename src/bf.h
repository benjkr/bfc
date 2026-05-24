#include "stdlib.h"
#include <stdbool.h>
#include <stdint.h>

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
} Token_Type;

typedef union {
    size_t loop_start_ip;
    size_t loop_end_ip;
    int8_t repeats;
} Token_Data;

typedef struct
{
    Token_Type t;
    Token_Data d;
} Token;

typedef struct
{
    Token *items;
    size_t count;
    size_t capacity;
} Tokens;

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
    Tokens tokens;
} Lexer;

#ifndef BF_H
#define BF_H 1
extern uint64_t program_start_time;
extern uint64_t lexer_start_time;
extern uint64_t compile_start_time;
extern uint64_t run_start_time;

extern uint64_t program_stop_time;
extern uint64_t lexer_stop_time;
extern uint64_t compile_stop_time;
extern uint64_t run_stop_time;

#define MEMORY_LEN 655360
extern uint8_t MEMORY[MEMORY_LEN];
#endif // BF_H


bool bf_init(const char *f, Lexer *lexer, bool show_metrics);
void bf_free(Lexer *lexer);
void bf_jit(Lexer *lexer);
void bf_run(Lexer *lexer);
void bf_compile(Lexer *lexer, char *out_file);
void bf_print_metrics(void);