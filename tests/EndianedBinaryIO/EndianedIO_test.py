import os
import tempfile
from io import BytesIO

import pytest

from bier.EndianedBinaryIO import EndianedBytesIO, EndianedFileIO, EndianedStreamIO
from bier.EndianedBinaryIO.C import (
    EndianedBytesIO as EndianedBytesIOC,
)
from bier.EndianedBinaryIO.C import (
    EndianedStreamIO as EndianedStreamIOC,
)
from tests.EndianedBinaryIO.EndianedIOTestHelper import EndianedIOTestHelper


class EndianedFileIOTemp(EndianedFileIO):
    @classmethod
    def gen_reader(cls, data, endian):
        temp_file = tempfile.NamedTemporaryFile(delete=False, mode="wb")
        temp_file.write(data)
        temp_file.flush()
        temp_file.close()
        return cls(temp_file.name, endian=endian)

    @classmethod
    def gen_writer(cls, endian):
        temp_file = tempfile.NamedTemporaryFile(delete=False)
        temp_file.close()
        return cls(temp_file.name, "w+b", endian=endian)

    def close(self):
        super().close()
        if os.path.exists(self.name):
            os.unlink(self.name)

    def __del__(self):
        if not self.closed:
            self.close()
        super().__del__()


@pytest.mark.parametrize(
    "io_class, stream_factory",
    [
        (
            EndianedStreamIO,
            lambda data, endian: EndianedStreamIO(BytesIO(data), endian),
        ),
        (EndianedBytesIO, lambda data, endian: EndianedBytesIO(data, endian)),
        (
            EndianedFileIO,
            lambda data, endian: EndianedFileIOTemp.gen_reader(data, endian),
        ),
        (
            EndianedStreamIOC,
            lambda data, endian: EndianedStreamIOC(BytesIO(data), endian),
        ),
        (
            EndianedBytesIOC,
            lambda data, endian: EndianedBytesIOC(data, endian),
        ),
    ],
)
def test_reader(io_class, stream_factory):
    helper = EndianedIOTestHelper(count=10)
    helper.test_reader(stream_factory)


@pytest.mark.parametrize(
    "io_class, stream_factory",
    [
        (EndianedStreamIO, lambda endian: EndianedStreamIO(BytesIO(), endian)),
        (EndianedBytesIO, lambda endian: EndianedBytesIO(bytearray(), endian)),
        (
            EndianedFileIO,
            lambda endian: EndianedFileIOTemp.gen_writer(endian),
        ),
        (
            EndianedStreamIOC,
            lambda endian: EndianedStreamIOC(BytesIO(), endian),
        ),
        (
            EndianedBytesIOC,
            lambda endian: EndianedBytesIOC(bytearray(1024), endian),
        ),
    ],
)
def test_writer(io_class, stream_factory):
    helper = EndianedIOTestHelper(count=10)
    helper.test_writer(stream_factory)
