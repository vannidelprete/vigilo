/**
 * @file IWire.h
 * @brief Abstract interface for I2C bus operations.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include <cstdint>

namespace vigilo {

    /**
     * @brief Abstract interface for I2C bus operations.
     *
     * Decouples drivers from the Arduino Wire singleton.
     * The bus must be initialized (begin) by the caller before injecting this interface.
     */
    class IWire {
    public:
        /** @brief Starts a transmission to the given I2C address. */
        virtual void beginTransmission(uint8_t addr) = 0;

        /** @brief Queues one byte for transmission. */
        virtual void write(uint8_t data) = 0;

        /**
         * @brief Ends the transmission and sends the queued bytes.
         * @param stop If true, sends a STOP condition; false sends a repeated START.
         * @return 0 on success, non-zero on error.
         */
        virtual uint8_t endTransmission(bool stop=true) = 0;

        /**
         * @brief Requests bytes from the given address.
         * @return Number of bytes actually received.
         */
        virtual uint8_t requestFrom(uint8_t addr, uint8_t quantity) = 0;

        /** @brief Reads one byte from the receive buffer. */
        virtual int read() = 0;

        virtual ~IWire() = default;
    protected:
        IWire() = default;
    };

} // namespace vigilo