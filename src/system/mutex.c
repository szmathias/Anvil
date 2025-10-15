//
//
//

#include "mutex.h"

#ifdef ANV_PLATFORM_WINDOWS

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

ANV_API int anv_mutex_trylock(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }

    const DWORD current_thread = GetCurrentThreadId();

    // Check if this thread already owns the mutex
    // We need to check this before attempting TryEnterCriticalSection
    if (mtx->owner_thread_id == current_thread && mtx->lock_count > 0)
    {
        // This thread already owns the mutex - return failure
        return 1;
    }

    // Try to acquire the critical section
    const BOOL acquired = TryEnterCriticalSection(&mtx->cs);
    if (acquired)
    {
        mtx->owner_thread_id = current_thread;
        mtx->lock_count = 1;
        return 0;
    }

    // Could not acquire the lock
    return 1;
}

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

ANV_API int anv_mutex_init(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }
    return pthread_mutex_init(mtx, NULL);
}

ANV_API int anv_mutex_lock(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }
    return pthread_mutex_lock(mtx);
}

ANV_API int anv_mutex_trylock(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }
    const int rc = pthread_mutex_trylock(mtx);
    return rc == 0 ? 0 : rc;
}

ANV_API int anv_mutex_unlock(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }
    return pthread_mutex_unlock(mtx);
}

ANV_API int anv_mutex_destroy(ANVMutex* mtx)
{
    if (!mtx)
    {
        return -1;
    }
    return pthread_mutex_destroy(mtx);
}

#endif