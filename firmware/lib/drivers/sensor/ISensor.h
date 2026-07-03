/**
 * @file ISensor.h
 * @brief Abstract interface for all sensor peripherals.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include <cstdint>

namespace vigilo {

    /**
     * @brief Result code returned by ISensor::begin().
     */
    enum class InitResult : uint8_t {
        Ok              = 0, ///< Initialization succeeded.
        DeviceNotFound  = 1, ///< Device did not respond on the bus.
        BusError        = 2, ///< Communication bus error during initialization.
        InvalidPin      = 3, ///< Specified GPIO pin does not support the required function.
    };

    /**
     * @brief Abstract interface for all sensor peripherals.
     * 
     * Provides a common lifecycle contract: initialize with begin(),
     * then query readiness with isReady() before reading data.
     */
    class ISensor {
    public:
        /**
         * @brief Initializes the sensor hardware.
         * @return InitResult::Ok on success, or a specific error code identifying the failure.
         */
        [[nodiscard]] virtual InitResult begin() = 0;

        /**
         * @brief Returns whether the sensor is initialized and operational.
         * @return true if the sensor is ready to provide data.
         */
        [[nodiscard]] virtual bool isReady() const noexcept = 0;

        /** @brief Virtual destructor. */
        virtual ~ISensor() = default;

    protected:
        ISensor() = default;
    };

} // namespace vigilo