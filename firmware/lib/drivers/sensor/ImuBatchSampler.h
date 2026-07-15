/**
 * @file ImuBatchSampler.h
 * @brief High-rate batched IMU sampling for FFT analysis.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-14
 */

#pragma once
#include "Imu.h"
#include "hal/IClock.h"
#include <cstdint>
#include <cstddef>

namespace vigilo {

    /**
     * @brief Collects a fixed-size batch of IMU samples at a precise interval.
     *
     * Paces itself using elapsed-time compensation: after each read, it
     * measures how long the read took and sleeps only the remainder of
     * sampleIntervalUs, so the actual I2C transaction time doesn't distort
     * the sample rate.
     */
    class ImuBatchSampler {
    public:
        static constexpr std::size_t BATCH_SIZE = 256;    ///< Fixed number of samples collected per batch.
        
        /**
         * @brief Constructs the sampler.
         * @param imu                 Already-initialized IMU driver to sample from.
         * @param clock               Injected timing interface.
         * @param sampleIntervalUs    Target time between samples, in microseconds.
         */
        explicit ImuBatchSampler(Imu& imu, IClock& clock, uint32_t sampleIntervalUs);

        ImuBatchSampler(const ImuBatchSampler&)            = delete; ///< Non-copyable.
        ImuBatchSampler& operator=(const ImuBatchSampler&) = delete; ///< Non-copyable.
        ImuBatchSampler(ImuBatchSampler&&)                 = delete; ///< Non-movable.
        ImuBatchSampler& operator=(ImuBatchSampler&&)      = delete; ///< Non-movable.

        /**
         * @brief Blocks until a full batch is collected or an IMU read fails.
         * @return Number of samples collected: BATCH_SIZE on success, or fewer
         *         if a read failed partway through.
         */
        [[nodiscard]] std::size_t collectBatch();

        /**
         * @brief Returns a pointer to the collected batch.
         * @return Pointer to the first of the collected samples, valid until the next collectBatch() call.
         */
        const ImuData* data() const noexcept { return _buffer; }

    private:
        Imu&         _imu;                ///< Injected, already-initialized IMU driver.
        IClock&      _clock;              ///< Injected timing interface.
        uint32_t     _sampleIntervalUs;   ///< Target time between samples.
        ImuData      _buffer[BATCH_SIZE]; ///< Collected samples.
    };

} // namespace vigilo