//
// Created by zack on 9/27/25.
//

#include "assertions.h"

// src/testing/assertions.c - Unchanged from previous implementation
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Color codes for output (if supported)
#ifdef ANV_PLATFORM_WINDOWS
    #define FG_COLOR_BLACK   ""
    #define FG_COLOR_RED     ""
    #define FG_COLOR_GREEN   ""
    #define FG_COLOR_YELLOW  ""
    #define FG_COLOR_BLUE    ""
    #define FG_COLOR_MAGENTA ""
    #define FG_COLOR_CYAN    ""
    #define FG_COLOR_WHITE   ""

    #define BG_COLOR_BLACK   ""
    #define BG_COLOR_RED     ""
    #define BG_COLOR_GREEN   ""
    #define BG_COLOR_YELLOW  ""
    #define BG_COLOR_BLUE    ""
    #define BG_COLOR_MAGENTA ""
    #define BG_COLOR_CYAN    ""
    #define BG_COLOR_WHITE   ""

    #define COLOR_RESET   ""
#else
    #define FG_COLOR_BLACK   "\x1B[30m"
    #define FG_COLOR_RED     "\x1B[31m"
    #define FG_COLOR_GREEN   "\x1B[32m"
    #define FG_COLOR_YELLOW  "\x1B[33m"
    #define FG_COLOR_BLUE    "\x1B[34m"
    #define FG_COLOR_MAGENTA "\x1B[35m"
    #define FG_COLOR_CYAN    "\x1B[36m"
    #define FG_COLOR_WHITE   "\x1B[37m"

    #define BG_COLOR_BLACK   "\x1B[40m"
    #define BG_COLOR_RED     "\x1B[41m"
    #define BG_COLOR_GREEN   "\x1B[42m"
    #define BG_COLOR_YELLOW  "\x1B[43m"
    #define BG_COLOR_BLUE    "\x1B[44m"
    #define BG_COLOR_MAGENTA "\x1B[45m"
    #define BG_COLOR_CYAN    "\x1B[46m"
    #define BG_COLOR_WHITE   "\x1B[47m"

    #define COLOR_RESET      "\x1B[0m"
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define ANV_PRINTF_LIKE(fmt, args) __attribute__((format(printf, 2, 3)))
#else
    #define ANV_PRINTF_LIKE(fmt, args)
#endif

ANV_PRINTF_LIKE(2, 3)
static char* anv_format_alloc(const ANVAllocator* alloc, const char* format, ...)
{
    va_list args, args_copy;
    va_start(args, format);
    va_copy(args_copy, args);

    const int needed = vsnprintf(NULL, 0, format, args) + 1;
    char* buffer = anv_alloc_malloc(alloc, needed);
    if (buffer)
    {
        vsnprintf(buffer, needed, format, args_copy);
    }

    va_end(args_copy);
    va_end(args);
    return buffer;
}

static void record_assertion(ANVTestCase* test, const bool passed)
{
    if (!test || !test->result)
    {
        return;
    }

    test->result->assertions_run++;
    if (passed)
    {
        test->result->assertions_passed++;
    }
    else
    {
        test->result->assertions_failed++;
    }
}

ANV_API ANVAssertionResult anv_assert_impl(ANVTestCase* test, const bool condition, const char* expression,
                             const char* message, const char* file, const int line)
{
    record_assertion(test, condition);

    if (!condition)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expression: %s\n", expression);
    }
}

ANV_API ANVAssertionResult anv_assert_equal_ptr_impl(ANVTestCase* test, const void* expected, const void* actual,
                                       const char* message, const char* file, const int line)
{
    const bool passed = expected == actual;
    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected: %p\n", expected);
        printf("  Actual:   %p\n", actual);
    }
}

ANV_API ANVAssertionResult anv_assert_not_equal_ptr_impl(ANVTestCase* test, const void* expected, const void* actual,
                                           const char* message, const char* file, const int line)
{
    const bool passed = expected != actual;
    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected NOT: %p\n", expected);
        printf("  Actual:       %p\n", actual);
    }
}

ANV_API ANVAssertionResult anv_assert_equal_int_impl(ANVTestCase* test, const long expected, const long actual,
                                       const char* message, const char* file, const int line)
{
    const bool passed = expected == actual;
    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected: %ld\n", expected);
        printf("  Actual:   %ld\n", actual);
    }
}

ANV_API ANVAssertionResult anv_assert_not_equal_int_impl(ANVTestCase* test, const long expected, const long actual,
                                           const char* message, const char* file, const int line)
{
    const bool passed = expected != actual;
    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected NOT: %ld\n", expected);
        printf("  Actual:       %ld\n", actual);
    }
}

