from abc import ABCMeta
from dataclasses import dataclass
from typing import Any, Callable, ClassVar, Self, Sequence, TypeVar

from .Serializable import Serializable, Serializer

T = TypeVar("T")


class TypeNode(Serializer[T], metaclass=ABCMeta): ...


@dataclass(init=False, frozen=True)
class PrimitiveNode(TypeNode[T]):
    """Primitive types are directly parsable and mapped to C types.

    U8, U16, U32, U64
    I8, I16, I32, I64
    F16, F32, F64
    """

    size: ClassVar[int]

    def __new__(cls, *args, **kwargs) -> Self:
        if cls in PRIMITIVE_INSTANCE_MAP:
            return PRIMITIVE_INSTANCE_MAP[cls]

        instance = super(PrimitiveNode, cls).__new__(cls, *args, **kwargs)
        PRIMITIVE_INSTANCE_MAP[cls] = instance
        return instance

    def __repr__(self):
        return f"{self.__class__.__name__}"


type PrimitiveInstanceMapType[T] = dict[type[PrimitiveNode[T]], PrimitiveNode[T]]
PRIMITIVE_INSTANCE_MAP: PrimitiveInstanceMapType = {}


class U8Node(PrimitiveNode[int]):
    size = 1

    def read_from(self, reader):
        return reader.read_u8()

    def write_to(self, value, writer):
        return writer.write_u8(value)


class U16Node(PrimitiveNode[int]):
    size = 2

    def read_from(self, reader):
        return reader.read_u16()

    def write_to(self, value, writer):
        return writer.write_u16(value)


class U32Node(PrimitiveNode[int]):
    size = 4

    def read_from(self, reader):
        return reader.read_u32()

    def write_to(self, value, writer):
        return writer.write_u32(value)


class U64Node(PrimitiveNode[int]):
    size = 8

    def read_from(self, reader):
        return reader.read_u64()

    def write_to(self, value, writer):
        return writer.write_u64(value)


class I8Node(PrimitiveNode[int]):
    size = 1

    def read_from(self, reader):
        return reader.read_i8()

    def write_to(self, value, writer):
        return writer.write_i8(value)


class I16Node(PrimitiveNode[int]):
    size = 2

    def read_from(self, reader):
        return reader.read_i16()

    def write_to(self, value, writer):
        return writer.write_i16(value)


class I32Node(PrimitiveNode[int]):
    size = 4

    def read_from(self, reader):
        return reader.read_i32()

    def write_to(self, value, writer):
        return writer.write_i32(value)


class I64Node(PrimitiveNode[int]):
    size = 8

    def read_from(self, reader):
        return reader.read_i64()

    def write_to(self, value, writer):
        return writer.write_i64(value)


class F16Node(PrimitiveNode[float]):
    size = 2

    def read_from(self, reader):
        return reader.read_f16()

    def write_to(self, value, writer):
        return writer.write_f16(value)


class F32Node(PrimitiveNode[float]):
    size = 4

    def read_from(self, reader):
        return reader.read_f32()

    def write_to(self, value, writer):
        return writer.write_f32(value)


class F64Node(PrimitiveNode[float]):
    size = 8

    def read_from(self, reader):
        return reader.read_f64()

    def write_to(self, value, writer):
        return writer.write_f64(value)


@dataclass(frozen=True)
class StringNode(TypeNode[str]):
    """StringNode is either a length-prefixed string or a C-style string.

    If type_info is None, it is a C-style string.
    If type_info is a TypeNode, it is a length-prefixed string with the given type as the length encoding.
    """

    size_node: TypeNode[int] | None = None
    encoding: str = "utf-8"
    errors: str = "surrogateescape"

    def read_from(self, reader):
        if self.size_node is None:
            # C-style string
            return reader.read_string_c()
        else:
            # Length-prefixed string
            length = self.size_node.read_from(reader)
            return reader.read(length).decode(self.encoding, self.errors)

    def write_to(self, value, writer):
        if self.size_node is None:
            # C-style string
            return writer.write_string_c(value)
        else:
            # Length-prefixed string
            encoded_value = value.encode(self.encoding, self.errors)
            total_size = self.size_node.write_to(len(encoded_value), writer)
            total_size += writer.write(encoded_value)
            return total_size


@dataclass(frozen=True)
class ListNode(TypeNode[list[T]]):
    """ListNode relates to a list of parsable nodes of the same type as type_info encoded with a variable length encoding.

    The first element of type_info is the length encoding type.
    The second element of type_info is the type of the list elements.
    """

    elem_node: TypeNode[T]
    size_node: TypeNode[int]

    def read_from(self, reader):
        length = self.size_node.read_from(reader)
        return [self.elem_node.read_from(reader) for _ in range(length)]

    def write_to(self, value: Sequence[T], writer) -> int:
        total_size = self.size_node.write_to(len(value), writer)
        total_size += sum(self.elem_node.write_to(element, writer) for element in value)
        return total_size


@dataclass(frozen=True)
class TupleNode(TypeNode[tuple[T, ...]]):
    """TupleNode relates to a tuple of parsable nodes of different types."""

    nodes: tuple[TypeNode[T], ...]

    def read_from(self, reader):
        return tuple(node.read_from(reader) for node in self.nodes)

    def write_to(self, value: tuple[T, ...], writer) -> int:
        return sum(node.write_to(val, writer) for node, val in zip(self.nodes, value))


@dataclass(frozen=True)
class ClassNode(TypeNode[T]):
    """ClassNode relates to a class of parsable nodes of different types."""

    nodes: tuple[TypeNode, ...]
    names: tuple[str, ...]
    call: Callable[[dict[str, Any]], T]

    def read_from(self, reader):
        raw_data = {
            name: node.read_from(reader) for name, node in zip(self.names, self.nodes)
        }
        return self.call(raw_data)

    def write_to(self, value: T, writer):
        return sum(
            node.write_to(getattr(value, name), writer)
            for name, node in zip(self.names, self.nodes)
        )


@dataclass(frozen=True)
class StructNode[T: Serializable](TypeNode[T]):
    """StructNode relates to a struct of parsable nodes of different types."""

    clz: type[T]

    def read_from(self, reader):
        return self.clz.read_from(reader)

    def write_to(self, value: T, writer):
        return value.write_to(writer)


__all__ = (
    "TypeNode",
    "PrimitiveNode",
    "StringNode",
    "ListNode",
    "TupleNode",
    "ClassNode",
    "StructNode",
    "U8Node",
    "U16Node",
    "U32Node",
    "U64Node",
    "I8Node",
    "I16Node",
    "I32Node",
    "I64Node",
    "F16Node",
    "F32Node",
    "F64Node",
)
