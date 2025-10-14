import abc
from io import IOBase
from struct import Struct
from struct import pack as struct_pack
from typing import Literal, Optional, Sequence, Tuple

from ._structs import (
    F16_BE,
    F16_LE,
    F32_BE,
    F32_LE,
    F64_BE,
    F64_LE,
    I8,
    I16_BE,
    I16_LE,
    I32_BE,
    I32_LE,
    I64_BE,
    I64_LE,
    U8,
    U16_BE,
    U16_LE,
    U32_BE,
    U32_LE,
    U64_BE,
    U64_LE,
)

Endianess = Literal["<", ">"]


class EndianedReaderIOBase(IOBase, metaclass=abc.ABCMeta):
    endian: Endianess

    def read_count(self) -> int:
        return self.read_i32()

    # helper functions
    def align(self, size: int) -> int:
        padding = size - (self.tell() % size)
        if padding != size:
            if self.seekable():
                self.seek(padding, 1)
            else:
                self.read(padding)
        return self.tell()

    # unsigned integer
    def read_u8(self) -> int:
        return U8.unpack(self.read(1))[0]

    def read_u8_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{count}B")
        return struct.unpack(self.read(struct.size))

    def read_u16(self) -> int:
        if self.endian == "<":
            return U16_LE.unpack(self.read(2))[0]
        elif self.endian == ">":
            return U16_BE.unpack(self.read(2))[0]
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def read_u16_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{self.endian}{count}H")
        return struct.unpack(self.read(struct.size))

    def read_u16_le(self) -> int:
        return U16_LE.unpack(self.read(2))[0]

    def read_u16_le_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"<{count}H")
        return struct.unpack(self.read(struct.size))

    def read_u16_be(self) -> int:
        return U16_BE.unpack(self.read(2))[0]

    def read_u16_be_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f">{count}H")
        return struct.unpack(self.read(struct.size))

    def read_u32(self) -> int:
        if self.endian == "<":
            return U32_LE.unpack(self.read(4))[0]
        elif self.endian == ">":
            return U32_BE.unpack(self.read(4))[0]
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def read_u32_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{self.endian}{count}I")
        return struct.unpack(self.read(struct.size))

    def read_u32_le(self) -> int:
        return U32_LE.unpack(self.read(4))[0]

    def read_u32_le_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"<{count}I")
        return struct.unpack(self.read(struct.size))

    def read_u32_be(self) -> int:
        return U32_BE.unpack(self.read(4))[0]

    def read_u32_be_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f">{count}I")
        return struct.unpack(self.read(struct.size))

    def read_u64(self) -> int:
        if self.endian == "<":
            return U64_LE.unpack(self.read(8))[0]
        elif self.endian == ">":
            return U64_BE.unpack(self.read(8))[0]
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def read_u64_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{self.endian}{count}Q")
        return struct.unpack(self.read(struct.size))

    def read_u64_le(self) -> int:
        return U64_LE.unpack(self.read(8))[0]

    def read_u64_le_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"<{count}Q")
        return struct.unpack(self.read(struct.size))

    def read_u64_be(self) -> int:
        return U64_BE.unpack(self.read(8))[0]

    def read_u64_be_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f">{count}Q")
        return struct.unpack(self.read(struct.size))

    # signed integer
    def read_i8(self) -> int:
        return I8.unpack(self.read(1))[0]

    def read_i8_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{count}b")
        return struct.unpack(self.read(struct.size))

    def read_i16(self) -> int:
        if self.endian == "<":
            return I16_LE.unpack(self.read(2))[0]
        elif self.endian == ">":
            return I16_BE.unpack(self.read(2))[0]
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def read_i16_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{self.endian}{count}h")
        return struct.unpack(self.read(struct.size))

    def read_i16_le(self) -> int:
        return I16_LE.unpack(self.read(2))[0]

    def read_i16_le_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"<{count}h")
        return struct.unpack(self.read(struct.size))

    def read_i16_be(self) -> int:
        return I16_BE.unpack(self.read(2))[0]

    def read_i16_be_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f">{count}h")
        return struct.unpack(self.read(struct.size))

    def read_i32(self) -> int:
        if self.endian == "<":
            return I32_LE.unpack(self.read(4))[0]
        elif self.endian == ">":
            return I32_BE.unpack(self.read(4))[0]
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def read_i32_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{self.endian}{count}i")
        return struct.unpack(self.read(struct.size))

    def read_i32_le(self) -> int:
        return I32_LE.unpack(self.read(4))[0]

    def read_i32_le_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"<{count}i")
        return struct.unpack(self.read(struct.size))

    def read_i32_be(self) -> int:
        return I32_BE.unpack(self.read(4))[0]

    def read_i32_be_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f">{count}i")
        return struct.unpack(self.read(struct.size))

    def read_i64(self) -> int:
        if self.endian == "<":
            return I64_LE.unpack(self.read(8))[0]
        elif self.endian == ">":
            return I64_BE.unpack(self.read(8))[0]
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def read_i64_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{self.endian}{count}q")
        return struct.unpack(self.read(struct.size))

    def read_i64_le(self) -> int:
        return I64_LE.unpack(self.read(8))[0]

    def read_i64_le_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"<{count}q")
        return struct.unpack(self.read(struct.size))

    def read_i64_be(self) -> int:
        return I64_BE.unpack(self.read(8))[0]

    def read_i64_be_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f">{count}q")
        return struct.unpack(self.read(struct.size))

    # floats
    def read_f16(self) -> float:
        if self.endian == "<":
            return F16_LE.unpack(self.read(2))[0]
        elif self.endian == ">":
            return F16_BE.unpack(self.read(2))[0]
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def read_f16_array(self, count: Optional[int] = None) -> Tuple[float, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{self.endian}{count}e")
        return struct.unpack(self.read(struct.size))

    def read_f16_le(self) -> float:
        return F16_LE.unpack(self.read(2))[0]

    def read_f16_le_array(self, count: Optional[int] = None) -> Tuple[float, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"<{count}e")
        return struct.unpack(self.read(struct.size))

    def read_f16_be(self) -> float:
        return F16_BE.unpack(self.read(2))[0]

    def read_f16_be_array(self, count: Optional[int] = None) -> Tuple[float, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f">{count}e")
        return struct.unpack(self.read(struct.size))

    def read_f32(self) -> float:
        if self.endian == "<":
            return F32_LE.unpack(self.read(4))[0]
        elif self.endian == ">":
            return F32_BE.unpack(self.read(4))[0]
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def read_f32_array(self, count: Optional[int] = None) -> Tuple[float, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{self.endian}{count}f")
        return struct.unpack(self.read(struct.size))

    def read_f32_le(self) -> float:
        return F32_LE.unpack(self.read(4))[0]

    def read_f32_le_array(self, count: Optional[int] = None) -> Tuple[float, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"<{count}f")
        return struct.unpack(self.read(struct.size))

    def read_f32_be(self) -> float:
        return F32_BE.unpack(self.read(4))[0]

    def read_f32_be_array(self, count: Optional[int] = None) -> Tuple[float, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f">{count}f")
        return struct.unpack(self.read(struct.size))

    def read_f64(self) -> float:
        if self.endian == "<":
            return F64_LE.unpack(self.read(8))[0]
        elif self.endian == ">":
            return F64_BE.unpack(self.read(8))[0]
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def read_f64_array(self, count: Optional[int] = None) -> Tuple[float, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"{self.endian}{count}d")
        return struct.unpack(self.read(struct.size))

    def read_f64_le(self) -> float:
        return F64_LE.unpack(self.read(8))[0]

    def read_f64_le_array(self, count: Optional[int] = None) -> Tuple[float, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f"<{count}d")
        return struct.unpack(self.read(struct.size))

    def read_f64_be(self) -> float:
        return F64_BE.unpack(self.read(8))[0]

    def read_f64_be_array(self, count: Optional[int] = None) -> Tuple[float, ...]:
        if count is None:
            count = self.read_count()
        struct = Struct(f">{count}d")
        return struct.unpack(self.read(struct.size))

    # strings
    def read_cstring(self, encoding: str = "utf-8", errors="surrogateescape") -> str:
        """ ""Read a null-terminated string from the stream.

        Args:
            encoding (str, optional): The encoding to use. Defaults to "utf-8".
            errors (str, optional): The error handling scheme to use. Defaults to "surrogateescape".
        """
        string = b""
        while True:
            char = self.read(1)
            if char == b"\x00":
                break
            string += char
        return string.decode("utf-8", errors=errors)

    def read_string(
        self,
        length: Optional[int] = None,
        encoding: str = "utf-8",
        errors="surrogateescape",
    ) -> str:
        """Read a string of a given length from the stream.

        Args:
            length (int, optional): The length of the string to read. If None, use read_count to determine the length.
            encoding (str, optional): The encoding to use. Defaults to "utf-8".
            errors (str, optional): The error handling scheme to use. Defaults to "surrogateescape".
        """
        if length is None:
            length = self.read_count()
        return self.read(length).decode(encoding, errors=errors)

    def read_bytes(
        self,
        length: Optional[int] = None,
    ) -> bytes:
        """Read a bytes object of a given length from the stream.

        Args:
            length (int, optional): The length of the bytes to read. If None, use read_count to determine the length.
        """
        if length is None:
            length = self.read_count()
        return self.read(length)

    # custom stuff
    def read_varint(self) -> int:
        """Read a variable-length integer from the stream.

        Returns:
            int: The variable-length integer.
        """
        result = 0
        shift = 0
        while True:
            byte = self.read_u8()
            result |= (byte & 0x7F) << shift
            if not (byte & 0x80):
                break
            shift += 7
        return result

    def read_varint_array(self, count: Optional[int] = None) -> Tuple[int, ...]:
        """Read a variable-length integer array from the stream.

        Args:
            count (int, optional): The number of variable-length integers to read. If None, use read_count to determine the length.

        Returns:
            Tuple[int, ...]: The variable-length integer array.
        """
        if count is None:
            count = self.read_count()
        return tuple(self.read_varint() for _ in range(count))


class EndianedWriterIOBase(IOBase, metaclass=abc.ABCMeta):
    endian: Endianess

    def align(self, size: int) -> int:
        padding = size - (self.tell() % size)
        if padding != size:
            if not self.readable():
                self.write(b"\x00" * padding)
            else:
                read = self.read(padding)
                self.write(b"\x00" * (padding - len(read)))
        return self.tell()

    def write_count(self, count: int) -> int:
        return self.write_i32(count)

    # unsigned integer
    def write_u8(self, v: int) -> int:
        return self.write(U8.pack(v))

    def write_u8_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{len(v)}B", *v))

    def write_u16(self, v: int) -> int:
        if self.endian == "<":
            return self.write(U16_LE.pack(v))
        elif self.endian == ">":
            return self.write(U16_BE.pack(v))
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def write_u16_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{self.endian}{len(v)}H", *v))

    def write_u16_le(self, v: int) -> int:
        return self.write(U16_LE.pack(v))

    def write_u16_le_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"<{len(v)}H", *v))

    def write_u16_be(self, v: int) -> int:
        return self.write(U16_BE.pack(v))

    def write_u16_be_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f">{len(v)}H", *v))

    def write_u32(self, v: int) -> int:
        if self.endian == "<":
            return self.write(U32_LE.pack(v))
        elif self.endian == ">":
            return self.write(U32_BE.pack(v))
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def write_u32_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{self.endian}{len(v)}I", *v))

    def write_u32_le(self, v: int) -> int:
        return self.write(U32_LE.pack(v))

    def write_u32_le_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"<{len(v)}I", *v))

    def write_u32_be(self, v: int) -> int:
        return self.write(U32_BE.pack(v))

    def write_u32_be_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f">{len(v)}I", *v))

    def write_u64(self, v: int) -> int:
        if self.endian == "<":
            return self.write(U64_LE.pack(v))
        elif self.endian == ">":
            return self.write(U64_BE.pack(v))
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def write_u64_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{self.endian}{len(v)}Q", *v))

    def write_u64_le(self, v: int) -> int:
        return self.write(U64_LE.pack(v))

    def write_u64_le_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"<{len(v)}Q", *v))

    def write_u64_be(self, v: int) -> int:
        return self.write(U64_BE.pack(v))

    def write_u64_be_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f">{len(v)}Q", *v))

    # signed integer
    def write_i8(self, v: int) -> int:
        return self.write(I8.pack(v))

    def write_i8_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{len(v)}b", *v))

    def write_i16(self, v: int) -> int:
        if self.endian == "<":
            return self.write(I16_LE.pack(v))
        elif self.endian == ">":
            return self.write(I16_BE.pack(v))
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def write_i16_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{self.endian}{len(v)}h", *v))

    def write_i16_le(self, v: int) -> int:
        return self.write(I16_LE.pack(v))

    def write_i16_le_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"<{len(v)}h", *v))

    def write_i16_be(self, v: int) -> int:
        return self.write(I16_BE.pack(v))

    def write_i16_be_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f">{len(v)}h", *v))

    def write_i32(self, v: int) -> int:
        if self.endian == "<":
            return self.write(I32_LE.pack(v))
        elif self.endian == ">":
            return self.write(I32_BE.pack(v))
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def write_i32_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{self.endian}{len(v)}i", *v))

    def write_i32_le(self, v: int) -> int:
        return self.write(I32_LE.pack(v))

    def write_i32_le_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"<{len(v)}i", *v))

    def write_i32_be(self, v: int) -> int:
        return self.write(I32_BE.pack(v))

    def write_i32_be_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f">{len(v)}i", *v))

    def write_i64(self, v: int) -> int:
        if self.endian == "<":
            return self.write(I64_LE.pack(v))
        elif self.endian == ">":
            return self.write(I64_BE.pack(v))
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def write_i64_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{self.endian}{len(v)}q", *v))

    def write_i64_le(self, v: int) -> int:
        return self.write(I64_LE.pack(v))

    def write_i64_le_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"<{len(v)}q", *v))

    def write_i64_be(self, v: int) -> int:
        return self.write(I64_BE.pack(v))

    def write_i64_be_array(self, v: Sequence[int], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f">{len(v)}q", *v))

    # floats
    def write_f16(self, v: float) -> int:
        if self.endian == "<":
            return self.write(F16_LE.pack(v))
        elif self.endian == ">":
            return self.write(F16_BE.pack(v))
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def write_f16_array(self, v: Sequence[float], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{self.endian}{len(v)}e", *v))

    def write_f16_le(self, v: float) -> int:
        return self.write(F16_LE.pack(v))

    def write_f16_le_array(self, v: Sequence[float], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"<{len(v)}e", *v))

    def write_f16_be(self, v: float) -> int:
        return self.write(F16_BE.pack(v))

    def write_f16_be_array(self, v: Sequence[float], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f">{len(v)}e", *v))

    def write_f32(self, v: float) -> int:
        if self.endian == "<":
            return self.write(F32_LE.pack(v))
        elif self.endian == ">":
            return self.write(F32_BE.pack(v))
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def write_f32_array(self, v: Sequence[float], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{self.endian}{len(v)}f", *v))

    def write_f32_le(self, v: float) -> int:
        return self.write(F32_LE.pack(v))

    def write_f32_le_array(self, v: Sequence[float], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"<{len(v)}f", *v))

    def write_f32_be(self, v: float) -> int:
        return self.write(F32_BE.pack(v))

    def write_f32_be_array(self, v: Sequence[float], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f">{len(v)}f", *v))

    def write_f64(self, v: float) -> int:
        if self.endian == "<":
            return self.write(F64_LE.pack(v))
        elif self.endian == ">":
            return self.write(F64_BE.pack(v))
        else:
            raise ValueError(f"Invalid endian: {self.endian}")

    def write_f64_array(self, v: Sequence[float], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"{self.endian}{len(v)}d", *v))

    def write_f64_le(self, v: float) -> int:
        return self.write(F64_LE.pack(v))

    def write_f64_le_array(self, v: Sequence[float], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f"<{len(v)}d", *v))

    def write_f64_be(self, v: float) -> int:
        return self.write(F64_BE.pack(v))

    def write_f64_be_array(self, v: Sequence[float], write_count: bool = True) -> int:
        if write_count:
            self.write_count(len(v))
        return self.write(struct_pack(f">{len(v)}d", *v))

    # strings
    def write_string_c(
        self, string: str, encoding: str = "utf-8", errors="surrogateescape"
    ) -> int:
        """Write a null-terminated string to the stream.

        Args:
            string (str): The string to write.
            encoding (str, optional): The encoding to use. Defaults to "utf-8".
            errors (str, optional): The error handling scheme to use. Defaults to "surrogateescape".
        """
        return self.write((string).encode(encoding, errors=errors) + b"\x00")

    def write_string(
        self,
        string: str,
        write_count: bool = True,
        encoding: str = "utf-8",
        errors="surrogateescape",
    ) -> int:
        """Write a string to the stream.

        Args:
            string (str): The string to write.
            write_count (bool, optional): Whether to write the length of the string first. Defaults to True.
            encoding (str, optional): The encoding to use. Defaults to "utf-8".
            errors (str, optional): The error handling scheme to use. Defaults to "surrogateescape".
        """
        raw_string = string.encode(encoding, errors=errors)
        if write_count:
            self.write_count(len(raw_string))
        return self.write(raw_string)

    def write_bytes(self, data: bytes, write_count: bool = True) -> int:
        """Write a bytes object to the stream.

        Args:
            data (bytes): The bytes object to write.
            write_count (bool, optional): Whether to write the length of the bytes first. Defaults to True.
        """
        if write_count:
            self.write_count(len(data))
        return self.write(data)

    # custom stuff
    def write_varint(self, v: int) -> int:
        """Write a variable-length integer to the stream.

        Args:
            v (int): The variable-length integer to write.
        """
        result = 0
        while v > 0x7F:
            self.write_u8((v & 0x7F) | 0x80)
            v >>= 7
            result += 1
        return self.write_u8(v)

    def write_varint_array(self, v: Sequence[int], write_count: bool = True) -> int:
        """Write a variable-length integer array to the stream.

        Args:
            v (Sequence[int]): The variable-length integer array to write.
            write_count (bool, optional): Whether to write the length of the array first. Defaults to True.
        """
        if write_count:
            self.write_count(len(v))
        return sum(self.write_varint(i) for i in v)


class EndianedIOBase(EndianedReaderIOBase, EndianedWriterIOBase):
    endian: Endianess

    def align(self, size: int) -> int:
        padding = size - (self.tell() % size)
        if padding != size:
            if not self.writable():
                if self.seekable():
                    self.seek(padding, 1)
                else:
                    self.read(padding)
            elif not self.readable():
                self.write(b"\x00" * padding)
            else:
                read = self.read(padding)
                self.write(b"\x00" * (padding - len(read)))
        return self.tell()


__all__ = (
    "EndianedIOBase",
    "EndianedReaderIOBase",
    "EndianedWriterIOBase",
    "Endianess",
)
