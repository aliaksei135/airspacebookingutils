#include "../include/airspacebookingutils/library.h"
#include "../include/airspacebookingutils/util/GeometryProjectionUtils.h"
#include "../include/airspacebookingutils/util/GeometryOperations.h"
#include "../include/airspacebookingutils/util/4DUtils.h"
#include "../include/airspacebookingutils/util/Bresenham3D.h"

#include <iostream>
#include <spdlog/spdlog.h>
#include <h3/h3api.h>
#include <s2/s2point.h>
#include <s2/s2latlng.h>
#include <s2/s2cell_id.h>


#define RADIANS(x) (x/180 * M_PI)
#define DEGREES(x) (x * 180 / M_PI)

constexpr int GRID_SCALE_FACTOR = 40.0f;


std::vector<ab::CellBooking>
ab::getH3CellBookings(const std::vector<d4::StateVector4D> &traj, int temporalBackwardBuffer, int temporalForwardBuffer,
                      FPScalar spatialLateralBuffer, FPScalar spatialVerticalBuffer, int h3Resolution) {
    std::function<std::string(FPScalar, FPScalar, FPScalar)> indexer = [h3Resolution](FPScalar lat, FPScalar lng,
                                                                                      FPScalar alt) {
        return geoToH3(h3Resolution, lat, lng);
    };
    return getIndexedCellBookings(traj, indexer, temporalBackwardBuffer, temporalForwardBuffer, spatialLateralBuffer,
                                  spatialVerticalBuffer);
}

std::vector<ab::CellBooking>
ab::getH3DCellBookings(const std::vector<d4::StateVector4D> &trajectory4D, int temporalBackwardBuffer,
                       int temporalForwardBuffer, FPScalar spatialLateralBuffer, FPScalar spatialVerticalBuffer,
                       int h3Resolution, int verticalResolution) {
    std::function<std::string(FPScalar, FPScalar, FPScalar)> indexer = [h3Resolution, verticalResolution](FPScalar lat,
                                                                                                          FPScalar lng,
                                                                                                          FPScalar alt) {
        return geoToH3D(h3Resolution, verticalResolution, lat, lng, alt);
    };
    return getIndexedCellBookings(trajectory4D, indexer, temporalBackwardBuffer, temporalForwardBuffer,
                                  spatialLateralBuffer, spatialVerticalBuffer);
}

std::vector<ab::CellBooking>
ab::getS2CellBookings(const std::vector<d4::StateVector4D> &trajectory4D, int temporalBackwardBuffer,
                      int temporalForwardBuffer, FPScalar spatialLateralBuffer, FPScalar spatialVerticalBuffer,
                      int s2Resolution) {
    std::function<std::string(FPScalar, FPScalar, FPScalar)> indexer = [s2Resolution](FPScalar lat, FPScalar lng,
                                                                                      FPScalar alt) {
        return geoToS2(s2Resolution, lat, lng);
    };
    return getIndexedCellBookings(trajectory4D, indexer, temporalBackwardBuffer, temporalForwardBuffer,
                                  spatialLateralBuffer, spatialVerticalBuffer);
}

std::vector<ab::CellBooking>
ab::getS23DCellBookings(const std::vector<d4::StateVector4D> &trajectory4D, int temporalBackwardBuffer,
                        int temporalForwardBuffer, FPScalar spatialLateralBuffer, FPScalar spatialVerticalBuffer,
                        int s2Resolution, int verticalResolution) {
    std::function<std::string(FPScalar, FPScalar, FPScalar)> indexer = [s2Resolution, verticalResolution](FPScalar lat,
                                                                                                          FPScalar lng,
                                                                                                          FPScalar alt) {
        return geoToS23D(s2Resolution, verticalResolution, lat, lng, alt);
    };
    return getIndexedCellBookings(trajectory4D, indexer, temporalBackwardBuffer, temporalForwardBuffer,
                                  spatialLateralBuffer, spatialVerticalBuffer);
}

