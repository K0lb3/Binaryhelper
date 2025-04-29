from io import BytesIO

from typing_extensions import Buffer

from .EndianedIOBase import EndianedIOBase, Endianess


class EndianedBytesIO(BytesIO, EndianedIOBase):
    def __init__(self, initial_bytes: Buffer = b"", endian: Endianess = "<") -> None:
        BytesIO.__init__(self, initial_bytes)
        self.endian = endian


__all__ = ("EndianedBytesIO",)
