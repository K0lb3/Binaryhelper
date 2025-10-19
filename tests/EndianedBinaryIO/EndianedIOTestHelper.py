import builtins
from random import randint, uniform
from struct import pack, unpack
from typing import Callable, List, Literal

from bier.EndianedBinaryIO import EndianedReaderIOBase, EndianedWriterIOBase, Endianess


class EndianedIOTestHelper:
    count: int
    bool: List[builtins.bool]
    u8: List[int]
    u16: List[int]
    u32: List[int]
    u64: List[int]
    i8: List[int]
    i16: List[int]
    i32: List[int]
    i64: List[int]
    f16: List[float]
    f32: List[float]
    f64: List[float]
    raw_u8_le: bytes
    raw_u16_le: bytes
    raw_u32_le: bytes
    raw_u64_le: bytes
    raw_i8_le: bytes
    raw_i16_le: bytes
    raw_i32_le: bytes
    raw_i64_le: bytes
    raw_f16_le: bytes
    raw_f32_le: bytes
    raw_f64_le: bytes
    raw_u8_be: bytes
    raw_u16_be: bytes
    raw_u32_be: bytes
    raw_u64_be: bytes
    raw_i8_be: bytes
    raw_i16_be: bytes
    raw_i32_be: bytes
    raw_i64_be: bytes
    raw_f16_be: bytes
    raw_f32_be: bytes
    raw_f64_be: bytes

    def __init__(self, count: int = 10):
        self.count = count
        self._generate_values()
        self._generate_bytes()

    def test_reader(
        self, reader_gen: Callable[[bytes, Endianess], EndianedReaderIOBase]
    ):
        self._test_reader_le(reader_gen)
        self._test_reader_be(reader_gen)

    def test_writer(self, writer_gen: Callable[[Endianess], EndianedWriterIOBase]):
        self._test_writer_le(writer_gen)
        self._test_writer_be(writer_gen)

    def _test_reader_le(
        self, reader_gen: Callable[[bytes, Endianess], EndianedReaderIOBase]
    ):
        instance = reader_gen(
            b"".join(
                [
                    self.raw_bool_le,
                    self.raw_u8_le,
                    self.raw_u16_le,
                    self.raw_u32_le,
                    self.raw_u64_le,
                    self.raw_i8_le,
                    self.raw_i16_le,
                    self.raw_i32_le,
                    self.raw_i64_le,
                    self.raw_f16_le,
                    self.raw_f32_le,
                    self.raw_f64_le,
                ]
            ),
            "<",
        )
        self._test_read(instance, "read_bool", self.bool, self.raw_bool_le)
        self._test_read(instance, "read_u8", self.u8, self.raw_u8_le)
        self._test_read(instance, "read_u16", self.u16, self.raw_u16_le)
        self._test_read(instance, "read_u32", self.u32, self.raw_u32_le)
        self._test_read(instance, "read_u64", self.u64, self.raw_u64_le)
        self._test_read(instance, "read_i8", self.i8, self.raw_i8_le)
        self._test_read(instance, "read_i16", self.i16, self.raw_i16_le)
        self._test_read(instance, "read_i32", self.i32, self.raw_i32_le)
        self._test_read(instance, "read_i64", self.i64, self.raw_i64_le)
        self._test_read(instance, "read_f16", self.f16, self.raw_f16_le)
        self._test_read(instance, "read_f32", self.f32, self.raw_f32_le)
        self._test_read(instance, "read_f64", self.f64, self.raw_f64_le)
        instance.close()

    def _test_reader_be(
        self, reader_gen: Callable[[bytes, Endianess], EndianedReaderIOBase]
    ):
        instance = reader_gen(
            b"".join(
                [
                    self.raw_bool_be,
                    self.raw_u8_be,
                    self.raw_u16_be,
                    self.raw_u32_be,
                    self.raw_u64_be,
                    self.raw_i8_be,
                    self.raw_i16_be,
                    self.raw_i32_be,
                    self.raw_i64_be,
                    self.raw_f16_be,
                    self.raw_f32_be,
                    self.raw_f64_be,
                ]
            ),
            ">",
        )
        self._test_read(instance, "read_bool", self.bool, self.raw_bool_be)
        self._test_read(instance, "read_u8", self.u8, self.raw_u8_be)
        self._test_read(instance, "read_u16", self.u16, self.raw_u16_be)
        self._test_read(instance, "read_u32", self.u32, self.raw_u32_be)
        self._test_read(instance, "read_u64", self.u64, self.raw_u64_be)
        self._test_read(instance, "read_i8", self.i8, self.raw_i8_be)
        self._test_read(instance, "read_i16", self.i16, self.raw_i16_be)
        self._test_read(instance, "read_i32", self.i32, self.raw_i32_be)
        self._test_read(instance, "read_i64", self.i64, self.raw_i64_be)
        self._test_read(instance, "read_f16", self.f16, self.raw_f16_be)
        self._test_read(instance, "read_f32", self.f32, self.raw_f32_be)
        self._test_read(instance, "read_f64", self.f64, self.raw_f64_be)
        instance.close()

    def _test_writer_le(self, writer_gen: Callable[[Endianess], EndianedWriterIOBase]):
        instance = writer_gen("<")
        self._test_write(instance, "write_bool", self.bool, self.raw_bool_le)
        self._test_write(instance, "write_u8", self.u8, self.raw_u8_le)
        self._test_write(instance, "write_u16", self.u16, self.raw_u16_le)
        self._test_write(instance, "write_u32", self.u32, self.raw_u32_le)
        self._test_write(instance, "write_u64", self.u64, self.raw_u64_le)
        self._test_write(instance, "write_i8", self.i8, self.raw_i8_le)
        self._test_write(instance, "write_i16", self.i16, self.raw_i16_le)
        self._test_write(instance, "write_i32", self.i32, self.raw_i32_le)
        self._test_write(instance, "write_i64", self.i64, self.raw_i64_le)
        self._test_write(instance, "write_f16", self.f16, self.raw_f16_le)
        self._test_write(instance, "write_f32", self.f32, self.raw_f32_le)
        self._test_write(instance, "write_f64", self.f64, self.raw_f64_le)
        instance.close()

    def _test_writer_be(self, writer_gen: Callable[[Endianess], EndianedWriterIOBase]):
        instance = writer_gen(">")
        self._test_write(instance, "write_bool", self.bool, self.raw_bool_be)
        self._test_write(instance, "write_u8", self.u8, self.raw_u8_be)
        self._test_write(instance, "write_u16", self.u16, self.raw_u16_be)
        self._test_write(instance, "write_u32", self.u32, self.raw_u32_be)
        self._test_write(instance, "write_u64", self.u64, self.raw_u64_be)
        self._test_write(instance, "write_i8", self.i8, self.raw_i8_be)
        self._test_write(instance, "write_i16", self.i16, self.raw_i16_be)
        self._test_write(instance, "write_i32", self.i32, self.raw_i32_be)
        self._test_write(instance, "write_i64", self.i64, self.raw_i64_be)
        self._test_write(instance, "write_f16", self.f16, self.raw_f16_be)
        self._test_write(instance, "write_f32", self.f32, self.raw_f32_be)
        self._test_write(instance, "write_f64", self.f64, self.raw_f64_be)
        instance.close()

    def _test_read(
        self,
        reader: EndianedReaderIOBase,
        call_name: str,
        values: list,
        values_raw: bytes,
    ):
        start_pos = reader.tell()
        call = getattr(reader, call_name)
        call_arr = getattr(reader, f"{call_name}_array")

        def check_read(values_read: list, ins: str):
            bytes_read = reader.tell() - start_pos
            assert bytes_read == len(values_raw), (
                f"Failed to read expected length ({ins}): expected {len(values_raw)}, got {bytes_read}"
            )
            for i, (expected_value, read_value) in enumerate(zip(values, values_read)):
                assert read_value == expected_value, (
                    f"Failed at index {i} in read ({ins}): expected {expected_value}, got {read_value}"
                )

        reader.seek(start_pos)
        values_read = [call() for _ in range(self.count)]
        check_read(values_read, "single")

        reader.seek(start_pos)
        values_read_arr = call_arr(self.count)
        check_read(values_read_arr, "array")

    def _test_write(
        self,
        writer: EndianedWriterIOBase,
        call_name: str,
        values: list,
        values_raw: bytes,
    ):
        writer.flush()
        start_pos = writer.tell()
        call = getattr(writer, call_name)
        call_arr = getattr(writer, f"{call_name}_array")

        def check_write(ins: str):
            writer.flush()
            bytes_written = writer.tell() - start_pos
            assert bytes_written == len(values_raw), (
                f"Failed to write expected length ({ins}): expected {len(values_raw)}, got {bytes_written}"
            )
            writer.seek(start_pos)
            read = writer.read(len(values_raw))
            assert read == values_raw, (
                f"Failed to read expected raw bytes ({ins}): expected {values_raw}, got {read}"
            )

        for i in range(self.count):
            call(values[i])
        check_write("single")

        writer.seek(start_pos)
        call_arr(values, False)
        check_write("array")

    def _generate_values(self):
        self.bool = [bool(v) for v in self._generate_random_ints(0, 1, self.count)]
        self.u8 = self._generate_random_ints(0, 0xFF, self.count)
        self.u16 = self._generate_random_ints(0, 0xFF, self.count)
        self.u32 = self._generate_random_ints(0, 0xFFFFFFFF, self.count)
        self.u64 = self._generate_random_ints(0, 0xFFFFFFFFFFFFFFFF, self.count)
        self.i8 = self._generate_random_ints(-0x80, 0x7F, self.count)
        self.i16 = self._generate_random_ints(-0x8000, 0x7FFF, self.count)
        self.i32 = self._generate_random_ints(-0x80000000, 0x7FFFFFFF, self.count)
        self.i64 = self._generate_random_ints(
            -0x8000000000000000, 0x7FFFFFFFFFFFFFFF, self.count
        )
        self.f16 = self._generate_random_floats(-32768.0, 32767.0, self.count, "e")
        self.f32 = self._generate_random_floats(
            -3.4028235e38, 3.4028235e38, self.count, "f"
        )
        self.f64 = self._generate_random_floats(
            -1.7976931348623157e308, 1.7976931348623157e308, self.count, "d"
        )

    def _generate_bytes(self):
        self.raw_bool_le = pack(f"<{self.count}?", *self.bool)
        self.raw_u8_le = pack(f"<{self.count}B", *self.u8)
        self.raw_u16_le = pack(f"<{self.count}H", *self.u16)
        self.raw_u32_le = pack(f"<{self.count}I", *self.u32)
        self.raw_u64_le = pack(f"<{self.count}Q", *self.u64)
        self.raw_i8_le = pack(f"<{self.count}b", *self.i8)
        self.raw_i16_le = pack(f"<{self.count}h", *self.i16)
        self.raw_i32_le = pack(f"<{self.count}i", *self.i32)
        self.raw_i64_le = pack(f"<{self.count}q", *self.i64)
        self.raw_f16_le = pack(f"<{self.count}e", *self.f16)
        self.raw_f32_le = pack(f"<{self.count}f", *self.f32)
        self.raw_f64_le = pack(f"<{self.count}d", *self.f64)
        self.raw_bool_be = pack(f">{self.count}?", *self.bool)
        self.raw_u8_be = pack(f">{self.count}B", *self.u8)
        self.raw_u16_be = pack(f">{self.count}H", *self.u16)
        self.raw_u32_be = pack(f">{self.count}I", *self.u32)
        self.raw_u64_be = pack(f">{self.count}Q", *self.u64)
        self.raw_i8_be = pack(f">{self.count}b", *self.i8)
        self.raw_i16_be = pack(f">{self.count}h", *self.i16)
        self.raw_i32_be = pack(f">{self.count}i", *self.i32)
        self.raw_i64_be = pack(f">{self.count}q", *self.i64)
        self.raw_f16_be = pack(f">{self.count}e", *self.f16)
        self.raw_f32_be = pack(f">{self.count}f", *self.f32)
        self.raw_f64_be = pack(f">{self.count}d", *self.f64)

    def _generate_random_ints(
        self, min_value: int, max_value: int, count: int
    ) -> List[int]:
        assert count >= 2, "Count must be at least 2 to include min and max values."
        return [
            min_value,
            *[randint(min_value, max_value) for _ in range(count - 2)],
            max_value,
        ]

    def _generate_random_floats(
        self,
        min_value: float,
        max_value: float,
        count: int,
        struct_format: Literal["e", "f", "d"],
    ) -> List[float]:
        assert count >= 2, "Count must be at least 2 to include min and max values."
        values = [
            min_value,
            *[uniform(min_value, max_value) for _ in range(count - 2)],
            max_value,
        ]
        # Pack and unpack to ensure correct byte representation
        value_raw = pack(f"<{count}{struct_format}", *values)
        return list(unpack(f"<{count}{struct_format}", value_raw))


__all__ = [
    "EndianedIOTestHelper",
]
