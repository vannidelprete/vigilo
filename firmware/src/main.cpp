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
#include "network/MqttPublisher.h"
#include "sensor/Imu.h"
#include "sensor/Tachometer.h"
#include "network/WifiConnector.h"
#include "config.h"
#include "secrets.h"

namespace {

    vigilo::ArduinoClock   g_clock;
    vigilo::ArduinoGpio    g_gpio;
    vigilo::ArduinoWire    g_wire;
    vigilo::ArduinoMPU6050 g_mpu;
    vigilo::ArduinoWifi    g_wifi_hal;
    vigilo::ArduinoMqtt    g_mqtt_hal;

    vigilo::Imu           g_imu(g_wire, g_mpu, vigilo::config::IMU_ADDR);
    vigilo::Tachometer    g_tacho(vigilo::config::PIN_TACHO, g_clock, g_gpio, vigilo::config::RPM_INTERVAL_MS);
    vigilo::WifiConnector g_wifi(WIFI_SSID, WIFI_PASSWORD, g_wifi_hal, g_clock);
    vigilo::MqttPublisher g_publisher(MQTT_BROKER, vigilo::config::MQTT_PORT, vigilo::config::MQTT_DEVICE_ID, g_mqtt_hal);

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

    Wire.begin(vigilo::config::PIN_SDA, vigilo::config::PIN_SCL);

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

    if (!g_publisher.connect())
    {
        halt("MQTT: connection failed");
    }

    Serial.println("Vigilo ready.");
}

void loop() {
    digitalWrite(vigilo::config::PIN_LED, !digitalRead(vigilo::config::PIN_LED));
    g_publisher.loop();

    vigilo::ImuData data;
    switch (g_imu.read(data)) {
        case vigilo::Imu::ReadResult::Ok:        break;
        case vigilo::Imu::ReadResult::BusError:  Serial.println("IMU: bus error"); return;
        case vigilo::Imu::ReadResult::DataError: Serial.println("IMU: data error"); return;
        default:                                  return;
    }
    float rpm = g_tacho.getRpm();
    Serial.printf("\rax=%6d ay=%6d az=%6d gx=%6d gy=%6d gz=%6d  rpm=%6.1f  ",
                  data.ax, data.ay, data.az, data.gx, data.gy, data.gz, rpm);

    const bool connected = g_publisher.isConnected();
    const bool ok = connected ? g_publisher.publish(data, rpm) : g_publisher.connect();

    if (!ok)
    {
        Serial.println(connected ? "MQTT: publish failed" : "MQTT: reconnect failed");
    }

    delay(100);
}