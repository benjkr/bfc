#include ".jit.c"

#define CONCAT_HIDDEN2(a, b) a##b
#define CONCAT2(a, b) CONCAT_HIDDEN2(a, b)

#define CONCAT_HIDDEN3(a, b, c) a##b##c
#define CONCAT3(a, b, c) CONCAT_HIDDEN3(a, b, c)

#define set_input(OPCODE, literal) *((CONCAT2(OPCODE, _INPUT_TYPE) *)(&OPCODE[CONCAT2(OPCODE, _INPUT_OFFSET)])) = (CONCAT2(OPCODE, _INPUT_TYPE))(literal)