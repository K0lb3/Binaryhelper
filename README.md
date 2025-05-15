# BInaryhelpER

A small python package to make binary serialization faster and easier.

modules:

- EndianedBinaryIO - for parsing and writing streamed binary data.
- ComplexStreams (TODO)
  - BlockStream - for more performant handling of e.g. streams consisting of compressed and/or encrypted blocks
  - MultiStream - for handling concatent streams as single streams
- struct (TODO) - extrend struct module with additional features
  - c - null terminated string
  - v - varint
  - () - tuples (groups)
  - x\*y - read an x, then read x times y (e.g. i*s for strings with their lengths described with a prior int)
- serialization - annotation based class serialization

## Serialization

### BinarySerializable

__Annotation Types__
- ints:
  - u8
  - u16
  - u32
  - u64
  - i8
  - i16
  - i32
  - i64
- floats:
  - f16
  - f32
  - f64
- strings:
  - cstr (null terminated string)
  - str (default delimited length)
  - str_d[T] (delimited length of type T)
- lists:
  - list[S] (default delimited length)
  - list_d[S, T] (delimited length of type T)
- tuples:
  - tuple[T1, T2, ...]
- objects
  - class (has to inherit from BinarySerializible as well)
- bytes:
  - bytes (default delimited length)
  - bytes_d[T] (delimited length of type T)

__Example__
```py
class MyStruct(BinarySerializable):
    field1: u8
    field2: cstr
    field3: str_d[u16]
    field4: list_d[f32, u8]
    field5: list_d[list_d[u16, u8], u8]
    field6: tuple[u8, u16]

class MyStruct2(BinarySerializable):
    field1: MyStruct
    field2: list_d["MyStruct2", u8]


class MyStruct3(BinarySerializable[u32]):
    field: str
    field2: list[tuple[f16, f16, f16]]
```
