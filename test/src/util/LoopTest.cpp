
#include "gtest/gtest.h"
#include "util/Loop.hpp"

TEST(LoopTest, test_pipe) {
    wall::Loop loop;

    int32_t expect_write = 42;
    auto* poll_pipe = loop.add_poll_pipe([&expect_write](wall::loop::PollPipe*, const std::vector<uint8_t>& buffer) {
        EXPECT_EQ(buffer.size(), 1);
        EXPECT_EQ(buffer[0], expect_write);
    });

    poll_pipe->write_one(42);

    loop.run();

    expect_write = 10;
    poll_pipe->write_one(expect_write);

    loop.run();
}
