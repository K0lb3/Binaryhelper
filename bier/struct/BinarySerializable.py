import inspect
from typing import (
    Any,
    Generic,
    GenericAlias,  # type: ignore
    Optional,
    Self,
    Type,
    TypeVar,
    _AnnotatedAlias,  # type: ignore
)

from ..EndianedBinaryIO import (
    EndianedBytesIO,
    EndianedReaderIOBase,
    EndianedWriterIOBase,
)
from .TypeNode import (
    ClassNode,
    ListNode,
    PrimitiveNode,
    StringNode,
    TupleNode,
    TypeNode,
    TypeNodeType,
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

METAMAP = {
    u8: TypeNodeType.U8,
    u16: TypeNodeType.U16,
    u32: TypeNodeType.U32,
    u64: TypeNodeType.U64,
    i8: TypeNodeType.I8,
    i16: TypeNodeType.I16,
    i32: TypeNodeType.I32,
    i64: TypeNodeType.I64,
    f16: TypeNodeType.F16,
    f32: TypeNodeType.F32,
    f64: TypeNodeType.F64,
}

T = TypeVar("T")

NODE_MAPS: dict[Type[object], ClassNode] = {}


class BinarySerializable(Generic[T]):
    @classmethod
    def _get_node(cls) -> TypeNode:
        return parse_annotation(
            name=cls.__name__,
            annotation=cls,
            ctx=cls,  # type: ignore
        )

    @classmethod
    def from_parsed_dict(cls, parsed_dict: dict[str, Any]) -> Self:
        return cls(**parsed_dict)

    @classmethod
    def parse(cls, src: EndianedReaderIOBase | bytes | bytearray | memoryview) -> Self:
        """Parse the data into the appropriate type."""
        if not isinstance(src, EndianedReaderIOBase):
            reader = EndianedBytesIO(src)
        else:
            reader = src
        return cls._get_node().parse(reader)

    def dump(self, writer: EndianedWriterIOBase | None = None) -> bytes:
        """Write the data to the appropriate type."""
        if writer is None:
            writer = EndianedBytesIO()
        size = self._get_node().write(writer, self)
        writer.seek(-size, 1)
        raw = writer.read(size)
        writer.seek(size, 1)
        return raw


def resolve_annotation_str(annotation: str, ctx: Optional[Any] = None) -> Any:
    # if inspect.ismodule(ctx):
    #    module = ctx
    if ctx and hasattr(ctx, "__module__"):
        # Try to get the class from the same module as ctx
        module = inspect.getmodule(ctx)
    else:
        module = None

    try:
        return eval(annotation, globals(), module.__dict__ if module else None)
    except NameError:
        raise ValueError(
            f"Could not resolve annotation '{annotation}'. Make sure the class is defined in the same module or import it."
        )


def get_generic_length(ctx: Optional[type]) -> PrimitiveNode:
    if ctx is None:
        raise ValueError(f"Trying to get generic length, but none specified for {ctx}")
    for base in ctx.__orig_bases__:
        if "BinarySerializable" in str(base):
            for arg in base.__args__:
                if arg in METAMAP:
                    return PrimitiveNode(METAMAP[arg])
    else:
        raise ValueError(f"Trying to get generic length, but none specified for {ctx}")


def parse_annotation(
    annotation: Any, name: str | None = None, ctx: Optional[type] = None
) -> TypeNode:  # type: ignore
    while isinstance(annotation, str):
        annotation = resolve_annotation_str(annotation, ctx)

    if annotation in METAMAP:
        node_type = METAMAP[annotation]
        if (
            node_type > TypeNodeType.PRIMITIVE_START
            and node_type < TypeNodeType.PRIMITIVE_END
        ):
            return PrimitiveNode(node_type)
    elif annotation is cstr:
        return StringNode()
    elif annotation is str:
        return StringNode(get_generic_length(ctx))
    elif annotation is list:
        raise NotImplementedError("List without type is not supported")

    elif isinstance(annotation, _AnnotatedAlias):
        # _AnnotatedAlias - should be caught by the first checks
        raise NotImplementedError(
            f"Unsupported type: {annotation}. Only Annotated and UnionType are supported."
        )

    elif isinstance(annotation, GenericAlias):
        origin = annotation.__origin__
        args = getattr(annotation, "__args__", None)
        if origin is list:
            assert args and len(args) == 1, "list must have a tuple argument"
            elem_node = parse_annotation(
                annotation=args[0],
                ctx=ctx,
            )
            return ListNode(
                elem_node,
                get_generic_length(ctx),
            )
        elif origin is tuple:
            assert args and len(args) >= 1, "tuple must have at least one argument"
            return TupleNode(
                tuple(
                    parse_annotation(name="", annotation=typ, ctx=ctx) for typ in args
                ),
            )
        elif origin is str_d:
            assert args and len(args) == 1, "str_d must have one argument"
            size_node = parse_annotation(
                annotation=args[0],
                ctx=ctx,
            )
            assert isinstance(size_node, PrimitiveNode)
            return StringNode(size_node)
        elif origin is list_d:
            assert args and len(args) == 2, "list_d must have two arguments"
            type_node = parse_annotation(
                annotation=args[0],
                ctx=ctx,
            )
            size_node = parse_annotation(
                annotation=args[1],
                ctx=ctx,
            )
            assert isinstance(size_node, PrimitiveNode)
            return ListNode(type_node, size_node)
        else:
            raise NotImplementedError(f"Unsupported type: {annotation}")
    elif (
        inspect.isclass(annotation) and issubclass(annotation, BinarySerializable)
    ) or issubclass(annotation.__class__, BinarySerializable):
        # should be a class...
        clz = annotation if inspect.isclass(annotation) else annotation.__class__
        if clz in NODE_MAPS:
            return NODE_MAPS[clz]
        node = ClassNode(
            names=tuple(),
            nodes=tuple(),
            call=clz.from_parsed_dict,
        )
        NODE_MAPS[clz] = node
        names = []
        nodes = []
        for name, typ in clz.__annotations__.items():
            if name == "__class__":
                continue
            names.append(name)
            nodes.append(parse_annotation(name=name, annotation=typ, ctx=clz))
        node.names = tuple(names)
        node.nodes = tuple(nodes)
        return node
    else:
        raise NotImplementedError()


__all__ = ("BinarySerializable",)
