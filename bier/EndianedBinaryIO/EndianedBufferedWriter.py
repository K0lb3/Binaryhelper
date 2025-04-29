from io import BufferedWriter, RawIOBase

from .EndianedIOBase import EndianedIOBase, Endianess


class EndianedBufferedWriter(BufferedWriter, EndianedIOBase):
    def __init__(
        self, raw: RawIOBase, buffer_size: int = 8192, endian: Endianess = "<"
    ) -> None:
        BufferedWriter.__init__(self, raw, buffer_size)
        self.endian = endian


__all__ = ("EndianedBufferedWriter",)
