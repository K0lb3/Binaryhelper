import sys

import pytest

if sys.version_info < (3, 12):

    def test_import_error():
        with pytest.raises(ImportError):
            from bier import serialization  # noqa: F401

else:
    from bier.serialization import (
        custom_root_node,
        BinarySerializable,
        u8,
        u16,
        u32,
        u64,
        ClassNode,
        SerializationContext,
        custom,
    )
    from bier.EndianedBinaryIO import EndianedReaderIOBase, EndianedWriterIOBase
    from dataclasses import dataclass
    from typing import TYPE_CHECKING, TypedDict, NotRequired

    # this is used to silence warnings for custom usage
    # include this in your own code if you want!
    if TYPE_CHECKING:
        from typing import Annotated as custom

    @dataclass(frozen=True)
    class CustomClassNode[T](ClassNode[T]):
        def read_from(self, reader, context):
            return super().read_from(reader, context)

        def write_to(self, value, writer, context):
            return super().write_to(value, writer, context)

    @dataclass(slots=True)
    class CustomBasicStruct(BinarySerializable[custom_root_node[CustomClassNode]]):
        field_0: u64
        field_1: str
        field_2: bytes

    def test_custom_wrapper_nochanges():
        custom_no_changes = CustomBasicStruct(
            1337, "aaa", bytes([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
        )
        print(custom_no_changes.to_bytes())
        custom_no_changes_rtt = CustomBasicStruct.from_bytes(
            custom_no_changes.to_bytes()
        )
        assert custom_no_changes == custom_no_changes_rtt

    @dataclass(frozen=True)
    class CustomOptionalClassNode[T](ClassNode[T]):
        def _read_member(self, member, reader, context):
            if not reader.read_bool():
                return None

            return super()._read_member(member, reader, context)

        def _write_member(self, value, member, writer, context):
            size = 0

            member_value = getattr(value, member.name)
            is_not_none = member_value is not None

            size += writer.write_bool(is_not_none)
            if is_not_none:
                size += super()._write_member(value, member, writer, context)

            return size

    @dataclass(slots=True)
    class MaybeOptionalClass(
        BinarySerializable[custom_root_node[CustomOptionalClassNode]]
    ):
        field_0: u8 | None
        field_1: u16 | None
        field_2: str | None

    def test_maybe_optional():
        aaa = MaybeOptionalClass(5, 100, "")
        assert MaybeOptionalClass.from_bytes(aaa.to_bytes()) == aaa

        bbb = MaybeOptionalClass(None, 135, None)
        assert MaybeOptionalClass.from_bytes(bbb.to_bytes()) == bbb

    # NOTE: This custom class node does not set metadata in SerializationContext, meaning
    # that metadata has to be handled in the class node instead of the type nodes.
    @dataclass(frozen=True)
    class TLVClassNode[T](ClassNode[T]):
        def read_from(
            self, reader: EndianedReaderIOBase, context: SerializationContext
        ):
            read_context = context.fork(state={})

            fields_by_id = {
                member.metadata.get("id", 0): member for member in self.members
            }

            assert reader.read_u8() == 0x50, "invalid struct start"
            while True:
                id = reader.read_u8()
                if id == 0xFF:
                    break

                type = reader.read_u8()

                member = fields_by_id[id]
                assert member.metadata.get("type_id", 0) == type, "Invalid type id"

                read_context.state[member.name] = member.node.read_from(
                    reader, read_context
                )

            for member in fields_by_id.values():
                if member.name not in read_context.state:
                    read_context.state[member.name] = None

            return self.call(read_context.state)

        def write_to(
            self, value, writer: EndianedWriterIOBase, context: SerializationContext
        ):
            write_context = context.fork(state=value)

            size = 0

            size += writer.write_u8(0x50)

            for member in self.members:
                member_value = getattr(value, member.name)
                if member_value is None:
                    continue

                assert member.metadata.get("id", 0) != 0xFF, "invalid member id 0xFF"

                size += writer.write_u8(member.metadata.get("id", 0))
                size += writer.write_u8(member.metadata.get("type_id", 0))
                size += member.node.write_to(member_value, writer, write_context)

            size += writer.write_u8(0xFF)

            return size

    @dataclass(slots=True)
    class TLVTestClass(BinarySerializable[custom_root_node[TLVClassNode]]):
        field_0: custom[u32 | None, {"id": 1, "type_id": 0}]  # noqa: F821
        field_1: custom[str | None, {"id": 2, "type_id": 1, "is_cool": True}]
        field_2: custom[str | None, {"id": 3, "type_id": 1}]  # noqa: F821

    def test_tlv_classnode():
        aaa = TLVTestClass(1337, "nyaaaaaa <|:}", ":33333")
        assert TLVTestClass.from_bytes(aaa.to_bytes()) == aaa  # pyright: ignore[reportInvalidTypeForm, reportGeneralTypeIssues]

    class SubclassedSerializable(BinarySerializable[custom_root_node[TLVClassNode]]):
        pass

    @dataclass(slots=True)
    class TLVTestClass2(SubclassedSerializable):
        field_0: custom[u32 | None, {"id": 1, "type_id": 0}]  # noqa: F821
        field_1: custom[str | None, {" id": 2, " type_id": 1, "is_cool": True}]
        field_2: custom[str | None, {"id": 3, "type_id": 1}]  # noqa: F821

    def test_subclassed_serializable():
        aaa = TLVTestClass2(1337, "nyaaaaaa <|:}", ":33333")
        assert TLVTestClass.from_bytes(aaa.to_bytes()) == TLVTestClass(
            1337, "nyaaaaaa <|:}", ":33333"
        )

    class TLVInfo(TypedDict):
        id: int
        type_id: int
        is_cool: NotRequired[bool]

    @dataclass(slots=True)
    class TLVTestClass3(SubclassedSerializable):
        field_0: custom[u32 | None, TLVInfo(id=1, type_id=0)]
        field_1: custom[str | None, TLVInfo(id=2, type_id=1, is_cool=True)]
        field_2: custom[str | None, TLVInfo(id=3, type_id=1)]

    def test_subclassed_serializable_typeddict():
        aaa = TLVTestClass3(1337, "nyaaaaaa <|:}", ":33333")
        assert TLVTestClass3.from_bytes(aaa.to_bytes()) == TLVTestClass3(
            1337, "nyaaaaaa <|:}", ":33333"
        )
