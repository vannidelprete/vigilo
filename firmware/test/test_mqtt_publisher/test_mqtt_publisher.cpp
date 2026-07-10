/**
 * @file test_mqtt_publisher.cpp
 * @brief Unit tests for MqttPublisher.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-10
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "network/MqttPublisher.h"

using namespace vigilo;
using namespace testing;

class MockMqtt : public IMqtt {
public:
    MOCK_METHOD(bool, connect,     (const char*, const char*, uint16_t), (override));
    MOCK_METHOD(bool, publish,     (const char*, const char*),           (override));
    MOCK_METHOD(bool, isConnected, (),                                   (const, noexcept, override));
    MOCK_METHOD(void, loop,        (),                                   (noexcept, override));
};

class MockClock : public IClock {
public:
    MOCK_METHOD(uint32_t, millis, (), (const, noexcept, override));
    MOCK_METHOD(void,     delay,  (uint32_t), (noexcept, override));
};

class MqttPublisherTest : public Test {
protected:
    static constexpr const char* BROKER               = "192.168.1.13";
    static constexpr uint16_t    PORT                 = 1883;
    static constexpr const char* DEVICE_ID             = "vigilo-01";
    static constexpr const char* EXPECTED_TOPIC        = "vigilo/vigilo-01/telemetry";
    static constexpr uint32_t    RECONNECT_INTERVAL_MS = 5000;

    MockMqtt      mqtt;
    MockClock     clock;
    MqttPublisher publisher{BROKER, PORT, DEVICE_ID, mqtt, clock, RECONNECT_INTERVAL_MS};

    void SetUp() override {
        ON_CALL(clock, millis()).WillByDefault(Return(0));
        ON_CALL(mqtt, isConnected()).WillByDefault(Return(false));
    }
};

TEST_F(MqttPublisherTest, ConnectPassesCredentialsToMqtt) {
    EXPECT_CALL(mqtt, connect(StrEq(DEVICE_ID), StrEq(BROKER), PORT))
        .WillOnce(Return(true));
    ASSERT_TRUE(publisher.connect());
}

TEST_F(MqttPublisherTest, ConnectReturnsFalseOnFailure) {
    EXPECT_CALL(mqtt, connect(_, _, _)).WillOnce(Return(false));
    ASSERT_FALSE(publisher.connect());
}

TEST_F(MqttPublisherTest, ConnectReturnsTrueWhenAlreadyConnected) {
    EXPECT_CALL(mqtt, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mqtt, connect(_, _, _)).Times(0);
    ASSERT_TRUE(publisher.connect());
}

TEST_F(MqttPublisherTest, ConnectThrottlesSecondAttemptWithinInterval) {
    EXPECT_CALL(clock, millis())
        .WillOnce(Return(0))
        .WillOnce(Return(RECONNECT_INTERVAL_MS - 1));
    EXPECT_CALL(mqtt, connect(_, _, _)).Times(1).WillOnce(Return(true));

    ASSERT_TRUE(publisher.connect());
    ASSERT_FALSE(publisher.connect());
}

TEST_F(MqttPublisherTest, ConnectAttemptsAgainAfterIntervalElapsed) {
    EXPECT_CALL(clock, millis())
        .WillOnce(Return(0))
        .WillOnce(Return(RECONNECT_INTERVAL_MS));
    EXPECT_CALL(mqtt, connect(_, _, _)).Times(2).WillRepeatedly(Return(true));

    ASSERT_TRUE(publisher.connect());
    ASSERT_TRUE(publisher.connect());
}

TEST_F(MqttPublisherTest, PublishSendsCorrectTopic) {
    EXPECT_CALL(mqtt, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mqtt, publish(StrEq(EXPECTED_TOPIC), _))
        .WillOnce(Return(true));
    ImuData data{0, 0, 0, 0, 0, 0};
    ASSERT_TRUE(publisher.publish(data, 0.0f));
}

TEST_F(MqttPublisherTest, PublishFormatsPayloadCorrectly) {
    EXPECT_CALL(mqtt, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mqtt, publish(_,
        StrEq("{\"ax\":100,\"ay\":-50,\"az\":16384,\"gx\":10,\"gy\":-20,\"gz\":30,\"rpm\":1200.5}")))
        .WillOnce(Return(true));
    ImuData data{100, -50, 16384, 10, -20, 30};
    ASSERT_TRUE(publisher.publish(data, 1200.5f));
}

TEST_F(MqttPublisherTest, PublishSkipsWhenNotConnected) {
    EXPECT_CALL(mqtt, isConnected()).WillOnce(Return(false));
    EXPECT_CALL(mqtt, publish(_, _)).Times(0);
    ImuData data{0, 0, 0, 0, 0, 0};
    ASSERT_FALSE(publisher.publish(data, 0.0f));
}

TEST_F(MqttPublisherTest, LoopDelegatesToMqtt) {
    EXPECT_CALL(mqtt, loop()).Times(1);
    publisher.loop();
}