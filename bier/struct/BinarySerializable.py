from dataclasses import dataclass
from inspect import isclass
from typing import (
    Annotated,
    Any,
    Generic,
    Self,
    Type,
    TypeVar,
    get_args,
    get_origin,
    get_type_hints,
)

from .Serializable import Serializable
from .TypeNode import (
    ClassNode,
    ListNode,
    StringNode,
    StructNode,
    TupleNode,
    TypeNode,
    U32Node,
)
from .typing import (
    cstr,
    f16,
    f32,
    f64,
    i8,
    i16,
    i32,
    i64,
    list_d,
    str_d,
    u8,
    u16,
    u32,
    u64,
)

T = TypeVar("T")

PRIMITIVES = (
    u8,
    u16,
    u32,
    u64,
    i8,
    i16,
    i32,
    i64,
    f16,
    f32,
    f64,
)
NODE_MAPS: dict[Type[object], ClassNode] = {}


class BinarySerializable(Serializable, Generic[T]):
    @classmethod
    def _get_node(cls) -> ClassNode[Self]:
        if cls in NODE_MAPS:
            return NODE_MAPS[cls]

        type_hints = get_type_hints(cls, include_extras=True)

        # get default length encoding from the class
        length_encoding = U32Node()
        for base in getattr(cls, "__orig_bases__", ()):
            if get_origin(base) is BinarySerializable:
                args = get_args(base)
                if len(args) == 1:
                    try:
                        length_encoding = parse_annotated_length(args[0])
                        break
                    except Exception:
                        pass
        options = BinarySerializableOptions(length_encoding)

        names = tuple(type_hints.keys())
        nodes = tuple(
            parse_annotation(annotation, options) for annotation in type_hints.values()
        )

        return ClassNode(
            names=names,
            nodes=nodes,
            call=cls.from_parsed_dict,
        )

    @classmethod
    def from_parsed_dict(cls, parsed_dict: dict[str, Any]) -> Self:
        return cls(**parsed_dict)

    @classmethod
    def read_from(cls, reader):
        return cls._get_node().read_from(reader)

    def write_to(self, writer):
        return self._get_node().write_to(self, writer)


@dataclass(frozen=True)
class BinarySerializableOptions:
    default_length_encoding: TypeNode[int] = U32Node()


def parse_annotated_length(annotation: Annotated) -> TypeNode[int]:
    args = get_args(annotation)
    assert len(args) >= 2, "Annotated length must have two arguments"
    assert args[0] is int, "Annotated length must be an int"
    if issubclass(args[1], TypeNode):
        return args[1]()
    elif isinstance(args[1], TypeNode):
        return args[1]
    else:
        raise ValueError(
            f"Annotated length must be a TypeNode[int] or an instance of it, got {args[1]} instead."
        )


def parse_annotation(annotation: type, options: BinarySerializableOptions) -> TypeNode:
    if isinstance(annotation, str):
        raise ValueError(
            f"Annotation is a string: {annotation}. Use resolve_annotation_str to resolve it."
        )

    if annotation in PRIMITIVES:
        args = get_args(annotation)
        assert len(args) >= 2 and issubclass(args[1], TypeNode), (
            f"Invalid primitive {annotation}"
        )
        return args[1]()

    origin = get_origin(annotation) or annotation
    args = get_args(annotation)

    if annotation is cstr:
        return StringNode()

    if annotation is str:
        return StringNode(options.default_length_encoding)

    if origin is list:
        assert len(args) == 1, "list must have one argument"
        elem_node = parse_annotation(
            annotation=args[0],
            options=options,
        )
        return ListNode(elem_node, options.default_length_encoding)

    if origin is list_d:
        assert len(args) == 2, "list_d must have two arguments"
        raw_elem, raw_size = args
        elem_node = parse_annotation(
            annotation=raw_elem,
            options=options,
        )
        size_node = parse_annotated_length(annotation=raw_size)
        return ListNode(elem_node, size_node)

    if origin is tuple:
        return TupleNode(
            tuple(parse_annotation(annotation=arg, options=options) for arg in args)
        )

    if origin is str_d:
        assert len(args) == 1, "str_d must have one argument"
        size_node = parse_annotated_length(annotation=args[0])
        return StringNode(size_node)

    if origin is Annotated:
        for arg in args:
            if isinstance(arg, TypeNode):
                return arg
            elif issubclass(arg, TypeNode):
                return arg()
            elif issubclass(arg, BinarySerializable):
                return StructNode(clz=arg)

    if isclass(annotation) and issubclass(annotation, Serializable):
        return StructNode(clz=annotation)

    raise NotImplementedError(
        f"Unsupported annotation: {annotation}. "
        f"Please use a type from the bier.struct module."
    )


__all__ = ("BinarySerializable",)
