#ifndef AB_4DUTILS_H
#define AB_4DUTILS_H

#include <chrono>
#include "GeometryProjectionUtils.h"
#include <cmath>
#include "../library.h"

#define RAD(x) x/180 * M_PI

namespace ab {
    namespace d4 {


        template<typename T>
        static T euclideanDistance3D(const ab::Position &p0, const ab::Position &p1) {
            return std::sqrt((p0 - p1).array().square().sum());
        }

        static TimeInstant projectNextETA(const StateVector4D &currentState, const Position &nextPosition,
                                          PJ *reproj) {
            const auto rcp = util::reprojectCoordinate_r(reproj, currentState.position[1],
                                                         currentState.position[0],
                                                         currentState.position[2]);
            const Position reprojCurrPosition(rcp.xyz.x, rcp.xyz.y, rcp.xyz.z);

            const auto rnp = util::reprojectCoordinate_r(reproj, nextPosition[1],
                                                         nextPosition[0],
                                                         nextPosition[2]);
            const Position reprojNextPosition(rnp.xyz.x, rnp.xyz.y, rnp.xyz.z);

            const auto dist = euclideanDistance3D<double>(reprojCurrPosition, reprojNextPosition);


            int nextSegmentDuration = std::round(dist / currentState.speed);
            if (nextSegmentDuration < 1) {
                nextSegmentDuration = 1;
            }
            return currentState.time + std::chrono::seconds(nextSegmentDuration);
        }
    }
}
#endif // AB_4DUTILS_H
