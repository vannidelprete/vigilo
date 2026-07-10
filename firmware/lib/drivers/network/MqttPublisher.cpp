/**
 * @file MqttPublisher.cpp
 * @brief Implementation of MqttPublisher.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-10
 */

#include "network/MqttPublisher.h"
#include <cstdio>

namespace {
    constexpr const char* TOPIC_FORMAT   = "vigilo/%s/telemetry";
    constexpr const char* PAYLOAD_FORMAT =
        "{\"ax\":%d,\"ay\":%d,\"az\":%d,\"gx\":%d,\"gy\":%d,\"gz\":%d,\"rpm\":%.1f}";
} // namespace

namespace vigilo {
    
    MqttPublisher::MqttPublisher(const char* broker, uint16_t port, const char* deviceId, IMqtt& mqtt)
        : _broker(broker), _port(port), _deviceId(deviceId), _mqtt(mqtt)
    {
        snprintf(_topic, sizeof(_topic), TOPIC_FORMAT, _deviceId);
    }

    bool MqttPublisher::connect() {
        return _mqtt.connect(_deviceId, _broker, _port);
    }

    bool MqttPublisher::isConnected() const noexcept {
        return _mqtt.isConnected();
    }

    bool MqttPublisher::publish(const ImuData& data, float rpm) {
        if (!_mqtt.isConnected()) return false;

        snprintf(_payload, sizeof(_payload), PAYLOAD_FORMAT,
                 data.ax, data.ay, data.az, data.gx, data.gy, data.gz, rpm);
        return _mqtt.publish(_topic, _payload);
    }

    void MqttPublisher::loop() noexcept {
        _mqtt.loop();
    }

} // namespace vigilo