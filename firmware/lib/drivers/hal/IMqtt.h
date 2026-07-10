/**
 * @file IMqtt.h
 * @brief Abstract MQTT client interface.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-10
 */

#pragma once
#include <cstdint>

namespace vigilo {

    class IMqtt {
    public:
        virtual ~IMqtt() = default;

        /**
         * @brief Connects to the MQTT broker.
         * @param clientId  Unique client identifier string.
         * @param broker    Broker hostname or IP address.
         * @param port      Broker TCP port.
         * @return true on success, false on failure.
         */
        [[nodiscard]] virtual bool connect(const char* clientId, const char* broker, uint16_t port) = 0;

        /**
         * @brief Publishes a paylaod to a topic.
         * @param topic     MQTT topic string.
         * @param payload   
         */
        [[nodiscard]] virtual bool publish(const char* topic, const char* payload) = 0;

        /**
         * @brief Returns weither the client is currently connected to the broker.
         * @return true if connected, false otherwise.
         */
        [[nodiscard]] virtual bool isConnected() const noexcept = 0;

        /**
         * @brief Processes incoming messages and maintains the connection keepalive.
         * 
         * Must be cladded regularly from the main loop.
         */
        virtual void loop() noexcept = 0;
    };

} // namespace vigilo