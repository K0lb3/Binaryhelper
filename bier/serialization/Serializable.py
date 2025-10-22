from abc import ABCMeta, abstractmethod
from typing import Any, Self

import dataclasses

from ..EndianedBinaryIO import (
    EndianedBytesIO,
    EndianedReaderIOBase,
    EndianedWriterIOBase,
    Endianess,
)


@dataclasses.dataclass(frozen=True)
class SerializationContext:
    settings: dict[str, Any] = dataclasses.field(default_factory=dict)
    state: dict[str, Any] | Any = dataclasses.field(
        default_factory=dict
    )  # note: ideally this is either generic or BinarySerializable

    def fork(self, state: dict[str, Any] | Any | None = None) -> "SerializationContext":
        return SerializationContext(self.settings, state if state is not None else {})


class Serializable(metaclass=ABCMeta):
    @classmethod
    @abstractmethod
    def read_from(
        cls,
        reader: EndianedReaderIOBase,
        context: SerializationContext | None = None,
    ) -> Self: ...

    @classmethod
    def from_bytes(
        cls,
        data: bytes,
        endian: Endianess = "<",
        context: SerializationContext | None = None,
    ):
        with EndianedBytesIO(data, endian) as reader:
            return cls.read_from(reader, context or SerializationContext())

    @abstractmethod
    def write_to(
        self,
        writer: EndianedWriterIOBase,
        context: SerializationContext | None = None,
    ) -> int: ...

    def to_bytes(
        self, endian: Endianess = "<", context: SerializationContext | None = None
    ) -> bytes:
        with EndianedBytesIO(endian=endian) as writer:
            self.write_to(writer, context or SerializationContext())
            return writer.getvalue()


class Serializer[T](metaclass=ABCMeta):
    @abstractmethod
    def read_from(
        self,
        reader: EndianedReaderIOBase,
        context: SerializationContext,
    ) -> T: ...

    def from_bytes(
        self,
        data: bytes,
        endian: Endianess = "<",
        context: SerializationContext | None = None,
    ):
        with EndianedBytesIO(data, endian) as reader:
            return self.read_from(reader, context or SerializationContext())

    @abstractmethod
    def write_to(
        self,
        value: T,
        writer: EndianedWriterIOBase,
        context: SerializationContext,
    ) -> int: ...

    def to_bytes(
        self,
        value: T,
        endian: Endianess = "<",
        context: SerializationContext | None = None,
    ) -> bytes:
        with EndianedBytesIO(endian=endian) as writer:
            self.write_to(value, writer, context or SerializationContext())
            return writer.getvalue()
