/**
 * @file MockGpio.h
 * @brief Shared gMock double for IGpio, used across firmware unit tests.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-14
 */

#pragma once
#include <gmock/gmock.h>
#include "hal/IGpio.h"

class MockGpio : public vigilo::IGpio {
public:
    MOCK_METHOD(bool, supportsInterrupt, (uint8_t),                      (const, noexcept, override));
    MOCK_METHOD(void, pinMode,           (uint8_t, PinMode),             (noexcept, override));
    MOCK_METHOD(void, attachInterrupt,   (uint8_t, void(*)(), Trigger),  (noexcept, override));
};