#include <stdlib.h>
#include <string.h>

#include <anvil/testing.h>
#include "TestAssert.h"
#include "TestHelpers.h"

//==============================================================================
// Automatic test framework hook registration
//==============================================================================

// Register the failing-allocator reset as a between-test hook so that
// anv_run_tests() (which lives in the Anvil DLL) can call it.
#if defined(_MSC_VER)
// MSVC: use CRT initializer table
#pragma section(".CRT$XCU", read)
static void __cdecl anv_test_auto_init(void)
{
    anv_test_set_between_hook(reset_alloc_fail_state);
}
__declspec(allocate(".CRT$XCU")) static void (__cdecl *anv_test_auto_init_ptr)(void) = anv_test_auto_init;
#elif defined(__GNUC__) || defined(__clang__)
__attribute__ ((constructor))

static void anv_test_auto_init(void)
{
    anv_test_set_between_hook(reset_alloc_fail_state);
}
#else
// Fallback: user must call anv_test_set_between_hook(reset_alloc_fail_state) manually
#endif

//==============================================================================
// Integer Allocation Helpers
//==============================================================================

int* make_int(const int value)
{
    int* p = malloc(sizeof(int));
    if (p)
    {
        *p = value;
    }
    return p;
}

int** make_int_array(const int* values, const int count)
{
    int** arr = malloc(sizeof(int*) * count);
    if (!arr)
        return NULL;
    for (int i = 0; i < count; i++)
    {
        arr[i] = make_int(values[i]);
        if (!arr[i])
        {
            // Cleanup on failure
            for (int j = 0; j < i; j++)
                free(arr[j]);
            free(arr);
            return NULL;
        }
    }
    return arr;
}

//==============================================================================
// Comparison Functions
//==============================================================================

// Integer comparison function
int int_cmp(const void* a, const void* b)
{
    return *(int*)a - *(int*)b;
}

// Custom comparison function for descending order
int int_cmp_desc(const void* a, const void* b)
{
    return *(int*)b - *(int*)a;
}

// Person comparison function
int person_cmp(const void* a, const void* b)
{
    const Person* p1 = a;
    const Person* p2 = b;
    return strcmp(p1->name, p2->name);
}

// String comparison function
int string_cmp(const void* a, const void* b)
{
    return strcmp(a, b);
}

// A copy function that always fails (returns NULL)
void* failing_copy_func(const void* data)
{
    (void)data;
    return NULL;
}

void int_free(void* a)
{
    free(a);
}

void person_free(void* p)
{
    free(p);
}

//==============================================================================
// Allocator Functions
//==============================================================================

void* test_calloc(const size_t size)
{
    return calloc(1, size);
}

void test_dealloc(void* ptr)
{
    free(ptr);
}

// Clone an integer
void* int_copy(const void* data)
{
    const int* original = data;
    int* copy = malloc(sizeof(int));
    if (copy)
    {
        *copy = *original;
    }
    return copy;
}

// Clone a string
void* string_copy(const void* data)
{
    const char* original = data;
    if (!original)
    {
        return NULL;
    }

    const size_t len = strlen(original) + 1;
    char* copy = malloc(len);
    if (copy)
    {
        strcpy(copy, original);
    }
    return copy;
}

// Clone a person
void* person_copy(const void* data)
{
    const Person* original = data;
    Person* copy = create_person(original->name, original->age);
    return copy;
}

// Helper to create a person
Person* create_person(const char* name, const int age)
{
    Person* p = malloc(sizeof(Person));
    if (p)
    {
        strncpy(p->name, name, 49);
        p->name[49] = '\0';
        p->age = age;
    }
    return p;
}

// Predicate function that returns non-zero for even numbers
bool is_even(const void* data)
{
    return *(int*)data % 2 == 0 ? 1 : 0;
}

// Predicate: is odd
bool is_odd(const void* data)
{
    const int* value = data;
    return *value % 2 != 0;
}

