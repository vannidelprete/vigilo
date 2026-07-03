/**
 * @file WifiConnector.h
 * @brief WiFi network connector for the ESP32.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include "IConnector.h"

namespace vigilo {

    /**
     * @brief WiFi network connector using the ESP32 Arduino WiFi API.
     *
     * @warning Non-copyable and non-movable: holds raw C-string pointers
     * whose lifetime must be managed by the caller.
     */
    class WifiConnector : public IConnector {
    public:
        /**
         * @brief Constructs the connector with the given credentials.
         * @param ssid      Null-terminated WiFi network name.
         * @param password  Null-terminated WiFi password.
         */
        explicit WifiConnector(const char* ssid, const char* password);

        WifiConnector(const WifiConnector&)            = delete; // Non-copyable: manages a unique network connection.
        WifiConnector& operator=(const WifiConnector&) = delete; // Non-copyable: manages a unique network connection.
        WifiConnector(WifiConnector&&)                 = delete; // Non-movable: the underlying WiFi stack state is global and cannot transfer.
        WifiConnector& operator=(WifiConnector&&)      = delete; // Non-movable: the underlying WiFi stack state is global and cannot transfer.

        /** @copydoc IConnector::connect() */
        [[nodiscard]] ConnectResult connect() override;

        /** @copydoc IConnector::isConnected() */
        [[nodiscard]] bool isConnected() const noexcept override;

    private:
        const char* _ssid;     ///< Null-terminated WiFi network name.
        const char* _password; ///< Null-terminated WiFi password.
    };

} // namespace vigilo