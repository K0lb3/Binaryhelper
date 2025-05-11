from __future__ import annotations

from dataclasses import dataclass
from enum import IntEnum, auto
from typing import (
    Callable,
    Generic,
    Sequence,
    TypeVar,
)

from ..EndianedBinaryIO import EndianedReaderIOBase, EndianedWriterIOBase

T = TypeVar("T")


class TypeNodeType(IntEnum):
    NONE = 0

    PRIMITIVE_START = auto()
    U8 = auto()
    U16 = auto()
    U32 = auto()
    U64 = auto()
    I8 = auto()
    I16 = auto()
    I32 = auto()
    I64 = auto()
    F16 = auto()
    F32 = auto()
    F64 = auto()
    VARINT = auto()
    VARUINT = auto()
    PRIMITIVE_END = auto()

    STRING_START = auto()
    STRING = auto()  # length prefixed
    CSTRING = auto()  # null terminated
    STRING_END = auto()

    LIST = auto()
    TUPLE = auto()
    CLASS = auto()


class TypeNode(Generic[T]):
    def parse(self, reader: EndianedReaderIOBase) -> T:
        """Parse the data into the appropriate type."""
        raise NotImplementedError("Subclasses must implement parse method.")

    def write(self, writer: EndianedWriterIOBase, value: T) -> int:
        """Write the data to the appropriate type."""
        raise NotImplementedError("Subclasses must implement write method.")


LAMBDA_PARSE = "lambda reader: reader.read_{}()"
LAMBDA_WRITE = "lambda writer, value: writer.write_{}(value)"

PRIMITIVE_NODE_MAP = {
    primitive: (
        eval(LAMBDA_PARSE.format(primitive.name.lower())),
        eval(LAMBDA_WRITE.format(primitive.name.lower())),
    )
    for primitive in [
        TypeNodeType.U8,
        TypeNodeType.U16,
        TypeNodeType.U32,
        TypeNodeType.U64,
        TypeNodeType.I8,
        TypeNodeType.I16,
        TypeNodeType.I32,
        TypeNodeType.I64,
        TypeNodeType.F16,
        TypeNodeType.F32,
        TypeNodeType.F64,
    ]
}


@dataclass()
class PrimitiveNode(TypeNode[T]):
    """Primitive types are directly parsable and mapped to C types.

    U8, U16, U32, U64
    I8, I16, I32, I64
    F16, F32, F64
    """

    primitive: TypeNodeType

    def __post_init__(self):
        if (
            self.primitive < TypeNodeType.PRIMITIVE_START
            or self.primitive > TypeNodeType.PRIMITIVE_END
        ):
            raise ValueError("Invalid primitive type")
        self.parse, self.write = PRIMITIVE_NODE_MAP[self.primitive]


@dataclass(frozen=True)
class StringNode(TypeNode[str]):
    """StringNode is either a length-prefixed string or a C-style string.

    If type_info is None, it is a C-style string.
    If type_info is a TypeNode, it is a length-prefixed string with the given type as the length encoding.
    """

    size_node: PrimitiveNode | None = None

    def parse(self, reader: EndianedReaderIOBase) -> str:
        if self.size_node is None:
            # C-style string
            return reader.read_string_c()
        else:
            # Length-prefixed string
            length = self.size_node.parse(reader)
            return reader.read(length).decode("utf-8")

    def write(self, writer: EndianedWriterIOBase, value: str) -> int:
        if self.size_node is None:
            # C-style string
            return writer.write_string_c(value)
        else:
            # Length-prefixed string
            encoded_value = value.encode("utf-8")
            total_size = self.size_node.write(writer, len(encoded_value))
            total_size += writer.write(encoded_value)
            return total_size


@dataclass(frozen=True)
class ListNode(TypeNode[list[T]]):
    """ListNode relates to a list of parsable nodes of the same type as type_info encoded with a variable length encoding.

    The first element of type_info is the length encoding type.
    The second element of type_info is the type of the list elements.
    """

    elem_node: TypeNode[T]
    size_node: PrimitiveNode

    def parse(self, reader: EndianedReaderIOBase):
        length = self.size_node.parse(reader)
        return [self.elem_node.parse(reader) for _ in range(length)]

    def write(self, writer: EndianedWriterIOBase, value: Sequence[T]) -> int:
        total_size = self.size_node.write(writer, len(value))
        for element in value:
            total_size += self.elem_node.write(writer, element)
        return total_size


@dataclass(frozen=True)
class TupleNode(TypeNode[tuple[T, ...]]):
    """TupleNode relates to a tuple of parsable nodes of different types."""

    nodes: tuple[TypeNode[T], ...]

    def parse(self, reader: EndianedReaderIOBase):
        return tuple(node.parse(reader) for node in self.nodes)

    def write(self, writer: EndianedWriterIOBase, value: tuple[T, ...]) -> int:
        total_size = 0
        for field, val in zip(self.nodes, value):
            total_size += field.write(writer, val)
        return total_size


@dataclass
class ClassNode(TypeNode[T]):
    """ClassNode relates to a class of parsable nodes of different types."""

    nodes: tuple[TypeNode]
    names: tuple[str]
    call: Callable

    def parse(self, reader: EndianedReaderIOBase):
        raw_data = {
            name: node.parse(reader) for name, node in zip(self.names, self.nodes)
        }
        return self.call(raw_data)

    def write(self, writer: EndianedWriterIOBase, value: T) -> int:
        return sum(
            node.write(writer, getattr(value, name))
            for name, node in zip(self.names, self.nodes)
        )


__all__ = (
    "TypeNode",
    "PrimitiveNode",
    "StringNode",
    "ListNode",
    "TupleNode",
    "ClassNode",
    "TypeNodeType",
)
