import os
import sys
from importlib.machinery import EXTENSION_SUFFIXES

from setuptools import setup
from setuptools import Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.bdist_wheel import bdist_wheel


# Use the last --and presumably most compatible-- suffix.
os.environ['SETUPTOOLS_EXT_SUFFIX'] = EXTENSION_SUFFIXES[-1]

if 'DONT_USE_ABI3T' in os.environ:
    extra_compile_args = [
        "-DPy_LIMITED_API=0x030f0000",
    ]
else:
    extra_compile_args = [
        "-DPy_TARGET_ABI3T=0x030f0000",
    ]


extension = Extension(
    name="abi3_abi3t_universal",
    sources=["extension.c"],
    extra_compile_args = extra_compile_args,
)


class BdistUniversalWheel(bdist_wheel):
    def get_tag(self):
        impl, abi, plat_name = super().get_tag()
        impl = 'cp313'
        abi = 'abi3.abi3t'
        if 'DONT_USE_ABI3T' in os.environ:
            abi = 'abi3'
        elif sys.platform == 'win32':
            abi = 'abi3t'
        if plat_name.startswith('linux_'):
            plat_name = plat_name.replace('linux_', 'manylinux1_', 1)
        return impl, abi, plat_name


setup(
    ext_modules=[extension],
    cmdclass={'bdist_wheel': BdistUniversalWheel},
)
