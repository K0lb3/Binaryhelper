import pytest

from bier.EndianedBinaryIO import EndianedBytesIO
from bier.serialization.TypeNode import (
    BytesNode,
    F16Node,
    F32Node,
    F64Node,
    I8Node,
    I16Node,
    I32Node,
    I64Node,
    PrimitiveNode,
    StaticLengthNode,
    StringNode,
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


@pytest.mark.parametrize(
    "string, size, error",
    [
        # string nodes
        ("Hello, World!", None, None),
        ("Hello, World!", U8Node(), None),
        ("Hello, World!", I64Node(), None),
        ("Hello, World!", StaticLengthNode(13), None),
        ("Hello, World!", StaticLengthNode(20), AssertionError),
        # (b"Hello, World!", None, None),
        (b"Hello, World!", U8Node(), None),
        (b"Hello, World!", I64Node(), None),
        (b"Hello, World!", StaticLengthNode(13), None),
        (b"Hello, World!", StaticLengthNode(20), AssertionError),
    ],
)
def test_string_bytes_node(
    string: str | bytes, size: TypeNode[int] | None, error: type | None
):
    writer = EndianedBytesIO(endian="<")
    if isinstance(string, str):
        node = StringNode(size)
    elif isinstance(string, bytes):
        assert size is not None, "Size must be provided for bytes nodes"
        node = BytesNode(size)

    if error:
        with pytest.raises(error):
            node.write_to(string, EndianedBytesIO(endian="<"))  # type: ignore
    else:
        writer = EndianedBytesIO(endian="<")
        node.write_to(string, writer)  # type: ignore
        writer.seek(0)
        read_string = node.read_from(writer)
        assert read_string == string
