from ..BinarySerializable import BinarySerializable, SerializationContext
from ...EndianedBinaryIO import (
    EndianedReaderIOBase,
    EndianedWriterIOBase,
    EndianedBytesIO,
)
from ..TypeNode import (
    ClassNode,
    ClassNodeMember,
    VarIntNode,
    SVarIntNode,
    I64Node,
    U64Node,
    F64Node,
    I32Node,
    U32Node,
    F32Node,
    StringNode,
    BytesNode,
    TypeNode,
    StructNode,
)
from ..options import custom_root_node

from typing import TypedDict, NotRequired, cast, ClassVar
from dataclasses import dataclass
from enum import IntEnum


class ProtoMetadata(TypedDict):
    id: int
    fixed: NotRequired[bool]


class ProtoWireType(IntEnum):
    VARINT = 0
    I64 = 1
    LEN = 2
    SGROUP = 3
    EGROUP = 4
    I32 = 5


@dataclass(slots=True)
class ProtoTag(BinarySerializable):
    id: int
    type: ProtoWireType

    @classmethod
    def read_from(cls, reader: EndianedReaderIOBase, context=None):
        value = reader.read_varint()
        return cls(value >> 3, ProtoWireType(value & 0b111))

    def write_to(self, writer: EndianedWriterIOBase, context=None):
        return writer.write_varint((self.id << 3) | self.type.value)


class ProtoClassNode[T](ClassNode[T]):
    PROTO_TYPE_MAPPING: ClassVar[dict[type[TypeNode], ProtoWireType]] = {
        VarIntNode: ProtoWireType.VARINT,
        SVarIntNode: ProtoWireType.VARINT,
        I64Node: ProtoWireType.I64,
        U64Node: ProtoWireType.I64,
        F64Node: ProtoWireType.I64,
        I32Node: ProtoWireType.I32,
        U32Node: ProtoWireType.I32,
        F32Node: ProtoWireType.I32,
        StringNode: ProtoWireType.LEN,
        BytesNode: ProtoWireType.LEN,
        StructNode: ProtoWireType.LEN,
    }

    def read_from(self, reader: EndianedReaderIOBase, context: SerializationContext):
        read_context = context.fork({})

        pos = reader.tell()
        reader.seek(0, 2)
        end = reader.tell()
        reader.seek(pos, 0)

        members_by_id: dict[int, ClassNodeMember] = {}
        for member in self.members:
            meta: ProtoMetadata = cast(ProtoMetadata, member.metadata)
            member_id = meta["id"]
            if member_id in members_by_id:
                raise ValueError(
                    f"Duplicate field ID {member_id} for {members_by_id[member_id].name} and {member.name}"
                )

            members_by_id[member_id] = member

        while reader.tell() != end:
            tag = ProtoTag.read_from(reader, read_context)
            if (member := members_by_id.get(tag.id, None)) is not None:
                if tag.type == ProtoWireType.LEN:
                    length = reader.read_varint()
                    data = reader.read_bytes(length)
                    with EndianedBytesIO(data) as sub_reader:
                        read_context.state[member.name] = member.node.read_from(
                            sub_reader, read_context.fork(metadata=member.metadata)
                        )
                else:
                    read_context.state[member.name] = member.node.read_from(
                        reader, read_context.fork(metadata=member.metadata)
                    )
            else:
                match tag.type:
                    case ProtoWireType.VARINT:
                        _ = reader.read_varint()
                    case ProtoWireType.I64:
                        _ = reader.read_i64()
                    case ProtoWireType.LEN:
                        _ = reader.read_bytes(reader.read_varint())
                    case ProtoWireType.I32:
                        _ = reader.read_i32()

        for member in self.members:
            if member.name not in read_context.state:
                read_context.state[member.name] = None

        return self.call(read_context.state)

    def write_to(
        self, value: T, writer: EndianedWriterIOBase, context: SerializationContext
    ):
        size = 0

        write_context = context.fork(T)
        for member in self.members:
            member_value = getattr(value, member.name)
            if member_value is None:
                continue

            meta = cast(ProtoMetadata, member.metadata)
            tag = ProtoTag(meta["id"], self.PROTO_TYPE_MAPPING[type(member.node)])

            size += tag.write_to(writer, write_context)
            if tag.type == ProtoWireType.LEN:
                with EndianedBytesIO() as sub_writer:
                    sub_size = member.node.write_to(
                        member_value,
                        sub_writer,
                        write_context.fork(metadata=member.metadata),
                    )

                    size += writer.write_varint(sub_size)
                    size += writer.write_bytes(sub_writer.getvalue(), False)
            else:
                size += member.node.write_to(
                    member_value, writer, write_context.fork(metadata=member.metadata)
                )

        return size


class ProtoBinarySerializable(BinarySerializable[custom_root_node[ProtoClassNode]]):
    pass


__all__ = [
    "ProtoBinarySerializable",
    "ProtoMetadata",
]
