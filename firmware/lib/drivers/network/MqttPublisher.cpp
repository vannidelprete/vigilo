/**
 * @file MqttPublisher.cpp
 * @brief Implementation of MqttPublisher.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-10
 */

#include "network/MqttPublisher.h"
#include <cstdio>
#include <cstring>

namespace {
    constexpr const char* STATUS_TOPIC_FORMAT = "vigilo/%s/status";
    constexpr const char* BATCH_TOPIC_FORMAT  = "vigilo/%s/telemetry/batch";
    constexpr const char* WILL_MESSAGE        = "offline";
    constexpr const char* ONLINE_MESSAGE      = "online";
} // namespace


namespace vigilo {
    
    MqttPublisher::MqttPublisher(const char* broker, uint16_t port, const char* deviceId,
                                  IMqtt& mqtt, IClock& clock, uint32_t reconnectIntervalMs)
        : _broker(broker), _port(port), _deviceId(deviceId),
          _mqtt(mqtt), _clock(clock), _reconnectIntervalMs(reconnectIntervalMs)
    {
        snprintf(_statusTopic, sizeof(_statusTopic), STATUS_TOPIC_FORMAT, _deviceId);
        snprintf(_batchTopic, sizeof(_batchTopic), BATCH_TOPIC_FORMAT, _deviceId);
    }

    bool MqttPublisher::connect() {
    if (_mqtt.isConnected()) return true;

    const uint32_t now = _clock.millis();
    if (_hasAttempted && (now - _lastAttemptMs < _reconnectIntervalMs)) return false;

    _hasAttempted  = true;
    _lastAttemptMs = now;

    if (!_mqtt.connect(_deviceId, _broker, _port, _statusTopic, WILL_MESSAGE)) return false;

    (void)_mqtt.publish(_statusTopic, ONLINE_MESSAGE, true);
    return true;
}

    bool MqttPublisher::isConnected() const noexcept {
        return _mqtt.isConnected();
    }

    bool MqttPublisher::publishBatch(const ImuData* samples, std::size_t sampleCount,
                                     uint32_t sampleIntervalUs, float rpm) {
        if (!_mqtt.isConnected()) return false;

        const std::size_t dataBytes = sampleCount * sizeof(ImuData);
        const std::size_t totalBytes = sizeof(BatchHeader) + dataBytes;
        if (totalBytes > sizeof(_batchPayload)) return false;

        const BatchHeader header{sampleIntervalUs, static_cast<uint16_t>(sampleCount), rpm};
        std::memcpy(_batchPayload, &header, sizeof(BatchHeader));
        std::memcpy(_batchPayload + sizeof(BatchHeader), samples, dataBytes);

        return _mqtt.publishBinary(_batchTopic, _batchPayload, totalBytes);
    }

    void MqttPublisher::loop() noexcept {
        _mqtt.loop();
    }

} // namespace vigilo