from typing import Annotated, TypeVar

from .TypeNode import TypeNodeType

T = TypeVar("T")
S = TypeVar("S")

u8 = Annotated[int, TypeNodeType.U8]
u16 = Annotated[int, TypeNodeType.U16]
u32 = Annotated[int, TypeNodeType.U32]
u64 = Annotated[int, TypeNodeType.U64]

i8 = Annotated[int, TypeNodeType.I8]
i16 = Annotated[int, TypeNodeType.I16]
i32 = Annotated[int, TypeNodeType.I32]
i64 = Annotated[int, TypeNodeType.I64]

f16 = Annotated[float, TypeNodeType.F16]
f32 = Annotated[float, TypeNodeType.F32]
f64 = Annotated[float, TypeNodeType.F64]

type cstr = Annotated[str, TypeNodeType.CSTRING]
type str_d[T] = Annotated[str, T]
type list_d[S, T] = Annotated[list[S], T]

__all__ = (
    "u8",
    "u16",
    "u32",
    "u64",
    "i8",
    "i16",
    "i32",
    "i64",
    "f16",
    "f32",
    "f64",
    "cstr",
    "str_d",
    "list_d",
)