std::vector<ab::CellBooking> ab::getIndexedCellBookings(std::vector<d4::StateVector4D> trajectory4D,
                                                        const std::function<std::string(double, double,
                                                                                        double)> &indexer,
                                                        int temporalBackwardBuffer, int temporalForwardBuffer,
                                                        FPScalar spatialLateralBuffer, FPScalar spatialVerticalBuffer) {
    // The Eckert VI projection is good enough for the whole world
    // The only distances being measured are between points on the same trajectory
    // rather than the start to end of the trajectory
    // UTM could be used for a more accurate projection, but the accuracy improvement is smaller
    // than the applied buffer and is much less than the eventual loss of accuracy after discretisation
    // to an indexing system
    const auto pj = util::makeProjObject("EPSG:4326", "ESRI:54010");
    PJ *reproj = std::get<0>(pj);
    const auto revpj = util::makeProjObject("ESRI:54010", "EPSG:4326");
    PJ *revReproj = std::get<0>(revpj);
    spdlog::info("Made PROJ contexts");
    GEOSContextHandle_t geosCtx = initGEOS_r(notice, log_and_exit);
    spdlog::info("Made GEOS Context");

    // Iterate through all points in the trajectory and rasterise between them
    const auto lsSize = trajectory4D.size();

    spdlog::info("Converting to GEOS objects...");
    auto *trajCoordSeq = GEOSCoordSeq_create_r(geosCtx, lsSize, 3);
    for (int i = 0; i < lsSize; ++i) {
        const auto &sv = trajectory4D[i];
        GEOSCoordSeq_setXYZ_r(geosCtx, trajCoordSeq, i, sv.position.x(), sv.position.y(), sv.position.z());
    }
    auto *trajLs = GEOSGeom_createLineString_r(geosCtx, trajCoordSeq);
    spdlog::info("\tCreated World LineString");

    auto *reprojTrajCoordSeq = util::reprojectCoordinates_r(reproj, trajCoordSeq, geosCtx);
    std::vector<ab::Index> reprojTrajIntCoords(lsSize);
    {
        double x, y, z;
        for (int i = 0; i < lsSize; ++i) {
            GEOSCoordSeq_getXYZ_r(geosCtx, reprojTrajCoordSeq, i, &x, &y, &z);
            spdlog::info(
                    "Reprojected coordinate " + std::to_string(trajectory4D[i].position.x()) + ", " +
                    std::to_string(trajectory4D[i].position.y()) + ", " + std::to_string(trajectory4D[i].position.z()) +
                    " to " +
                    std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z));
            reprojTrajIntCoords[i] = ab::Index(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z));
        }
    }
    auto *reprojLs = GEOSGeom_createLineString_r(geosCtx, reprojTrajCoordSeq);
    if (reprojLs == nullptr) {
        spdlog::error("Reprojected LineString is null");
    }
    spdlog::info("\tCreated Projected LineString");
    auto *reprojBufferPoly = GEOSBuffer_r(geosCtx, reprojLs, spatialLateralBuffer, 30);
    if (reprojBufferPoly == nullptr) {
        spdlog::error("Reprojected buffer is null");
    }
    auto reprojBufferGeoPoly = util::asGeoPolygon_r(reprojBufferPoly, geosCtx);
    spdlog::info("\tBuffered Projected LineString");
    auto *bufferPoly = util::swapCoordOrder_r(util::reprojectPolygon_r(revReproj, reprojBufferPoly, geosCtx), geosCtx);
    auto bufferGeoPoly = util::asGeoPolygon_r(bufferPoly, geosCtx);
    spdlog::info("\tConverted to World GeoPolygon");

    GEOSGeom_destroy_r(geosCtx, bufferPoly);
    spdlog::info("\tFreed World Buffer Polygon");
    GEOSGeom_destroy_r(geosCtx, reprojBufferPoly);
    spdlog::info("\tFreed Projected Buffer Polygon");
    GEOSGeom_destroy_r(geosCtx, reprojLs);
    spdlog::info("\tFreed Projected LineString");
    GEOSGeom_destroy_r(geosCtx, trajLs);
    spdlog::info("\tFreed World LineString");

    spdlog::info("Assigning nearest trajectory points to buffer cells...");
    for (auto &coord: bufferGeoPoly) {
        std::vector<std::pair<int, FPScalar>> distances;
        distances.reserve(trajectory4D.size());
        for (int j = 0; j < trajectory4D.size(); ++j) {
            distances.emplace_back(j, util::euclideanDistance<2>(coord, trajectory4D[j].position));
        }
        const auto minNode = std::min_element(distances.cbegin(), distances.cend(),
                                              [](const auto &a, const auto &b) {
                                                  return a.second < b.second;
                                              });
        coord[2] = trajectory4D[minNode->first].position[2];
    }

    auto indexCmp = [](const Index &i1, const Index &i2) {
        for (int d = 0; d < i1.size(); ++d) {
            if (i1(d) != i2(d))
                return i1(d) < i2(d);
        }
        return false;
    };
    std::vector<Index, Eigen::aligned_allocator<Index>> trajPoints;
    std::map<Index, d4::TimeSlice, decltype(indexCmp)> trajPointMap(indexCmp);

    spdlog::info("Projecting cell ETAs forward...");
    for (int i = 0; i < lsSize - 1; ++i) {
        // Narrow down the possible voxels intersected by passing through bresenham algo
        // This requires projection to local grid coords as bresenham is integer based
        const auto &prevProjP = reprojTrajIntCoords[i] / GRID_SCALE_FACTOR;
        const auto &projP = reprojTrajIntCoords[i + 1] / GRID_SCALE_FACTOR;
        auto points = util::Bresenham3D::line3d(prevProjP, projP);

        ab::d4::TimeInstant posETA;
        ab::d4::TimeSlice desiredTimeSlice({}, {}); // Initialise with random values

        for (const auto &c: points) {
            // Get the Euclidean distance from the previous point to this point
            const auto dist = std::sqrt(((prevProjP - c) * GRID_SCALE_FACTOR).array().square().sum());
            // Project the ETA to this cell based on a linear interpolation of the speed
            posETA = trajectory4D[i].time + std::chrono::seconds(static_cast<int>(dist / trajectory4D[i].speed));

            // Buffer around the ETA
            desiredTimeSlice = d4::TimeSlice(posETA - std::chrono::seconds(temporalBackwardBuffer),
                                             posETA + std::chrono::seconds(temporalForwardBuffer));
            trajPoints.emplace_back(c * GRID_SCALE_FACTOR);
            trajPointMap.emplace(c * GRID_SCALE_FACTOR, desiredTimeSlice);
        }
    }

    // We store the deconflicted bookings first before committing them to the grid
    // This is in case the trajectory fails to deconflict at a later stage and we
    // have already booked previous cells in the grid
    std::vector<CellBooking> clearedTimeSlices;

    spdlog::info("\tGetting bounds of buffer...");
    const auto bounds = util::getPolyBounds<3>(reprojBufferGeoPoly);
    // Cast down to ints as they will be iterated over
    // The scale is so small that no precision is lost
    int xMin = static_cast<int>(bounds[0]), xMax = static_cast<int>(bounds[3] + 1);
    int yMin = static_cast<int>(bounds[1]), yMax = static_cast<int>(bounds[4] + 1);

    spdlog::info("Iterating buffer bounds to book cells...");
