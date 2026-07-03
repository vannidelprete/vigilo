/**
 * @file IMPU6050.h
 * @brief Abstract interface for MPU6050 chip operations.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once

namespace vigilo {

    /**
     * @brief Abstract interface for MPU6050 chip operations.
     */
    class IMPU6050 {
    public:
        /** @brief Initializes the MPU6050 registers to their default configuration. */
        virtual void initialize() = 0;

        /**
         * @brief Tests whether the chip responds on the I2C bus.
         * @return true if the device acknowledged the connection.
         */
        virtual bool testConnection() = 0;

        /** @brief Virtual destructor. */
        virtual ~IMPU6050() = default;

    protected:
        IMPU6050() = default;
    };

} // namespace vigilo