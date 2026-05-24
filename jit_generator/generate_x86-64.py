from instruction import *
from runner import *

REGISTERS_16BIT = [f"{i}x" for i in "abcd"] + [f"{i}i" for i in "ds"]
REGISTERS_32BIT = [f"e{reg}" for reg in REGISTERS_16BIT]
REGISTERS_64BIT = [f"r{reg}" for reg in REGISTERS_16BIT]
REGISTERS_ALL = REGISTERS_16BIT+REGISTERS_32BIT+REGISTERS_64BIT

opcodes = []
opcodes += [OneParamInstruction("inc", reg)
            for reg in REGISTERS_ALL]
opcodes += [OneParamAddressInstruction("inc", reg, sz)
            for reg in REGISTERS_64BIT for sz in BitSize.signed_members()]
opcodes += [OneParamInstruction("dec", reg)
            for reg in REGISTERS_ALL]
opcodes += [OneParamAddressInstruction("dec", reg, sz)
            for reg in REGISTERS_64BIT for sz in BitSize.signed_members()]

opcodes += [ConstantToParamInstruction("add", reg, sz=sz)
            for reg in REGISTERS_64BIT for sz in BitSize.signed_members()]
opcodes += [ConstantToParamAddressInstruction("add", reg, sz=sz)
            for reg in REGISTERS_64BIT for sz in BitSize.signed_members()]
opcodes += [ConstantToParamInstruction("sub", reg, sz=sz)
            for reg in REGISTERS_64BIT for sz in BitSize.signed_members()]
opcodes += [ConstantToParamAddressInstruction("sub", reg, sz=sz)
            for reg in REGISTERS_64BIT for sz in BitSize.signed_members()]

opcodes += [Instruction("xor", [Param.literal(reg)]*2)
            for reg in REGISTERS_ALL]

opcodes += [OneParamInstruction("push", reg)
            for reg in REGISTERS_64BIT]
opcodes += [OneParamInstruction("pop", reg)
            for reg in REGISTERS_64BIT]
opcodes += [ConstantToParamInstruction("movabs", reg, sz=BitSize.uint64)
            for reg in REGISTERS_64BIT]
opcodes += [OneParamInstruction("call", reg)
            for reg in REGISTERS_64BIT]
opcodes += [SingleInstruction("ret")]

opcodes += [ConstantToParamAddressInstruction("cmp", reg, sz=sz)
            for reg in REGISTERS_64BIT for sz in BitSize.signed_members()]
opcodes += [OneSizedParamInstruction("jmp", sz=BitSize.int32),
            OneSizedParamInstruction("jz", sz=BitSize.int32),
            OneSizedParamInstruction("jnz", sz=BitSize.int32)]

opcodes += [Instruction("mov", [Param.as_address("rbx",
                        BitSize.int8), Param.literal("al")])]
opcodes += [Instruction("mov", [Param.as_address("rcx",
                        BitSize.int8), Param.literal("al")])]
opcodes += [Instruction("mov", [Param.literal("dil"),
                        Param.as_address("rbx", BitSize.int8)])]
opcodes += [Instruction("mov", [Param.literal("dil"),
                        Param.as_address("rcx", BitSize.int8)])]

Runner(opcodes).generate_intel_x86_64_C_header()

