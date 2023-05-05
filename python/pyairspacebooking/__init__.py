from __future__ import annotations
from ctypes import cdll
import os
from pathlib import Path

current_dir = Path(os.path.dirname(os.path.realpath(__file__)))

if os.path.exists(current_dir / 'lib'):
    lib_dir = current_dir / 'lib'
elif os.path.exists(current_dir / 'lib64'):
    lib_dir = current_dir / 'lib64'

for lib in lib_dir.glob('*'):
    cdll.LoadLibrary(lib)

from ._pyairspacebooking import __doc__, __version__, get_H3_cell_bookings, get_S2_cell_bookings, get_H3D_cell_bookings, \
    get_S23D_cell_bookings, CellBooking, StateVector4D, TimeSlice

__all__ = [
    "__doc__",
    "__version__",
    "get_H3_cell_bookings",
    "get_S2_cell_bookings",
    "get_H3D_cell_bookings",
    "get_S23D_cell_bookings",
    "CellBooking",
    "StateVector4D",
    "TimeSlice",
]

__dir__ = __all__
