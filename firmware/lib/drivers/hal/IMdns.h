/**
 * @file IMdns.h
 * @brief Abstract interface for mDNS service discovery.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-09-02
 */

#pragma once
#include <cstddef>

namespace vigilo {

/**
 * @brief Abstract interface for mDNS service discovery.
 *
 * Wraps ESPmDNS's begin()/queryService() so MqttDiscovery is decoupled
 * from the Arduino mDNS library and can be tested on the host.
 */
class IMdns {
public:
    /** @brief Result code returned by queryService(). */
    enum class QueryResult {
        Found       = 0, ///< A matching service was found; addressOut was written.
        NotFound    = 1, ///< No matching service was found; addressOut is unchanged.
    };

    /**
     * @brief Starts the mDNS responder with the given hostname.
     * 
     * @param hostname  Local hostname to advertise (without ".local")
     * @return true on success.
     */
    [[nodiscard]] virtual bool begin(const char* hostname) = 0;

    /**
     * @brief Queries the network for the first instance of a service.
     * 
     * @param service           Service name without leading underscore (e.g. "mqtt").
     * @param proto             Protocol without leading undescore (e.g. "tcp").
     * @param addressOut        Buffer written with the resolved address as a null-terminated string, only when Found is returned.
     * @param addressOutSize    Capacity of addressOut, including the null terminator.
     * @return QueryResult::Found if at least one instance was resolved, QueryResult::NotFound otherwise.
     */
    [[nodiscard]] virtual QueryResult queryService(const char* service, const char* proto,
                                                    char* addressOut, std::size_t addressOutSize) = 0;
    
    /** @brief Virtual destructor. */
    virtual ~IMdns() = default;

protected:
    IMdns() = default;
};

} // namespace vigilo