/**
 * @file Tachometer.h
 * @brief RPM measurement via interrupt-driven pulse counting.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include "ISensor.h"
#include <cstdint>

namespace vigilo {

    /**
     * @brief RPM measurement via interrupt-driven pulse counting.
     * 
     * Counts falling edges on a tachometer pin and converts them to RPM
     * over a fixed interval defined by config::RPM_INTERVAL_MS.
     * Assumes 2 pulses per revolution (standard 3-pin PC fan).
     * 
     * @warning Only one instance is supported due to the static ISR.
     */
    class Tachometer : public ISensor {
    public:
        /**
         * @brief Constructs the tachometer for the given GPIO pin.
         * @param pin GPIO pin connected to the tachometer signal (requires external pull-up).
         */
        explicit Tachometer(uint8_t pin);

        Tachometer(const Tachometer&)            = delete; // Non-copyable: maps to a unique static ISR.
        Tachometer& operator=(const Tachometer&) = delete; // Non-copyable: maps to a unique static ISR.
        Tachometer(Tachometer&&)                 = delete; // Non-movable: the static ISR cannot be re-associated after construction.
        Tachometer& operator=(Tachometer&&)      = delete; // Non-movable: the static ISR cannot be re-associated after construction.

        /** @copydoc ISensor::begin() */
        [[nodiscard]] InitResult begin() override;

        /** @copydoc ISensor::isReady() */
        [[nodiscard]] bool isReady() const noexcept override;

        /**
         * @brief Returns the current RPM, recalculated every config::RPM_INTERVAL_MS.
         * 
         * If called more frequently than the interval, returns the last computed value
         * without accessing the pulse counter.
         * 
         * @return Rotations per minute.
         */
        [[nodiscard]] float getRpm() noexcept;

        /**
         * @brief ISR invoked on each tachometer falling edge.
         * @note IRAM_ATTR is applied on the definition to keep this header platform-agnostic.
         */
        static void isr();

    private:
        uint8_t  _pin;                          ///< GPIO pin number for the tachometer signal.
        bool     _ready  = false;               ///< True after a successful begin() call.
        uint32_t _lastMs = 0;                   ///< Timestamp of the last RPM recalculation (milliseconds).
        float    _rpm    = 0.0f;                ///< Last computed RPM value.

        static volatile uint32_t _pulseCount;   ///< Pulse counter incremented by the ISR.
    };

} // namespace vigilo