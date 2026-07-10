/**
 * @file test_wifi_connector.cpp
 * @brief Unit tests for the WifiConnector driver.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "network/WifiConnector.h"

using namespace vigilo;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::_;

class MockWifi : public IWifi {
public:
    MOCK_METHOD(void,   begin,  (const char*, const char*), (override));
    MOCK_METHOD(Status, status, (),                         (override));
};

class MockClock : public IClock {
public:
    MOCK_METHOD(uint32_t, millis, (), (const, noexcept, override));
    MOCK_METHOD(void,     delay,  (uint32_t), (noexcept, override));
};

class WifiConnectorTest : public ::testing::Test {
protected:
    MockWifi  wifi;
    MockClock clock;
    WifiConnector connector{"test-ssid", "test-pass", wifi, clock};
};

TEST_F(WifiConnectorTest, ConnectOk) {
    EXPECT_CALL(wifi, begin(StrEq("test-ssid"), StrEq("test-pass")));
    EXPECT_CALL(wifi, status()).WillRepeatedly(Return(IWifi::Status::Connected));
    EXPECT_CALL(clock, delay(_)).Times(0);

    EXPECT_EQ(connector.connect(), ConnectResult::Ok);
}

TEST_F(WifiConnectorTest, ConnectOkAfterRetries) {
    EXPECT_CALL(wifi, begin(_, _));
    EXPECT_CALL(wifi, status())
        .WillOnce(Return(IWifi::Status::Disconnected))
        .WillOnce(Return(IWifi::Status::Disconnected))
        .WillRepeatedly(Return(IWifi::Status::Connected));
    EXPECT_CALL(clock, delay(500)).Times(2);

    EXPECT_EQ(connector.connect(), ConnectResult::Ok);
}

TEST_F(WifiConnectorTest, ConnectNetworkNotFound) {
    EXPECT_CALL(wifi, begin(_, _));
    EXPECT_CALL(wifi, status()).WillRepeatedly(Return(IWifi::Status::NoSsid));
    EXPECT_CALL(clock, delay(500)).Times(20);

    EXPECT_EQ(connector.connect(), ConnectResult::NetworkNotFound);
}

TEST_F(WifiConnectorTest, ConnectAuthFailed) {
    EXPECT_CALL(wifi, begin(_, _));
    EXPECT_CALL(wifi, status()).WillRepeatedly(Return(IWifi::Status::ConnectFailed));
    EXPECT_CALL(clock, delay(500)).Times(20);

    EXPECT_EQ(connector.connect(), ConnectResult::AuthFailed);
}

TEST_F(WifiConnectorTest, ConnectTimeout) {
    EXPECT_CALL(wifi, begin(_, _));
    EXPECT_CALL(wifi, status()).WillRepeatedly(Return(IWifi::Status::Disconnected));
    EXPECT_CALL(clock, delay(500)).Times(20);

    EXPECT_EQ(connector.connect(), ConnectResult::Timeout);
}

TEST_F(WifiConnectorTest, IsConnectedTrue) {
    EXPECT_CALL(wifi, status()).WillOnce(Return(IWifi::Status::Connected));
    EXPECT_TRUE(connector.isConnected());
}

TEST_F(WifiConnectorTest, IsConnectedFalse) {
    EXPECT_CALL(wifi, status()).WillOnce(Return(IWifi::Status::Disconnected));
    EXPECT_FALSE(connector.isConnected());
}