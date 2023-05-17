import datetime

import numpy as np
import pyairspacebooking as pab
from geopy.distance import distance

soton1 = [
    pab.StateVector4D(
        np.array([-1.39200210, 50.90768760, 200]),
        datetime.datetime(2020, 1, 1, 12, 0, 0),
        10,
    ),
    pab.StateVector4D(
        np.array([-1.45465850, 50.93035940, 10]),
        datetime.datetime(2020, 1, 1, 12, 3, 0),
        15,
    ),
]

soton_vol1 = pab.Volume4D(
    [
        np.array([
            -1.3990738237161793,
            50.939269655986465
        ]),
        np.array([
            -1.4072955004378116,
            50.93887864410402
        ]),
        np.array([
            -1.404968613468128,
            50.9223555576003
        ]),
        np.array([
            -1.3790626052039556,
            50.9277335377366
        ]),
        np.array([
            -1.375494711850365,
            50.931839940288654
        ]),
        np.array([
            -1.3570347418888389,
            50.93799886438836
        ]),
        np.array([
            -1.3413670268645603,
            50.95627531580857
        ]),
        np.array([
            -1.3553283486832868,
            50.95969524295688
        ]),
        np.array([
            -1.3827856149278261,
            50.93985616161723
        ]),
        np.array([
            -1.3990738237161793,
            50.939269655986465
        ])
    ],
    20, 120,
    pab.TimeSlice(datetime.datetime(2020, 1, 1, 12, 0, 0), datetime.datetime(2020, 1, 1, 12, 10, 0))
)


def get_distance_2d(sv1, sv2):
    # GeoPy wants lat, lon so need to swap
    return distance((sv1.position[1], sv1.position[0]),
                    (sv2.position[1], sv2.position[0])).meters


def test_h3_cell_booking():
    temporal_backward_buffer = 60 * 5
    temporal_forward_buffer = 60 * 10

    cells_r7 = pab.get_H3_cell_bookings(soton1, h3_resolution=7, temporal_backward_buffer=temporal_backward_buffer,
                                        temporal_forward_buffer=temporal_forward_buffer)
    cells_r8 = pab.get_H3_cell_bookings(soton1, h3_resolution=8, temporal_backward_buffer=temporal_backward_buffer,
                                        temporal_forward_buffer=temporal_forward_buffer)
    cells_r9 = pab.get_H3_cell_bookings(soton1, h3_resolution=9, temporal_backward_buffer=temporal_backward_buffer,
                                        temporal_forward_buffer=temporal_forward_buffer)

    # Test correct number of cells
    assert len(cells_r7) == 5
    assert len(cells_r8) == 11
    assert len(cells_r9) == 30

    # Test correct timeslice backward buffering
    assert cells_r7[0].time_slice.start == soton1[0].time - datetime.timedelta(seconds=temporal_backward_buffer)
    assert cells_r8[0].time_slice.start == soton1[0].time - datetime.timedelta(seconds=temporal_backward_buffer)
    assert cells_r9[0].time_slice.start == soton1[0].time - datetime.timedelta(seconds=temporal_backward_buffer)

    # Test correct timeslice forward buffering
    assert cells_r7[0].time_slice.end >= soton1[0].time + datetime.timedelta(seconds=temporal_forward_buffer)
    assert cells_r8[0].time_slice.end >= soton1[0].time + datetime.timedelta(seconds=temporal_forward_buffer)
    assert cells_r9[0].time_slice.end >= soton1[0].time + datetime.timedelta(seconds=temporal_forward_buffer)

    # Test duration is greater than the forward and backward buffers
    assert cells_r7[-1].time_slice.end - cells_r7[0].time_slice.start > datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer)
    assert cells_r8[-1].time_slice.end - cells_r8[0].time_slice.start > datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer)
    assert cells_r9[-1].time_slice.end - cells_r9[0].time_slice.start > datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer)

    # Test duration is approximately correct
    distance = get_distance_2d(soton1[0], soton1[-1])
    time_to_travel = datetime.timedelta(minutes=distance / soton1[0].speed)

    assert (cells_r7[-1].time_slice.end - cells_r7[0].time_slice.start) - time_to_travel - datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer) < datetime.timedelta(minutes=3)
    assert (cells_r8[-1].time_slice.end - cells_r8[0].time_slice.start) - time_to_travel - datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer) < datetime.timedelta(minutes=3)
    assert (cells_r9[-1].time_slice.end - cells_r9[0].time_slice.start) - time_to_travel - datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer) < datetime.timedelta(minutes=3)

    # Test sorted timeslices
    for i in range(len(cells_r7) - 1):
        assert cells_r7[i].time_slice.start <= cells_r7[i + 1].time_slice.start
    for i in range(len(cells_r8) - 1):
        assert cells_r8[i].time_slice.start <= cells_r8[i + 1].time_slice.start
    for i in range(len(cells_r9) - 1):
        assert cells_r9[i].time_slice.start <= cells_r9[i + 1].time_slice.start

    # Test correct cell resolution
    for cell in cells_r7:
        assert cell.cell_id.endswith('ffffff')
    for cell in cells_r8:
        assert cell.cell_id.endswith('fffff')
    for cell in cells_r9:
        assert cell.cell_id.endswith('ffff')


