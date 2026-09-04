/**
 * @file MqttDiscovery.h
 * @brief Resolves the MQTT broker address via mDNS, falling back to a configured default.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-09-02
 */

#pragma once
#include "hal/IMdns.h"
#include <cstddef>

namespace vigilo {

    /**
     * @brief Resolves the MQTT broker address via mDNS, falling back to a configured default.
     *
     * Queries for a "_mqtt._tcp" service advertised by the gateway. If found, writes
     * the resolved address into the caller's buffer; otherwise writes the given
     * fallback address unchanged, so the caller always ends up with a usable value.
     *
     * Only the address is discovered - the broker port is a fixed project-wide
     * constant (vigilo::config::MQTT_PORT) on both ends, so it is never queried.
     */
    class MqttDiscovery {
    public:
        static constexpr std::size_t BROKER_ADDRESS_CAPACITY = 32; ///< Buffer size required for a resolved or fallback broker address string.

        /**
         * @brief Constructs the discovery step with the given HAL dependency.
         * @param mdns mDNS hardware interface.
         */
        explicit MqttDiscovery(IMdns& mdns);

        MqttDiscovery(const MqttDiscovery&)            = delete; // Non-copyable: holds a non-rebindable reference.
        MqttDiscovery& operator=(const MqttDiscovery&) = delete; // Non-copyable: holds a non-rebindable reference.
        MqttDiscovery(MqttDiscovery&&)                 = delete; // Non-movable: holds a non-rebindable reference.
        MqttDiscovery& operator=(MqttDiscovery&&)      = delete; // Non-movable: holds a non-rebindable reference.

        /**
         * @brief Resolves the broker address, via mDNS if possible.
         * 
         * @param hostname          Local hostname to advertise while querying (without ".local").
         * @param brokerOut         Buffer written with the resolved or fallback address as a null-terminated string.
         * @param brokerOutSize     Capacity of brokerOut, including the null terminator.
         * @param fallback          Address to write into brokerOut if no service is found.
         * @return true if a broker was found via mDNS, false if the fallback was used.
         */
        [[nodiscard]] bool discover(const char* hostname, const char* service, const char* proto,
                                    char* brokerOut, std::size_t brokerOutSize, const char* fallback);

    private:
        IMdns& _mdns; ///< Injected mDNS hardware interface.
    };

} // namespace vigilo