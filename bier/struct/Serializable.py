from abc import ABCMeta, abstractmethod
from typing import Generic, Self, TypeVar

from ..EndianedBinaryIO import (
    EndianedBytesIO,
    EndianedReaderIOBase,
    EndianedWriterIOBase,
    Endianess,
)

T = TypeVar("T")


class Serializable(metaclass=ABCMeta):
    @classmethod
    @abstractmethod
    def read_from(cls, reader: EndianedReaderIOBase) -> Self: ...

    @classmethod
    def from_bytes(cls, data: bytes, endian: Endianess = "<"):
        with EndianedBytesIO(data, endian) as reader:
            return cls.read_from(reader)

    @abstractmethod
    def write_to(self, writer: EndianedWriterIOBase) -> int: ...

    def to_bytes(self, endian: Endianess = "<") -> bytes:
        with EndianedBytesIO(endian=endian) as writer:
            self.write_to(writer)
            return writer.getvalue()


class Serializer(Generic[T], metaclass=ABCMeta):
    @abstractmethod
    def read_from(self, reader: EndianedReaderIOBase) -> T: ...

    def from_bytes(self, data: bytes, endian: Endianess = "<"):
        with EndianedBytesIO(data, endian) as reader:
            return self.read_from(reader)

    @abstractmethod
    def write_to(self, value: T, writer: EndianedWriterIOBase) -> int: ...

    def to_bytes(self, value: T, endian: Endianess = "<") -> bytes:
        with EndianedBytesIO(endian=endian) as writer:
            self.write_to(value, writer)
            return writer.getvalue()
