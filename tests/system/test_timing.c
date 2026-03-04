#include <anvil/testing.h>
#include "system/timing.h"

//==============================================================================
// Timestamp Tests
//==============================================================================

int test_timing_get_ns_nonzero(void)
{
    const uint64_t t = anv_time_get_ns();
    ASSERT_NOT_EQ(t, 0);
    return TEST_SUCCESS;
}

int test_timing_get_ns_monotonic(void)
{
    const uint64_t t1 = anv_time_get_ns();
    // Tiny busy-loop to ensure time passes
    volatile int dummy = 0;
    for (int i = 0; i < 10000; i++)
    {
        dummy += i;
    }
    const uint64_t t2 = anv_time_get_ns();
    ASSERT_GTE(t2, t1);
    return TEST_SUCCESS;
}

int test_timing_diff_ns(void)
{
    ASSERT_EQ(anv_time_diff_ns(100, 200), 100);
    ASSERT_EQ(anv_time_diff_ns(0, 1000000000ULL), 1000000000ULL);
    ASSERT_EQ(anv_time_diff_ns(500, 500), 0);
    return TEST_SUCCESS;
}

int test_timing_diff_ns_reversed(void)
{
    // When end < start, should return 0
    ASSERT_EQ(anv_time_diff_ns(200, 100), 0);
    return TEST_SUCCESS;
}

//==============================================================================
// Conversion Tests
//==============================================================================

int test_timing_ns_to_seconds(void)
{
    ASSERT_EQ_FLOAT(anv_time_ns_to_seconds(1000000000ULL), 1.0, 1e-9);
    ASSERT_EQ_FLOAT(anv_time_ns_to_seconds(500000000ULL), 0.5, 1e-9);
    ASSERT_EQ_FLOAT(anv_time_ns_to_seconds(0), 0.0, 1e-9);
    return TEST_SUCCESS;
}

int test_timing_ns_to_ms(void)
{
    ASSERT_EQ_FLOAT(anv_time_ns_to_ms(1000000ULL), 1.0, 1e-6);
    ASSERT_EQ_FLOAT(anv_time_ns_to_ms(500000ULL), 0.5, 1e-6);
    ASSERT_EQ_FLOAT(anv_time_ns_to_ms(0), 0.0, 1e-6);
    return TEST_SUCCESS;
}

int test_timing_ns_to_us(void)
{
    ASSERT_EQ_FLOAT(anv_time_ns_to_us(1000ULL), 1.0, 1e-3);
    ASSERT_EQ_FLOAT(anv_time_ns_to_us(500ULL), 0.5, 1e-3);
    ASSERT_EQ_FLOAT(anv_time_ns_to_us(0), 0.0, 1e-3);
    return TEST_SUCCESS;
}

int test_timing_seconds_to_ns(void)
{
    ASSERT_EQ(anv_time_seconds_to_ns(1.0), 1000000000ULL);
    ASSERT_EQ(anv_time_seconds_to_ns(0.5), 500000000ULL);
    ASSERT_EQ(anv_time_seconds_to_ns(0.0), 0);
    return TEST_SUCCESS;
}

int test_timing_ms_to_ns(void)
{
    ASSERT_EQ(anv_time_ms_to_ns(1.0), 1000000ULL);
    ASSERT_EQ(anv_time_ms_to_ns(0.5), 500000ULL);
    ASSERT_EQ(anv_time_ms_to_ns(0.0), 0);
    return TEST_SUCCESS;
}

int test_timing_us_to_ns(void)
{
    ASSERT_EQ(anv_time_us_to_ns(1.0), 1000ULL);
    ASSERT_EQ(anv_time_us_to_ns(0.5), 500ULL);
    ASSERT_EQ(anv_time_us_to_ns(0.0), 0);
    return TEST_SUCCESS;
}

//==============================================================================
// Roundtrip Conversion Tests
//==============================================================================

