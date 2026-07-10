/**
 * @file MqttPublisher.h
 * @brief MQTT publisher driver for IMU and RPM telemetry.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-10
 */

#pragma once
#include "hal/IMqtt.h"
#include "sensor/Imu.h"
#include <cstdint>

namespace vigilo {

    /**
     * @brief Publishes sensor telemetry to an MQTT broker.
     *
     * Formats IMU and RPM readings as a JSON payload and publishes them to
     * `vigilo/<deviceId>/telemetry`. The caller is responsible for connection
     * management: call connect() once in setup(), then call isConnected() +
     * connect() in loop() to recover after a drop.
     *
     * @note No heap allocation. The topic and payload are pre-allocated as
     *       fixed-size member buffers.
     */
    class MqttPublisher {
    public:
        /**
         * @brief Constructs the publisher and pre-formats the topic string.
         * @param broker   Broker hostname or IP address. Must outlive this object.
         * @param port     Broker TCP port.
         * @param deviceId Unique device identifier used as the MQTT client ID and
         *                 embedded in the topic. Must outlive this object.
         * @param mqtt     Injected MQTT interface.
         */
        explicit MqttPublisher(const char* broker, uint16_t port, const char* deviceId, IMqtt& mqtt);

        MqttPublisher(const MqttPublisher&)            = delete; ///< Non-copyable.
        MqttPublisher& operator=(const MqttPublisher&) = delete; ///< Non-copyable.
        MqttPublisher(MqttPublisher&&)                 = delete; ///< Non-movable.
        MqttPublisher& operator=(MqttPublisher&&)      = delete; ///< Non-movable.

        /**
         * @brief Connects to the MQTT broker.
         * @return true on success, false on failure.
         */
        [[nodiscard]] bool connect();

        /**
         * @brief Returns whether the underlying client is connected to the broker.
         * @return true if connected, false otherwise.
         */
        [[nodiscard]] bool isConnected() const noexcept;

        /**
         * @brief Publishes a telemetry JSON payload to `vigilo/<deviceId>/telemetry`.
         *
         * Returns false immediately without calling the underlying MQTT client
         * when not connected.
         *
         * Payload format:
         * `{"ax":<int>,"ay":<int>,"az":<int>,"gx":<int>,"gy":<int>,"gz":<int>,"rpm":<float:.1f>}`
         *
         * @param data IMU accelerometer and gyroscope readings.
         * @param rpm  Current fan RPM.
         * @return true if the broker accepted the message, false otherwise.
         */
        [[nodiscard]] bool publish(const ImuData& data, float rpm);
        
        /**
         * @brief Maintains the MQTT keepalive. Must be called every loop iteration.
         */
        void loop() noexcept;

    private:
        static constexpr std::size_t TOPIC_CAPACITY   = 64;  ///< Buffer size for the pre-formatted topic string.
        static constexpr std::size_t PAYLOAD_CAPACITY = 128; ///< Buffer size for the JSON payload.

        const char* _broker;                    ///< Broker address pointer (not owned).
        uint16_t    _port;                      ///< Broker TCP port.
        const char* _deviceId;                  ///< Device identifier pointer (not owned).
        IMqtt&      _mqtt;                      ///< Injected MQTT interface.
        char        _topic[TOPIC_CAPACITY];     ///< Pre-formatted topic: `vigilo/<deviceId>/telemetry`.
        char        _payload[PAYLOAD_CAPACITY]; ///< Reusable JSON payload buffer.
    };

} // namespace vigilo