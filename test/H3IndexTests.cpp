#include <gtest/gtest.h>
#include "airspacebookingutils/library.h"
#include <h3/h3api.h>

TEST(H3IndexTests, GeoToH3Tests) {
    ASSERT_EQ("8919591565bffff", ab::geoToH3(9, 50.90768760, -1.39200210));
}

TEST(H3IndexTests, H3ToGeoTests) {
    LatLng latLng;
    H3Index out;
    stringToH3("8919591565bffff", &out);
    cellToLatLng(out, &latLng);
    ASSERT_NEAR(50.90768760, latLng.lat * 180 / M_PI, 1e-2);
    ASSERT_NEAR(-1.39200210, latLng.lng * 180 / M_PI, 1e-2);
}