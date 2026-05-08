This minimal extension demoes wide Stable ABI compatibility.

Four wheels to cover 3 CPython versions × 3 operating systems × 2
GIL/free-threading variants.

It targets pre-`abi3t` build and install tools; see Caveats below.

CPython versions: 3.13, 3.14, and 3.15.0b1+.

OSs: Linux (64-bit x86), macOS (universal2), Windows (64-bit x86)
(Covering other CPU architectures is possible but would mostly just complicate
this demo.)



## Installation

    python -m pip install abi3-abi3t-universal


## Usage

There's one trivial function that increments and returns a number,
mostly to demo module state.

    >>> import abi3_abi3t_universal
    >>> abi3_abi3t_universal.increment_value()
    0
    >>> abi3_abi3t_universal.increment_value()
    1
    >>> abi3_abi3t_universal.increment_value()
    2


## Caveats

CPython 3.15 is in Alpha; its ABI (and API) may change, making the released
wheels crash.

As `abi3t` is not widely supported in build tools at
the time of writing, we use Setuptools, and reach into its internals
to select the appropriate tags.

Windows requires `abi3` and `abi3t` wheels due to DLL linkage details;
this might be fixed in CPython 3.15.0 (but not in earlier versions).

As `abi3t` is not widely supported in CPython and *install* tools,
the built wheels are tagged a bit weirdly:

- Linux:
  - has: abi3_abi3t_universal-0.1-**py3**-**none**-manylinux1_x86_64.manylinux_2_5_x86_64.whl
  - should be: abi3_abi3t_universal-0.1-**cp313**-**abi3.abi3t**-manylinux1_x86_64.manylinux_2_5_x86_64.whl

- Mac:
  - has: abi3_abi3t_universal-0.1-**py3**-**none**-macosx_10_15_universal2.whl
  - should be: abi3_abi3t_universal-0.1-**cp313**-**abi3.abi3t**-macosx_10_15_universal2.whl

- Windows GIL:
  - has: abi3_abi3t_universal-0.1-cp313-abi3-win_amd64.whl
  - which is right :)

- Windows free-threading (also compatible with 3.15+ GIL):
  - has: abi3_abi3t_universal-0.1-**cp313.cp314.cp315-cp313t.cp314t.cp315t**-win_amd64.whl
  - should be: abi3_abi3t_universal-0.1-**cp313-abi3t**-win_amd64.whl
  - or: abi3_abi3t_universal-0.1-**cp315-abi3.abi3t**-win_amd64.whl


## Licence

All my contributions are marked CC0 1.0, see `LICENCE.CC0`.

The file `ft_compat.h` copies/adapts definitions from CPython headers,
as necessary for compatibility.
I'm pretty sure that's fair use, but I'm not a lawyer.
Follow the [PSF licence](https://docs.python.org/3/license.html) to be safe.
