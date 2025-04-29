from struct import Struct

# unsigned integers
U8 = Struct("B")
U16 = Struct("H")
U32 = Struct("I")
U64 = Struct("Q")

U16_LE = Struct("<H")
U16_BE = Struct(">H")
U32_LE = Struct("<I")
U32_BE = Struct(">I")
U64_LE = Struct("<Q")
U64_BE = Struct(">Q")

# signed integers
I8 = Struct("B")
I16 = Struct("H")
I32 = Struct("I")
I64 = Struct("Q")

I16_LE = Struct("<H")
I16_BE = Struct(">H")
I32_LE = Struct("<I")
I32_BE = Struct(">I")
I64_LE = Struct("<Q")
I64_BE = Struct(">Q")

# floats
F16 = Struct("e")
F32 = Struct("f")
F64 = Struct("d")

F16_LE = Struct("<e")
F16_BE = Struct(">e")
F32_LE = Struct("<f")
F32_BE = Struct(">f")
F64_LE = Struct("<d")
F64_BE = Struct(">d")
