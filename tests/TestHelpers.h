//
// Created by zack on 9/1/25.
//

#ifndef ANVIL_TESTHELPERS_H
#define ANVIL_TESTHELPERS_H

#include "common/common.h"

// --- Types ---
typedef struct
{
    char name[50];
    int age;
} Person;

// --- Comparison Functions ---
int int_cmp(const void* a, const void* b);

// Custom comparison function for descending order
int int_cmp_desc(const void* a, const void* b);

// Person comparison function
int person_cmp(const void* a, const void* b);

// --- Memory Management ---
void int_free(void* a);

// Person free function
void person_free(void* p);

// Custom allocator for testing
void* test_calloc(size_t size);
void test_dealloc(void* ptr);

// --- Copy/Clone Helpers ---

// Clone an integer
void* int_copy(const void* data);

// Clone a string
void* string_copy(const void* data);

// Clone a person
void* person_copy(const void* data);

// --- Person Helpers ---
// Helper to create a person
Person* create_person(const char* name, int age);

// --- Predicate Functions ---
// Predicate function that returns non-zero for even numbers
bool is_even(const void* data);

// Predicate: is odd
bool is_odd(const void* data);

// Predicate: is greater than 5
bool is_greater_than_five(const void* data);

// Predicate: is greater than 10
bool is_greater_than_10(const void* data);

// Predicate: is greater than 20
bool is_greater_than_20(const void* data);

// Predicate: is divisible by 3
bool is_divisible_by_3(const void* data);

// Predicate: is divisible by 4
bool is_divisible_by_4(const void* data);

// Predicate: is divisible by 6
bool is_divisible_by_six(const void* data);

// --- Transform Functions ---
// Transform function that doubles a number
void* double_value(const void* data);

// Transform: square a number
void* square_func(const void* data);

// Transform: add 1 to a value
void* add_one(const void* data);

// Transform: add 5 to a value
void* add_five(const void* data);

// Transform: add 10 to a value
void* add_ten_func(const void* data);

// Transform: multiply by 3
void* multiply_by_three(const void* data);

// --- Action Functions ---
// Action function that increments a number
void increment(void* data);

// --- Failing Allocator Helpers ---
// A custom allocator that will fail after a certain number of calls
void* failing_alloc(size_t size);

void failing_free(void* ptr);

// A copy function that uses the failing allocator
void* failing_int_copy(const void* data);

// A transform function that uses the failing allocator
void* double_value_failing(const void* data);

// Helper to set up the failing allocator
void set_alloc_fail_countdown(int count);

ANVAllocator create_failing_int_allocator(void);
ANVAllocator create_int_allocator(void);
ANVAllocator create_person_allocator(void);
ANVAllocator create_string_allocator(void);

#endif //ANVIL_TESTHELPERS_H