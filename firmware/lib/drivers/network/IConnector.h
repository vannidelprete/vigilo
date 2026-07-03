/**
 * @file IConnector.h
 * @brief Abstract interface for network connectivity.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include <cstdint>

namespace vigilo {

    /**
     * @brief Result code returned by IConnector::connect().
     */
    enum class ConnectResult : uint8_t {
        Ok              = 0, ///< Connection established successfully.
        NetworkNotFound = 1, ///< SSID not found.
        AuthFailed      = 2, ///< Wrong credentials.
        Timeout         = 3, ///< Connection attempt timed out.
        HardwareError   = 4, ///< Network hardware failure.
    };

    /**
     * @brief Abstract interface for network connectors.
     *
     * Lifecycle contract: establish connection with connect(), then verify
     * with isConnected() before sending data.
     */
    class IConnector {
    public:
        /**
         * @brief Establishes the network connection.
         * @return ConnectResult::Ok on success, or a specific error code identifying the failure.
         */
        [[nodiscard]] virtual ConnectResult connect() = 0;

        /**
         * @brief Returns whether the network connection is currently active.
         * @return true if connected.
         */
        [[nodiscard]] virtual bool isConnected() const noexcept = 0;

         /** @brief Virtual destructor. */
        virtual ~IConnector() = default;

    protected:
        IConnector() = default;
    };

} // namespace vigilo