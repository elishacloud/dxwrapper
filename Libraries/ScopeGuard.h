#pragma once

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

template<typename T>
struct HeapBuffer
{
private:
    T* buffer = nullptr;
public:
    // Constructor: allocate and initialize buffer
    HeapBuffer(size_t bufferSize)
    {
        buffer = new T[bufferSize](); // value-initialized
    }
    // Destructor: free memory
    ~HeapBuffer()
    {
        delete[] buffer;
    }
    // Get buffer pointer
    T* data()
    {
        return buffer;
    }
    // Disable copy to prevent accidental copies
    HeapBuffer(const HeapBuffer&) = delete;
    HeapBuffer& operator=(const HeapBuffer&) = delete;
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
