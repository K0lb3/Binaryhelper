import os

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

INSTALL_DIR = os.path.dirname(os.path.realpath(__file__))
UNITYPYBOOST_DIR = os.path.join(INSTALL_DIR, "UnityPyBoost")


class BuildExt(build_ext):
    def build_extensions(self):
        compiler = self.compiler
        # msvc - only ever used c++20, never c++2a
        if compiler.compiler_type == "msvc":
            cpp_version_flag = "/std:c++23preview"
        # gnu & clang
        else:
            cpp_version_flag = "-std=c++23"

        for ext in self.extensions:
            ext.extra_compile_args = [cpp_version_flag]

        build_ext.build_extensions(self)


setup(
    name="bier",
    package_dir={"": "bier"},
    ext_modules=[
        Extension(
            "EndianedBinaryIO._EndianedBytesIO",
            ["src/EndianedBinaryIO/EndianedBytesIO.cpp"],
            depends=["src/EndianedBinaryIO/PyConverter.hpp"],
            language="c++",
            include_dirs=["src"],
        ),
        Extension(
            "EndianedBinaryIO._EndianedStreamIO",
            ["src/EndianedBinaryIO/EndianedStreamIO.cpp"],
            depends=["src/EndianedBinaryIO/PyConverter.hpp"],
            language="c++",
            include_dirs=["src"],
        ),
        Extension(
            "EndianedBinaryIO._EndianedIOBase",
            ["src/EndianedBinaryIO/EndianedIOBase.cpp"],
            depends=["src/PyConverter.hpp"],
            language="c++",
            include_dirs=["src"],
        ),
    ],
    cmdclass={"build_ext": BuildExt},
)
