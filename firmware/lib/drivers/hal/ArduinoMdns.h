/**
 * @file ArduinoMdns.h
 * @brief Arduino ESPmDNS library implementation of IMdns.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-09-02
 */

#pragma once
#include "IMdns.h"
#include <ESPmDNS.h>
#include <cstring>

namespace vigilo {

    class ArduinoMdns : public IMdns {
    public:
        /** @copydoc IMdns::begin() */
        bool begin(const char* hostname) override {
            return MDNS.begin(hostname);
        }

        QueryResult queryService(const char* service, const char* proto,
                                 char* addressOut, std::size_t addressOutSize) override {
            if (MDNS.queryService(service, proto) == 0) {
                return QueryResult::NotFound;
            }
            std::strncpy(addressOut, MDNS.IP(0).toString().c_str(), addressOutSize - 1);
            addressOut[addressOutSize - 1] = '\0';
            return QueryResult::Found;
        }
    };

} // namespace vigilo