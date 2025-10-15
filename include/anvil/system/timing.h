//
// Created by zack on 9/27/25.
//

#ifndef ANVIL_TIMING_H
#define ANVIL_TIMING_H

#include "anvil/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Time units.
 */
typedef enum ANVTime
{
    ANV_TIME_NANOSECONDS,
    ANV_TIME_MICROSECONDS,
    ANV_TIME_MILLISECONDS,
    ANV_TIME_SECONDS,
    ANV_TIME_COUNT // Helper to track number of time units
} ANVTime;


/**
 * Get high-resolution timestamp in nanoseconds.
 * Uses the most accurate timer available on the platform.
 *
 * @return Time in nanoseconds since an arbitrary epoch, or 0 on complete failure
 *
 * Notes:
 * - On Windows uses QueryPerformanceCounter for high precision
 * - On POSIX uses clock_gettime(CLOCK_MONOTONIC) when available
 * - Falls back to clock() if high-resolution timers are unavailable
 * - The returned value is only meaningful for calculating differences
 */
ANV_API uint64_t anv_time_get_ns(void);

/**
 * Get time difference in nanoseconds.
 *
 * @param start_time Start timestamp from anv_get_time_ns()
 * @param end_time End timestamp from anv_get_time_ns()
 * @return Difference in nanoseconds (end_time - start_time)
 */
ANV_API uint64_t anv_time_diff_ns(uint64_t start_time, uint64_t end_time);

/**
 * Convert time from nanoseconds to seconds.
 *
 * @param nanoseconds Time in nanoseconds
 * @return Time in seconds as double
 */
ANV_API double anv_time_ns_to_seconds(uint64_t nanoseconds);

/**
 * Convert time from nanoseconds to milliseconds.
 *
 * @param nanoseconds Time in nanoseconds
 * @return Time in milliseconds as double
 */
ANV_API double anv_time_ns_to_ms(uint64_t nanoseconds);

/**
 * Convert time from nanoseconds to microseconds.
 *
 * @param nanoseconds Time in nanoseconds
 * @return Time in microseconds as double
 */
ANV_API double anv_time_ns_to_us(uint64_t nanoseconds);

/**
 * Convert time from seconds to nanoseconds.
 *
 * @param seconds Time in seconds
 * @return Time in nanoseconds as uint64_t
 */
ANV_API uint64_t anv_time_seconds_to_ns(double seconds);

/**
 * Convert time from milliseconds to nanoseconds.
 *
 * @param milliseconds Time in milliseconds
 * @return Time in nanoseconds as uint64_t
 */
ANV_API uint64_t anv_time_ms_to_ns(double milliseconds);

/**
 * Convert time from microseconds to nanoseconds.
 *
 * @param microseconds Time in microseconds
 * @return Time in nanoseconds as uint64_t
 */
ANV_API uint64_t anv_time_us_to_ns(double microseconds);

/**
 * Convert time from nanoseconds to specified target unit.
 *
 * @param time_ns Time in nanoseconds
 * @param target_unit Target time unit
 * @return Converted time in target unit as double
 */
ANV_API double anv_time_convert(uint64_t time_ns, ANVTime target_unit);

/**
 * Convert time from specified source unit to nanoseconds.
 *
 * @param time Time in source unit
 * @param source_unit Source time unit
 * @return Converted time in nanoseconds as uint64_t
 */
ANV_API uint64_t anv_time_convert_to_ns(double time, ANVTime source_unit);

#ifdef __cplusplus
}
#endif

#endif //ANVIL_TIMING_H
