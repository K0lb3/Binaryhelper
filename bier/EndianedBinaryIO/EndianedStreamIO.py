from .EndianedIOBase import EndianedIOBase, Endianess
from io import BytesIO


class EndianedStreamIO(EndianedIOBase):
    stream: BytesIO
    endian: Endianess

    def __init__(self, stream: BytesIO, endian: Endianess) -> None:
        self.stream = stream
        self.endian = endian

        # io base methods
        self.close = self.stream.close
        self.fileno = self.stream.fileno
        self.flush = self.stream.flush
        self.isatty = self.stream.isatty
        self.readable = self.stream.readable
        self.read = self.stream.read
        self.readlines = self.stream.readlines
        self.seek = self.stream.seek
        self.seekable = self.stream.seekable
        self.tell = self.stream.tell
        self.truncate = self.stream.truncate
        self.writable = self.stream.writable
        self.write = self.stream.write
        self.writelines = self.stream.writelines
        self.readline = self.stream.readline
        self.closed = self.stream.closed  # type: ignore

        # raw io base methods
        # self.readall = self.stream.readall
        self.readinto = self.stream.readinto

        # buffered io base methods
        self.detach = self.stream.detach
        self.readinto = self.stream.readinto
        self.readinto1 = self.stream.readinto1
        self.read1 = self.stream.read1
        # bytesio methods
        self.name = self.stream.name
        self.getvalue = self.stream.getvalue
        self.getbuffer = self.stream.getbuffer

        # IO
        self.truncate = self.stream.truncate


__all__ = ["EndianedStreamIO"]