int test_timing_roundtrip_seconds(void)
{
    const uint64_t ns = 1234567890ULL;
    const double sec = anv_time_ns_to_seconds(ns);
    const uint64_t back = anv_time_seconds_to_ns(sec);
    // Allow 1ns tolerance for floating point
    ASSERT_LTE(ns > back ? ns - back : back - ns, 1);
    return TEST_SUCCESS;
}

int test_timing_roundtrip_ms(void)
{
    const uint64_t ns = 123456000ULL;
    const double ms = anv_time_ns_to_ms(ns);
    const uint64_t back = anv_time_ms_to_ns(ms);
    ASSERT_LTE(ns > back ? ns - back : back - ns, 1);
    return TEST_SUCCESS;
}

int test_timing_roundtrip_us(void)
{
    const uint64_t ns = 123000ULL;
    const double us = anv_time_ns_to_us(ns);
    const uint64_t back = anv_time_us_to_ns(us);
    ASSERT_LTE(ns > back ? ns - back : back - ns, 1);
    return TEST_SUCCESS;
}

//==============================================================================
// Generic Conversion Tests
//==============================================================================

int test_timing_convert_all_units(void)
{
    const uint64_t ns = 2000000000ULL; // 2 seconds

    ASSERT_EQ_FLOAT(anv_time_convert(ns, ANV_TIME_NANOSECONDS), (double)ns, 1.0);
    ASSERT_EQ_FLOAT(anv_time_convert(ns, ANV_TIME_MICROSECONDS), 2000000.0, 1e-3);
    ASSERT_EQ_FLOAT(anv_time_convert(ns, ANV_TIME_MILLISECONDS), 2000.0, 1e-6);
    ASSERT_EQ_FLOAT(anv_time_convert(ns, ANV_TIME_SECONDS), 2.0, 1e-9);
    return TEST_SUCCESS;
}

int test_timing_convert_to_ns_all_units(void)
{
    ASSERT_EQ(anv_time_convert_to_ns(2.0, ANV_TIME_SECONDS), 2000000000ULL);
    ASSERT_EQ(anv_time_convert_to_ns(2.0, ANV_TIME_MILLISECONDS), 2000000ULL);
    ASSERT_EQ(anv_time_convert_to_ns(2.0, ANV_TIME_MICROSECONDS), 2000ULL);
    ASSERT_EQ(anv_time_convert_to_ns(2.0, ANV_TIME_NANOSECONDS), 2);
    return TEST_SUCCESS;
}

int test_timing_convert_invalid_unit(void)
{
    // Invalid unit should fall through to nanoseconds (default case)
    const uint64_t ns = 12345ULL;
    ASSERT_EQ_FLOAT(anv_time_convert(ns, (ANVTime)999), (double)ns, 1.0);
    ASSERT_EQ(anv_time_convert_to_ns(12345.0, (ANVTime)999), 12345);
    return TEST_SUCCESS;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    const ANVTestCase tests[] = {
        // Timestamp
        TEST_REGISTER(test_timing_get_ns_nonzero),
        TEST_REGISTER(test_timing_get_ns_monotonic),
        TEST_REGISTER(test_timing_diff_ns),
        TEST_REGISTER(test_timing_diff_ns_reversed),

        // Conversions
        TEST_REGISTER(test_timing_ns_to_seconds),
        TEST_REGISTER(test_timing_ns_to_ms),
        TEST_REGISTER(test_timing_ns_to_us),
        TEST_REGISTER(test_timing_seconds_to_ns),
        TEST_REGISTER(test_timing_ms_to_ns),
        TEST_REGISTER(test_timing_us_to_ns),

        // Roundtrips
        TEST_REGISTER(test_timing_roundtrip_seconds),
        TEST_REGISTER(test_timing_roundtrip_ms),
        TEST_REGISTER(test_timing_roundtrip_us),

        // Generic conversions
        TEST_REGISTER(test_timing_convert_all_units),
        TEST_REGISTER(test_timing_convert_to_ns_all_units),
        TEST_REGISTER(test_timing_convert_invalid_unit),
    };

    return anv_run_tests("Timing", tests, sizeof(tests) / sizeof(tests[0]));
}

