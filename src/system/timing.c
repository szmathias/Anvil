//
// Created by zack on 9/27/25.
//


#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L
#endif

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE 1
#endif

#ifdef ANV_PLATFORM_WINDOWS
    #include <Windows.h>
#else
    #include <time.h>
#endif

#include "timing.h"

ANV_API uint64_t anv_time_get_ns(void)
{
    #ifdef ANV_PLATFORM_WINDOWS
    static LARGE_INTEGER frequency = {0};
    LARGE_INTEGER counter;

    if (frequency.QuadPart == 0)
    {
        if (!QueryPerformanceFrequency(&frequency))
        {
            return 0;
        }
    }

    if (!QueryPerformanceCounter(&counter))
    {
        return 0;
    }

    return (counter.QuadPart * 1000000000ULL) / frequency.QuadPart;

    #else
    struct timespec ts;

    // Try CLOCK_MONOTONIC first
    #ifdef CLOCK_MONOTONIC
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    {
        return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    }
    #endif

    // Try CLOCK_REALTIME as fallback
    #ifdef CLOCK_REALTIME
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
    {
        return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    }
    #endif

    // Final fallback to clock()
    const clock_t fallback_clock = clock();
    if (fallback_clock != (clock_t) - 1)
    {
        return (uint64_t)fallback_clock * (1000000000ULL / CLOCKS_PER_SEC);
    }

    // Complete failure
    return 0;
    #endif
}

ANV_API uint64_t anv_time_diff_ns(const uint64_t start_time, const uint64_t end_time)
{
    if (end_time >= start_time)
    {
        return end_time - start_time;
    }
    return 0;
}

ANV_API double anv_time_ns_to_seconds(const uint64_t nanoseconds)
{
    return (double)nanoseconds / 1000000000.0;
}

ANV_API double anv_time_ns_to_ms(const uint64_t nanoseconds)
{
    return (double)nanoseconds / 1000000.0;
}

ANV_API double anv_time_ns_to_us(const uint64_t nanoseconds)
{
    return (double)nanoseconds / 1000.0;
}

ANV_API uint64_t anv_time_seconds_to_ns(const double seconds)
{
    return (uint64_t)(seconds * 1000000000.0);
}

ANV_API uint64_t anv_time_ms_to_ns(const double milliseconds)
{
    return (uint64_t)(milliseconds * 1000000.0);
}

ANV_API uint64_t anv_time_us_to_ns(const double microseconds)
{
    return (uint64_t)(microseconds * 1000.0);
}

ANV_API double anv_time_convert(const uint64_t time_ns, const ANVTime target_unit)
{
    switch (target_unit)
    {
        case ANV_TIME_SECONDS:
            return anv_time_ns_to_seconds(time_ns);
        case ANV_TIME_MILLISECONDS:
            return anv_time_ns_to_ms(time_ns);
        case ANV_TIME_MICROSECONDS:
            return anv_time_ns_to_us(time_ns);
        case ANV_TIME_NANOSECONDS:
        default:
            return (double)time_ns;
    }
}

ANV_API uint64_t anv_time_convert_to_ns(const double time, const ANVTime source_unit)
{
    switch (source_unit)
    {
        case ANV_TIME_SECONDS:
            return anv_time_seconds_to_ns(time);
        case ANV_TIME_MILLISECONDS:
            return anv_time_ms_to_ns(time);
        case ANV_TIME_MICROSECONDS:
            return anv_time_us_to_ns(time);
        case ANV_TIME_NANOSECONDS:
        default:
            return (uint64_t)time;
    }
}