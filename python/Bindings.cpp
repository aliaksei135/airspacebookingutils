#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/eigen.h>
#include <airspacebookingutils/library.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(_pyairspacebooking, m) {
    m.doc() = "Python bindings for airspacebookingutils";
#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif

    py::class_<ab::d4::StateVector4D>(m, "StateVector4D")
            .def(py::init<ab::Position, ab::d4::TimeInstant, ab::FPScalar>())
            .def_readwrite("position", &ab::d4::StateVector4D::position, "Longitude, latitude, altitude vector")
            .def_readwrite("time", &ab::d4::StateVector4D::time, "Time instant")
            .def_readwrite("speed", &ab::d4::StateVector4D::speed, "Speed in m/s")
            .def("__repr__",
                 [](const ab::d4::StateVector4D &sv) {
                     return "StateVector4D(position=" + std::to_string(sv.position[0]) + "," +
                            std::to_string(sv.position[1]) + "," + std::to_string(sv.position[2]) + "," +
                            std::to_string(sv.position[3]) + ", time=" +
                            std::to_string(sv.time.time_since_epoch().count()) +
                            ", speed=" + std::to_string(sv.speed) + ")";
                 }
            );

    py::class_<ab::d4::TimeSlice>(m, "TimeSlice")
            .def(py::init<ab::d4::TimeInstant, ab::d4::TimeInstant>())
            .def_readwrite("start", &ab::d4::TimeSlice::start, "Start time instant")
            .def_readwrite("end", &ab::d4::TimeSlice::end, "End time instant")
            .def("__repr__",
                 [](const ab::d4::TimeSlice &ts) {
                     return "TimeSlice(start=" + std::to_string(ts.start.time_since_epoch().count()) + ", end=" +
                            std::to_string(ts.end.time_since_epoch().count()) + ")";
                 }
            );

    py::class_<ab::CellBooking>(m, "CellBooking")
            .def(py::init<ab::d4::TimeSlice, std::string>())
            .def_readwrite("time_slice", &ab::CellBooking::timeSlice, "Time slice")
            .def_readwrite("cell_id", &ab::CellBooking::cellId, "Cell ID")
            .def("__repr__",
                 [](const ab::CellBooking &cb) {
                     return "CellBooking(time_slice=" + std::to_string(cb.timeSlice.start.time_since_epoch().count()) +
                            "," + std::to_string(cb.timeSlice.end.time_since_epoch().count()) + ", cell_id=" +
                            cb.cellId + ")";
                 }
            );

    m.def("get_H3_cell_bookings", &ab::getH3CellBookings, "Get H3 cell bookings",
          "trajectory_4d"_a, "temporal_backward_buffer"_a = 60 * 5, "temporal_forward_buffer"_a = 60 * 10,
          "spatial_lateral_buffer"_a = 100.0, "spatial_vertical_buffer"_a = 30.0, "h3_resolution"_a = 8,
          R"pbdoc(
    Get the H3 cells that are intersected by the trajectory with their time slices

    Args:
        trajectory_4d (list): a list of 4D state vectors
        temporal_backward_buffer (int): the temporal buffer applied before the expected cell ETA in seconds
        temporal_forward_buffer (int): the temporal buffer applied after the expected cell ETA in seconds
        spatial_lateral_buffer (float): the lateral spatial buffer applied to the trajectory in meters
        spatial_vertical_buffer (float): the vertical spatial buffer applied to the trajectory in meters
        h3_resolution (int): the H3 resolution to use

    Returns:
        list: a list of cell bookings
    )pbdoc");

    m.def("get_H3D_cell_bookings", &ab::getH3DCellBookings, "Get H3D cell bookings",
          "trajectory_4d"_a, "temporal_backward_buffer"_a = 60 * 5, "temporal_forward_buffer"_a = 60 * 10,
          "spatial_lateral_buffer"_a = 100.0, "spatial_vertical_buffer"_a = 30.0, "h3_resolution"_a = 8,
          "vertical_resolution"_a = 40,
          R"pbdoc(
    Get the H3D cells that are intersected by the trajectory with their time slices

    Args:
        trajectory_4d (list): a list of 4D state vectors
        temporal_backward_buffer (int): the temporal buffer applied before the expected cell ETA in seconds
        temporal_forward_buffer (int): the temporal buffer applied after the expected cell ETA in seconds
        spatial_lateral_buffer (float): the lateral spatial buffer applied to the trajectory in meters
        spatial_vertical_buffer (float): the vertical spatial buffer applied to the trajectory in meters
        h3_resolution (int): the H3 resolution to use
        vertical_resolution (int): the vertical resolution of the grid cells in meters

    Returns:
        list: a list of cell bookings
    )pbdoc");

    m.def("get_S2_cell_bookings", &ab::getS2CellBookings, "Get S2 cell bookings",
          "trajectory_4d"_a, "temporal_backward_buffer"_a = 60 * 5, "temporal_forward_buffer"_a = 60 * 10,
          "spatial_lateral_buffer"_a = 100.0, "spatial_vertical_buffer"_a = 30.0, "s2_resolution"_a = 8,
          R"pbdoc(
    Get the S2 cells that are intersected by the trajectory with their time slices

    Args:
        trajectory_4d (list): a list of 4D state vectors
        temporal_backward_buffer (int): the temporal buffer applied before the expected cell ETA in seconds
        temporal_forward_buffer (int): the temporal buffer applied after the expected cell ETA in seconds
        spatial_lateral_buffer (float): the lateral spatial buffer applied to the trajectory in meters
        spatial_vertical_buffer (float): the vertical spatial buffer applied to the trajectory in meters
        s2_resolution (int): the S2 resolution to use

    Returns:
        list: a list of cell bookings
    )pbdoc");

    m.def("get_S23D_cell_bookings", &ab::getS23DCellBookings, "Get S23D cell bookings",
          "trajectory_4d"_a, "temporal_backward_buffer"_a = 60 * 5, "temporal_forward_buffer"_a = 60 * 10,
          "spatial_lateral_buffer"_a = 100.0, "spatial_vertical_buffer"_a = 30.0, "s2_resolution"_a = 8,
          "vertical_resolution"_a = 40,
          R"pbdoc(
    Get the S23D cells that are intersected by the trajectory with their time slices

    Args:
        trajectory_4d (list): a list of 4D state vectors
        temporal_backward_buffer (int): the temporal buffer applied before the expected cell ETA in seconds
        temporal_forward_buffer (int): the temporal buffer applied after the expected cell ETA in seconds
        spatial_lateral_buffer (float): the lateral spatial buffer applied to the trajectory in meters
        spatial_vertical_buffer (float): the vertical spatial buffer applied to the trajectory in meters
        s2_resolution (int): the S2 resolution to use
        vertical_resolution (int): the vertical resolution of the grid cells in meters

    Returns:
        list: a list of cell bookings
    )pbdoc");

}
