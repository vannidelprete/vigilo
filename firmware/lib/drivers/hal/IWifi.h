/**
 * @file IWifi.h
 * @brief Abstract interface for WiFi connectivity.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once

namespace vigilo {

    /**
     * @brief Abstract interface for WiFi hardware operations.
     *
     * Wraps WiFi.begin() and WiFi.status() so WifiConnector is decoupled
     * from the Arduino WiFi library and can be tested on the host.
     */
    class IWifi {
    public:
        /** @brief Connection state codes, independent of Arduino WiFi constants. */
        enum class Status {
            Connected     = 0, ///< Association complete, IP assigned.
            NoSsid        = 1, ///< SSID not found in scan results.
            ConnectFailed = 2, ///< Authentication rejected.
            Disconnected  = 3, ///< Not connected (idle or connecting).
        };

        /**
         * @brief Starts the connection attempt.
         * @param ssid     Network name (null-terminated).
         * @param password Network password (null-terminated).
         */
        virtual void begin(const char* ssid, const char* password) = 0;

        /**
         * @brief Returns the current connection state.
         * @return Status code reflecting the current WiFi state.
         */
        [[nodiscard]] virtual Status status() = 0;

        /** @brief Virtual destructor. */
        virtual ~IWifi() = default;

    protected:
        IWifi() = default;
    };

} // namespace vigilo