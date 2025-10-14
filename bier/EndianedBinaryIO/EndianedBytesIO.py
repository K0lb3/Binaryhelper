from io import BytesIO
from sys import version_info

if version_info >= (3, 12):
    from collections.abc import Buffer
elif version_info >= (3, 9):
    from typing_extensions import Buffer
# Buffer is not available in Python 3.8 and below

from .EndianedIOBase import EndianedIOBase, Endianess


class EndianedBytesIO(BytesIO, EndianedIOBase):
    def __init__(self, initial_bytes: "Buffer" = b"", endian: Endianess = "<") -> None:
        BytesIO.__init__(self, initial_bytes)
        self.endian = endian


__all__ = ("EndianedBytesIO",)