// Predicate: is greater than 5
bool is_greater_than_five(const void* data)
{
    const int* value = data;
    return (*value > 5);
}

// Predicate: is greater than 10
bool is_greater_than_10(const void* data)
{
    const int* value = data;
    return *value > 10;
}

// Predicate: is greater than 20
bool is_greater_than_20(const void* data)
{
    const int* value = data;
    return *value > 20;
}

// Predicate: is divisible by 3
bool is_divisible_by_3(const void* data)
{
    const int* value = data;
    return *value % 3 == 0;
}

// Predicate: is divisible by 4
bool is_divisible_by_4(const void* data)
{
    const int* value = data;
    return *value % 4 == 0;
}

// Predicate: is divisible by 6
bool is_divisible_by_six(const void* data)
{
    const int* value = data;
    return *value % 6 == 0;
}

// Transform function that doubles a number
void* double_value(const void* data)
{
    int* result = malloc(sizeof(int));
    if (result)
    {
        *result = *(int*)data * 2;
    }
    return result;
}

// Transform: square a number
void* square_func(const void* data)
{
    const int* original = data;
    int* result = malloc(sizeof(int));
    if (result)
    {
        *result = *original * *original;
    }
    return result;
}

// Transform: add 1 to a value
void* add_one(const void* data)
{
    const int* original = data;
    int* result = malloc(sizeof(int));
    *result = *original + 1;
    return result;
}

// Transform: add 5 to a value
void* add_five(const void* data)
{
    const int* original = data;
    int* result = malloc(sizeof(int));
    *result = *original + 5;
    return result;
}

// Transform: add 10 to a value
void* add_ten_func(const void* data)
{
    const int* original = data;
    int* result = malloc(sizeof(int));
    *result = *original + 10;
    return result;
}

// Transform: multiply by 3
void* multiply_by_three(const void* data)
{
    const int* original = data;
    int* result = malloc(sizeof(int));
    *result = *original * 3;
    return result;
}

// Action function that increments a number
void increment(void* data)
{
    (*(int*)data)++;
}

// State for the failing allocator
static int alloc_fail_countdown = -1; // -1 means never fail

// A custom allocator that will fail after a certain number of calls
void* failing_alloc(const size_t size)
{
    if (alloc_fail_countdown == 0)
    {
        return NULL; // Trigger failure
    }
    if (alloc_fail_countdown > 0)
    {
        alloc_fail_countdown--;
    }
    return calloc(1, size); // Use calloc for safety and consistency
}

void failing_free(void* ptr)
{
    free(ptr);
}

// A copy function that uses the failing allocator
void* failing_int_copy(const void* data)
{
    const int* original = data;
    // Use failing_alloc instead of malloc
    int* copy = failing_alloc(sizeof(int));
    if (copy)
    {
        *copy = *original;
    }
    return copy;
}

// A transform function that uses the failing allocator
void* double_value_failing(const void* data)
{
    int* result = failing_alloc(sizeof(int)); // Use the failing allocator
    if (result)
    {
        *result = *(int*)data * 2;
    }
    return result;
}

// Helper to set up the failing allocator
void set_alloc_fail_countdown(const int count)
{
    alloc_fail_countdown = count;
}

// Reset the failing allocator to a clean state (never fail)
void reset_alloc_fail_state(void)
{
    alloc_fail_countdown = -1;
}

ANVAllocator create_failing_int_allocator(void)
{
    return anv_alloc_custom(failing_alloc, failing_free, failing_free, failing_int_copy);
}

ANVAllocator create_int_allocator(void)
{
    return anv_alloc_custom(test_calloc, test_dealloc, int_free, int_copy);
}

ANVAllocator create_person_allocator(void)
{
    return anv_alloc_custom(test_calloc, test_dealloc, person_free, person_copy);
}

ANVAllocator create_string_allocator(void)
{
    return anv_alloc_custom(test_calloc, test_dealloc, free, string_copy);
}