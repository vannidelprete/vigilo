/**
 * @file Platform.h
 * @brief Platform abstraction macros for cross-compilation between Arduino and native host.
 * @author Giovanni Del Prete (giovannidelprete95@gmail.com)
 * @date 2026-07-03
 */

#pragma once

#ifdef ARDUINO
    #include <Arduino.h>  // provides IRAM_ATTR, noInterrupts(), interrupts()
#else
    #define IRAM_ATTR
    #define noInterrupts() ((void)0)
    #define interrupts()   ((void)0)
#endif