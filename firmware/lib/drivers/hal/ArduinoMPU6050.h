/**
 * @file ArduinoMPU6050.h
 * @brief Arduino MPU6050 library implementation of IMPU6050.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once
#include "IMPU6050.h"
#include <MPU6050.h>

namespace vigilo {

    /** @brief Delegates IMPU6050 calls to the electroniccats MPU6050 library. */
    class ArduinoMPU6050 : public IMPU6050 {
    public:
        /** @copydoc IMPU6050::initialize() */
        void initialize() override { _mpu.initialize(); }

        /** @copydoc IMPU6050::testConnection() */
        bool testConnection() override { return _mpu.testConnection(); }

    private:
        MPU6050 _mpu; ///< Underlying library object.
    };

} // namespace vigilo