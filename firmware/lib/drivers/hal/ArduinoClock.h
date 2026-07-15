/**
 * @file ArduinoClock.h
 * @brief Arduino timing implementation of IClock.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include "IClock.h"
#include <Arduino.h>

namespace vigilo {

    /** @brief Delegates IClock calls to the Arduino millis() and delay() functions. */
    class ArduinoClock : public IClock {
    public:
        /** @copydoc IClock::millis() */
        uint32_t millis() const noexcept override { return ::millis(); }

        /** @copydoc IClock::delayMillis() */
        void delayMillis(uint32_t ms) noexcept override { ::delay(ms); }

        /** @copydoc IClock::micros() */
        uint32_t micros() const noexcept override { return ::micros(); }

        /** @copydoc IClock::delayMicros() */
        void delayMicros(uint32_t us) noexcept override { ::delayMicroseconds(us); }
    };

} // namespace vigilo