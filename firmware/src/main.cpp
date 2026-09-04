/**
 * @file main.cpp
 * @brief Entry point for the Vigilo sensor node firmware.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include <Arduino.h>
#include "hal/ArduinoWire.h"
#include "hal/ArduinoMPU6050.h"
#include "hal/ArduinoClock.h"
#include "hal/ArduinoGpio.h"
#include "hal/ArduinoWifi.h"
#include "hal/ArduinoMqtt.h"
#include "hal/ArduinoMdns.h"
#include "network/MqttPublisher.h"
#include "network/MqttDiscovery.h"
#include "sensor/Imu.h"
#include "sensor/ImuBatchSampler.h"
#include "sensor/Tachometer.h"
#include "network/WifiConnector.h"
#include "config.h"
#include "secrets.h"

namespace {

    vigilo::ArduinoClock        g_clock;
    vigilo::ArduinoGpio         g_gpio;
    vigilo::ArduinoWire         g_wire;
    vigilo::ArduinoMPU6050      g_mpu;
    vigilo::ArduinoWifi         g_wifi_hal;
    vigilo::ArduinoMdns         g_mdns_hal;
    vigilo::ArduinoMqtt         g_mqtt_hal(vigilo::config::MQTT_BUFFER_SIZE);

    char g_brokerAddress[vigilo::MqttDiscovery::BROKER_ADDRESS_CAPACITY] = MQTT_BROKER; // fallback default; may be overwritten by mDNS discovery in setup()

    vigilo::Imu                 g_imu(g_wire, g_mpu, vigilo::config::IMU_ADDR);
    vigilo::ImuBatchSampler     g_batchSampler(g_imu, g_clock, vigilo::config::IMU_SAMPLE_INTERVAL_US);
    vigilo::Tachometer          g_tacho(vigilo::config::PIN_TACHO, g_clock, g_gpio, vigilo::config::RPM_INTERVAL_MS);
    vigilo::WifiConnector       g_wifi(WIFI_SSID, WIFI_PASSWORD, g_wifi_hal, g_clock);
    vigilo::MqttDiscovery       g_discovery(g_mdns_hal);
    vigilo::MqttPublisher       g_publisher(g_brokerAddress, vigilo::config::MQTT_PORT, vigilo::config::MQTT_DEVICE_ID,
                                            g_mqtt_hal, g_clock, vigilo::config::MQTT_RECONNECT_INTERVAL_MS);

    uint32_t g_lastBatchMs = 0;

    [[noreturn]] void halt(const char* msg) {
        Serial.println(msg);
        while (true) {
            digitalWrite(vigilo::config::PIN_LED, HIGH);
            delay(200);
            digitalWrite(vigilo::config::PIN_LED, LOW);
            delay(200);
        }
    }

} // namespace

void setup() {
    Serial.begin(vigilo::config::BAUD_RATE);
    pinMode(vigilo::config::PIN_LED, OUTPUT);

    Wire.begin(vigilo::config::PIN_SDA, vigilo::config::PIN_SCL, vigilo::config::I2C_CLOCK_HZ);

    switch (g_imu.begin()) {
        case vigilo::InitResult::Ok:            break;
        case vigilo::InitResult::DeviceNotFound: halt("IMU: device not found");
        case vigilo::InitResult::BusError:       halt("IMU: I2C bus error");
        default:                                 halt("IMU: unknown error");
    }

    switch (g_tacho.begin()) {
        case vigilo::InitResult::Ok:         break;
        case vigilo::InitResult::InvalidPin: halt("Tachometer: invalid pin");
        default:                             halt("Tachometer: unknown error");
    }

    switch (g_wifi.connect()) {
        case vigilo::ConnectResult::Ok:             break;
        case vigilo::ConnectResult::NetworkNotFound: halt("WiFi: SSID not found");
        case vigilo::ConnectResult::AuthFailed:      halt("WiFi: authentication failed");
        case vigilo::ConnectResult::Timeout:         halt("WiFi: connection timed out");
        default:                                     halt("WiFi: unknown error");
    }

    if (g_discovery.discover(vigilo::config::MQTT_DEVICE_ID, vigilo::config::MDNS_SERVICE_TYPE, vigilo::config::MDNS_PROTOCOL,
                             g_brokerAddress, sizeof(g_brokerAddress), MQTT_BROKER)) {
        Serial.printf("MQTT: discovered broker via mDNS: %s\n", g_brokerAddress);
    } else {
        Serial.printf("MQTT: mDNS discovery failed, using configured broker: %s\n", g_brokerAddress);
    }

    if (!g_publisher.connect())
    {
        Serial.printf("MQTT: initial connection failed (state=%d), retying in the background\n", g_mqtt_hal.lastError());
    }

    Serial.println("Vigilo ready.");
}

void loop() {
    digitalWrite(vigilo::config::PIN_LED, !digitalRead(vigilo::config::PIN_LED));
    g_publisher.loop();
    (void)g_publisher.connect();

    const uint32_t now = g_clock.millis();
    if ((now - g_lastBatchMs) >= vigilo::config::BATCH_INTERVAL_MS) {
        g_lastBatchMs = now;

        const std::size_t collected = g_batchSampler.collectBatch();
        const bool published = g_publisher.publishBatch(g_batchSampler.data(), collected, 
                                                        vigilo::config::IMU_SAMPLE_INTERVAL_US, g_tacho.getRpm());
        
        Serial.printf("batch: collected=%3u published=%s\n",
                      static_cast<unsigned>(collected), published ? "ok": "failed");
    }

    delay(100);
}