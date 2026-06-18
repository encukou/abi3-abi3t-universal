This minimal extension demoes wide Stable ABI compatibility.

Four wheels to cover 3 CPython versions × 3 operating systems × 2
GIL/free-threading variants.

Note that up-to date tools -- ones that are aware of the `abi3t` wheel tag
(for example, `pip` >= 26.1, `packaging` >= 26.1, `uv` >= 0.11.3).

CPython versions: 3.13, 3.14, and 3.15.0b1+.

OSs: Linux (64-bit x86), macOS (universal2), Windows (64-bit x86)
(Covering other CPU architectures is possible but would mostly just complicate
this demo.)


## The shim

The file ``ft_compat.h`` contains the compatibility shim needed to make the
new API necessary for ``abi3t`` (PEP 793 ``PyModExport`` & PEP 803 ``PySlot``)
work on older versions of Python.
It's intended to be reusable, and includes usage comment.

Note that the extension must be *compiled* with CPython 3.15+.


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

As `abi3t` is not widely supported in build tools at
the time of writing, we use Setuptools, and reach into its internals
to select the appropriate tags.

On Windows, until CPython 3.15.0b2, separate `abi3` and `abi3t` wheels
are required due to DLL linkage details.
(This will not be fixed for 3.13 and 3.14.)

This project builds two wheels:

- abi3_abi3t_universal-0.2-cp313-**abi3**-win_amd64.whl
- abi3_abi3t_universal-0.2-cp313-**abi3t**-win_amd64.whl

For real libraries, it might be more practical to build 4 wheels,
three of which can be dropped when 3.13 & 3.14 support is removed:

- abi3_abi3t_universal-0.2-**cp313-cp313t**-win_amd64.whl (for 3.13 FT)
- abi3_abi3t_universal-0.2-**cp314-cp314t**-win_amd64.whl (for 3.14 FT)
- abi3_abi3t_universal-0.2-cp313-abi3-win_amd64.whl  (for 3.13 GIL & 3.14 GIL)
- abi3_abi3t_universal-0.2-**cp315**-abi3.abi3t-win_amd64.whl (3.15+)

(The first two can be merged into **cp313.cp314-cp313t.cp314t** but I'd be
a bit worried about tool support for such a tag.)


## Licence

All my contributions are marked CC0 1.0, see `LICENCE.CC0`.

The file `ft_compat.h` copies/adapts definitions from CPython headers,
as necessary for compatibility.
I'm pretty sure that's fair use, but I'm not a lawyer.
Follow the [PSF licence](https://docs.python.org/3/license.html) to be safe.
