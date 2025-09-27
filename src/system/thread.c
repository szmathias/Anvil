//
// Created by zack on 8/31/25.
//
// Threads.c
// Cross-platform thread implementation
//
// Provides a small portable wrapper around native thread primitives:
// - pthreads on POSIX
// - Win32 threads (CreateThread) on Windows
//
// The header (Threads.h) declares a minimal API: create, join and detach.
// The implementation uses a lightweight wrapper on Windows to store a
// thread function return value and support join/detach semantics similar to
// pthreads. All public functions return 0 on success and non-zero on
// failure where possible; platform-specific return codes are preserved for
// pthread functions.

#include "thread.h"

#ifdef ANVIL_PLATFORM_WINDOWS
    #include <windows.h>

// Wrapper for a user-provided thread function and argument. The wrapper
// stores the return value so that join can retrieve it.
typedef struct thread_wrapper
{
        anvthread_func func;
        void* arg;
        void* result;
} thread_wrapper;

// Linked-list entry used to map native HANDLEs to our thread_wrapper so
// join/detach can locate the wrapper and free resources correctly.
typedef struct handle_entry
{
        ANVThread thread;
        thread_wrapper* w;
        int done;     // set when thread finished
        int detached; // set when detach called
        struct handle_entry* next;
} handle_entry;

static CRITICAL_SECTION g_map_lock;
static handle_entry* g_map_head = NULL;
static int g_map_initialized = 0;

// Ensure the global map lock is initialized before use.
static void ensure_map_init(void)
{
    if (!g_map_initialized)
    {
        InitializeCriticalSection(&g_map_lock);
        g_map_initialized = 1;
    }
}

// Add a mapping from a thread identifier to the wrapper.
static void add_mapping(ANVThread thread, thread_wrapper* w)
{
    ensure_map_init();
    handle_entry* e = (handle_entry*)malloc(sizeof(handle_entry));
    if (!e)
    {
        return;
    }

    e->thread = thread;
    e->w = w;
    e->done = 0;
    e->detached = 0;
    EnterCriticalSection(&g_map_lock);
    e->next = g_map_head;
    g_map_head = e;
    LeaveCriticalSection(&g_map_lock);
}

// Find and remove mapping for a given thread. Returns the removed entry or
// NULL if not found. Caller is responsible for freeing returned entry.
static handle_entry* find_and_remove_mapping(ANVThread thread)
{
    ensure_map_init();
    handle_entry* prev = NULL;
    handle_entry* cur = NULL;
    EnterCriticalSection(&g_map_lock);
    cur = g_map_head;
    while (cur)
    {
        if (cur->thread == thread)
        {
            if (prev)
            {
                prev->next = cur->next;
            }
            else
            {
                g_map_head = cur->next;
            }

            cur->next = NULL;
            LeaveCriticalSection(&g_map_lock);
            return cur;
        }
        prev = cur;
        cur = cur->next;
    }

    LeaveCriticalSection(&g_map_lock);
    return NULL;
}

    // Find mapping without removing it. Returns pointer to entry while leaving
    // it owned by the global map (do not free the returned pointer).
    #ifdef _MSC_VER // Only include if used, or for debugging
static handle_entry* find_mapping(ANVThread thread)
{
    ensure_map_init();
    handle_entry* cur = NULL;
    EnterCriticalSection(&g_map_lock);
    cur = g_map_head;
    while (cur)
    {
        if (cur->thread == thread)
        {
            LeaveCriticalSection(&g_map_lock);
            return cur;
        }
        cur = cur->next;
    }

    LeaveCriticalSection(&g_map_lock);
    return NULL;
}
    #endif

