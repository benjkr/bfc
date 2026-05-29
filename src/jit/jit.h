#include ".jit.h"

#define CONCAT_HIDDEN2(a, b) a##b
#define CONCAT2(a, b) CONCAT_HIDDEN2(a, b)

#define CONCAT_HIDDEN3(a, b, c) a##b##c
#define CONCAT3(a, b, c) CONCAT_HIDDEN3(a, b, c)

#define _set_input_raw(OPCODE, offset, literal, type) *(type *)(&OPCODE[(offset)]) = (type)(literal)
#define set_input(OPCODE, literal) _set_input_raw(OPCODE, CONCAT2(OPCODE, _INPUT_OFFSET), literal, CONCAT2(OPCODE, _INPUT_TYPE))
#define write_opcode(OPCODE, ptr) memcpy((ptr), OPCODE, sizeof(OPCODE))