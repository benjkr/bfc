#include "bf.h"
#include "jit/jit.c"
#include "nob.h"
#include <stdio.h>
#include <sys/mman.h>

char INS_PRELUDE[] = {
    OPCODE__PUSH_RAX, OPCODE__PUSH_RBX, OPCODE__PUSH_RCX, OPCODE__XOR_RAX_RAX, OPCODE__XOR_RBX_RBX, OPCODE__XOR_RCX_RCX,
};

char INS_CHECKPOINT_PUSH[] = {
    OPCODE__PUSH_RCX,
};
char INS_CHECKPOINT_POP[] = {
    OPCODE__POP_RCX,
};

char INS_FOOTER[] = {
    OPCODE__POP_RCX,
    OPCODE__POP_RBX,
    OPCODE__POP_RAX,
    OPCODE__RET,
};

typedef struct
{
    size_t *items;
    size_t count;
    size_t capacity;
} SizeTStack;

#define sb_append_arr(sb, buf) sb_append_buf(sb, buf, sizeof(buf) / sizeof(buf[0]))

typedef void (*func_ptr)();
void bf_jit(Interpreter *interpreter)
{
    compile_start_time = nanos_since_unspecified_epoch();
    String_Builder sb = {0};

    sb_append_arr(&sb, INS_PRELUDE);
    set_input(MOVABS_RCX_UINT64, &MEMORY);
    sb_append_arr(&sb, MOVABS_RCX_UINT64);

    SizeTStack stack = {0};
    for (interpreter->ip = 0; interpreter->ip < interpreter->instructions.count; interpreter->ip++)
    {
        Instruction ins = interpreter->instructions.items[interpreter->ip];
        switch (ins.t)
        {
        case TYPE_BACK:
            set_input(SUB_RCX_INT8, ins.d.repeats);
            sb_append_arr(&sb, SUB_RCX_INT8);
            break;
        case TYPE_FORWORD:
            set_input(ADD_RCX_INT8, ins.d.repeats);
            sb_append_arr(&sb, ADD_RCX_INT8);
            break;
        case TYPE_INC:
            set_input(ADD_AT_BYTE_PTR_RCX_INT8, ins.d.repeats);
            sb_append_arr(&sb, ADD_AT_BYTE_PTR_RCX_INT8);
            break;
        case TYPE_DEC:
            set_input(SUB_AT_BYTE_PTR_RCX_INT8, ins.d.repeats);
            sb_append_arr(&sb, SUB_AT_BYTE_PTR_RCX_INT8);
            break;
        case TYPE_IN:
            sb_append_arr(&sb, PUSH_RCX);
            set_input(MOVABS_RAX_UINT64, &getchar);
            sb_append_arr(&sb, MOVABS_RAX_UINT64);
            sb_append_arr(&sb, CALL_RAX);
            sb_append_arr(&sb, POP_RCX);
            sb_append_arr(&sb, MOV_AT_BYTE_PTR_RCX_AL);
            break;
        case TYPE_OUT:
            sb_append_arr(&sb, INS_CHECKPOINT_PUSH);

            sb_append_arr(&sb, MOV_DIL_AT_BYTE_PTR_RCX);
            set_input(MOVABS_RAX_UINT64, &putchar);
            sb_append_arr(&sb, MOVABS_RAX_UINT64);
            sb_append_arr(&sb, CALL_RAX);

            sb_append_arr(&sb, INS_CHECKPOINT_POP);
            break;
        case TYPE_LOOP_START:
            da_append(&stack, sb.count);
            sb_append_arr(&sb, JMP_INT32);
            da_append(&stack, sb.count);
            break;
        case TYPE_LOOP_END:
            size_t index_after_jmp = da_pop(&stack);
            size_t index_jmp_command = da_pop(&stack);

            int32_t while_block_size = (int32_t)(sb.count - index_after_jmp);
            *(int32_t *)&sb.items[index_jmp_command + JMP_INT32_INPUT_OFFSET] = while_block_size;

            set_input(CMP_AT_BYTE_PTR_RCX_INT8, 0);
            sb_append_arr(&sb, CMP_AT_BYTE_PTR_RCX_INT8);

            size_t start_of_loop = (sb.count + sizeof(JNZ_INT32)) - index_after_jmp;
            set_input(JNZ_INT32, (-(int32_t)start_of_loop));
            sb_append_arr(&sb, JNZ_INT32);
            break;
        default:
            break;
        }
    }

    sb_append_arr(&sb, INS_FOOTER);

    long pagesize = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = (uintptr_t)sb.items & ~(pagesize - 1);
    size_t total_length = sb.count + ((uintptr_t)sb.items - page_start);
    if (mprotect((void *)page_start, total_length, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
    {
        perror("mprotect failed");
        return;
    }

    compile_stop_time = nanos_since_unspecified_epoch();

    write_entire_file("./out.bin", sb.items, sb.count);
    run_start_time = nanos_since_unspecified_epoch();
    ((func_ptr)sb.items)();
    run_stop_time = nanos_since_unspecified_epoch();
}
