#pragma once

#include <atomic>

inline ULONG AtomicRead(ULONG& ref)
{
    return InterlockedCompareExchange(&ref, 0, 0);
}

inline ULONG InterlockedDecrementIfNotNull(ULONG* value)
{
    if (!value)
    {
        return 0;
    }

    while (true)
    {
        ULONG current = *value;

        if (current <= 0)
        {
            return 0;
        }

        if (InterlockedCompareExchange(value, current - 1, current) == current)
        {
            return current - 1;
        }
    }
}

class CriticalSectionInit
{
private:
    CRITICAL_SECTION Cs;
    volatile LONG InitState;

    void Init()
    {
        if (InterlockedCompareExchange(&InitState, 2, 2) != 2)
        {
            if (InterlockedCompareExchange(&InitState, 1, 0) == 0)
            {
                InitializeCriticalSection(&Cs);
                InterlockedExchange(&InitState, 2);
            }
            else
            {
                while (InterlockedCompareExchange(&InitState, 2, 2) != 2)
                {
                    SwitchToThread();
                }
            }
        }
    }

public:
    CriticalSectionInit() : InitState(0)
    {
        ZeroMemory(&Cs, sizeof(Cs));
    }
    ~CriticalSectionInit()
    {
        if (InterlockedCompareExchange(&InitState, 2, 2) == 2)
        {
            DeleteCriticalSection(&Cs);
        }
    }
    void Enter()
    {
        Init();
        EnterCriticalSection(&Cs);
    }

    void Leave()
    {
        LeaveCriticalSection(&Cs);
    }

    // Prevent copying and moving
    CriticalSectionInit(const CriticalSectionInit&) = delete;
    CriticalSectionInit& operator=(const CriticalSectionInit&) = delete;
    CriticalSectionInit(CriticalSectionInit&&) = delete;
    CriticalSectionInit& operator=(CriticalSectionInit&&) = delete;

    class Lock
    {
    private:
        CriticalSectionInit& LockObj;

    public:
        Lock(CriticalSectionInit& Obj) : LockObj(Obj)
        {
            LockObj.Enter();
        }

        ~Lock()
        {
            LockObj.Leave();
        }

        // Prevent copying and moving
        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;
        Lock(Lock&&) = delete;
        Lock& operator=(Lock&&) = delete;
    };
};

struct ScopedAtomicFlagSet
{
private:
    bool enable;
    std::atomic<bool>& flag;

public:
    ScopedAtomicFlagSet(std::atomic<bool>& setflag, bool setenable = true)
        : flag(setflag), enable(setenable)
    {
        if (enable)
        {
            bool expected = false;

            while (!flag.compare_exchange_weak(
                expected,
                true,
                std::memory_order_acquire,
                std::memory_order_relaxed))
            {
                expected = false;

#ifdef YieldProcessor
                YieldProcessor();
#else
                _mm_pause();
#endif
            }
        }
    }

    ~ScopedAtomicFlagSet()
    {
        if (enable)
        {
            flag.store(false, std::memory_order_release);
        }
    }

    // Prevent copying and moving
    ScopedAtomicFlagSet(const ScopedAtomicFlagSet&) = delete;
    ScopedAtomicFlagSet& operator=(const ScopedAtomicFlagSet&) = delete;
    ScopedAtomicFlagSet(ScopedAtomicFlagSet&&) = delete;
    ScopedAtomicFlagSet& operator=(ScopedAtomicFlagSet&&) = delete;
};

class ScopedAtomicLock
{
private:
    volatile LONG& State;
    bool Locked = false;

public:
    ScopedAtomicLock(volatile LONG& State) : State(State)
    {
        while (true)
        {
            if (InterlockedCompareExchange(&State, 1, 0) == 0)
            {
                Locked = true;
                break;
            }

            SwitchToThread();
        }
    }

    ~ScopedAtomicLock()
    {
        if (Locked)
        {
            InterlockedExchange(&State, 0);
        }
    }

    // Prevent copying and moving
    ScopedAtomicLock(const ScopedAtomicLock&) = delete;
    ScopedAtomicLock& operator=(const ScopedAtomicLock&) = delete;
    ScopedAtomicLock(ScopedAtomicLock&&) = delete;
    ScopedAtomicLock& operator=(ScopedAtomicLock&&) = delete;
};

template<typename T>
struct ScopedFlagSet
{
private:
    bool enable;
    T& flag;
public:
    // Constructor sets the flag to true
    ScopedFlagSet(T& setflag, bool activate = true) : flag(setflag), enable(activate)
    {
        if (enable)
        {
            flag = true;
        }
    }
    // Destructor sets the flag back to false
    ~ScopedFlagSet()
    {
        if (enable)
        {
            flag = false;
        }
    }

