#ifndef AIRSPACEBOOKINGUTILS_LIBRARY_H
#define AIRSPACEBOOKINGUTILS_LIBRARY_H

#include <chrono>
#include <Eigen/Dense>
#include <ranges>
#include <utility>

namespace ab {
    namespace d4 {
        typedef std::chrono::system_clock::time_point TimeInstant;

        struct TimeSlice {
            TimeInstant start;
            TimeInstant end;

            TimeSlice() = default;

            TimeSlice(const TimeInstant &start, const TimeInstant &end)
                    : start(start),
                      end(end) {
            }

            friend bool intersects(const TimeSlice &s1, const TimeSlice &s2) {
                return (s1.start < s2.end && s1.end > s2.start)
                       || (s2.start < s1.end && s2.end > s1.start)
                       || (s1.start < s2.start && s1.end > s2.end)
                       || (s2.start < s1.start && s2.end > s1.end);
            }
        };
    }

    typedef double FPScalar;
    typedef Eigen::Vector<FPScalar, 3> Position;
    typedef Eigen::Array3i Index;

    // A polygon in local coordinates directly into gridmap matrices
    typedef std::vector<Index> Polygon;
    // A polygon in world coordinates with points in lon lat or xy order
    typedef std::vector<Position> GeoPolygon;

    namespace d4 {

        struct StateVector4D {
        public:
            ab::Position position;
            ab::d4::TimeInstant time;
            FPScalar speed;

            StateVector4D(ab::Position position, const ab::d4::TimeInstant &time, FPScalar speed)
                    : position(std::move(position)),
                      time(time),
                      speed(speed) {
            }
        };
    }

    struct CellBooking {
    public:
        ab::d4::TimeSlice timeSlice;
//        ab::Position position;
        std::string cellId;

        CellBooking(const ab::d4::TimeSlice &timeSlice, std::string cellId)
                : timeSlice(timeSlice),
                  cellId(std::move(cellId)) {
        }
    };

    /**
     * @brief Get the H3 cells that are intersected by the trajectory with their time slices
     * @param trajectory4D a vector of 4D state vectors
     * @param temporalBackwardBuffer the temporal buffer applied before the expected cell ETA in seconds
     * @param temporalForwardBuffer the temporal buffer applied after the expected cell ETA in seconds
     * @param spatialLateralBuffer the lateral spatial buffer applied to the trajectory in meters
     * @param spatialVerticalBuffer the vertical spatial buffer applied to the trajectory in meters
     * @param h3Resolution the H3 resolution to use
     * @return
     */
    std::vector<CellBooking>
    getH3CellBookings(const std::vector<d4::StateVector4D> &trajectory4D, int temporalBackwardBuffer = 60 * 5,
                      int temporalForwardBuffer = 60 * 10, FPScalar spatialLateralBuffer = 100,
                      FPScalar spatialVerticalBuffer = 30, int h3Resolution = 8);

    /**
     * @brief Get the H3D cells that are intersected by the trajectory with their time slices
     * @param trajectory4D a vector of 4D state vectors
     * @param temporalBackwardBuffer the temporal buffer applied before the expected cell ETA in seconds
     * @param temporalForwardBuffer the temporal buffer applied after the expected cell ETA in seconds
     * @param spatialLateralBuffer the lateral spatial buffer applied to the trajectory in meters
     * @param spatialVerticalBuffer the vertical spatial buffer applied to the trajectory in meters
     * @param h3Resolution the H3 resolution to use
     * @param verticalResolution the vertical resolution of the grid cells in meters
     * @return
     */
    std::vector<CellBooking>
    getH3DCellBookings(const std::vector<d4::StateVector4D> &trajectory4D, int temporalBackwardBuffer = 60 * 5,
                       int temporalForwardBuffer = 60 * 10, FPScalar spatialLateralBuffer = 100,
                       FPScalar spatialVerticalBuffer = 30, int h3Resolution = 8, int verticalResolution = 40);

    /**
     * @brief Get the S2 cells that are intersected by the trajectory with their time slices
     * @param trajectory4D a vector of 4D state vectors
     * @param temporalBackwardBuffer the temporal buffer applied before the expected cell ETA in seconds
     * @param temporalForwardBuffer the temporal buffer applied after the expected cell ETA in seconds
     * @param spatialLateralBuffer the lateral spatial buffer applied to the trajectory in meters
     * @param spatialVerticalBuffer the vertical spatial buffer applied to the trajectory in meters
     * @param s2Resolution the S2 resolution to use
     * @return
     */
    std::vector<CellBooking>
    getS2CellBookings(const std::vector<d4::StateVector4D> &trajectory4D, int temporalBackwardBuffer = 60 * 5,
                      int temporalForwardBuffer = 60 * 10, FPScalar spatialLateralBuffer = 100,
                      FPScalar spatialVerticalBuffer = 30, int s2Resolution = 13);

    /**
     * @brief Get the S2 3D cells that are intersected by the trajectory with their time slices
     * @param trajectory4D a vector of 4D state vectors
     * @param temporalBackwardBuffer the temporal buffer applied before the expected cell ETA in seconds
     * @param temporalForwardBuffer the temporal buffer applied after the expected cell ETA in seconds
     * @param spatialLateralBuffer the lateral spatial buffer applied to the trajectory in meters
     * @param spatialVerticalBuffer the vertical spatial buffer applied to the trajectory in meters
     * @param s2Resolution the S2 resolution to use
     * @param verticalResolution the vertical resolution of the grid cells in meters
     * @return
     */
    std::vector<CellBooking>
    getS23DCellBookings(const std::vector<d4::StateVector4D> &trajectory4D, int temporalBackwardBuffer = 60 * 5,
                        int temporalForwardBuffer = 60 * 10, FPScalar spatialLateralBuffer = 100,
                        FPScalar spatialVerticalBuffer = 30, int s2Resolution = 13, int verticalResolution = 40);


    std::vector<CellBooking>
    getIndexedCellBookings(std::vector<d4::StateVector4D> trajectory4D,
                           const std::function<std::string(double, double, double)> &indexer,
                           int temporalBackwardBuffer = 60 * 5,
                           int temporalForwardBuffer = 60 * 10, FPScalar spatialLateralBuffer = 100,
                           FPScalar spatialVerticalBuffer = 30);


    std::string
    geoToH3(int h3Resolution, FPScalar latitude, FPScalar longitude);

    std::string
    geoToH3D(int h3Resolution, int verticalResolution, FPScalar latitude, FPScalar longitude, FPScalar altitude);

    std::string
    geoToS2(int s2Resolution, FPScalar latitude, FPScalar longitude);

    std::string
    geoToS23D(int s2Resolution, int verticalResolution, FPScalar latitude, FPScalar longitude, FPScalar altitude);
}

#endif //AIRSPACEBOOKINGUTILS_LIBRARY_H
