/**
 * @file ArduinoWifi.h
 * @brief Arduino WiFi library implementation of IWifi.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include "IWifi.h"
#include <WiFi.h>

namespace vigilo {

    /** @brief Delegates IWifi calls to the Arduino WiFi library. */
    class ArduinoWifi : public IWifi {
    public:
        /** @copydoc IWifi::begin() */
        void begin(const char* ssid, const char* password) override {
            WiFi.begin(ssid, password);
        }

        /** @copydoc IWifi::status() */
        Status status() override {
            switch (WiFi.status()) {
                case WL_CONNECTED:      return Status::Connected;
                case WL_NO_SSID_AVAIL:  return Status::NoSsid;
                case WL_CONNECT_FAILED: return Status::ConnectFailed;
                default:                return Status::Disconnected;
            }
        }
    };

} // namespace vigilo