/**
 * @file Tachometer.cpp
 * @brief RPM tachometer driver implementation.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include "Tachometer.h"
#include "config.h"
#include <Arduino.h>
#include <esp_attr.h>

namespace vigilo {

    volatile uint32_t Tachometer::_pulseCount = 0;

    Tachometer::Tachometer(uint8_t pin) : _pin(pin) {}

    InitResult Tachometer::begin() {
        if (digitalPinToInterrupt(_pin) == NOT_AN_INTERRUPT) return InitResult::InvalidPin;
        pinMode(_pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(_pin), isr, FALLING);
        _lastMs = millis();
        _ready  = true;
        return InitResult::Ok;
    }

    bool Tachometer::isReady() const noexcept {
        return _ready;
    }

    float Tachometer::getRpm() noexcept {
        if (!_ready) return 0.0f;
        
        uint32_t now = millis();
        if (now - _lastMs < config::RPM_INTERVAL_MS) {
            return _rpm;
        }

        noInterrupts();
        uint32_t count = _pulseCount;
        _pulseCount = 0;
        interrupts();

        _rpm = (count / 2.0f) * 60.0f;
        _lastMs = now;
        
        return _rpm;
    }

    void IRAM_ATTR Tachometer::isr() {
        _pulseCount++;
    }

} // namespace vigilo