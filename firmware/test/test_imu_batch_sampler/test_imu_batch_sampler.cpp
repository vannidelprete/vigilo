/**
 * @file test_imu_batch_sampler.cpp
 * @brief Unit tests for ImuBatchSampler.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-14
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "sensor/ImuBatchSampler.h"
#include "../mocks/MockWire.h"
#include "../mocks/MockMPU6050.h"
#include "../mocks/MockClock.h"

using namespace vigilo;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::AnyNumber;
using ::testing::_;

class ImuBatchSamplerTest : public ::testing::Test {
protected:
    static constexpr uint32_t SAMPLE_INTERVAL_US = 2000;

    MockWire        wire;
    MockMPU6050     mpu;
    MockClock       clock;
    Imu             imu{wire, mpu};
    ImuBatchSampler sampler{imu, clock, SAMPLE_INTERVAL_US};

    void SetUp() override {
        EXPECT_CALL(mpu, initialize());
        EXPECT_CALL(mpu, testConnection()).WillOnce(Return(true));
        ASSERT_EQ(imu.begin(), InitResult::Ok);

        ON_CALL(wire, endTransmission(false)).WillByDefault(Return(uint8_t{0}));
        ON_CALL(wire, requestFrom(_, _)).WillByDefault(Return(uint8_t{14}));
        ON_CALL(wire, read()).WillByDefault(Return(0));
    }
};

TEST_F(ImuBatchSamplerTest, CollectBatchReturnsFullBatchSizeOnSuccess) {
    EXPECT_CALL(clock, micros()).WillRepeatedly(Return(0));
    EXPECT_CALL(clock, delayMicros(_)).Times(AnyNumber());

    EXPECT_EQ(sampler.collectBatch(), ImuBatchSampler::BATCH_SIZE);
}

TEST_F(ImuBatchSamplerTest, CollectBatchStopsEarlyOnReadFailure) {
    EXPECT_CALL(wire, endTransmission(false)).WillOnce(Return(uint8_t{1}));
    EXPECT_CALL(clock, micros()).WillOnce(Return(0));
    EXPECT_CALL(clock, delayMicros(_)).Times(0);

    EXPECT_EQ(sampler.collectBatch(), 0u);
}

TEST_F(ImuBatchSamplerTest, CollectBatchSkipsDelayWhenReadIsSlow) {
    uint32_t t = 0;
    EXPECT_CALL(clock, micros()).WillRepeatedly(Invoke([&t]() {
        t += SAMPLE_INTERVAL_US * 2;
        return t;
    }));
    EXPECT_CALL(clock, delayMicros(_)).Times(0);

    EXPECT_EQ(sampler.collectBatch(), ImuBatchSampler::BATCH_SIZE);
}

TEST_F(ImuBatchSamplerTest, CollectBatchDelaysByExactRemainder) {
    EXPECT_CALL(clock, micros()).WillRepeatedly(Return(1000));
    EXPECT_CALL(clock, delayMicros(SAMPLE_INTERVAL_US)).Times(static_cast<int>(ImuBatchSampler::BATCH_SIZE));

    EXPECT_EQ(sampler.collectBatch(), ImuBatchSampler::BATCH_SIZE);
}

TEST_F(ImuBatchSamplerTest, DataPointsToCollectedSamples) {
    EXPECT_CALL(clock, micros()).WillRepeatedly(Return(0));
    EXPECT_CALL(clock, delayMicros(_)).Times(AnyNumber());

    ASSERT_EQ(sampler.collectBatch(), ImuBatchSampler::BATCH_SIZE);
    EXPECT_EQ(sampler.data()[0].ax, 0);
}