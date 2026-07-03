/**
 * @file WifiConnector.cpp
 * @brief WiFi connector implementation for the ESP32.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include "WifiConnector.h"
#include <WiFi.h>
#include <Arduino.h>

namespace vigilo {
    
    WifiConnector::WifiConnector(const char* ssid, const char* password)
        : _ssid(ssid), _password(password) {}
    
    ConnectResult WifiConnector::connect() {
        Serial.printf("Connecting to %s", _ssid);
        WiFi.begin(_ssid, _password);

        constexpr uint8_t  maxRetries  = 20;
        constexpr uint16_t retryDelayMs = 500;
        
        for (uint8_t i = 0; i < maxRetries && WiFi.status() != WL_CONNECTED; ++i) {
            delay(retryDelayMs);
        }

        switch (WiFi.status()) {
            case WL_CONNECTED:      return ConnectResult::Ok;
            case WL_NO_SSID_AVAIL:  return ConnectResult::NetworkNotFound;
            case WL_CONNECT_FAILED: return ConnectResult::AuthFailed;
            default:                return ConnectResult::Timeout;
        }
    }

    bool WifiConnector::isConnected() const noexcept {
        return WiFi.status() == WL_CONNECTED;
    }

} // namespace vigilo