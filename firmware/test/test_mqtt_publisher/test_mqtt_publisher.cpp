/**
 * @file test_mqtt_publisher.cpp
 * @brief Unit tests for MqttPublisher.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-10
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "network/MqttPublisher.h"
#include "sensor/ImuBatchSampler.h"
#include "../mocks/MockMqtt.h"
#include "../mocks/MockClock.h"
#include <vector>
#include <cstring>

using namespace vigilo;
using namespace testing;

class MqttPublisherTest : public Test {
protected:
    static constexpr const char* BROKER               = "192.168.1.13";
    static constexpr uint16_t    PORT                 = 1883;
    static constexpr const char* DEVICE_ID             = "vigilo-01";
    static constexpr const char* EXPECTED_STATUS_TOPIC = "vigilo/vigilo-01/status";
    static constexpr const char* EXPECTED_BATCH_TOPIC  = "vigilo/vigilo-01/telemetry/batch";
    static constexpr uint32_t    RECONNECT_INTERVAL_MS = 5000;

    MockMqtt      mqtt;
    MockClock     clock;
    MqttPublisher publisher{BROKER, PORT, DEVICE_ID, mqtt, clock, RECONNECT_INTERVAL_MS};

    void SetUp() override {
        ON_CALL(clock, millis()).WillByDefault(Return(0));
        ON_CALL(mqtt, isConnected()).WillByDefault(Return(false));
        ON_CALL(mqtt, connect(_, _, _, _, _)).WillByDefault(Return(true));
        ON_CALL(mqtt, publish(_, _, _)).WillByDefault(Return(true));
    }
};

TEST_F(MqttPublisherTest, ConnectPassesCredentialsAndWillToMqtt) {
    EXPECT_CALL(mqtt, connect(StrEq(DEVICE_ID), StrEq(BROKER), PORT,
                               StrEq(EXPECTED_STATUS_TOPIC), StrEq("offline")))
        .WillOnce(Return(true));
    ASSERT_TRUE(publisher.connect());
}

TEST_F(MqttPublisherTest, ConnectPublishesRetainedOnlineStatusOnSuccess) {
    EXPECT_CALL(mqtt, connect(_, _, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(mqtt, publish(StrEq(EXPECTED_STATUS_TOPIC), StrEq("online"), true))
        .WillOnce(Return(true));
    ASSERT_TRUE(publisher.connect());
}

TEST_F(MqttPublisherTest, ConnectReturnsFalseOnFailure) {
    EXPECT_CALL(mqtt, connect(_, _, _, _, _)).WillOnce(Return(false));
    EXPECT_CALL(mqtt, publish(_, _, _)).Times(0);
    ASSERT_FALSE(publisher.connect());
}

TEST_F(MqttPublisherTest, ConnectReturnsTrueWhenAlreadyConnected) {
    EXPECT_CALL(mqtt, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mqtt, connect(_, _, _, _, _)).Times(0);
    ASSERT_TRUE(publisher.connect());
}

TEST_F(MqttPublisherTest, ConnectThrottlesSecondAttemptWithinInterval) {
    EXPECT_CALL(clock, millis())
        .WillOnce(Return(0))
        .WillOnce(Return(RECONNECT_INTERVAL_MS - 1));
    EXPECT_CALL(mqtt, connect(_, _, _, _, _)).Times(1).WillOnce(Return(true));

    ASSERT_TRUE(publisher.connect());
    ASSERT_FALSE(publisher.connect());
}

TEST_F(MqttPublisherTest, ConnectAttemptsAgainAfterIntervalElapsed) {
    EXPECT_CALL(clock, millis())
        .WillOnce(Return(0))
        .WillOnce(Return(RECONNECT_INTERVAL_MS));
    EXPECT_CALL(mqtt, connect(_, _, _, _, _)).Times(2).WillRepeatedly(Return(true));

    ASSERT_TRUE(publisher.connect());
    ASSERT_TRUE(publisher.connect());
}

TEST_F(MqttPublisherTest, PublishBatchSkipsWhenNotConnected) {
    EXPECT_CALL(mqtt, isConnected()).WillOnce(Return(false));
    EXPECT_CALL(mqtt, publishBinary(_, _, _)).Times(0);

    const ImuData samples[1] = {{0, 0, 0, 0, 0, 0}};
    ASSERT_FALSE(publisher.publishBatch(samples, 1, 2000, 0.0f));
}

TEST_F(MqttPublisherTest, PublishBatchPublishesToCorrectTopic) {
    EXPECT_CALL(mqtt, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mqtt, publishBinary(StrEq(EXPECTED_BATCH_TOPIC), _, _)).WillOnce(Return(true));

    const ImuData samples[1] = {{0, 0, 0, 0, 0, 0}};
    ASSERT_TRUE(publisher.publishBatch(samples, 1, 2000, 0.0f));
}

TEST_F(MqttPublisherTest, PublishBatchReturnsFalseWhenBatchTooLarge) {
    EXPECT_CALL(mqtt, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(mqtt, publishBinary(_, _, _)).Times(0);

    const ImuData samples[1] = {{0, 0, 0, 0, 0, 0}};
    const std::size_t tooMany = ImuBatchSampler::BATCH_SIZE + 1;
    ASSERT_FALSE(publisher.publishBatch(samples, tooMany, 2000, 0.0f));
}

TEST_F(MqttPublisherTest, PublishBatchSerializesHeaderAndSamplesCorrectly) {
    EXPECT_CALL(mqtt, isConnected()).WillOnce(Return(true));

    std::vector<uint8_t> captured;
    EXPECT_CALL(mqtt, publishBinary(StrEq(EXPECTED_BATCH_TOPIC), _, _))
        .WillOnce(Invoke([&captured](const char*, const uint8_t* payload, std::size_t length) {
            captured.assign(payload, payload + length);
            return true;
        }));

    const ImuData samples[2] = {
        {100, -50, 16384, 10, -20, 30},
        {200, -60, 16000, 20, -30, 40},
    };
    ASSERT_TRUE(publisher.publishBatch(samples, 2, 2000, 1234.5f));

    ASSERT_EQ(captured.size(), sizeof(BatchHeader) + 2 * sizeof(ImuData));

    BatchHeader header;
    std::memcpy(&header, captured.data(), sizeof(BatchHeader));
    EXPECT_EQ(header.sampleIntervalUs, 2000u);
    EXPECT_EQ(header.sampleCount, 2u);
    EXPECT_FLOAT_EQ(1234.5f, header.rpm);

    ImuData decoded[2];
    std::memcpy(decoded, captured.data() + sizeof(BatchHeader), 2 * sizeof(ImuData));
    EXPECT_EQ(decoded[0].ax, 100);
    EXPECT_EQ(decoded[0].gz, 30);
    EXPECT_EQ(decoded[1].ax, 200);
    EXPECT_EQ(decoded[1].gz, 40);
}

TEST_F(MqttPublisherTest, LoopDelegatesToMqtt) {
    EXPECT_CALL(mqtt, loop()).Times(1);
    publisher.loop();
}