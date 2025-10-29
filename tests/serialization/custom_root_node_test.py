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
        metadata,
    )
    from bier.EndianedBinaryIO import EndianedReaderIOBase, EndianedWriterIOBase
    from dataclasses import dataclass

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
        def read_from(
            self, reader: EndianedReaderIOBase, context: SerializationContext
        ):
            read_context = context.fork()
            for name, node in zip(self.names, self.nodes):
                if not reader.read_bool():
                    read_context.state[name] = None
                else:
                    read_context.state[name] = node.read_from(reader, read_context)

            return self.call(read_context.state)

        def write_to(
            self, value: T, writer: EndianedWriterIOBase, context: SerializationContext
        ):
            write_context = context.fork(value)
            size = 0
            for name, node in zip(self.names, self.nodes):
                member_value = getattr(value, name)
                is_not_none = member_value is not None
                size += writer.write_bool(is_not_none)
                if is_not_none:
                    size += node.write_to(member_value, writer, write_context)

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

    @dataclass(frozen=True)
    class TLVClassNode[T](ClassNode[T]):
        def read_from(
            self, reader: EndianedReaderIOBase, context: SerializationContext
        ):
            read_context = context.fork()

            fields_by_id = {
                node_metadata.get("id", 0): (name, node, node_metadata)
                for name, node, node_metadata in zip(
                    self.names, self.nodes, self.metadatas
                )
            }

            assert reader.read_u8() == 0x50, "invalid struct start"
            while True:
                id = reader.read_u8()
                if id == 0xFF:
                    break

                type = reader.read_u8()

                name, node, node_metadata = fields_by_id[id]
                assert node_metadata.get("type_id", 0) == type, "Invalid type id"

                read_context.state[name] = node.read_from(reader, read_context)

            for name, _, _ in fields_by_id.values():
                if name not in read_context.state:
                    read_context.state[name]

            return self.call(read_context.state)

        def write_to(
            self, value, writer: EndianedWriterIOBase, context: SerializationContext
        ):
            write_context = context.fork(value)

            size = 0

            size += writer.write_u8(0x50)

            for name, node, node_metadata in zip(
                self.names, self.nodes, self.metadatas
            ):
                member_value = getattr(value, name)
                if member_value is None:
                    continue

                assert node_metadata.get("id", 0) != 0xFF, "invalid member id 0xFF"

                size += writer.write_u8(node_metadata.get("id", 0))
                size += writer.write_u8(node_metadata.get("type_id", 0))
                size += node.write_to(member_value, writer, write_context)

            size += writer.write_u8(0xFF)

            return size

    @dataclass(slots=True)
    class TLVTestClass(BinarySerializable[custom_root_node[TLVClassNode]]):
        field_0: custom[u32 | None, metadata["id", 1], metadata["type_id", 0]]  # noqa: F821
        field_1: custom[
            str | None,
            metadata["id", 2],  # noqa: F821
            metadata["type_id", 1],  # noqa: F821
            metadata["is_cool", True],  # noqa: F821
        ]
        field_2: custom[str | None, metadata["id", 3], metadata["type_id", 1]]  # noqa: F821

    def test_tlv_classnode():
        aaa = TLVTestClass(1337, "nyaaaaaa <|:}", ":33333")
        assert TLVTestClass.from_bytes(aaa.to_bytes()) == aaa  # pyright: ignore[reportInvalidTypeForm, reportGeneralTypeIssues]

    class SubclassedSerializable(BinarySerializable[custom_root_node[TLVClassNode]]):
        pass

    @dataclass(slots=True)
    class TLVTestClass2(SubclassedSerializable):
        field_0: custom[u32 | None, metadata["id", 1], metadata["type_id", 0]]  # noqa: F821
        field_1: custom[
            str | None,
            metadata[" id", 2],  # noqa: F722
            metadata[" type_id", 1],  # noqa: F722
            metadata["is_cool", True],  # noqa: F821
        ]
        field_2: custom[str | None, metadata["id", 3], metadata["type_id", 1]]  # noqa: F821

    def test_subclassed_serializable():
        aaa = TLVTestClass2(1337, "nyaaaaaa <|:}", ":33333")
        assert TLVTestClass.from_bytes(aaa.to_bytes()) == TLVTestClass(
            1337, "nyaaaaaa <|:}", ":33333"
        )
