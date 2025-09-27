//
// Created by zack on 9/26/25.
//

#ifndef ANVIL_ASSERTIONS_H
#define ANVIL_ASSERTIONS_H

#include "suite.h"

#ifdef __cplusplus
extern "C" {
#endif

// Internal assertion function - don't call directly
void _anv_assert_impl(ANVTestCase* test, bool condition, const char* expression,
                      const char* message, const char* file, int line);

// Internal assertion function - don't call directly
void _anv_assert_impl(ANVTestCase* test, bool condition, const char* expression,
                      const char* message, const char* file, int line);

// Basic assertions
#define ANV_ASSERT(condition, message) \
    _anv_assert_impl(test, (condition), #condition, (message), __FILE__, __LINE__)

#define ANV_ASSERT_TRUE(condition, message) \
    ANV_ASSERT((condition), (message))

#define ANV_ASSERT_FALSE(condition, message) \
    ANV_ASSERT(!(condition), (message))

// Null checks
#define ANV_ASSERT_NULL(ptr, message) \
    ANV_ASSERT((ptr) == NULL, (message))

#define ANV_ASSERT_NOT_NULL(ptr, message) \
    ANV_ASSERT((ptr) != NULL, (message))

// Pointer comparisons
#define ANV_ASSERT_EQUAL_PTR(expected, actual, message) \
    _anv_assert_equal_ptr_impl(test, (expected), (actual), (message), __FILE__, __LINE__)

#define ANV_ASSERT_NOT_EQUAL_PTR(expected, actual, message) \
    _anv_assert_not_equal_ptr_impl(test, (expected), (actual), (message), __FILE__, __LINE__)

// Integer comparisons
#define ANV_ASSERT_EQUAL_INT(expected, actual, message) \
    _anv_assert_equal_int_impl(test, (expected), (actual), (message), __FILE__, __LINE__)

#define ANV_ASSERT_NOT_EQUAL_INT(expected, actual, message) \
    _anv_assert_not_equal_int_impl(test, (expected), (actual), (message), __FILE__, __LINE__)

#define ANV_ASSERT_GREATER_INT(actual, minimum, message) \
    _anv_assert_greater_int_impl(test, (actual), (minimum), (message), __FILE__, __LINE__)

#define ANV_ASSERT_LESS_INT(actual, maximum, message) \
    _anv_assert_less_int_impl(test, (actual), (maximum), (message), __FILE__, __LINE__)

#define ANV_ASSERT_GREATER_EQUAL_INT(actual, minimum, message) \
    _anv_assert_greater_equal_int_impl(test, (actual), (minimum), (message), __FILE__, __LINE__)

#define ANV_ASSERT_LESS_EQUAL_INT(actual, maximum, message) \
    _anv_assert_less_equal_int_impl(test, (actual), (maximum), (message), __FILE__, __LINE__)

// String comparisons
#define ANV_ASSERT_EQUAL_STR(expected, actual, message) \
    _anv_assert_equal_str_impl(test, (expected), (actual), (message), __FILE__, __LINE__)

#define ANV_ASSERT_NOT_EQUAL_STR(expected, actual, message) \
    _anv_assert_not_equal_str_impl(test, (expected), (actual), (message), __FILE__, __LINE__)

// Memory comparisons
#define ANV_ASSERT_EQUAL_MEM(expected, actual, size, message) \
    _anv_assert_equal_mem_impl(test, (expected), (actual), (size), (message), __FILE__, __LINE__)

#define ANV_ASSERT_NOT_EQUAL_MEM(expected, actual, size, message) \
    _anv_assert_not_equal_mem_impl(test, (expected), (actual), (size), (message), __FILE__, __LINE__)

// Float comparisons (with epsilon)
#define ANV_ASSERT_EQUAL_FLOAT(expected, actual, epsilon, message) \
    _anv_assert_equal_float_impl(test, (expected), (actual), (epsilon), (message), __FILE__, __LINE__)

#define ANV_ASSERT_NOT_EQUAL_FLOAT(expected, actual, epsilon, message) \
    _anv_assert_not_equal_float_impl(test, (expected), (actual), (epsilon), (message), __FILE__, __LINE__)

// Implementation functions
void _anv_assert_equal_ptr_impl(ANVTestCase* test, const void* expected, const void* actual,
                                const char* message, const char* file, int line);

void _anv_assert_not_equal_ptr_impl(ANVTestCase* test, const void* expected, const void* actual,
                                    const char* message, const char* file, int line);

void _anv_assert_equal_int_impl(ANVTestCase* test, long expected, long actual,
                                const char* message, const char* file, int line);

void _anv_assert_not_equal_int_impl(ANVTestCase* test, long expected, long actual,
                                    const char* message, const char* file, int line);

void _anv_assert_greater_int_impl(ANVTestCase* test, long actual, long minimum,
                                  const char* message, const char* file, int line);

void _anv_assert_less_int_impl(ANVTestCase* test, long actual, long maximum,
                               const char* message, const char* file, int line);

void _anv_assert_greater_equal_int_impl(ANVTestCase* test, long actual, long minimum,
                                        const char* message, const char* file, int line);

void _anv_assert_less_equal_int_impl(ANVTestCase* test, long actual, long maximum,
                                     const char* message, const char* file, int line);

void _anv_assert_equal_str_impl(ANVTestCase* test, const char* expected, const char* actual,
                                const char* message, const char* file, int line);

void _anv_assert_not_equal_str_impl(ANVTestCase* test, const char* expected, const char* actual,
                                    const char* message, const char* file, int line);

void _anv_assert_equal_mem_impl(ANVTestCase* test, const void* expected, const void* actual,
                                size_t size, const char* message, const char* file, int line);

void _anv_assert_not_equal_mem_impl(ANVTestCase* test, const void* expected, const void* actual,
                                    size_t size, const char* message, const char* file, int line);

void _anv_assert_equal_float_impl(ANVTestCase* test, double expected, double actual, double epsilon,
                                  const char* message, const char* file, int line);

void _anv_assert_not_equal_float_impl(ANVTestCase* test, double expected, double actual, double epsilon,
                                      const char* message, const char* file, int line);

#ifdef __cplusplus
}
#endif

#endif // ANVIL_ASSERTIONS_H
