#pragma once

#include <gmock/gmock.h>
#include "display/Display.hpp"
#include "util/CommandProcessor.hpp"
#include "util/Loop.hpp"
#include "util/SignalHandler.hpp"
#include "wallock/Wallock.hpp"

class WallockMock : public wall::Wallock {
   public:
    WallockMock(wall::Config* config, wall::Loop* loop) : wall::Wallock(config, loop) {}
    ~WallockMock() override = default;

    MOCK_METHOD(void, run, (bool), (override));
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, stop, (), (override));
};
