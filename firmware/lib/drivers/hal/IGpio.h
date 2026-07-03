/**
 * @file IGpio.h
 * @brief Abstract interface for GPIO and interrupt configuration.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include <cstdint>

namespace vigilo {

    /**
     * @brief Abstract interface for GPIO pin control and interrupt attachment.
     *
     * Exposes platform-independent enums so drivers never reference
     * Arduino constants (INPUT, OUTPUT, RISING, etc.) directly.
     */
    class IGpio {
    public:
        /** @brief Pin direction and pull-up configuration. */
        enum class PinMode : uint8_t {
            Input       = 0, ///< Floating input.
            Output      = 1, ///< Push-pull output.
            InputPullUp = 2, ///< Input with internal pull-up resistor.
        };

        /** @brief Edge sensitivity for interrupt attachment. */
        enum class Trigger : uint8_t {
            Rising  = 0, ///< Trigger on rising edge.
            Falling = 1, ///< Trigger on falling edge.
            Change  = 2, ///< Trigger on any edge.
        };

        /**
         * @brief Returns whether the pin supports external interrupts.
         * @param pin GPIO pin number.
         * @return true if the pin can be attached to an interrupt.
         */
        [[nodiscard]] virtual bool supportsInterrupt(uint8_t pin) const noexcept = 0;

        /**
         * @brief Configures the pin direction.
         * @param pin  GPIO pin number.
         * @param mode Desired pin mode.
         */
        virtual void pinMode(uint8_t pin, PinMode mode) noexcept = 0;

        /**
         * @brief Attaches an ISR to the pin.
         * @param pin     GPIO pin number.
         * @param isr     Interrupt service routine (must be IRAM_ATTR on the concrete side).
         * @param trigger Edge sensitivity.
         */
        virtual void attachInterrupt(uint8_t pin, void(*isr)(), Trigger trigger) noexcept = 0;

        /** @brief Virtual destructor. */
        virtual ~IGpio() = default;

    protected:
        IGpio() = default;
    };

} // namespace vigilo