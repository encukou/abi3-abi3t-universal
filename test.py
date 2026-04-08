#! /usr/bin/python

import sys
import shlex
import shutil
import functools
import subprocess
from pathlib import Path


HERE = Path()
TESTDIR = HERE / '.testdir'
DISTDIR = HERE / '.dist'
TESTSCRIPT = TESTDIR / 'script.py'

if TESTDIR.exists():
    shutil.rmtree(TESTDIR)
if DISTDIR.exists():
    shutil.rmtree(DISTDIR)
TESTDIR.mkdir()

TESTSCRIPT.write_text("""
import abi3_abi3t_universal
print(abi3_abi3t_universal.increment_value())
print(abi3_abi3t_universal.increment_value())
print(abi3_abi3t_universal.increment_value())
""")

def run(*args, **kwargs):
    kwargs.setdefault('check', True)
    kwargs.setdefault('encoding', 'utf-8')
    print('run:', *(shlex.quote(str(a)) for a in args))
    return subprocess.run(args, **kwargs)

run(sys.executable, '-m', 'build', '--wheel', '-o', DISTDIR)

[wheel] = DISTDIR.resolve().glob('*.whl')

for pyver in '3.13', '3.13t', '3.14', '3.14t', '3.15', '3.15t', None:
    if pyver:
        outer_python = 'python' + pyver
        VENVDIR = TESTDIR / f'venv_{pyver}'
    else:
        outer_python = sys.executable
        VENVDIR = TESTDIR / f'venv_this'
    run(outer_python, '--version')
    run(outer_python, '-m', 'venv', VENVDIR)
    venv_python = VENVDIR / 'bin/python'
    run(venv_python, '-m', 'pip', 'install', wheel)
    result = run(venv_python, TESTSCRIPT, stdout=subprocess.PIPE)
    print(result.stdout)
    if result.stdout.split() != ['0', '1', '2']:
        raise AssertionError('bad output')
