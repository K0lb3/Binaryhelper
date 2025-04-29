from io import BufferedReader, RawIOBase

from .EndianedIOBase import EndianedIOBase, Endianess


class EndianedBufferedReader(BufferedReader, EndianedIOBase):
    def __init__(
        self, raw: RawIOBase, buffer_size: int = 8192, endian: Endianess = "<"
    ) -> None:
        BufferedReader.__init__(self, raw, buffer_size)
        self.endian = endian


__all__ = ("EndianedBufferedReader",)
