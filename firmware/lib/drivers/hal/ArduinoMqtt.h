/**
 * @file ArduinoMqtt.h
 * @brief Arduino PubSubClient wrapper implementing IMqtt.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-10
 */

#pragma once
#include "IMqtt.h"
#include <WiFi.h>
#include <PubSubClient.h>

namespace vigilo {

    /**
     * @brief Concrete IMqtt implementation backed by PubSubClient over WiFi.
     *
     * @note Only compiled on Arduino targets. On the native test host,
     *       use a gMock-generated mock of IMqtt instead.
     */
    class ArduinoMqtt : public IMqtt {
    public:
        /**
         * @brief Constructs the MQTT client with an internal WiFiClient.
         * @param bufferSize Maximum MQTT packet size, in bytes. Must be large enough
         *                   to hold the biggest published payload (batch payloads are
         *                   the largest, see MqttPublisher::BATCH_PAYLOAD_CAPACITY).
         */
        explicit ArduinoMqtt(uint16_t bufferSize) : _client(_wifi) {
            _client.setBufferSize(bufferSize);
        }

        ArduinoMqtt(const ArduinoMqtt&)            = delete; ///< Non-copyable.
        ArduinoMqtt& operator=(const ArduinoMqtt&) = delete; ///< Non-copyable.
        ArduinoMqtt(ArduinoMqtt&&)                 = delete; ///< Non-movable.
        ArduinoMqtt& operator=(ArduinoMqtt&&)      = delete; ///< Non-movable.

        /** @copydoc IMqtt::connect() */
        [[nodiscard]] bool connect(const char* clientId, const char* broker, uint16_t port,
                                   const char* willTopic, const char* willMessage) override {
            _client.setServer(broker, port);
            return _client.connect(clientId, willTopic, 1, true, willMessage);
        }

        /** @copydoc IMqtt::publish() */
        [[nodiscard]] bool publish(const char* topic, const char* payload, bool retained) override {
            return _client.publish(topic, payload, retained);
        }

        /** @copydoc IMqtt::publishBinary() */
        [[nodiscard]] bool publishBinary(const char* topic, const uint8_t* payload, std::size_t length) override {
            return _client.publish(topic, payload, static_cast<unsigned int>(length));
        }

        /** @copydoc IMqtt::isConnected() */
        [[nodiscard]] bool isConnected() const noexcept override {
            return _client.connected();
        }

        /** @copydoc IMqtt::loop() */
        void loop() noexcept override {
            _client.loop();
        }

    private:
        WiFiClient              _wifi;   ///< Underlying TCP socket provided to PubSubClient.
        mutable PubSubClient    _client; ///< PubSubClient instance wrapping the WiFiClient.
    };

} // namespace vigilo