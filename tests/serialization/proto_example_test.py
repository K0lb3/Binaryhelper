from bier.serialization.examples.protobuf import ProtoBinarySerializable, ProtoMetadata
from bier.serialization import custom, u32, uvarint, f64
from dataclasses import dataclass
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from typing import Annotated as custom


@dataclass
class ProtoTestType(ProtoBinarySerializable):
    field_0: custom[uvarint, ProtoMetadata(id=1)]
    field_1: custom[u32, ProtoMetadata(id=2)]
    field_2: custom[f64, ProtoMetadata(id=15)]


def test_proto_serialization():
    a = ProtoTestType(500, 700, 5.315)
    b = ProtoTestType.from_bytes(a.to_bytes())
    assert a == b
    assert a.to_bytes().hex() == "08f40315bc02000079c3f5285c8f421540"
