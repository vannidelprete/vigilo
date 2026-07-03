/**
 * @file Imu.h
 * @brief Driver for the MPU6050 6-axis IMU over I2C.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include "ISensor.h"
#include <cstdint>
#include <memory>

namespace vigilo {

    /**
     * @brief Raw motion data from the MPU6050.
     * 
     * Accelerometer values are in raw ADC units (range depends on full-scale setting).
     * Gyroscope values are in raw ADC units.
     */
    struct ImuData {
        int16_t ax; ///< Accelerometer X axis
        int16_t ay; ///< Accelerometer Y axis
        int16_t az; ///< Accelerometer Z axis
        int16_t gx; ///< Gyroscope X axis
        int16_t gy; ///< Gyroscope Y axis
        int16_t gz; ///< Gyroscope Z axis
    };

    /**
     * @brief Driver for the MPU6050 6-axis IMU over I2C.
     */
    class Imu : public ISensor {
    public:
        /**
         * @brief Result code returned by read().
         *
         * Distinguishes the failure cause so the caller can react appropriately.
         */
        enum class ReadResult : uint8_t {
            Ok        = 0, ///< Data successfully read and written to out.
            NotReady  = 1, ///< Sensor not initialized - call begin() first.
            BusError  = 2, ///< I2C transmission failed (NACK or arbitration loss).
            DataError = 3, ///< Received fewer bytes than expected - bus may be unstable.
        };
        
        /**
         * @brief Constructs the IMU driver with the given I2C pins.
         * @param sda GPIO pin for I2C data.
         * @param scl GPIO pin for I2C clock.
         * @param addr I2C device address (default 0x68, AD0 pin low). 
         */
        explicit Imu(uint8_t sda, uint8_t scl, uint8_t addr);

        /** @brief Destructor. Declared here to allow forward declaration of Impl. */
        ~Imu() override;

        Imu(const Imu&)                 = delete; // Non-copyable: each instance owns a unique hardware resource.
        Imu& operator=(const Imu&)      = delete; // Non-copyable: each instance owns a unique hardware resource.

        Imu(Imu&&) noexcept             = default; // Move constructor: transfers Pimpl ownership to the new instance.
        Imu& operator=(Imu&&) noexcept  = default; // Move constructor: transfers Pimpl ownership, leaving source in a valid empty state.

        /** @copydoc ISensor::begin() */
        [[nodiscard]] InitResult begin() override;
        
        /** @copydoc ISensor::isReady() */
        [[nodiscard]] bool isReady() const noexcept override;

        /**
         * @brief Reads the latest accelerometer and gyroscope values via I2C.
         * @param[out] out Populated with sensor data only when ReadResult::Ok is returned.
         * @return ReadResult::Ok on success, or a specific error code identifying the failure.
         */
        [[nodiscard]] ReadResult read(ImuData& out) noexcept;

    private:
        struct Impl;                    ///< Forward declaration of the implementation struct (Pimpl idiom).
        std::unique_ptr<Impl> _impl;    ///< Pointer to the opaque implementation, allocated at construction.
    };

} // namespace vigilo