// Thread entry used with CreateThread. It invokes the user function and
// stores the return value in the wrapper. If the thread has been detached
// before finishing, this function will free the wrapper and its mapping.
static DWORD WINAPI thread_func_wrapper(LPVOID param)
{
    ensure_map_init();
    thread_wrapper* w = (thread_wrapper*)param;
    if (!w)
    {
        return 0;
    }

    w->result = w->func(w->arg);

    // mark done and free if detached
    EnterCriticalSection(&g_map_lock);
    handle_entry* e = g_map_head;
    handle_entry* prev = NULL;
    while (e)
    {
        if (e->w == w)
        {
            break;
        }

        prev = e;
        e = e->next;
    }

    if (e)
    {
        e->done = 1;
        if (e->detached)
        {
            // remove mapping from list inline while holding the lock
            if (prev)
            {
                prev->next = e->next;
            }
            else
            {
                g_map_head = e->next;
            }

            e->next = NULL;
            // capture pointers to free after leaving lock
            thread_wrapper* to_free_w = e->w;
            handle_entry* to_free_e = e;
            LeaveCriticalSection(&g_map_lock);
            if (to_free_w)
            {
                free(to_free_w);
            }

            free(to_free_e);
            return 0;
        }
    }

    LeaveCriticalSection(&g_map_lock);
    return 0;
}

// Public API (Windows): create a thread. On success the ANVThread is set
// to a valid HANDLE value. Returns 0 on success, -1 on invalid args or
// thread creation failure.
ANV_API int anv_thread_create(ANVThread* thread, anvthread_func func, void* arg)
{
    if (!thread || !func)
    {
        return -1;
    }

    thread_wrapper* w = (thread_wrapper*)malloc(sizeof(thread_wrapper));
    if (!w)
    {
        return -1;
    }

    w->func = func;
    w->arg = arg;
    w->result = NULL;

    HANDLE h = CreateThread(NULL, 0, thread_func_wrapper, w, 0, NULL);
    if (!h)
    {
        free(w);
        return -1;
    }

    *thread = (ANVThread)h;
    add_mapping(*thread, w);
    return 0;
}

// Public API (Windows): join a thread. Waits for the thread to exit and
// retrieves the stored result pointer if retval is non-NULL. Returns 0 on
// success and -1 on error.
ANV_API int anv_thread_join(ANVThread thread, void** retval)
{
    if (!thread)
    {
        return -1;
    }

    DWORD wait = WaitForSingleObject((HANDLE)thread, INFINITE);
    if (wait != WAIT_OBJECT_0)
    {
        CloseHandle((HANDLE)thread);
        return -1;
    }

    // find and remove mapping
    handle_entry* e = find_and_remove_mapping(thread);
    if (!e)
    {
        CloseHandle((HANDLE)thread);
        return -1;
    }

    if (retval)
        *retval = e->w ? e->w->result : NULL;

    if (e->w)
    {
        free(e->w);
    }

    free(e);
    CloseHandle((HANDLE)thread);
    return 0;
}

// Public API (Windows): detach a thread. Marks the thread as detached and
// closes the native handle. If the thread already finished, resources are
// removed immediately.
ANV_API int anv_thread_detach(ANVThread thread)
{
    if (!thread)
    {
        return -1;
    }

    ensure_map_init();
    EnterCriticalSection(&g_map_lock);
    handle_entry* cur = g_map_head;
    while (cur)
    {
        if (cur->thread == thread)
        {
            cur->detached = 1;
            int already_done = cur->done;
            LeaveCriticalSection(&g_map_lock);
            // if already done, remove and free now
            if (already_done)
            {
                handle_entry* e = find_and_remove_mapping(thread);
                if (e)
                {
                    if (e->w)
                    {
                        free(e->w);
                    }
                    free(e);
                }
            }
            // close the handle
            CloseHandle((HANDLE)thread);
            return 0;
        }
        cur = cur->next;
    }
    LeaveCriticalSection(&g_map_lock);
    // mapping not found
    CloseHandle((HANDLE)thread);
    return 0;
}

#else
    #include <pthread.h>

// POSIX implementations are thin wrappers around pthreads. They forward
// return values from pthread functions directly so callers can inspect
// platform-specific error codes where appropriate.

ANV_API int anv_thread_create(ANVThread* thread, const anvthread_func func, void* arg)
{
    if (!thread || !func)
    {
        return -1;
    }
    return pthread_create(thread, NULL, func, arg);
}

ANV_API int anv_thread_join(const ANVThread thread, void** retval)
{
    return pthread_join(thread, retval);
}

ANV_API int anv_thread_detach(const ANVThread thread)
{
    return pthread_detach(thread);
}

#endif
