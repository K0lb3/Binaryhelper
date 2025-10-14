import platform
import sys

from setuptools import Extension, find_packages, setup
from setuptools.command.bdist_wheel import bdist_wheel

if platform.system() == "Windows":
    extra_compile_args = ["/std:c++latest"]
else:
    extra_compile_args = ["-std=c++23"]

default_sources = [
    "src/EndianedBinaryIO/PyFloat_Half.cpp",
]
default_depends = [
    "src/EndianedBinaryIO/EndianedIOBase.hpp",
    "src/EndianedBinaryIO/PyConverter.hpp",
    "src/EndianedBinaryIO/PyFloat_Half.hpp",
]


class bdist_wheel_abi3(bdist_wheel):
    def get_tag(self):
        python, abi, plat = super().get_tag()

        if python.startswith("cp"):
            # on CPython, our wheels are abi3 and compatible back to 3.6
            return python, "abi3", plat

        return python, abi, plat


# only use the limited API if Python 3.11 or newer is used
# 3.11 added PyBuffer support to the limited API,
py_limited_api = sys.version_info >= (3, 12)
cmdclass = {"bdist_wheel": bdist_wheel_abi3} if py_limited_api else {}

setup(
    name="bier",
    packages=find_packages(),
    ext_modules=[
        Extension(
            "bier.EndianedBinaryIO.C.EndianedBytesIO",
            ["src/EndianedBinaryIO/EndianedBytesIO.cpp", *default_sources],
            depends=default_depends,
            language="c++",
            include_dirs=["src"],
            extra_compile_args=extra_compile_args,
            py_limited_api=py_limited_api,
        ),
        Extension(
            "bier.EndianedBinaryIO.C.EndianedStreamIO",
            ["src/EndianedBinaryIO/EndianedStreamIO.cpp", *default_sources],
            depends=default_depends,
            language="c++",
            include_dirs=["src"],
            extra_compile_args=extra_compile_args,
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
    cmdclass=cmdclass,
)
