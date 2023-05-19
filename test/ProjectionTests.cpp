#include <gtest/gtest.h>
#include <proj.h>
#include "airspacebookingutils/util/GeometryProjectionUtils.h"
#include "airspacebookingutils/library.h"

const std::vector<ab::Position> test_positions{
        {-90.8941761, 14.4516739, 10},
        {-90.8941761, 14.4516739, 100},
        {-90.8941761, 14.4516739, 1000},
        {-90.8941761, 14.4516739, 10000},
        {-1.391015,   50.905473,  10},
        {-1.391015,   50.905473,  100},
        {-1.391015,   50.905473,  1000},
        {-1.391015,   50.905473,  10000},
        {36.931524,   -17.854491, 10},
        {36.931524,   -17.854491, 100},
        {36.931524,   -17.854491, 1000},
        {36.931524,   -17.854491, 10000},
};

TEST(ProjectionTests, TestEckertVIProjection) {
    const auto pj = ab::util::makeProjObject("EPSG:4326", "ESRI:54010");
    PJ *reproj = std::get<0>(pj);
    const auto revpj = ab::util::makeProjObject("ESRI:54010", "EPSG:4326");
    PJ *revReproj = std::get<0>(revpj);
    for (const auto &pos: test_positions) {
        const auto projected = ab::util::reprojectCoordinate_r(reproj, pos.y(), pos.x(), pos.z());
        std::cout << "Position: (" << pos.x() << ", " << pos.y() << ", " << pos.z() << ")" << "\t Projected: ("
                  << projected.xyz.x << ", " << projected.xyz.y << ", " << projected.xyz.z << ")" << std::endl;
        const auto unprojected = ab::util::reprojectCoordinate_r(revReproj, projected.xyz.x, projected.xyz.y,
                                                                 projected.xyz.z);
        EXPECT_NEAR(pos.x(), unprojected.xyz.y, 0.0001);
        EXPECT_NEAR(pos.y(), unprojected.xyz.x, 0.0001);
        EXPECT_NEAR(pos.z(), unprojected.xyz.z, 0.1);
    }
}