//#pragma omp parallel for collapse(2) schedule(dynamic)
    for (int x = xMin; x < xMax; x += GRID_SCALE_FACTOR) {
        for (int y = yMin; y < yMax; y += GRID_SCALE_FACTOR) {
            const Eigen::Vector2i xyC{x, y};
            if (!util::isInsidePolygon(reprojBufferGeoPoly, xyC)) continue;

            std::vector<std::pair<int, FPScalar>> distances;
            distances.reserve(trajPoints.size());
            for (int i = 0; i < trajPoints.size(); ++i) {
                distances.emplace_back(i, util::euclideanDistance<2>({x, y, 0}, trajPoints[i]));
            }
            const auto minNode = std::min_element(distances.cbegin(), distances.cend(),
                                                  [](const auto &a, const auto &b) {
                                                      return a.second < b.second;
                                                  });
            const auto trajPoint = trajPoints[minNode->first];
            const auto desiredTimeSlice = trajPointMap.at(trajPoint);
            const auto midZ = static_cast<FPScalar>(trajPoint.z());
            const int minZ = static_cast<int>(std::max(midZ - spatialVerticalBuffer, static_cast<FPScalar>(0)));
            const int maxZ = static_cast<int>(midZ + spatialVerticalBuffer);

            for (int z = minZ; z < maxZ; z += GRID_SCALE_FACTOR) {
                const Index xyzC{x, y, z};
                const auto projCoord = util::reprojectCoordinate_r(revReproj, xyzC.x(), xyzC.y(), xyzC.z());
                clearedTimeSlices.emplace_back(desiredTimeSlice,
                                               indexer(projCoord.xyz.y, projCoord.xyz.x, projCoord.xyz.z));
            }
        }
    }

    // Map each cell ID to a vector of time slices from clearedTimeSlices
    std::unordered_map<std::string, std::vector<d4::TimeSlice>> cellTimeSlices;
    for (const auto &booking: clearedTimeSlices) {
        cellTimeSlices[booking.cellId].emplace_back(booking.timeSlice);
    }
    // For each cell ID in the map, combine all overlapping time slices by checking their intersections
    std::vector<CellBooking> finalBookings;
    for (const auto &cellTimeSlice: cellTimeSlices) {
        const auto &cellId = cellTimeSlice.first;
        const auto &timeSlices = cellTimeSlice.second;
        if (timeSlices.size() == 1) {
            finalBookings.emplace_back(timeSlices[0], cellId);
            continue;
        }
        // Sort time slices by start time
        std::vector<d4::TimeSlice> sortedTimeSlices = timeSlices;
        std::sort(sortedTimeSlices.begin(), sortedTimeSlices.end(),
                  [](const auto &a, const auto &b) {
                      return a.start < b.start;
                  });
        // Merge time slices
        std::vector<d4::TimeSlice> mergedTimeSlices;
        mergedTimeSlices.emplace_back(sortedTimeSlices[0]);
        for (int i = 1; i < sortedTimeSlices.size(); ++i) {
            const auto &prev = mergedTimeSlices.back();
            const auto &curr = sortedTimeSlices[i];
            if (prev.end >= curr.start) {
                mergedTimeSlices.back().end = curr.end;
            } else {
                mergedTimeSlices.emplace_back(curr);
            }
        }
        // Add merged time slices to final bookings
        for (const auto &timeSlice: mergedTimeSlices) {
            finalBookings.emplace_back(timeSlice, cellId);
        }
    }

    // Sort final bookings by start time
    std::sort(finalBookings.begin(), finalBookings.end(),
              [](const auto &a, const auto &b) {
                  return a.timeSlice.start < b.timeSlice.start;
              });


    return finalBookings;
}


