/**
 * @file MockWire.h
 * @brief Shared gMock double for IWire, used across firmware unit tests.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-14
 */

#pragma once
#include <gmock/gmock.h>
#include "hal/IWire.h"

class MockWire : public vigilo::IWire {
public:
    MOCK_METHOD(void,    beginTransmission, (uint8_t),          (override));
    MOCK_METHOD(void,    write,             (uint8_t),          (override));
    MOCK_METHOD(uint8_t, endTransmission,   (bool),             (override));
    MOCK_METHOD(uint8_t, requestFrom,       (uint8_t, uint8_t), (override));
    MOCK_METHOD(int,     read,              (),                 (override));
};