import sys

import pytest

if sys.version_info < (3, 12):

    def test_import_error():
        with pytest.raises(ImportError):
            from bier import serialization  # noqa: F401

else:
    from dataclasses import dataclass
    from enum import IntEnum, StrEnum
    from typing import Literal

    from bier.EndianedBinaryIO import EndianedBytesIO
    from bier.serialization import (
        BinarySerializable,
        cstr,
        custom,
        member_length,
        prefixed_length,
        static_length,
        u8,
    )
    from bier.serialization.TypeNode import (
        BytesNode,
        ClassNode,
        ConvertNode,
        EnumNode,
        F16Node,
        F32Node,
        F64Node,
        I8Node,
        I16Node,
        I32Node,
        I64Node,
        ListNode,
        MemberLengthNode,
        PrimitiveNode,
        StaticLengthNode,
        StringNode,
        StructNode,
        TupleNode,
        TypeNode,
        U8Node,
        U16Node,
        U32Node,
        U64Node,
    )
    from tests.EndianedBinaryIO.EndianedIOTestHelper import EndianedIOTestHelper

    HELPER = EndianedIOTestHelper()

    def test_primitive_node_singleton():
        class TestPrimitiveNode(PrimitiveNode):
            def read_from(self, reader, context=None):
                raise NotImplementedError("This is a test node, not for actual use.")

            def write_to(self, value, writer, context=None):
                raise NotImplementedError("This is a test node, not for actual use.")

        node1 = TestPrimitiveNode()
        node2 = TestPrimitiveNode()
        assert node1 is node2, "PrimitiveNode should be a singleton"

    @pytest.mark.parametrize(
        "Node, name",
        [
            (U8Node, "u8"),
            (U16Node, "u16"),
            (U32Node, "u32"),
            (U64Node, "u64"),
            (I8Node, "i8"),
            (I16Node, "i16"),
            (I32Node, "i32"),
            (I64Node, "i64"),
            (F16Node, "f16"),
            (F32Node, "f32"),
            (F64Node, "f64"),
        ],
    )
    def test_primitive_node_read_write(Node: type[PrimitiveNode], name: str):
        node = Node()
        # Test singleton
        node2 = Node()
        assert node is node2

        # Test write
        values = getattr(HELPER, name)
        writer = EndianedBytesIO(endian="<")
        for value in values:
            node.write_to(value, writer)

        bytes_written = writer.tell()
        assert bytes_written == node.size * len(values), (
            f"Expected {node.size * len(values)} bytes, got {bytes_written}"
        )

        raw = writer.getvalue()
        expected_raw = getattr(HELPER, f"raw_{name}_le")
        assert raw == expected_raw, f"Expected {expected_raw}, got {raw}"

        # Test read
        writer.seek(0)
        values_read = [node.read_from(writer) for _ in range(len(values))]
        assert values_read == values, f"Expected {values}, got {values_read}"

        assert writer.tell() == node.size * len(getattr(HELPER, name))

    class DummyIntEnum(IntEnum):
        X = 1
        Y = 2

    class DummyStrEnum(StrEnum):
        X = "x"

    @dataclass(slots=True)
    class DummyClass(BinarySerializable):
        u8v: u8
        strv: cstr

    @dataclass(slots=True)
    class DummyClassWithPrefixedLength(BinarySerializable):
        string_value: custom[str, prefixed_length[u8]]

    @dataclass(slots=True)
    class DummyClassWithStaticLength(BinarySerializable):
        string_value: custom[str, static_length[12]]

    @dataclass(slots=True)
    class DummyClassWithMemberLength(BinarySerializable):
        string_length: u8
        string_value: custom[str, member_length[Literal["string_length"]]]

    @pytest.mark.parametrize(
        "node_cls, instance_args, value, expected_raw, error",
        [
            # StringNode
            (StringNode, (), "StringNode", b"StringNode\x00", None),
            (StringNode, (U8Node(),), "StringNode", b"\x0aStringNode", None),
            (StringNode, (StaticLengthNode(10),), "StringNode", b"StringNode", None),
            (
                StringNode,
                (StaticLengthNode(12),),
                "StringNode",
                b"StringNode\x00\x00",
                AssertionError,
            ),
            # BytesNode
            (BytesNode, (U8Node(),), b"BytesNode", b"\x09BytesNode", None),
            (BytesNode, (StaticLengthNode(9),), b"BytesNode", b"BytesNode", None),
            (
                BytesNode,
                (StaticLengthNode(11),),
                b"BytesNode",
                b"BytesNode\x00\x00",
                AssertionError,
            ),
            # ListNode
            (
                ListNode,
                (U8Node(), StaticLengthNode(HELPER.count)),
                HELPER.u8,
                HELPER.raw_u8_le,
                None,
            ),
            (
                ListNode,
                (F64Node(), StaticLengthNode(HELPER.count)),
                HELPER.f64,
                HELPER.raw_f64_le,
                None,
            ),
            (
                ListNode,
                (StringNode(), U8Node()),
                ["X", "YZ", ""],
                b"\x03X\00YZ\00\00",
                None,
            ),
            # TupleNode
            (
                TupleNode,
                ((U8Node(), U16Node(), U32Node()),),
                (1, 2, 3),
                b"\x01\x02\x00\x03\x00\x00\x00",
                None,
            ),
            (
                TupleNode,
                (
                    (
                        BytesNode(StaticLengthNode(9)),
                        ListNode(U8Node(), StaticLengthNode(2)),
                    ),
                ),
                (b"TupleNode", [1, 2]),
                b"TupleNode\x01\x02",
                None,
            ),
            # ClassNode
            (
                ClassNode,
                (
                    # nodes
                    (U8Node(), StringNode()),
                    # names
                    ("u8v", "strv"),
                    # call
                    DummyClass.from_dict,
                ),
                DummyClass(u8v=1, strv="test"),
                b"\x01test\x00",
                None,
            ),
            # StructNode
            (
                StructNode,
                (DummyClass,),
                DummyClass(u8v=1, strv="test"),
                b"\x01test\x00",
                None,
            ),
            # EnumNode
            (
                EnumNode,
                (DummyIntEnum, U8Node()),
                DummyIntEnum.X,
                b"\x01",
                None,
            ),
            (
                EnumNode,
                (DummyStrEnum, StringNode()),
                DummyStrEnum.X,
                b"x\x00",
                None,
            ),
            # ConvertNode
            (
                ConvertNode,
                (U16Node(), lambda x: x + 1, lambda x: x - 1),
                1,
                b"\x00\x00",
                None,
            ),
            # StaticLengthNode
            # ClassNode
            (
                ClassNode,
                (
                    # nodes
                    (StringNode(StaticLengthNode(len("cool string!"))),),
                    # names
                    ("string_value",),
                    # call
                    DummyClassWithStaticLength.from_dict,
                ),
                DummyClassWithStaticLength(string_value="cool string!"),
                b"cool string!",
                None,
            ),
            # PrefixedLengthNode
            # ClassNode
            (
                ClassNode,
                (
                    # nodes
                    (StringNode(U8Node()),),
                    # names
                    ("string_value",),
                    # call
                    DummyClassWithPrefixedLength.from_dict,
                ),
                DummyClassWithPrefixedLength(string_value="cool string!"),
                b"\x0ccool string!",
                None,
            ),
            # MemberLengthNode
            # ClassNode
            (
                ClassNode,
                (
                    # nodes
                    (U8Node(), StringNode(MemberLengthNode("string_length"))),
                    # names
                    ("string_length", "string_value"),
                    # call
                    DummyClassWithMemberLength.from_dict,
                ),
                DummyClassWithMemberLength(
                    string_length=len("cool string!"), string_value="cool string!"
                ),
                b"\x0ccool string!",
                None,
            ),
        ],
    )
    def test_nodetype(
        node_cls: type[TypeNode],
        instance_args: tuple,
        value,
        expected_raw: bytes,
        error: type[Exception] | None,
    ):
        node = node_cls(*instance_args)

        # Test write
        writer = EndianedBytesIO(endian="<")
        try:
            node.write_to(value, writer)
        except Exception as e:
            if error and isinstance(e, error):
                return
            raise e

        raw = writer.getvalue()
        assert raw == expected_raw, f"Expected {expected_raw}, got {raw}"

        # Test read
        writer.seek(0)
        value_read = node.read_from(writer)
        assert value_read == value, f"Expected {value}, got {value_read}"
