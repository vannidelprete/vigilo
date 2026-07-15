/**
 * @file config.h
 * @brief Hardware pin assignments and timing constants for the Vigilo sensor node.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include <cstdint>

namespace vigilo {
    namespace config {
        constexpr uint8_t       PIN_LED                     = 2;            ///< Onboard status LED.
        constexpr uint8_t       PIN_SDA                     = 21;           ///< I2C data line.
        constexpr uint8_t       PIN_SCL                     = 22;           ///< I2C clock line.
        constexpr uint8_t       PIN_TACHO                   = 4;            ///< Tachometer pulse input (active low, pull-up required).
        
        constexpr uint8_t       IMU_ADDR                    = 0x68;         ///< MPU6050 I2C address (AD0 pin low).

        constexpr uint32_t      BAUD_RATE                   = 115200UL;     ///< Baud rate parameter.
        constexpr uint32_t      RPM_INTERVAL_MS             = 1000UL;       ///< RPM recalculation period in milliseconds.

        constexpr uint16_t      MQTT_PORT                   = 1883;         ///< MQTT broker TCP port.
        constexpr const char*   MQTT_DEVICE_ID              = "vigilo-01";  ///< Unique device identifier used as. 
        constexpr uint32_t      MQTT_RECONNECT_INTERVAL_MS  = 5000UL;       ///< Minimum interval between MQTT reconnect attempts.   

        constexpr uint32_t      I2C_CLOCK_HZ                = 400000UL;     ///< I2C Fast Mode clock speed for high-rate IMU batch sampling.
        constexpr uint32_t      IMU_SAMPLE_INTERVAL_US      = 2000UL;       ///< Target interval between batch samples (500 Hz).
        constexpr uint32_t      BATCH_INTERVAL_MS           = 30000UL;      ///< Period between batch capture cycles.
        constexpr uint16_t      MQTT_BUFFER_SIZE            = 4096;         ///< PubSubClient buffer size, sized for a full batch payload.

    } // namespace config

} // namespace vigilo