import os
from importlib.machinery import EXTENSION_SUFFIXES

from setuptools import setup
from setuptools import Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.bdist_wheel import bdist_wheel


# Use the last --and presumably most compatible-- suffix.
os.environ['SETUPTOOLS_EXT_SUFFIX'] = EXTENSION_SUFFIXES[-1]


extension = Extension(
    name="abi3_abi3t_universal",
    sources=["extension.c"],
    extra_compile_args = [
        "-DPy_LIMITED_API=0x030f0000",
        "-DPy_TARGET_ABI3T=0x030f0000",
    ],
)

class BdistUniversalWheel(bdist_wheel):
    def get_tag(self):
        impl, abi, plat_name = super().get_tag()
        return 'py3', 'none', plat_name

class BuildUniversalExt(build_ext):
    def get_ext_filename(self, *args, **kwargs):
        return ext_filename.replace('.abi3.', '.abi3t.')


setup(
    ext_modules=[extension],
    cmdclass={'bdist_wheel': BdistUniversalWheel},
)
