/**
 * @file main.cpp
 * @brief Entry point - setup and loop orchestration only.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include <Arduino.h>
#include "sensor/Imu.h"
#include "sensor/Tachometer.h"
#include "network/WifiConnector.h"
#include "config.h"
#include "secrets.h"

namespace {
    vigilo::Imu           imu(vigilo::config::PIN_SDA, vigilo::config::PIN_SCL, vigilo::config::IMU_ADDR);
    vigilo::Tachometer    tacho(vigilo::config::PIN_TACHO);
    vigilo::WifiConnector wifi(WIFI_SSID, WIFI_PASSWORD);

    [[noreturn]] void halt(const char* msg) {
        Serial.println(msg);
        while (true) {
            digitalWrite(vigilo::config::PIN_LED, !digitalRead(vigilo::config::PIN_LED));
            delay(100);
        }
    }
}

void setup() {
    Serial.begin(vigilo::config::BAUD_RATE);
    pinMode(vigilo::config::PIN_LED, OUTPUT);

    switch (imu.begin()) {
        case vigilo::InitResult::Ok:             break;
        case vigilo::InitResult::DeviceNotFound: halt("IMU: device not found on I2C bus");
        case vigilo::InitResult::BusError:       halt("IMU: I2C bus error during init");
        case vigilo::InitResult::InvalidPin:     halt("IMU: invalid pin configuration");
    }

    switch (tacho.begin()) {
        case vigilo::InitResult::Ok:             break;
        case vigilo::InitResult::InvalidPin:     halt("Tachometer: pin does not support interrupts");
        case vigilo::InitResult::DeviceNotFound: halt("Tachometer: unexpected error");
        case vigilo::InitResult::BusError:       halt("Tachometer: unexpected error");
    }

    switch (wifi.connect()) {
        case vigilo::ConnectResult::Ok:              Serial.println("WiFi connected"); break;
        case vigilo::ConnectResult::NetworkNotFound: Serial.println("WiFi: network not found - continuing offline"); break;
        case vigilo::ConnectResult::AuthFailed:      Serial.println("WiFi: auth failed - continuing offline"); break;
        case vigilo::ConnectResult::Timeout:         Serial.println("WiFi: timeout - continuing offline"); break;
        case vigilo::ConnectResult::HardwareError:   halt("WiFi: hardware failure");
    }
}

void loop() {
    vigilo::ImuData data;
    switch (imu.read(data)) {
        case vigilo::Imu::ReadResult::Ok:
            // TODO: publish data (task #31)
            break;
        case vigilo::Imu::ReadResult::NotReady:
            Serial.println("IMU: not ready");
            break;
        case vigilo::Imu::ReadResult::BusError:
            Serial.println("IMU: I2C bus error");
            break;
        case vigilo::Imu::ReadResult::DataError:
            Serial.println("IMU: incomplete read");
            break;
    }

    float rpm = tacho.getRpm();
    (void)rpm; // TODO: publish rpm (task #31)
}