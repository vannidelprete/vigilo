/**
 * @file Imu.cpp
 * @brief MPU6050 IMU driver implementation.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include "Imu.h"
#include <MPU6050.h>
#include <Wire.h>

namespace vigilo {

    struct Imu::Impl {
        static constexpr uint8_t MOTION_BYTES = 14; ///< 6 accel + 2 temp + 6 gyro.

        Impl(uint8_t sda, uint8_t scl, uint8_t addr) : sda(sda), scl(scl), addr(addr) {}

        uint8_t sda;
        uint8_t scl;
        uint8_t addr;
        bool ready = false;
        MPU6050 mpu;
    };

    Imu::Imu(uint8_t sda, uint8_t scl, uint8_t addr) : _impl(std::make_unique<Impl>(sda, scl, addr)) {}

    Imu::~Imu() = default;

    InitResult Imu::begin() {
        Wire.begin(_impl->sda, _impl->scl);
        _impl->mpu.initialize();
        if (!_impl->mpu.testConnection()) return InitResult::DeviceNotFound;

        _impl->ready = true;

        return InitResult::Ok;
    }

    bool Imu::isReady() const noexcept {
        return _impl->ready;
    }

    Imu::ReadResult Imu::read(ImuData& out) noexcept {
        if (!_impl->ready) return ReadResult::NotReady;

        Wire.beginTransmission(_impl->addr);
        Wire.write(MPU6050_RA_ACCEL_XOUT_H);
        if (Wire.endTransmission(false) != 0) return ReadResult::BusError;
        if (Wire.requestFrom(_impl->addr, Impl::MOTION_BYTES) != Impl::MOTION_BYTES) return ReadResult::DataError;

        out.ax = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
        out.ay = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
        out.az = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
        Wire.read(); Wire.read(); // temperature register, not used
        out.gx = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
        out.gy = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
        out.gz = static_cast<int16_t>((Wire.read() << 8) | Wire.read());

        return ReadResult::Ok;
    }

} // namespace vigilo