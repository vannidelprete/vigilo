/**
 * @file MockClock.h
 * @brief Shared gMock double for IClock, used across firmware unit tests.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-14
 */

#pragma once
#include <gmock/gmock.h>
#include "hal/IClock.h"

class MockClock : public vigilo::IClock {
public:
    MOCK_METHOD(uint32_t, millis,            (),         (const, noexcept, override));
    MOCK_METHOD(void,     delayMillis,       (uint32_t), (noexcept, override));
    MOCK_METHOD(uint32_t, micros,            (),         (const, noexcept, override));
    MOCK_METHOD(void,     delayMicros,       (uint32_t), (noexcept, override));
};