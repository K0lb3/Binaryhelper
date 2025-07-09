import sys
import platform
from setuptools import Extension, find_packages, setup

if platform.system() == "Windows":
    # msvc compiler does not support C++23 yet, so have to use zig
    # currently zig runs into some issues on setuptools on the gh workers
    from st_zig import enforce_via_build_ext

    enforce_via_build_ext()

# only use the limited API if Python 3.11 or newer is used
# 3.11 added PyBuffer support to the limited API,
py_limited_api = sys.version_info >= (3, 11)

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
            py_limited_api=py_limited_api,
        ),
        Extension(
            "bier.EndianedBinaryIO.C.EndianedStreamIO",
            ["src/EndianedBinaryIO/EndianedStreamIO.cpp"],
            depends=["src/EndianedBinaryIO/PyConverter.hpp"],
            language="c++",
            include_dirs=["src"],
            extra_compile_args=["-std=c++23"],
            py_limited_api=py_limited_api,
        ),
        # somehow slower than the pure python version
        # Extension(
        #     "bier.EndianedBinaryIO.C.EndianedIOBase",
        #     ["src/EndianedBinaryIO/EndianedIOBase.cpp"],
        #     depends=["src/PyConverter.hpp"],
        #     language="c++",
        #     include_dirs=["src"],
        #     extra_compile_args=["-std=c++23"],
        # ),
    ],
    install_requires=['st_zig; platform_system=="Windows"'],
)