    // Prevent copying and moving
    ScopedFlagSet(const ScopedFlagSet&) = delete;
    ScopedFlagSet& operator=(const ScopedFlagSet&) = delete;
    ScopedFlagSet(ScopedFlagSet&&) = delete;
    ScopedFlagSet& operator=(ScopedFlagSet&&) = delete;
};

template<typename T>
struct ScopedIncrement
{
private:
    bool enable;
    T& num;

public:
    // Constructor increments num
    ScopedIncrement(T& num, bool activate = true) : num(num), enable(activate)
    {
        if (enable)
        {
            num++;
        }
    }
    // Destructor decrements num
    ~ScopedIncrement()
    {
        if (enable)
        {
            num--;
        }
    }

    // Prevent copying and moving
    ScopedIncrement(const ScopedIncrement&) = delete;
    ScopedIncrement& operator=(const ScopedIncrement&) = delete;
    ScopedIncrement(ScopedIncrement&&) = delete;
    ScopedIncrement& operator=(ScopedIncrement&&) = delete;
};

struct ScopedCriticalSection
{
private:
    bool enable;
    CRITICAL_SECTION* cs;
public:
    // Constructor enters critical section
    ScopedCriticalSection(CRITICAL_SECTION* cs, bool activate = true) : cs(cs), enable(activate)
    {
        if (enable && cs)
        {
            EnterCriticalSection(cs);
        }
    }
    // Destructor leaves critical section
    ~ScopedCriticalSection()
    {
        if (enable && cs)
        {
            LeaveCriticalSection(cs);
        }
    }

    // Prevent copying and moving
    ScopedCriticalSection(const ScopedCriticalSection&) = delete;
    ScopedCriticalSection& operator=(const ScopedCriticalSection&) = delete;
    ScopedCriticalSection(ScopedCriticalSection&&) = delete;
    ScopedCriticalSection& operator=(ScopedCriticalSection&&) = delete;
};

struct ScopedLeaveCriticalSection
{
private:
    bool enable;
    CRITICAL_SECTION* cs;
public:
    ScopedLeaveCriticalSection(CRITICAL_SECTION* cs, bool activate = true) : cs(cs), enable(activate) {}
    // Destructor leaves critical section
    ~ScopedLeaveCriticalSection()
    {
        if (enable && cs)
        {
            LeaveCriticalSection(cs);
        }
    }

    // Prevent copying and moving
    ScopedLeaveCriticalSection(const ScopedLeaveCriticalSection&) = delete;
    ScopedLeaveCriticalSection& operator=(const ScopedLeaveCriticalSection&) = delete;
    ScopedLeaveCriticalSection(ScopedLeaveCriticalSection&&) = delete;
    ScopedLeaveCriticalSection& operator=(ScopedLeaveCriticalSection&&) = delete;
};

#define CreateScopedHeapBuffer(type, name, size) \
    __HeapBuffer<type> name##_heap(size); \
    type* const name = name##_heap.buffer;

template<typename T>
struct __HeapBuffer
{
    T* buffer;

    explicit __HeapBuffer(size_t count)
        : buffer(new T[count]()) // zero-initialize
    {
    }

    ~__HeapBuffer()
    {
        delete[] buffer;
    }

    // Prevent copying
    __HeapBuffer(const __HeapBuffer&) = delete;
    __HeapBuffer& operator=(const __HeapBuffer&) = delete;

    // Allow move
    __HeapBuffer(__HeapBuffer&& other) noexcept : buffer(other.buffer)
    {
        other.buffer = nullptr;
    }
    __HeapBuffer& operator=(__HeapBuffer&& other) noexcept
    {
        if (this != &other)
        {
            delete[] buffer;
            buffer = other.buffer;
            other.buffer = nullptr;
        }
        return *this;
    }
};

template <typename T, std::size_t Alignment>
struct aligned_allocator {
    static_assert(Alignment >= alignof(void*), "Alignment must be at least pointer size");
    static_assert((Alignment& (Alignment - 1)) == 0, "Alignment must be a power of two");

    using value_type = T;

    aligned_allocator() noexcept = default;

    template <typename U>
    aligned_allocator(const aligned_allocator<U, Alignment>&) noexcept {}

    T* allocate(std::size_t n) {
        void* ptr = nullptr;
#if defined(_MSC_VER)
        ptr = _aligned_malloc(n * sizeof(T), Alignment);
        if (!ptr) throw std::bad_alloc();
#else
        if (posix_memalign(&ptr, Alignment, n * sizeof(T)) != 0)
            throw std::bad_alloc();
#endif
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, std::size_t) noexcept {
#if defined(_MSC_VER)
        _aligned_free(p);
#else
        free(p);
#endif
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new ((void*)p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) {
        p->~U();
    }

    template <typename U>
    struct rebind {
        using other = aligned_allocator<U, Alignment>;
    };

    // Comparison operators for allocator compatibility
    bool operator==(const aligned_allocator&) const noexcept { return true; }
    bool operator!=(const aligned_allocator&) const noexcept { return false; }
};
