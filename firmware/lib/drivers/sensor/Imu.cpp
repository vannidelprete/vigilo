/**
 * @file Imu.cpp
 * @brief MPU6050 IMU driver implementation.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include "Imu.h"

namespace vigilo {

    Imu::Imu(IWire& wire, IMPU6050& mpu, uint8_t addr) : _wire(wire), _mpu(mpu), _addr(addr) {}

    InitResult Imu::begin() {
        _mpu.initialize();
        if (!_mpu.testConnection()) return InitResult::DeviceNotFound;
        _ready = true;
        return InitResult::Ok;
    }

    bool Imu::isReady() const noexcept {
        return _ready;
    }

    Imu::ReadResult Imu::read(ImuData& out) noexcept {
        if (!_ready) return ReadResult::NotReady;

        _wire.beginTransmission(_addr);
        _wire.write(0x3B); // MPU6050_RA_ACCEL_XOUT_H
        if (_wire.endTransmission(false) != 0) return ReadResult::BusError;
        if (_wire.requestFrom(_addr, uint8_t{14}) != 14) return ReadResult::DataError;

        out.ax = static_cast<int16_t>((_wire.read() << 8) | _wire.read());
        out.ay = static_cast<int16_t>((_wire.read() << 8) | _wire.read());
        out.az = static_cast<int16_t>((_wire.read() << 8) | _wire.read());
        _wire.read(); _wire.read(); // temperature register, not used
        out.gx = static_cast<int16_t>((_wire.read() << 8) | _wire.read());
        out.gy = static_cast<int16_t>((_wire.read() << 8) | _wire.read());
        out.gz = static_cast<int16_t>((_wire.read() << 8) | _wire.read());

        return ReadResult::Ok;
    }

} // namespace vigilo