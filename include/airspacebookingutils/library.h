#ifndef AIRSPACEBOOKINGUTILS_LIBRARY_H
#define AIRSPACEBOOKINGUTILS_LIBRARY_H

#include <chrono>
#include <Eigen/Dense>
#include <ranges>
#include <utility>

namespace ab {
    namespace d4 {
        typedef std::chrono::time_point<std::chrono::steady_clock> TimeInstant;

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


    std::vector<CellBooking>
    getH3CellBookings(const std::vector<d4::StateVector4D> &trajectory4D, int temporalBackwardBuffer = 60 * 5,
                      int temporalForwardBuffer = 60 * 10, FPScalar spatialLateralBuffer = 100,
                      FPScalar spatialVerticalBuffer = 30, int h3Resolution = 8);

    std::vector<CellBooking>
    getH3DCellBookings(const std::vector<d4::StateVector4D> &trajectory4D, int temporalBackwardBuffer = 60 * 5,
                       int temporalForwardBuffer = 60 * 10, FPScalar spatialLateralBuffer = 100,
                       FPScalar spatialVerticalBuffer = 30, int h3Resolution = 8, int verticalResolution = 40);

    std::vector<CellBooking>
    getS2CellBookings(const std::vector<d4::StateVector4D> &trajectory4D, int temporalBackwardBuffer = 60 * 5,
                      int temporalForwardBuffer = 60 * 10, FPScalar spatialLateralBuffer = 100,
                      FPScalar spatialVerticalBuffer = 30, int s2Resolution = 13);

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


    std::string geoToH3(int h3Resolution, FPScalar latitude, FPScalar longitude);

    std::string
    geoToH3D(int h3Resolution, int verticalResolution, FPScalar latitude, FPScalar longitude, FPScalar altitude);

    std::string geoToS2(int s2Resolution, FPScalar latitude, FPScalar longitude);

    std::string
    geoToS23D(int s2Resolution, int verticalResolution, FPScalar latitude, FPScalar longitude, FPScalar altitude);
}

#endif //AIRSPACEBOOKINGUTILS_LIBRARY_H
