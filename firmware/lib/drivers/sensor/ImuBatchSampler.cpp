/**
 * @file ImuBatchSampler.cpp
 * @brief Implementation of ImuBatchSampler.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-14
 */

#include "ImuBatchSampler.h"

namespace vigilo {

    ImuBatchSampler::ImuBatchSampler(Imu& imu, IClock& clock, uint32_t sampleIntervalUs)
        : _imu(imu), _clock(clock), _sampleIntervalUs(sampleIntervalUs)
    {}

    std::size_t ImuBatchSampler::collectBatch() {
        for (std::size_t i = 0; i < BATCH_SIZE; i++) {
            const uint32_t start = _clock.micros();

            if (_imu.read(_buffer[i]) != Imu::ReadResult::Ok)
            {
                return i;
            }

            const uint32_t elapsed = _clock.micros() - start;
            if (elapsed < _sampleIntervalUs) {
                _clock.delayMicros(_sampleIntervalUs - elapsed);
            }
        }
        return BATCH_SIZE;
    }

} // namespace vigilo