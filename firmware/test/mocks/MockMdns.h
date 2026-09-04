/**
 * @file MockMdns.h
 * @brief Shared gMock double for IMdns, used across firmware unit tests.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-09-02
 */

#pragma once
#include <gmock/gmock.h>
#include "hal/IMdns.h"

class MockMdns : public vigilo::IMdns {
public:
    MOCK_METHOD(bool,        begin,        (const char*), (override));
    MOCK_METHOD(QueryResult, queryService, (const char*, const char*, char*, std::size_t), (override));
};