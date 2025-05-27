from setuptools import Extension, find_packages, setup
from st_zig import enforce_via_build_ext

enforce_via_build_ext()


setup(
    name="bier",
    packages=find_packages(),
    ext_modules=[
        Extension(
            "bier.EndianedBinaryIO.C.EndianedBytesIO",
            ["src/EndianedBinaryIO/EndianedBytesIO.cpp"],
            depends=["src/EndianedBinaryIO/PyConverter.hpp"],
            language="c++",
            include_dirs=["src"],
            extra_compile_args=["-std=c++23"],
        ),
        Extension(
            "bier.EndianedBinaryIO.C.EndianedStreamIO",
            ["src/EndianedBinaryIO/EndianedStreamIO.cpp"],
            depends=["src/EndianedBinaryIO/PyConverter.hpp"],
            language="c++",
            include_dirs=["src"],
            extra_compile_args=["-std=c++23"],
        ),
        Extension(
            "bier.EndianedBinaryIO.C.EndianedIOBase",
            ["src/EndianedBinaryIO/EndianedIOBase.cpp"],
            depends=["src/PyConverter.hpp"],
            language="c++",
            include_dirs=["src"],
            extra_compile_args=["-std=c++23"],
        ),
    ],
)
