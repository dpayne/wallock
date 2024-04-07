#include <wallock/version.h>
#include <wallock/Wallock.hpp>
#include "gtest/gtest.h"

#include <string>

TEST(WallockTest, Version) {
    static_assert(std::string_view(WALLOCK_LIB_VERSION) == std::string_view("1.0"));
    ASSERT_EQ(std::string(WALLOCK_LIB_VERSION), std::string("1.0"));
}
