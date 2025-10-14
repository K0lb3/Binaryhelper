# **Bi**naryhelp**er**

[![PyPI supported Python versions](https://img.shields.io/pypi/pyversions/bier.svg)](https://pypi.python.org/pypi/bier)
[![Win/Mac/Linux](https://img.shields.io/badge/platform-windows%20%7C%20macos%20%7C%20linux-informational)]()
[![MIT](https://img.shields.io/github/license/K0lb3/Binaryhelper)](https://github.com/K0lb3/Binaryhelper/blob/master/LICENSE)
[![Test](https://github.com/K0lb3/Binaryhelper/actions/workflows/test.yml/badge.svg)](https://github.com/K0lb3/Binaryhelper/actions/workflows/test.yml)
[![Build & Publish wheels](https://github.com/K0lb3/Binaryhelper/actions/workflows/release.yml/badge.svg)](https://pypi.org/project/bier/)
[![bier docs](https://github.com/K0lb3/Binaryhelper/actions/workflows/docs.yml/badge.svg)](https://k0lb3.github.io/Binaryhelper)

Binary IO and annotation driven serialization utilities for Python.

## Highlights

- Stream-friendly readers and writers that respect explicit endianness.
- C++ backed implementations for `EndianedStreamIO` and `EndianedBytesIO` to keep hot loops fast.
- Python 3.12+ serialization helpers that turn type hints into binary schemas.
- Works with in-memory buffers, files, and arbitrary Python streams.

## Installation

```powershell
pip install bier
```

## EndianedBinaryIO (Python 3.8+)

`bier.EndianedBinaryIO` wraps common binary IO patterns so you can describe data once and reuse it across different stream types.

### Core classes

- `EndianedBytesIO`: in-memory buffer with read and write helpers for integers, floats, strings, and byte arrays.
- `EndianedStreamIO`: fast wrapper around any file-like object that exposes endian aware helpers by delegating to an underlying stream.
- `EndianedBufferedReader` and `EndianedBufferedWriter`: buffered adaptors for existing binary readers and writers.
- `EndianedFileIO`: convenience subclass that opens files and exposes the same API as in-memory streams.

### Quick start

```python
from bier.EndianedBinaryIO import EndianedBytesIO

stream = EndianedBytesIO(endian=">")
stream.write_u16(0x1234)
stream.write_cstring("bier")
stream.seek(0)

value = stream.read_u16()
text = stream.read_cstring()

assert value == 0x1234
assert text == "bier"
```

Stick with a single API whether you are reading from a `BytesIO`, a file handle, or a socket-like object. The helpers handle array packing, alignment, and endian swapping for you.

## Serialization (Python 3.12+)

`bier.serialization` turns annotated data classes into binary serializers.

- Subclass `BinarySerializable` and describe fields with provided marker types like `u32`, `f32`, `cstr`, `list_d`, and `tuple`.
- Use `convert`, `custom`, `member`, and `length_type` options to tweak encoding without hand writing parsing logic.
- Serialization uses the same readers and writers as `EndianedBinaryIO`, so the two modules fit together.

```python
from bier.EndianedBinaryIO import EndianedBytesIO
from bier.serialization import BinarySerializable, cstr, list_d, u16, u32

class Player(BinarySerializable[u16]):
    id: u32
    name: cstr
    scores: list_d[u16, u16]

class Team(BinarySerializable[u16]):
    head: "Player"
    members: list[Player]

buffer = EndianedBytesIO(endian="<")
player = Player(id=7, name="Ada", scores=[10, 20, 30])
player.write_to(buffer)

buffer.seek(0)
roundtrip = Player.read_from(buffer)
assert roundtrip.id == 7
```

### Annotation Types

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


## Roadmap

- Block oriented `ComplexStreams` for compressed or encrypted segments.
- Multi-stream wrappers for treating concatenated streams as one logical source.
- Extended struct-like helpers for varints, grouped tuples, and length prefixed fields.
