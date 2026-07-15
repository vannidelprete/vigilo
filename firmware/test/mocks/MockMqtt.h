/**
 * @file MockMqtt.h
 * @brief Shared gMock double for IMqtt, used across firmware unit tests.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-14
 */

#pragma once
#include <gmock/gmock.h>
#include "hal/IMqtt.h"

class MockMqtt : public vigilo::IMqtt {
public:
    MOCK_METHOD(bool, connect,       (const char*, const char*, uint16_t, const char*, const char*), (override));
    MOCK_METHOD(bool, publish,       (const char*, const char*, bool),      (override));
    MOCK_METHOD(bool, publishBinary, (const char*, const uint8_t*, size_t), (override));
    MOCK_METHOD(bool, isConnected,   (),                                    (const, noexcept, override));
    MOCK_METHOD(void, loop,          (),                                    (noexcept, override));
};