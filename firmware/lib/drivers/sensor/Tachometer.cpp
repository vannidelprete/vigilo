/**
 * @file Tachometer.cpp
 * @brief RPM tachometer driver implementation.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#include "Tachometer.h"
#include "hal/Platform.h"

namespace vigilo {

    volatile uint32_t Tachometer::_pulseCount = 0;

    Tachometer::Tachometer(uint8_t pin, IClock& clock, IGpio& gpio, uint32_t intervalMs)
    : _pin(pin), _clock(clock), _gpio(gpio), _intervalMs(intervalMs) {}

    InitResult Tachometer::begin() {
        if (!_gpio.supportsInterrupt(_pin)) return InitResult::InvalidPin;
        _gpio.pinMode(_pin, IGpio::PinMode::InputPullUp);
        _gpio.attachInterrupt(_pin, isr, IGpio::Trigger::Falling);
        _lastMs = _clock.millis();
        _ready  = true;
        return InitResult::Ok;
    }

    bool Tachometer::isReady() const noexcept {
        return _ready;
    }

    float Tachometer::getRpm() noexcept {
        if (!_ready) return 0.0f;

        const uint32_t now = _clock.millis();
        const uint32_t elapsedMs = now - _lastMs;
        if (elapsedMs < _intervalMs) return _rpm;

        noInterrupts();
        uint32_t count = _pulseCount;
        _pulseCount    = 0;
        interrupts();

        _rpm    = (count / 2.0f) * (60000.0f / elapsedMs);
        _lastMs = now;
        return _rpm;
    }

    void IRAM_ATTR Tachometer::isr() { _pulseCount = _pulseCount + 1; }

} // namespace vigilo