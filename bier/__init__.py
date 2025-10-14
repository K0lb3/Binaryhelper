from sys import version_info

from . import EndianedBinaryIO as EndianedBinaryIO

if version_info >= (3, 12):
    from . import serialization as serialization

__version__ = "0.0.1"

__all__ = ["EndianedBinaryIO", "serialization", "__version__"]
