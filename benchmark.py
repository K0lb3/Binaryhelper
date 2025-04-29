from io import BytesIO
from time import time_ns

from bier.EndianedBinaryIO._EndianedBytesIO import EndianedBytesIO as EndianBytesIOC
from bier.EndianedBinaryIO._EndianedIOBase import EndianedIOBase as EndianedIOBaseC
from bier.EndianedBinaryIO._EndianedStreamIO import EndianedStreamIO
from bier.EndianedBinaryIO.EndianedBytesIO import EndianedBytesIO as EndianBytesIOPy

TEST_DATA = bytes(range(256))


class EndianedBytesIOPyC(BytesIO, EndianedIOBaseC):
    def __init__(self, initial_bytes, endian="<", *args, **kwargs):
        BytesIO.__init__(self, initial_bytes, *args, **kwargs)
        self.endian = endian


def benchmark(get_instance):
    instance = get_instance(TEST_DATA, "<")
    times = []
    for i in range(len(TEST_DATA)):
        start = time_ns()
        c = instance.read_u8()
        end = time_ns()
        times.append(end - start)
        # assert c == i, f"Expected {i}, got {c}"
    return sum(times) / len(times)


print("Benchmarking EndianedBytesIO in Python")
print("EndianedBytesIOPy: ", benchmark(lambda d, e: EndianBytesIOPy(d, e)))
print("EndianedBytesIOPyC: ", benchmark(lambda d, e: EndianedBytesIOPyC(d, e)))
print(
    "EndianedBytesIOC: ",
    benchmark(lambda d, e: EndianBytesIOC(d, b">" if e == ">" else b"<")),
)
print(
    "EndianedIoStream: ",
    benchmark(lambda d, e: EndianedStreamIO(BytesIO(d), b">" if e == ">" else b"<")),
)
