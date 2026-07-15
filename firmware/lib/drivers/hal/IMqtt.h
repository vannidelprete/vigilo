/**
 * @file IMqtt.h
 * @brief Abstract MQTT client interface.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-10
 */

#pragma once
#include <cstdint>
#include <cstddef>

namespace vigilo {

    class IMqtt {
    public:
        virtual ~IMqtt() = default;

        /**
         * @brief Connects to the MQTT broker.
         * @param clientId  Unique client identifier string.
         * @param broker    Broker hostname or IP address.
         * @param port      Broker TCP port.
         * @param willTopic   Topic the broker publishes the will message to.
         * @param willMessage Message the broker publishes on unexpected disconnect. 
         * @return true on success, false on failure.
         */
        [[nodiscard]] virtual bool connect(const char* clientId, const char* broker, uint16_t port,
                                           const char* willTopic, const char* willMessage) = 0;

        /**
         * @brief Publishes a payload to a topic.
         * @param topic     MQTT topic string.
         * @param payload   Null-terminated payload string.
         * @param retained  If true, the broker keeps this message as the topic's last known value
         *                  for new subscribers.
         * @return true on success, false on failure.
         */
        [[nodiscard]] virtual bool publish(const char* topic, const char* payload, bool retained) = 0;

        /**
         * @brief Publishes a raw binary payload to a topic.
         * @param topic     MQTT topic string.
         * @param payload   Pointer to the raw payload bytes.
         * @param length    Number of bytes in payload.
         * @return true on success, false on failure.
         */
        [[nodiscard]] virtual bool publishBinary(const char* topic, const uint8_t* payload, std::size_t length) = 0;

        /**
         * @brief Returns whether the client is currently connected to the broker.
         * @return true if connected, false otherwise.
         */
        [[nodiscard]] virtual bool isConnected() const noexcept = 0;

        /**
         * @brief Processes incoming messages and maintains the connection keepalive.
         * 
         * Must be called regularly from the main loop.
         */
        virtual void loop() noexcept = 0;
    };

} // namespace vigilo