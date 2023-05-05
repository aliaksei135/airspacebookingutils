import datetime

import numpy as np
import pyairspacebooking as pab


soton1 = [
    pab.StateVector4D(
        np.array([-1.39200210, 50.90768760, 100]),
        datetime.datetime(2020, 1, 1, 12, 0, 0),
        20,
    ),
    pab.StateVector4D(
        np.array([-1.45465850, 50.93035940, 100]),
        datetime.datetime(2020, 1, 1, 12, 3, 0),
        125,
    ),
]

def test_h3_cell_booking():
    print("test_h3_cell_booking")
    cells = pab.get_H3_cell_bookings(soton1)
    print(len(cells))


if __name__ == '__main__':
    test_h3_cell_booking()