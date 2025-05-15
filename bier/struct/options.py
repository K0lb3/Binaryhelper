from abc import ABCMeta
from dataclasses import dataclass, replace
from inspect import isclass
from typing import (
    Annotated,
    Any,
    Literal,
    Protocol,
    Self,
    get_args,
)

from ._typing_helpers import get_origin_type
from .TypeNode import StaticLengthNode, TypeNode, U32Node


def parse_length_type(annotation: Annotated[Any, ...]) -> TypeNode[int]:
    args = get_args(annotation)
    assert len(args) >= 2, "Length type must have two arguments"
    typ, val = args[0], args[1]
    assert typ is int, "Length type must be an int"
    if isclass(val):
        if issubclass(val, TypeNode):
            return val()
        else:
            raise ValueError(
                f"Length type must be a TypeNode[int] or an instance of it, got {val} instead."
            )

    if isinstance(val, TypeNode):
        return args[1]

    if get_origin_type(val) is Literal:
        val_args = get_args(val)
        assert len(val_args) == 1, "Length type must be a Literal with one argument"
        assert isinstance(val_args[0], int), "Length type must be an int"
        return StaticLengthNode(val_args[0])

    raise ValueError(
        f"Length type must be a TypeNode[int] or an instance of it, got {args[1]} instead."
    )


@dataclass(frozen=True)
class BinarySerializableOptions:
    length_type: TypeNode[int] = U32Node()

    def update_by_type(self, option_type: Any) -> Self:
        arguments = get_args(option_type)
        origin = get_origin_type(option_type)

        if origin is length_type:
            new_encoding = parse_length_type(arguments[0])
            return replace(self, length_type=new_encoding)

        return self


class BinarySerializableOption(metaclass=ABCMeta): ...


type member[TType] = TType  # TODO: Implement member-only serialization behavior
"""
Used for defining the members to serialize when not all class members should be serialized.
"""

type custom[TType, *TOptions] = Annotated[TType, *TOptions]
"""
Used for overriding type-level serialization settings for a given member.
"""


class static_length:
    def __class_getitem__(cls, item: int):
        return Annotated[int, Literal[item]]


type length_type[T: int | TypeNode[int] | static_length] = BinarySerializableOption
"""
Used for specifying the type to use when reading a length-providing field.
Must be serializing an 'int' type.
"""


class Converter[TValue, TRaw](Protocol):
    @staticmethod
    def from_raw(raw: TRaw) -> TValue: ...

    @staticmethod
    def to_raw(value: TValue) -> TRaw: ...


type convert[TValue, TRaw, TConverter: Converter] = Annotated[
    TValue, Converter[TValue, TRaw], TConverter
]
"""
Used for directly converting a value to/from a different type when serializing and deserializing.
"""
# pyright complains about Callable | None, but Union seems to be fine????

__all__ = (
    "member",
    "convert",
    "custom",
    "length_type",
    "static_length",
    "BinarySerializableOptions",
)
