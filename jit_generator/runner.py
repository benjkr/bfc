import tempfile
import subprocess
import sys
from typing import List

from instruction import Instruction


class Runner:
    def __init__(self, instructions: List[Instruction]):
        self.instructions = instructions

    def _build_asm_src(self):
        asm_src = ".intel_syntax noprefix\n"
        for ins in self.instructions:
            asm_src += str(ins) + "\n"
        return asm_src

    def _assemble_and_disassemble(self):
        with tempfile.NamedTemporaryFile("w", suffix=".s") as asm_f:
            asm_f.write(self._build_asm_src())
            asm_f.flush()
            with tempfile.NamedTemporaryFile("w", suffix=".o") as object_f:
                assert subprocess.run(f"as {asm_f.name} -o {object_f.name}".split(),
                                      stdout=sys.stdout, stderr=sys.stdout).returncode == 0
                return subprocess.check_output(f"objdump -d -w -M intel {object_f.name}".split(), stderr=subprocess.STDOUT).decode()

    def generate_intel_x86_64_C_header(self):
        disassembly = self._assemble_and_disassemble()

        out = "// This file is auto-generated.\n\n"
        opcodes_iter = map(
            lambda l: bytes.fromhex(l.split("\t")[1]),
            filter(
                lambda l: l.count('\t') == 2,
                disassembly.splitlines()
            )
        )
        for ins in self.instructions:
            opcode = ins.pop_opcode_from_iter(opcodes_iter)
            opcode_c = ",".join([f"0x{b:x}" for b in opcode])
            ins_name = ins.name()
            opcode_name = f"OPCODE__{ins_name}"
            out += f"// {ins}\n"
            out += f"#define {opcode_name} {opcode_c}\n"
            out += f"char {ins_name}[] = {{{opcode_name}}};\n"
            input_size_hint = ins.input_size_hint()
            if input_size_hint:
                out += f"#define {ins_name}_INPUT_OFFSET (sizeof({ins_name})-{input_size_hint})\n"
                out += f"#define {ins_name}_INPUT_TYPE {ins.input_type()}_t\n"
            out += "\n"

        print(out)
        return out
