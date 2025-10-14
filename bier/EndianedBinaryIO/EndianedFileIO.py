from io import FileIO
from os import PathLike
from sys import version_info
from typing import Callable, Optional, Union

from .EndianedIOBase import EndianedIOBase, Endianess

if version_info >= (3, 9):
    StrOrBytesPath = Union[str, bytes, PathLike[str], PathLike[bytes]]
    FileDescriptorOrPath = Union[int, StrOrBytesPath]
    _Opener = Callable[[str, int], int]


class EndianedFileIO(FileIO, EndianedIOBase):
    def __init__(
        self,
        file: "FileDescriptorOrPath",
        mode: str = "r",
        closefd: bool = True,
        opener: Optional["_Opener"] = None,
        endian: Endianess = "<",
    ) -> None:
        FileIO.__init__(self, file, mode, closefd, opener)
        self.endian = endian


__all__ = ("EndianedFileIO",)
