/**
 * @file ArduinoGpio.h
 * @brief Arduino GPIO implementation of IGpio.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include "IGpio.h"
#include <Arduino.h>

namespace vigilo {

    /** @brief Delegates IGpio calls to Arduino pin and interrupt functions. */
    class ArduinoGpio : public IGpio {
    public:
        /** @copydoc IGpio::supportsInterrupt() */
        bool supportsInterrupt(uint8_t pin) const noexcept override {
            return digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT;
        }

        /** @copydoc IGpio::pinMode() */
        void pinMode(uint8_t pin, PinMode mode) noexcept override {
            switch (mode) {
                case PinMode::Input:       ::pinMode(pin, INPUT);        break;
                case PinMode::Output:      ::pinMode(pin, OUTPUT);       break;
                case PinMode::InputPullUp: ::pinMode(pin, INPUT_PULLUP); break;
            }
        }

        /** @copydoc IGpio::attachInterrupt() */
        void attachInterrupt(uint8_t pin, void(*isr)(), Trigger trigger) noexcept override {
            int mode;
            switch (trigger) {
                case Trigger::Rising:  mode = RISING;  break;
                case Trigger::Falling: mode = FALLING; break;
                default:               mode = CHANGE;  break;
            }
            ::attachInterrupt(digitalPinToInterrupt(pin), isr, mode);
        }
    };

} // namespace vigilo