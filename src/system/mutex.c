// Mutex.c
// Cross-platform mutex implementation
//
// Provides a thin portable wrapper around platform native mutex primitives:
// - pthread_mutex_t on POSIX systems
// - CRITICAL_SECTION on Windows
//
// The functions mirror the declarations in Mutex.h and return 0 on success
// and non-zero on failure. POSIX functions return the underlying pthread
// return codes where applicable.

#include "mutex.h"

#ifdef ANVIL_PLATFORM_WINDOWS
    #include <windows.h>

/**
 * Initialize a ANVMutex (CRITICAL_SECTION) instance.
 *
 * @param mtx Pointer to an uninitialized ANVMutex.
 * @return 0 on success, -1 on invalid argument.
 */
ANV_API int anv_mutex_init(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }

    if (!InitializeCriticalSectionAndSpinCount(&mtx->cs, 4000))
    {
        return -1;
    }
    mtx->owner_thread_id = 0;
    mtx->lock_count = 0;
    return 0;
}

/**
 * Lock the mutex (blocking).
 *
 * @param mtx Pointer to an initialized ANVMutex.
 * @return 0 on success, -1 on invalid argument.
 */
ANV_API int anv_mutex_lock(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }

    EnterCriticalSection(&mtx->cs);
    mtx->owner_thread_id = GetCurrentThreadId();
    mtx->lock_count = 1;
    return 0;
}

/**
 * Try to lock the mutex without blocking.
 *
 * @param mtx Pointer to an initialized ANVMutex.
 * @return 0 if lock acquired, 1 if not acquired, -1 on invalid argument.
 */
ANV_API int anv_mutex_trylock(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }

    DWORD current_thread = GetCurrentThreadId();

    // Check if this thread already owns the mutex (simulate non-recursive behavior)
    // We need to check this BEFORE attempting TryEnterCriticalSection
    if (mtx->owner_thread_id == current_thread && mtx->lock_count > 0)
    {
        // This thread already owns the mutex - return failure for non-recursive behavior
        return 1; // Would block/already owned
    }

    // Try to acquire the critical section
    BOOL acquired = TryEnterCriticalSection(&mtx->cs);
    if (acquired)
    {
        // Successfully acquired - set ownership
        mtx->owner_thread_id = current_thread;
        mtx->lock_count = 1;
        return 0; // Success
    }

    // Could not acquire the lock (another thread owns it)
    return 1; // Would block
}

/**
 * Unlock the mutex.
 *
 * @param mtx Pointer to an initialized ANVMutex.
 * @return 0 on success, -1 on invalid argument.
 */
ANV_API int anv_mutex_unlock(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }

    mtx->owner_thread_id = 0;
    mtx->lock_count = 0;
    LeaveCriticalSection(&mtx->cs);
    return 0;
}

/**
 * Destroy the mutex and free underlying resources.
 *
 * @param mtx Pointer to an initialized ANVMutex.
 * @return 0 on success, -1 on invalid argument.
 */
ANV_API int anv_mutex_destroy(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }

    DeleteCriticalSection(&mtx->cs);
    mtx->owner_thread_id = 0;
    mtx->lock_count = 0;
    return 0;
}

#else
    #include <pthread.h>

/**
 * Initialize a ANVMutex (pthread_mutex_t) instance.
 *
 * @param mtx Pointer to an uninitialized ANVMutex.
 * @return 0 on success, non-zero pthread error code on failure, -1 on invalid argument.
 */
ANV_API int anv_mutex_init(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }
    return pthread_mutex_init(mtx, NULL);
}

/**
 * Lock the mutex (blocking).
 *
 * @param mtx Pointer to an initialized ANVMutex.
 * @return 0 on success, non-zero pthread error code on failure, -1 on invalid argument.
 */
ANV_API int anv_mutex_lock(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }
    return pthread_mutex_lock(mtx);
}

/**
 * Try to lock the mutex without blocking.
 *
 * @param mtx Pointer to an initialized ANVMutex.
 * @return 0 if lock acquired, EBUSY if already locked, other pthread error codes on failure, -1 on invalid argument.
 */
ANV_API int anv_mutex_trylock(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }
    const int rc = pthread_mutex_trylock(mtx);
    return rc == 0 ? 0 : rc; // return 0 on success, error code otherwise
}

/**
 * Unlock the mutex.
 *
 * @param mtx Pointer to an initialized ANVMutex.
 * @return 0 on success, non-zero pthread error code on failure, -1 on invalid argument.
 */
ANV_API int anv_mutex_unlock(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }
    return pthread_mutex_unlock(mtx);
}

/**
 * Destroy the mutex and free underlying resources.
 *
 * @param mtx Pointer to an initialized ANVMutex.
 * @return 0 on success, non-zero pthread error code on failure, -1 on invalid argument.
 */
ANV_API int anv_mutex_destroy(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }
    return pthread_mutex_destroy(mtx);
}

#endif
