#include <Arduino.h>
#include <WiFi.h>
#include <MPU6050.h>
#include "secrets.h"

#define LED_PIN 2
#define I2C_SDA 21
#define I2C_SCL 22

MPU6050 imu;

void connectWifi() {
    Serial.printf("Connecting to %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected - IP: %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    Wire.begin(I2C_SDA, I2C_SCL);
    imu.initialize();

    if (!imu.testConnection()) {
        Serial.println("MPU6050 connection failed");
        while (true) {}
    }
    Serial.println("MPU6050 connected");

    connectWifi();
}

void loop() {
    int16_t ax, ay, az, gx, gy, gz;
    imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    Serial.printf("ax=%d ay=%d az=%d | gx=%d gy=%d gz=%d\n",
                  ax, ay, az, gx, gy, gz);

    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
}