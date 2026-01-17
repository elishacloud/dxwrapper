#pragma once

#include <atomic>

template<typename T>
struct ScopedFlagSet
{
private:
    bool enable;
    T& flag;
public:
    // Constructor sets the flag to true
    ScopedFlagSet(T& setflag, bool setenable = true) : flag(setflag), enable(setenable)
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
};

struct ScopedAtomicFlagSet
{
private:
    bool enable;
    std::atomic<bool>& flag;
public:
    // Constructor sets the flag to true
    ScopedAtomicFlagSet(std::atomic<bool>& setflag, bool setenable = true) : flag(setflag), enable(setenable)
    {
        if (enable)
        {
            while (flag.exchange(true, std::memory_order_relaxed))
            {
#ifdef YieldProcessor
                YieldProcessor();
#else
                _mm_pause();
#endif
            }
        }
    }
    // Destructor sets the flag back to false
    ~ScopedAtomicFlagSet()
    {
        if (enable)
        {
            flag.store(false, std::memory_order_relaxed);
        }
    }
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

    // Disable copy
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

struct ScopedCriticalSection
{
private:
    bool enable;
    CRITICAL_SECTION* cs;
public:
    // Constructor enters critical section
    ScopedCriticalSection(CRITICAL_SECTION* cs, bool setenable = true) : cs(cs), enable(setenable)
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
};

struct ScopedLeaveCriticalSection
{
private:
    bool enable;
    CRITICAL_SECTION* cs;
public:
    ScopedLeaveCriticalSection(CRITICAL_SECTION* cs, bool setenable = true) : cs(cs), enable(setenable) {}
    // Destructor leaves critical section
    ~ScopedLeaveCriticalSection()
    {
        if (enable && cs)
        {
            LeaveCriticalSection(cs);
        }
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
