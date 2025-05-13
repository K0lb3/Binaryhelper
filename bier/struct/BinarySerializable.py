from dataclasses import dataclass, replace
from inspect import isclass
from functools import cache
from types import get_original_bases
from abc import ABCMeta
from typing import (
    Annotated,
    Any,
    Self,
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
    u8,
    u16,
    u32,
    u64,
)

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


class BinarySerializable[*TOptions](Serializable):
    @classmethod
    def _get_node(cls) -> ClassNode[Self]:
        return build_type_node(cls)

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


class BinarySerializableOption(metaclass=ABCMeta): ...


type AllowedLengthTypes = i8 | i16 | i32 | i64 | u8 | u16 | u32 | u64
type default_length_encoding[T: AllowedLengthTypes] = BinarySerializableOption
type member[TType, *TOptions] = Annotated[TType, *TOptions]


def get_origin_type(cls: type) -> type:
    return get_origin(cls) or cls


def get_binary_serializable_spec(cls: type[BinarySerializable]) -> Any:
    for base in get_original_bases(cls):
        if get_origin_type(base) is BinarySerializable:
            return base

    raise ValueError(f"Type {cls} does not inherit from BinarySerializable")


def parse_length_type(annotation: Annotated) -> TypeNode[int]:
    args = get_args(annotation)
    assert len(args) >= 2, "Length type must have two arguments"
    assert args[0] is int, "Length type must be an int"
    if issubclass(args[1], TypeNode):
        return args[1]()
    elif isinstance(args[1], TypeNode):
        return args[1]
    else:
        raise ValueError(
            f"Length type must be a TypeNode[int] or an instance of it, got {args[1]} instead."
        )


def parse_annotation(annotation: Any, options: BinarySerializableOptions) -> TypeNode:
    if annotation in PRIMITIVES:
        args = get_args(annotation)
        assert len(args) >= 2 and issubclass(args[1], TypeNode), (
            f"Invalid primitive {annotation}"
        )
        return args[1]()

    origin = get_origin_type(annotation)
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

    if origin is tuple:
        return TupleNode(
            tuple(parse_annotation(annotation=arg, options=options) for arg in args)
        )

    if origin is member:
        assert len(args) >= 1, "member must have at least one argument"
        member_type = args[0]
        member_options = replace(options)
        for option in args[1:]:
            member_options = parse_option(option, member_options)

        return parse_annotation(member_type, member_options)

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


def parse_option(
    option_type: Any, options: BinarySerializableOptions
) -> BinarySerializableOptions:
    arguments = get_args(option_type)
    origin = get_origin_type(option_type)

    if origin is default_length_encoding:
        new_encoding = parse_length_type(arguments[0])
        return replace(options, default_length_encoding=new_encoding)

    return options


def get_serialization_options(
    arguments: tuple[Any, ...], current_options: BinarySerializableOptions
) -> BinarySerializableOptions:
    if len(arguments) == 0:
        return current_options

    options = current_options
    for argument in arguments:
        options = parse_option(argument, options)

    return options


def get_type_serialization_options(cls: type[BinarySerializable]):
    spec = get_binary_serializable_spec(cls)
    arguments = get_args(spec)
    return get_serialization_options(arguments, BinarySerializableOptions())


@cache
def build_type_node[T: BinarySerializable](cls: type[T]) -> ClassNode[T]:
    # get default options from the class
    serialization_options = get_type_serialization_options(cls)

    # get all member type hintss
    type_hints = get_type_hints(cls, include_extras=True)

    names = tuple(type_hints.keys())
    nodes = tuple(
        parse_annotation(annotation, serialization_options)
        for annotation in type_hints.values()
    )

    return ClassNode(
        names=names,
        nodes=nodes,
        call=cls.from_parsed_dict,
    )


__all__ = ("BinarySerializable", "default_length_encoding", "member")
