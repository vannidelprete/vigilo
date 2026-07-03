/**
 * @file ArduinoWire.h
 * @brief Arduino Wire implementation of IWire.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include "IWire.h"
#include <Wire.h>

namespace vigilo {

    /**
     * @brief Delegates IWire calls to the global Arduino Wire object.
     *
     * @note Wire.begin() must be called by main.cpp before constructing
     *       any driver that depends on this object.
     */
    class ArduinoWire : public IWire {
    public:
        /** @copydoc IWire::beginTransmission() */
        void beginTransmission(uint8_t addr) override { Wire.beginTransmission(addr); }

        /** @copydoc IWire::write() */
        void write(uint8_t data) override { Wire.write(data); }

        /** @copydoc IWire::endTransmission() */
        uint8_t endTransmission(bool stop = true) override { return Wire.endTransmission(stop); }

        /** @copydoc IWire::requestFrom() */
        uint8_t requestFrom(uint8_t addr, uint8_t quantity) override { return Wire.requestFrom(addr, quantity); }

        /** @copydoc IWire::read() */
        int read() override { return Wire.read(); }
    };

} // namespace vigilo