ANV_API ANVAssertionResult anv_assert_greater_int_impl(ANVTestCase* test, const long actual, const long minimum,
                                         const char* message, const char* file, const int line)
{
    const bool passed = actual > minimum;
    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected > %ld\n", minimum);
        printf("  Actual:    %ld\n", actual);
    }
}

ANV_API ANVAssertionResult anv_assert_less_int_impl(ANVTestCase* test, const long actual, const long maximum,
                                      const char* message, const char* file, const int line)
{
    const bool passed = actual < maximum;
    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected < %ld\n", maximum);
        printf("  Actual:    %ld\n", actual);
    }
}

ANV_API ANVAssertionResult anv_assert_greater_equal_int_impl(ANVTestCase* test, const long actual, const long minimum,
                                               const char* message, const char* file, const int line)
{
    const bool passed = actual >= minimum;
    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected >= %ld\n", minimum);
        printf("  Actual:     %ld\n", actual);
    }
}

ANV_API ANVAssertionResult anv_assert_less_equal_int_impl(ANVTestCase* test, const long actual, const long maximum,
                                            const char* message, const char* file, const int line)
{
    const bool passed = actual <= maximum;
    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected <= %ld\n", maximum);
        printf("  Actual:     %ld\n", actual);
    }
}

ANV_API ANVAssertionResult anv_assert_equal_str_impl(ANVTestCase* test, const char* expected, const char* actual,
                                       const char* message, const char* file, const int line)
{
    bool passed = false;

    if (expected == NULL && actual == NULL)
    {
        passed = true;
    }
    else if (expected != NULL && actual != NULL)
    {
        passed = strcmp(expected, actual) == 0;
    }

    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected: \"%s\"\n", expected ? expected : "(null)");
        printf("  Actual:   \"%s\"\n", actual ? actual : "(null)");
    }
}

ANV_API ANVAssertionResult anv_assert_not_equal_str_impl(ANVTestCase* test, const char* expected, const char* actual,
                                           const char* message, const char* file, const int line)
{
    bool passed = false;

    if (expected == NULL && actual != NULL)
    {
        passed = true;
    }
    else if (expected != NULL && actual == NULL)
    {
        passed = true;
    }
    else if (expected != NULL && actual != NULL)
    {
        passed = strcmp(expected, actual) != 0;
    }

    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected NOT: \"%s\"\n", expected ? expected : "(null)");
        printf("  Actual:       \"%s\"\n", actual ? actual : "(null)");
    }
}

ANV_API ANVAssertionResult anv_assert_equal_mem_impl(ANVTestCase* test, const void* expected, const void* actual,
                                       const size_t size, const char* message, const char* file, const int line)
{
    bool passed = false;

    if (expected == NULL && actual == NULL)
    {
        passed = true;
    }
    else if (expected != NULL && actual != NULL)
    {
        passed = memcmp(expected, actual, size) == 0;
    }

    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Memory comparison failed (%zu bytes)\n", size);
        printf("  Expected: %p\n", expected);
        printf("  Actual:   %p\n", actual);
    }
}

ANV_API ANVAssertionResult anv_assert_not_equal_mem_impl(ANVTestCase* test, const void* expected, const void* actual,
                                           const size_t size, const char* message, const char* file, const int line)
{
    bool passed = false;

    if (expected == NULL && actual != NULL)
    {
        passed = true;
    }
    else if (expected != NULL && actual == NULL)
    {
        passed = true;
    }
    else if (expected != NULL && actual != NULL)
    {
        passed = (memcmp(expected, actual, size) != 0);
    }

    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Memory should NOT be equal (%zu bytes)\n", size);
    }
}

ANV_API ANVAssertionResult anv_assert_equal_float_impl(ANVTestCase* test, const double expected, const double actual, const double epsilon,
                                         const char* message, const char* file, const int line)
{
    const bool passed = fabs(expected - actual) <= epsilon;
    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected: %f (±%f)\n", expected, epsilon);
        printf("  Actual:   %f\n", actual);
        printf("  Diff:     %f\n", fabs(expected - actual));
    }
}

ANV_API ANVAssertionResult anv_assert_not_equal_float_impl(ANVTestCase* test, const double expected, const double actual, const double epsilon,
                                             const char* message, const char* file, const int line)
{
    const bool passed = fabs(expected - actual) > epsilon;
    record_assertion(test, passed);

    if (!passed)
    {
        printf("%s[FAIL]%s %s:%d - %s\n", FG_COLOR_RED, COLOR_RESET, file, line, message);
        printf("  Expected NOT: %f (±%f)\n", expected, epsilon);
        printf("  Actual:       %f\n", actual);
        printf("  Diff:         %f\n", fabs(expected - actual));
    }
}
