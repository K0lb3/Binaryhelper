from abc import ABCMeta
from dataclasses import dataclass, replace
from functools import cache
from inspect import isclass
from types import get_original_bases
from typing import (
    Annotated,
    Any,
    Literal,
    Self,
    TypeVar,
    get_args,
    get_origin,
    get_type_hints,
)

from .Serializable import Serializable
from .TypeNode import (
    BytesNode,
    ClassNode,
    ListNode,
    StaticLengthNode,
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
    length_type: TypeNode[int] = U32Node()


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

default_length_encoding = length_type
"""
Used for specifying the default type to use when reading a length-providing field.
Must be serializing an 'int' type.
"""

T = TypeVar("T", bound=length_type)

type list_d[TType, LType: length_type] = custom[list[TType], LType]
str_d = custom[str, T]
bytes_d = custom[bytes, T]


def resolve_genericalias(origin: type, args: tuple[Any, ...]):
    """
    Remap a GenericAlias instance to its underlying __value__ GenericAlias,
    preserving the correct parameter mapping.
    """
    src_parameters = getattr(origin, "__parameters__", ())
    dst_generic = getattr(origin, "__value__", None)
    dst_parameters = getattr(dst_generic, "__parameters__", ())

    if not dst_generic:
        raise TypeError(f"Origin {origin} does not define __value__, cannot resolve.")

    if len(src_parameters) != len(dst_parameters):
        raise ValueError(
            f"Generic type {origin} has {len(src_parameters)} parameters, "
            f"but {len(dst_parameters)} were expected."
        )

    # Fast path: identical parameter order
    if src_parameters == dst_parameters:
        return dst_generic[args]

    # Build mapping: src_param → arg
    param_to_arg = dict(zip(src_parameters, args))

    # Remap args to dst_parameter order
    try:
        reordered_args = tuple(param_to_arg[dst_param] for dst_param in dst_parameters)
    except KeyError as e:
        raise ValueError(
            f"Parameter {e.args[0]} in destination generic is missing in source parameters."
        ) from None

    return dst_generic[reordered_args]


def get_origin_type(cls: type) -> type:
    return get_origin(cls) or cls


def get_binary_serializable_spec(cls: type[BinarySerializable]) -> Any:
    for base in get_original_bases(cls):
        if get_origin_type(base) is BinarySerializable:
            return base

    raise ValueError(f"Type {cls} does not inherit from BinarySerializable")


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

    if get_origin(val) is Literal:
        val_args = get_args(val)
        assert len(val_args) == 1, "Length type must be a Literal with one argument"
        assert isinstance(val_args[0], int), "Length type must be an int"
        return StaticLengthNode(val_args[0])

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
        return StringNode(options.length_type)

    if origin is bytes:
        return BytesNode(options.length_type)

    if origin is list:
        assert len(args) == 1, "list must have one argument"
        elem_node = parse_annotation(
            annotation=args[0],
            options=options,
        )
        return ListNode(elem_node, options.length_type)

    if origin is tuple:
        return TupleNode(
            tuple(parse_annotation(annotation=arg, options=options) for arg in args)
        )

    if origin is custom:
        assert len(args) >= 1, "member must have at least one argument"
        member_type = args[0]
        member_options = replace(options)
        for option in args[1:]:
            member_options = parse_option(option, member_options)

        return parse_annotation(member_type, member_options)

    if origin is member:
        assert len(args) == 1, "member must have one argument"
        return parse_annotation(args[0], options)

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

    if get_origin(annotation.__value__) is custom:
        return parse_annotation(
            resolve_genericalias(origin, args),
            options=options,
        )

    raise NotImplementedError(
        f"Unsupported annotation: {annotation}. "
        f"Please use a type from the bier.struct module."
    )


def parse_option(
    option_type: Any, options: BinarySerializableOptions
) -> BinarySerializableOptions:
    arguments = get_args(option_type)
    origin = get_origin_type(option_type)

    if origin is length_type:
        new_encoding = parse_length_type(arguments[0])
        return replace(options, length_type=new_encoding)

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


__all__ = (
    "BinarySerializable",
    "length_type",
    "member",
    "default_length_encoding",
    "custom",
    "static_length",
    "list_d",
    "str_d",
    "bytes_d",
)
