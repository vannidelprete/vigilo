/**
 * @file WifiConnector.cpp
 * @brief WiFi connector implementation for the ESP32.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include "WifiConnector.h"

namespace vigilo {
    
    WifiConnector::WifiConnector(const char* ssid, const char* password, IWifi& wifi, IClock& clock)
        : _ssid(ssid), _password(password), _wifi(wifi), _clock(clock) {}

    ConnectResult WifiConnector::connect() {
        _wifi.begin(_ssid, _password);

        constexpr uint8_t  maxRetries   = 20;
        constexpr uint32_t retryDelayMs = 500;

        for (uint8_t i = 0; i < maxRetries && _wifi.status() != IWifi::Status::Connected; ++i) {
            _clock.delayMillis(retryDelayMs);
        }

        switch (_wifi.status()) {
            case IWifi::Status::Connected:     return ConnectResult::Ok;
            case IWifi::Status::NoSsid:        return ConnectResult::NetworkNotFound;
            case IWifi::Status::ConnectFailed: return ConnectResult::AuthFailed;
            default:                           return ConnectResult::Timeout;
        }
    }

    bool WifiConnector::isConnected() const noexcept {
        return _wifi.status() == IWifi::Status::Connected;
    }

} // namespace vigilo