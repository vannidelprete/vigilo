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

class MqttPublisherTest : public Test {
protected:
    static constexpr const char* BROKER    = "192.168.1.13";
    static constexpr uint16_t    PORT      = 1883;
    static constexpr const char* DEVICE_ID = "vigilo-01";

    MockMqtt      mqtt;
    MqttPublisher publisher{BROKER, PORT, DEVICE_ID, mqtt};
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

TEST_F(MqttPublisherTest, PublishSendsCorrectTopic) {
    EXPECT_CALL(mqtt, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mqtt, publish(StrEq("vigilo/vigilo-01/telemetry"), _))
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