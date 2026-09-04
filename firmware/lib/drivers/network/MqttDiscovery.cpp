/**
 * @file MqttDiscovery.cpp
 * @brief Resolves the MQTT broker address via mDNS, falling back to a configured default.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-09-02
 */

#include "MqttDiscovery.h"
#include <cstring>

namespace vigilo {

    MqttDiscovery::MqttDiscovery(IMdns& mdns) : _mdns(mdns) {}

    bool MqttDiscovery::discover(const char* hostname, const char* service, const char* proto,
                                 char* brokerOut, std::size_t brokerOutSize, const char* fallback) {
        if (_mdns.begin(hostname) &&
            _mdns.queryService(service, proto, brokerOut, brokerOutSize)
                == IMdns::QueryResult::Found) {
            return true;
        }

        std::strncpy(brokerOut, fallback, brokerOutSize - 1);
        brokerOut[brokerOutSize - 1] = '\0';
        return false;
    }

} // namespace vigilo