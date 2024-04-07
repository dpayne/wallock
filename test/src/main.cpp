#include "conf/Config.hpp"
#include "gtest/gtest.h"

auto main(int argc, char** argv) -> int {
    testing::InitGoogleTest(&argc, argv);

    wall::Config config{argc, (const char**)argv};
    wall::Log::setup_debug_logger(config);
    wall::Log::set_log_level("debug");

    auto retval = RUN_ALL_TESTS();

    return retval;
}