def test_h3_volume_booking():
    cells_r7 = pab.get_H3_volume_bookings(soton_vol1, h3_resolution=7)
    cells_r8 = pab.get_H3_volume_bookings(soton_vol1, h3_resolution=8)
    cells_r9 = pab.get_H3_volume_bookings(soton_vol1, h3_resolution=9)

    # Test correct number of cells
    assert len(cells_r7) == 7
    assert len(cells_r8) == 27
    assert len(cells_r9) == 63

    # Test correct timeslice
    for cell in cells_r7:
        assert cell.time_slice.start == soton_vol1[0].time
        assert cell.time_slice.end == soton_vol1[-1].time
    for cell in cells_r8:
        assert cell.time_slice.start == soton_vol1[0].time
        assert cell.time_slice.end == soton_vol1[-1].time
    for cell in cells_r9:
        assert cell.time_slice.start == soton_vol1[0].time
        assert cell.time_slice.end == soton_vol1[-1].time

    # Test correct cell resolution
    for cell in cells_r7:
        assert cell.cell_id.endswith('ffffff')
    for cell in cells_r8:
        assert cell.cell_id.endswith('fffff')
    for cell in cells_r9:
        assert cell.cell_id.endswith('ffff')


def test_h3d_cell_booking():
    temporal_backward_buffer = 60 * 5
    temporal_forward_buffer = 60 * 10

    cells_r7 = pab.get_H3D_cell_bookings(soton1, h3_resolution=7, temporal_backward_buffer=temporal_backward_buffer,
                                         temporal_forward_buffer=temporal_forward_buffer)
    cells_r8 = pab.get_H3D_cell_bookings(soton1, h3_resolution=8, temporal_backward_buffer=temporal_backward_buffer,
                                         temporal_forward_buffer=temporal_forward_buffer)
    cells_r9 = pab.get_H3D_cell_bookings(soton1, h3_resolution=9, temporal_backward_buffer=temporal_backward_buffer,
                                         temporal_forward_buffer=temporal_forward_buffer)

    # Test correct number of cells
    assert len(cells_r7) == 14
    assert len(cells_r8) == 26
    assert len(cells_r9) == 62

    # Test correct timeslice backward buffering
    assert cells_r7[0].time_slice.start == soton1[0].time - datetime.timedelta(seconds=temporal_backward_buffer)
    assert cells_r8[0].time_slice.start == soton1[0].time - datetime.timedelta(seconds=temporal_backward_buffer)
    assert cells_r9[0].time_slice.start == soton1[0].time - datetime.timedelta(seconds=temporal_backward_buffer)

    # Test correct timeslice forward buffering
    assert cells_r7[0].time_slice.end >= soton1[0].time + datetime.timedelta(seconds=temporal_forward_buffer)
    assert cells_r8[0].time_slice.end >= soton1[0].time + datetime.timedelta(seconds=temporal_forward_buffer)
    assert cells_r9[0].time_slice.end >= soton1[0].time + datetime.timedelta(seconds=temporal_forward_buffer)

    # Test duration is greater than the forward and backward buffers
    assert cells_r7[-1].time_slice.end - cells_r7[0].time_slice.start > datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer)
    assert cells_r8[-1].time_slice.end - cells_r8[0].time_slice.start > datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer)
    assert cells_r9[-1].time_slice.end - cells_r9[0].time_slice.start > datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer)

    # Test duration is approximately correct
    distance = get_distance_2d(soton1[0], soton1[-1])
    time_to_travel = datetime.timedelta(minutes=distance / soton1[0].speed)

    assert (cells_r7[-1].time_slice.end - cells_r7[0].time_slice.start) - time_to_travel - datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer) < datetime.timedelta(minutes=3)
    assert (cells_r8[-1].time_slice.end - cells_r8[0].time_slice.start) - time_to_travel - datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer) < datetime.timedelta(minutes=3)
    assert (cells_r9[-1].time_slice.end - cells_r9[0].time_slice.start) - time_to_travel - datetime.timedelta(
        seconds=temporal_backward_buffer + temporal_forward_buffer) < datetime.timedelta(minutes=3)

    # Test sorted timeslices
    for i in range(len(cells_r7) - 1):
        assert cells_r7[i].time_slice.start <= cells_r7[i + 1].time_slice.start
    for i in range(len(cells_r8) - 1):
        assert cells_r8[i].time_slice.start <= cells_r8[i + 1].time_slice.start
    for i in range(len(cells_r9) - 1):
        assert cells_r9[i].time_slice.start <= cells_r9[i + 1].time_slice.start

    # Test correct cell resolution
    import re
    for cell in cells_r7:
        assert re.match('.*ffff.{2}$', cell.cell_id)
    for cell in cells_r8:
        assert re.match('.*fff.{2}$', cell.cell_id)
    for cell in cells_r9:
        assert re.match('.*ff.{2}$', cell.cell_id)


if __name__ == '__main__':
    test_h3_cell_booking()
    test_h3d_cell_booking()