std::string ab::geoToH3(int h3Resolution, FPScalar latitude, FPScalar longitude) {
    const LatLng latLng{RADIANS(latitude), RADIANS(longitude)};
    H3Index out;
    latLngToCell(&latLng, h3Resolution, &out);
    char h3Str[20];
    h3ToString(out, h3Str, 20);
    return {h3Str};
}

std::string
ab::geoToH3D(int h3Resolution, int verticalResolution, FPScalar latitude, FPScalar longitude, FPScalar altitude) {
    // Lateral
    const LatLng latLng{RADIANS(latitude), RADIANS(longitude)};
    H3Index out = 0;
    latLngToCell(&latLng, h3Resolution, &out);
    char h3Str[20];
    h3ToString(out, h3Str, 20);
    auto h3String = std::string(h3Str);

    //Vertical
    const auto layer = altitude / verticalResolution;
    std::stringstream stream;
    stream << std::hex << static_cast<int>(layer);
    std::string hexString = stream.str();
    if (hexString.length() == 1) hexString = "0" + hexString;

    return h3String.substr(0, h3String.length() - 2) + hexString.substr(0, 2);
}

std::string ab::geoToS2(int s2Resolution, FPScalar latitude, FPScalar longitude) {
    const S2LatLng latLng = S2LatLng::FromDegrees(latitude, longitude);
    const S2CellId cellId(latLng);
    const S2CellId parentCellId = cellId.parent(s2Resolution);
    return parentCellId.ToToken();
}

std::string
ab::geoToS23D(int s2Resolution, int verticalResolution, FPScalar latitude, FPScalar longitude, FPScalar altitude) {
    // Lateral
    const S2LatLng latLng = S2LatLng::FromDegrees(latitude, longitude);
    const S2CellId cellId(latLng);
    const S2CellId parentCellId = cellId.parent(s2Resolution);
    auto lateralS2Index = parentCellId.ToToken();

    // Vertical
    const auto layer = altitude / verticalResolution;
    std::stringstream stream;
    stream << std::hex << static_cast<int>(layer);
    std::string hexString = stream.str();
    if (hexString.length() == 1) hexString = "0" + hexString;

    return lateralS2Index.substr(0, lateralS2Index.length() - 2) + hexString.substr(0, 2);
}