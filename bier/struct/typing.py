from typing import Annotated, TypeVar

from .TypeNode import (
    F16Node,
    F32Node,
    F64Node,
    I8Node,
    I16Node,
    I32Node,
    I64Node,
    StringNode,
    U8Node,
    U16Node,
    U32Node,
    U64Node,
)

T = TypeVar("T")
S = TypeVar("S")

u8 = Annotated[int, U8Node]
u16 = Annotated[int, U16Node]
u32 = Annotated[int, U32Node]
u64 = Annotated[int, U64Node]

i8 = Annotated[int, I8Node]
i16 = Annotated[int, I16Node]
i32 = Annotated[int, I32Node]
i64 = Annotated[int, I64Node]

f16 = Annotated[float, F16Node]
f32 = Annotated[float, F32Node]
f64 = Annotated[float, F64Node]

cstr = Annotated[str, StringNode, None]

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
)
