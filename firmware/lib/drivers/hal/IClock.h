/**
 * @file IClock.h
 * @brief Abstract interface for system timing.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include <cstdint>

namespace vigilo {

    /**
     * @brief Abstract interface for millisecond-resolution timing.
     *
     * Wraps millis()/delay() and micros()/delayMicroseconds() so drivers are
     * decoupled from the Arduino runtime and can be tested on the host.
     */
    class IClock {
    public:
        /**
         * @brief Returns milliseconds elapsed since boot.
         * @return Milliseconds since power-on.
         */
        [[nodiscard]] virtual uint32_t millis() const noexcept = 0;

        /**
         * @brief Blocks for the given number of milliseconds.
         * @param ms Duration in milliseconds.
         */
        virtual void delayMillis(uint32_t ms) noexcept = 0;

        /**
         * @brief Returns microseconds elapsed since boot.
         * @return uint32_t Microseconds since power-on.
         */
        [[nodiscard]] virtual uint32_t micros() const noexcept = 0;

        /**
         * @brief Blocks for the given number of microseconds.
         * @param us Duration in microseconds.
         */
        virtual void delayMicros(uint32_t us) noexcept = 0;

        /** @brief Virtual destructor. */
        virtual ~IClock() = default;

    protected:
        IClock() = default;
    };

} // namespace vigilo