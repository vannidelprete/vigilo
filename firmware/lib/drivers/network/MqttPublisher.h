/**
 * @file MqttPublisher.h
 * @brief MQTT publisher driver for high-rate IMU batches and connection status.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-10
 */

#pragma once
#include "hal/IMqtt.h"
#include "hal/IClock.h"
#include "sensor/Imu.h"
#include "sensor/ImuBatchSampler.h"
#include <cstdint>
#include <cstddef>

namespace vigilo {

#pragma pack(push, 1)
    /**
     * @brief Fixed-size header prepended to each binary batch payload.
     *
     * Followed immediately in the MQTT payload by sampleCount consecutive
     * ImuData records (12 bytes each, no padding). Matches the Python-side
     * unpack format "<IHf" for this header, then "<6h" per sample.
     */
    struct BatchHeader {
        uint32_t sampleIntervalUs; ///< Actual pacing interval used between samples.
        uint16_t sampleCount;      ///< Number of ImuData records following this header.
        float    rpm;              ///< Fan RPM measured at batch time.
    };
#pragma pack(pop)

    /**
     * @brief Publishes high-rate IMU batches and connection status to an MQTT broker.
     *
     * Publishes batches captured by ImuBatchSampler as a binary payload to
     * `vigilo/<deviceId>/telemetry/batch`, for FFT analysis on the receiving end.
     *
     * Connection status is tracked on `vigilo/<deviceId>/status`: "online" (retained)
     * is published right after a successful connect, and "offline" (retained) is
     * registered as an MQTT Last Will and Testament, so the broker publishes it
     * automatically if this device disconnects without warning (crash, power loss,
     * WiFi drop) - no periodic heartbeat needed.
     *
     * The caller is responsible for connection management: call connect() once in
     * setup(), then call it again every loop() iteration to recover after a drop
     * (throttled internally, safe to call unconditionally).
     *
     * @note No heap allocation. Topics and payloads are pre-allocated as
     *       fixed-size member buffers.
     */
    class MqttPublisher {
    public:
        /**
         * @brief Constructs the publisher and pre-formats the topic strings.
         * @param broker                Broker hostname or IP address. Must outlive this object.
         * @param port                  Broker TCP port.
         * @param deviceId              Unique device identifier used as the MQTT client ID and
         *                              embedded in the topics. Must outlive this object.
         * @param mqtt                  Injected MQTT interface.
         * @param clock                 Injected timing interface, used to throttle reconnect attempts.
         * @param reconnectIntervalMs   Minimum time between reconnect() attempts, in milliseconds.
         */
        explicit MqttPublisher(const char* broker, uint16_t port, const char* deviceId, IMqtt& mqtt,
                               IClock& clock, uint32_t reconnectIntervalMs);

        MqttPublisher(const MqttPublisher&)            = delete; ///< Non-copyable.
        MqttPublisher& operator=(const MqttPublisher&) = delete; ///< Non-copyable.
        MqttPublisher(MqttPublisher&&)                 = delete; ///< Non-movable.
        MqttPublisher& operator=(MqttPublisher&&)      = delete; ///< Non-movable.

        /**
         * @brief Connects to the MQTT broker, or attempts to restore the connection.
         *
         * Returns true immediately without attempting when already connected.
         * The first call ever made always attempts immediately, regardless of
         * elapsed time. Later calls while disconnected are throttled: they
         * return false without attempting if called again before
         * reconnectIntervalMs has elapsed since the last attempt.
         *
         * Registers the "offline" Last Will and Testament on `vigilo/<deviceId>/status`,
         * then publishes "online" (retained) to the same topic on success.
         *
         * @return true if connected (already, or as a result of this call),
         *         false if disconnected and no attempt was made or the attempt failed.
         */
        [[nodiscard]] bool connect();

        /**
         * @brief Returns whether the underlying client is connected to the broker.
         * @return true if connected, false otherwise.
         */
        [[nodiscard]] bool isConnected() const noexcept;
        
        /**
         * @brief Publishes a high-rate batch as a binary payload to `vigilo/<deviceId>/telemetry/batch`.
         *
         * Returns false immediately without calling the underlying MQTT client
         * when not connected, or when the batch does not fit BATCH_PAYLOAD_CAPACITY.
         *
         * Wire format: a packed BatchHeader followed by sampleCount consecutive
         * ImuData records.
         *
         * @param samples           Pointer to the collected samples (see ImuBatchSampler::data()).
         * @param sampleCount       Number of valid samples in samples.
         * @param sampleIntervalUs  Actual pacing interval used to collect the batch.
         * @param rpm               Current fan RPM.
         * @return true if the broker accepted the message, false otherwise.
         */
        [[nodiscard]] bool publishBatch(const ImuData* samples, std::size_t sampleCount,
                                        uint32_t sampleIntervalUs, float rpm);

        /**
         * @brief Maintains the MQTT keepalive. Must be called every loop iteration.
         */
        void loop() noexcept;

    private:
        static constexpr std::size_t STATUS_TOPIC_CAPACITY  = 64;  ///< Buffer size for the pre-formatted status topic string.
        static constexpr std::size_t BATCH_TOPIC_CAPACITY   = 64;  ///< Buffer size for the pre-formatted batch topic string.
        static constexpr std::size_t BATCH_PAYLOAD_CAPACITY =
            sizeof(BatchHeader) + ImuBatchSampler::BATCH_SIZE * sizeof(ImuData); ///< Buffer size for the largest possible batch payload.

        const char* _broker;                               ///< Broker address pointer (not owned).
        uint16_t    _port;                                 ///< Broker TCP port.
        const char* _deviceId;                              ///< Device identifier pointer (not owned).
        IMqtt&      _mqtt;                                  ///< Injected MQTT interface.
        IClock&     _clock;                                 ///< Injected timing interface.
        uint32_t    _reconnectIntervalMs;                   ///< Minimum interval between attempts after the first.
        uint32_t    _lastAttemptMs = 0;                     ///< Timestamp of the last connect() attempt.
        bool        _hasAttempted  = false;                 ///< True once connect() has been called at least once.
        char        _statusTopic[STATUS_TOPIC_CAPACITY];    ///< Pre-formatted topic: `vigilo/<deviceId>/status`.
        char        _batchTopic[BATCH_TOPIC_CAPACITY];      ///< Pre-formatted topic: `vigilo/<deviceId>/telemetry/batch`.
        uint8_t     _batchPayload[BATCH_PAYLOAD_CAPACITY];  ///< Reusable binary batch payload buffer.
    };

} // namespace vigilo