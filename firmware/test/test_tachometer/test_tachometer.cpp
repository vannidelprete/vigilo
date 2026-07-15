/**
 * @file test_tachometer.cpp
 * @brief Unit tests for the Tachometer driver.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "sensor/Tachometer.h"
#include "config.h"
#include "../mocks/MockClock.h"
#include "../mocks/MockGpio.h"

using namespace vigilo;
using ::testing::Return;
using ::testing::_;

class TachometerTest : public ::testing::Test {
protected:
    MockClock  clock;
    MockGpio   gpio;
    Tachometer tacho{config::PIN_TACHO, clock, gpio, config::RPM_INTERVAL_MS};
};

TEST_F(TachometerTest, BeginOkOnValidPin) {
    EXPECT_CALL(gpio, supportsInterrupt(config::PIN_TACHO)).WillOnce(Return(true));
    EXPECT_CALL(gpio, pinMode(config::PIN_TACHO, IGpio::PinMode::InputPullUp));
    EXPECT_CALL(gpio, attachInterrupt(config::PIN_TACHO, _, IGpio::Trigger::Falling));
    EXPECT_CALL(clock, millis()).WillOnce(Return(0));

    EXPECT_EQ(tacho.begin(), InitResult::Ok);
    EXPECT_TRUE(tacho.isReady());
}

TEST_F(TachometerTest, BeginFailsOnInvalidPin) {
    EXPECT_CALL(gpio, supportsInterrupt(config::PIN_TACHO)).WillOnce(Return(false));

    EXPECT_EQ(tacho.begin(), InitResult::InvalidPin);
    EXPECT_FALSE(tacho.isReady());
}

TEST_F(TachometerTest, GetRpmReturnsZeroWhenNotStarted) {
    EXPECT_FLOAT_EQ(0.0f, tacho.getRpm());
}

TEST_F(TachometerTest, GetRpmCorrectAfterPulses) {
    EXPECT_CALL(gpio, supportsInterrupt(_)).WillOnce(Return(true));
    EXPECT_CALL(gpio, pinMode(_, _));
    EXPECT_CALL(gpio, attachInterrupt(_, _, _));
    EXPECT_CALL(clock, millis())
        .WillOnce(Return(0))
        .WillOnce(Return(config::RPM_INTERVAL_MS));

    ASSERT_EQ(tacho.begin(), InitResult::Ok);
    for (int i = 0; i < 100; ++i) Tachometer::isr();

    // 100 pulses / 2 per revolution * 60 s/min = 3000 RPM
    EXPECT_FLOAT_EQ(3000.0f, tacho.getRpm());
}

TEST_F(TachometerTest, GetRpmReturnsCachedValueWithinInterval) {
    EXPECT_CALL(gpio, supportsInterrupt(_)).WillOnce(Return(true));
    EXPECT_CALL(gpio, pinMode(_, _));
    EXPECT_CALL(gpio, attachInterrupt(_, _, _));
    EXPECT_CALL(clock, millis())
        .WillOnce(Return(0))
        .WillOnce(Return(config::RPM_INTERVAL_MS))
        .WillOnce(Return(config::RPM_INTERVAL_MS + config::RPM_INTERVAL_MS / 2));

    ASSERT_EQ(tacho.begin(), InitResult::Ok);
    for (int i = 0; i < 100; ++i) Tachometer::isr();
    (void)tacho.getRpm();

    for (int i = 0; i < 10; ++i) Tachometer::isr();
    EXPECT_FLOAT_EQ(3000.0f, tacho.getRpm());
}

TEST_F(TachometerTest, GetRpmScalesCorrectlyWhenPolledLessFrequently) {
    EXPECT_CALL(gpio, supportsInterrupt(_)).WillOnce(Return(true));
    EXPECT_CALL(gpio, pinMode(_, _));
    EXPECT_CALL(gpio, attachInterrupt(_, _, _));
    EXPECT_CALL(clock, millis())
        .WillOnce(Return(0))
        .WillOnce(Return(1000))
        .WillOnce(Return(31000));

    ASSERT_EQ(tacho.begin(), InitResult::Ok);
    (void)tacho.getRpm();  // drains any pulse count left over by static state from a previous test

    for (int i = 0; i < 3000; ++i) Tachometer::isr();

    // 3000 pulses / 2 per revolution = 1500 revolutions in 30s = 3000 rev/min.
    EXPECT_FLOAT_EQ(3000.0f, tacho.getRpm());
}