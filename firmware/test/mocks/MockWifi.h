/**
 * @file MockWifi.h
 * @brief Shared gMock double for IWifi, used across firmware unit tests.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-14
 */

#pragma once
#include <gmock/gmock.h>
#include "hal/IWifi.h"

class MockWifi : public vigilo::IWifi {
public:
    MOCK_METHOD(void,   begin,  (const char*, const char*), (override));
    MOCK_METHOD(Status, status, (),                         (override));
};