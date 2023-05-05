#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include "airspacebookingutils/library.h"

using namespace std::chrono;

class ConflictTests : public ::testing::Test {
protected:
    std::vector<ab::d4::StateVector4D> traj1{
            ab::d4::StateVector4D{ab::Position{-1.39200210, 50.90768760, 100.0}, ab::d4::TimeInstant::clock::now(),
                                  20.0},
            ab::d4::StateVector4D{ab::Position{-1.45465850, 50.93035940, 100.0}, ab::d4::TimeInstant{}, 20.0},
    };

    std::vector<ab::d4::StateVector4D> traj2{
            ab::d4::StateVector4D{ab::Position{-1.45448680, 50.90877000, 100.0}, ab::d4::TimeInstant::clock::now(),
                                  20.0},
            ab::d4::StateVector4D{ab::Position{-1.39260290, 50.92733150, 100.0}, ab::d4::TimeInstant{}, 20.0},
    };
};

TEST_F(ConflictTests, BasicXTest) {
    const auto cells1 = ab::getH3CellBookings(traj1);
    const auto cells2 = ab::getH3CellBookings(traj2);
}

