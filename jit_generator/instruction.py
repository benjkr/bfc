from dataclasses import dataclass
from enum import Enum, IntEnum
import typing


class ParamType(Enum):
    Literal = 1,
    AsAddress = 2,

    def prefix(self):
        if self == ParamType.Literal:
            return ""
        elif self == ParamType.AsAddress:
            return "AT_"


class BitSize(IntEnum):
    NONE = 0,

    int8 = (2**7)-1,
    int32 = (2**31)-1,
    int64 = (2**63)-1,

    uint8 = (2**8)-1,
    uint32 = (2**32)-1,
    uint64 = (2**64)-1,

    @staticmethod
    def members():
        mems = dict(BitSize.__members__)
        del mems[BitSize.NONE.name]
        return mems.values()

    @staticmethod
    def signed_members():
        return [BitSize.int8, BitSize.int32]

    @staticmethod
    def unsigned_members():
        return [BitSize.uint8, BitSize.uint32]

    def bits(self):
        import re
        if self == BitSize.NONE:
            return None
        return int(re.match("u?int([0-9]+)", self.name).group(1))

    def size_hint(self):
        return self.bits() // 8

    def pointer_hint(self):
        bits = self.bits()
        if bits == 8:
            return "BYTE PTR"
        elif bits == 32:
            return "DWORD PTR"
        elif bits == 64:
            return "QWORD PTR"
        return ""


@dataclass
class Param:
    _type: ParamType
    p: str
    pointer_size: BitSize = BitSize.NONE

    @classmethod
    def literal(cls, p: str):
        return cls(_type=ParamType.Literal, p=p)

    @classmethod
    def as_address(cls, p: str, sz: BitSize):
        return cls(_type=ParamType.AsAddress, p=p, pointer_size=sz)

    def as_asm(self):
        if self._type == ParamType.AsAddress:
            out = f"[{self.p}]"
        else:
            out = f"{self.p}"
        out = f"{self.pointer_size.pointer_hint()} {out}"
        return out

    def __str__(self):
        ptr_hint = self.pointer_size.pointer_hint().replace(' ', '_')
        ptr_hint = f"{ptr_hint}_" if ptr_hint else ""
        return f"{self._type.prefix()}{ptr_hint}{self.p}"


class Instruction:
    def __init__(self, ins: str, params: typing.List[Param] = []):
        self.ins = ins
        self.params = params

    def filter_params(self):
        return self.params

    def name(self):
        res = self.ins
        params = self.filter_params()
        if params:
            res += f"_{'_'.join(map(str, params))}"
        return res.upper()

    def input_size_hint(self):
        return None

    def input_type(self):
        return None

    def pop_opcode_from_iter(self, opcodes: typing.Iterable[bytes]) -> bytes:
        return next(opcodes)

    def __str__(self):
        res = f"{self.ins} "
        res += ', '.join(map(Param.as_asm, self.params))
        return res


class SingleInstruction(Instruction):
    def __init__(self, ins):
        super().__init__(ins, [])


class OneParamInstruction(Instruction):
    def __init__(self, ins, dest: str):
        super().__init__(ins, [Param.literal(dest)])


class OneSizedParamInstruction(Instruction):
    def __init__(self, ins, sz: BitSize):
        super().__init__(ins, [Param.literal(str(int(sz)))])        
        self.sz = sz
        
    def filter_params(self):
        return [Param.literal(self.sz.name)]

    def input_size_hint(self):
        return self.sz.size_hint()

    def input_type(self):
        return self.sz.name



class OneParamAddressInstruction(Instruction):
    def __init__(self, ins, dest, sz: BitSize):
        super().__init__(ins, [Param.as_address(dest, sz)])


class ConstantToParamInstruction(Instruction):
    def __init__(self, ins, dest: str, sz=BitSize.NONE):
        super().__init__(
            ins, [Param.literal(dest), Param.literal(str(int(sz)))])
        self.sz = sz

    def filter_params(self):
        return self.params[:1] + [Param.literal(self.sz.name)] if self.sz != BitSize.NONE else []

    def input_size_hint(self):
        return self.sz.size_hint()

    def input_type(self):
        return self.sz.name


class ConstantToParamAddressInstruction(Instruction):
    def __init__(self, ins, dest: str, sz=BitSize.NONE):
        super().__init__(ins, [Param.as_address(
            dest, sz), Param.literal(str(int(sz)))])
        self.sz = sz

    def filter_params(self):
        return self.params[:1] + [Param.literal(self.sz.name)] if self.sz != BitSize.NONE else []

    def input_size_hint(self):
        return self.sz.size_hint()

    def input_type(self):
        return self.sz.name
