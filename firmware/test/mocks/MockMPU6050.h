/**
 * @file MockMPU6050.h
 * @brief Shared gMock double for IMPU6050, used across firmware unit tests.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-14
 */

#pragma once
#include <gmock/gmock.h>
#include "hal/IMPU6050.h"

class MockMPU6050 : public vigilo::IMPU6050 {
public:
    MOCK_METHOD(void, initialize,     (), (override));
    MOCK_METHOD(bool, testConnection, (), (override));
};