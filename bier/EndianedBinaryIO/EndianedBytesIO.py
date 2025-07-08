from io import BytesIO

from collections.abc import Buffer

from .EndianedIOBase import EndianedIOBase, Endianess


class EndianedBytesIO(BytesIO, EndianedIOBase):
    def __init__(self, initial_bytes: Buffer = b"", endian: Endianess = "<") -> None:
        BytesIO.__init__(self, initial_bytes)
        self.endian = endian


__all__ = ("EndianedBytesIO",)
