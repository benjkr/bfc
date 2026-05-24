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

#ifndef BF_H
#define BF_H 1
extern uint64_t program_start_time;
extern uint64_t interpreter_start_time;
extern uint64_t compile_start_time;
extern uint64_t run_start_time;

extern uint64_t program_stop_time;
extern uint64_t interpreter_stop_time;
extern uint64_t compile_stop_time;
extern uint64_t run_stop_time;

#define MEMORY_LEN 655360
extern uint8_t MEMORY[MEMORY_LEN];
#endif // BF_H


bool bf_init(const char *f, Interpreter *interpreter, bool show_metrics);
void bf_free(Interpreter *interpreter);
void bf_jit(Interpreter *interpreter);
void bf_run(Interpreter *interpreter);
void bf_compile(Interpreter *interpreter, char *out_file);
void bf_print_metrics(void);