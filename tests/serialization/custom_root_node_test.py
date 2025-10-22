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
        def read_from(self, reader: EndianedReaderIOBase, context):
            read_context = context.fork()
            for name, node in zip(self.names, self.nodes):
                if not reader.read_bool():
                    read_context.state[name] = None
                else:
                    read_context.state[name] = node.read_from(reader, read_context)

            return self.call(read_context.state)

        def write_to(self, value: T, writer: EndianedWriterIOBase, context):
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
