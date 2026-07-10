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
        /** @brief Constructs the MQTT client with an internal WiFiClient. */
        ArduinoMqtt() : _client(_wifi) {}

        ArduinoMqtt(const ArduinoMqtt&)            = delete; ///< Non-copyable.
        ArduinoMqtt& operator=(const ArduinoMqtt&) = delete; ///< Non-copyable.
        ArduinoMqtt(ArduinoMqtt&&)                 = delete; ///< Non-movable.
        ArduinoMqtt& operator=(ArduinoMqtt&&)      = delete; ///< Non-movable.

        /** @copydoc IMqtt::connect() */
        [[nodiscard]] bool connect(const char* clientId, const char* broker, uint16_t port) override {
            _client.setServer(broker, port);
            return _client.connect(clientId);
        }

        /** @copydoc IMqtt::publish() */
        [[nodiscard]] bool publish(const char* topic, const char* payload) override {
            return _client.publish(topic, payload);
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