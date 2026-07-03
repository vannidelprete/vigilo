/**
 * @file test_imu.cpp
 * @brief Unit tests for the Imu sensor driver.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "sensor/Imu.h"

using namespace vigilo;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::_;

class MockWire : public IWire {
public:
    MOCK_METHOD(void,    beginTransmission, (uint8_t),           (override));
    MOCK_METHOD(void,    write,             (uint8_t),           (override));
    MOCK_METHOD(uint8_t, endTransmission,   (bool),              (override));
    MOCK_METHOD(uint8_t, requestFrom,       (uint8_t, uint8_t),  (override));
    MOCK_METHOD(int,     read,              (),                  (override));
};

class MockMPU6050 : public IMPU6050 {
public:
    MOCK_METHOD(void, initialize,     (), (override));
    MOCK_METHOD(bool, testConnection, (), (override));
};

class ImuTest : public ::testing::Test {
protected:
    MockWire    wire;
    MockMPU6050 mpu;
    Imu         imu{wire, mpu}; // default addr 0x68
};

TEST_F(ImuTest, BeginOk) {
    EXPECT_CALL(mpu, initialize());
    EXPECT_CALL(mpu, testConnection()).WillOnce(Return(true));

    EXPECT_EQ(imu.begin(), InitResult::Ok);
    EXPECT_TRUE(imu.isReady());
}

TEST_F(ImuTest, BeginDeviceNotFound) {
    EXPECT_CALL(mpu, initialize());
    EXPECT_CALL(mpu, testConnection()).WillOnce(Return(false));

    EXPECT_EQ(imu.begin(), InitResult::DeviceNotFound);
    EXPECT_FALSE(imu.isReady());
}

TEST_F(ImuTest, ReadNotReady) {
    ImuData data{};
    EXPECT_EQ(imu.read(data), Imu::ReadResult::NotReady);
}

TEST_F(ImuTest, ReadBusError) {
    EXPECT_CALL(mpu, initialize());
    EXPECT_CALL(mpu, testConnection()).WillOnce(Return(true));
    ASSERT_EQ(imu.begin(),   InitResult::Ok);

    EXPECT_CALL(wire, beginTransmission(uint8_t{0x68}));
    EXPECT_CALL(wire, write(uint8_t{0x3B}));
    EXPECT_CALL(wire, endTransmission(false)).WillOnce(Return(uint8_t{1}));

    ImuData data{};
    EXPECT_EQ(imu.read(data), Imu::ReadResult::BusError);
}

TEST_F(ImuTest, ReadDataError) {
    EXPECT_CALL(mpu, initialize());
    EXPECT_CALL(mpu, testConnection()).WillOnce(Return(true));
    ASSERT_EQ(imu.begin(),   InitResult::Ok);

    EXPECT_CALL(wire, beginTransmission(uint8_t{0x68}));
    EXPECT_CALL(wire, write(uint8_t{0x3B}));
    EXPECT_CALL(wire, endTransmission(false)).WillOnce(Return(uint8_t{0}));
    EXPECT_CALL(wire, requestFrom(uint8_t{0x68}, uint8_t{14})).WillOnce(Return(uint8_t{10}));

    ImuData data{};
    EXPECT_EQ(imu.read(data), Imu::ReadResult::DataError);
}

TEST_F(ImuTest, ReadPopulatesData) {
    EXPECT_CALL(mpu, initialize());
    EXPECT_CALL(mpu, testConnection()).WillOnce(Return(true));
    ASSERT_EQ(imu.begin(),   InitResult::Ok);

    InSequence seq;
    EXPECT_CALL(wire, beginTransmission(uint8_t{0x68}));
    EXPECT_CALL(wire, write(uint8_t{0x3B}));
    EXPECT_CALL(wire, endTransmission(false)).WillOnce(Return(uint8_t{0}));
    EXPECT_CALL(wire, requestFrom(uint8_t{0x68}, uint8_t{14})).WillOnce(Return(uint8_t{14}));
    // ax = 100  (0x0064)
    EXPECT_CALL(wire, read()).WillOnce(Return(0x00));
    EXPECT_CALL(wire, read()).WillOnce(Return(0x64));
    // ay = 200  (0x00C8)
    EXPECT_CALL(wire, read()).WillOnce(Return(0x00));
    EXPECT_CALL(wire, read()).WillOnce(Return(0xC8));
    // az = 300  (0x012C)
    EXPECT_CALL(wire, read()).WillOnce(Return(0x01));
    EXPECT_CALL(wire, read()).WillOnce(Return(0x2C));
    // temperature - discarded
    EXPECT_CALL(wire, read()).WillOnce(Return(0x00));
    EXPECT_CALL(wire, read()).WillOnce(Return(0x00));
    // gx = 500  (0x01F4)
    EXPECT_CALL(wire, read()).WillOnce(Return(0x01));
    EXPECT_CALL(wire, read()).WillOnce(Return(0xF4));
    // gy = -100 (0xFF9C)
    EXPECT_CALL(wire, read()).WillOnce(Return(0xFF));
    EXPECT_CALL(wire, read()).WillOnce(Return(0x9C));
    // gz = 0
    EXPECT_CALL(wire, read()).WillOnce(Return(0x00));
    EXPECT_CALL(wire, read()).WillOnce(Return(0x00));

    ImuData data{};
    EXPECT_EQ(imu.read(data), Imu::ReadResult::Ok);
    EXPECT_EQ(data.ax,   100);
    EXPECT_EQ(data.ay,   200);
    EXPECT_EQ(data.az,   300);
    EXPECT_EQ(data.gx,   500);
    EXPECT_EQ(data.gy,  -100);
    EXPECT_EQ(data.gz,     0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}