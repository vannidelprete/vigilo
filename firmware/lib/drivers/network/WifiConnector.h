/**
 * @file WifiConnector.h
 * @brief WiFi network connector for the ESP32.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include "hal/IWifi.h"
#include "hal/IClock.h"
#include "IConnector.h"

namespace vigilo {

    /**
     * @brief Manages WiFi connectivity via injected HAL interfaces.
     *
     * Attempts connection with up to 20 retries at 500 ms intervals.
     * The IWifi and IClock dependencies are injected at construction,
     * enabling full unit testing without hardware.
     */
    class WifiConnector : public IConnector {
    public:
        /**
         * @brief Constructs the connector with the given credentials and HAL dependencies.
         * @param ssid     Network name (null-terminated, lifetime must exceed this object).
         * @param password Network password (null-terminated, lifetime must exceed this object).
         * @param wifi     WiFi hardware interface.
         * @param clock    Timing interface for retry delays.
         */
        explicit WifiConnector(const char* ssid, const char* password, IWifi& wifi, IClock& clock);

        WifiConnector(const WifiConnector&)            = delete; // Non-copyable: manages a unique network connection.
        WifiConnector& operator=(const WifiConnector&) = delete; // Non-copyable: manages a unique network connection.
        WifiConnector(WifiConnector&&)                 = delete; // Non-movable: the underlying WiFi stack state is global and cannot transfer.
        WifiConnector& operator=(WifiConnector&&)      = delete; // Non-movable: the underlying WiFi stack state is global and cannot transfer.

        /** @copydoc IConnector::connect() */
        [[nodiscard]] ConnectResult connect() override;

        /** @copydoc IConnector::isConnected() */
        [[nodiscard]] bool isConnected() const noexcept override;

    private:
        const char* _ssid;     ///< Network SSID pointer (not owned).
        const char* _password; ///< Network password pointer (not owned).
        IWifi&      _wifi;     ///< Injected WiFi hardware interface.
        IClock&     _clock;    ///< Injected timing interface.
    };

} // namespace vigilo