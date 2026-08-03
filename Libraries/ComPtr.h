#pragma once

#include <objbase.h>    // CoTaskMemFree

template <typename T>
class ComPtr
{
private:
    T* ptr = nullptr;

public:
    // Default constructor
    ComPtr() = default;

    // Constructor from raw pointer
    ComPtr(T* rawPtr) : ptr(rawPtr) {}

    // Destructor - releases the interface
    ~ComPtr()
    {
        if (ptr)
        {
            ptr->Release();
        }
    }

    // Copy constructor (deleted to avoid accidental AddRef)
    ComPtr(const ComPtr&) = delete;
    ComPtr& operator=(const ComPtr&) = delete;

    // Move constructor
    ComPtr(ComPtr&& other) noexcept
    {
        ptr = other.ptr;
        other.ptr = nullptr;
    }

    // Move assignment
    ComPtr& operator=(ComPtr&& other) noexcept
    {
        if (this != &other)
        {
            Reset();
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    // Get raw pointer
    T* Get() const
    {
        return ptr;
    }

    // Release ownership and return raw pointer
    T* Detach()
    {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    // Release current pointer
    void Reset()
    {
        if (ptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    }

    // Assign a new pointer
    void Attach(T* rawPtr)
    {
        Reset();
        ptr = rawPtr;
    }

    // Pointer operators
    T* operator->() const
    {
        return ptr;
    }

    T& operator*() const
    {
        return *ptr;
    }

    operator T* () const
    {
        return ptr;
    }

    operator const T* () const
    {
        return ptr;
    }

    T** operator&()
    {
        Reset();
        return &ptr;
    }

    // Get address of pointer for functions like GetAddressOf()
    T** GetAddressOf()
    {
        return &ptr;
    }

    // Get address of pointer for functions like ReleaseAndGetAddressOf()
    T** ReleaseAndGetAddressOf()
    {
        Reset();
        return &ptr;
    }

    // Implicit conversion to bool (to check if the pointer is valid)
    operator bool() const
    {
        return ptr != nullptr;
    }

    static ULONG GetRefCount(IUnknown* pComObject)
    {
        if (pComObject)
        {
            pComObject->AddRef();
            return pComObject->Release();
        }
        return 0;
    }
};

template <typename T>
class ComHeapPtr
{
private:
    T* ptr = nullptr;

public:
    // Default constructor
    ComHeapPtr() = default;

    // Constructor from raw pointer
    explicit ComHeapPtr(T* rawPtr) : ptr(rawPtr) {}

    // Destructor
    ~ComHeapPtr()
    {
        Reset();
    }

    // Copy disabled
    ComHeapPtr(const ComHeapPtr&) = delete;
    ComHeapPtr& operator=(const ComHeapPtr&) = delete;

    // Move constructor
    ComHeapPtr(ComHeapPtr&& other) noexcept
    {
        ptr = other.ptr;
        other.ptr = nullptr;
    }

    // Move assignment
    ComHeapPtr& operator=(ComHeapPtr&& other) noexcept
    {
        if (this != &other)
        {
            Reset();
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    // Get raw pointer
    T* Get() const
    {
        return ptr;
    }

    // Release ownership without freeing
    T* Detach()
    {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    // Free memory
    void Reset()
    {
        if (ptr)
        {
            CoTaskMemFree(ptr);
            ptr = nullptr;
        }
    }

    // Take ownership of an existing allocation
    void Attach(T* rawPtr)
    {
        if (ptr != rawPtr)
        {
            Reset();
            ptr = rawPtr;
        }
    }

    // Pointer operators
    T* operator->() const
    {
        return ptr;
    }

    T& operator*() const
    {
        return *ptr;
    }

    operator T* () const
    {
        return ptr;
    }

    operator const T* () const
    {
        return ptr;
    }

    T** operator&()
    {
        Reset();
        return &ptr;
    }

    // Address-of for COM APIs
    T** GetAddressOf()
    {
        return &ptr;
    }

    // Clears existing allocation then returns address
    T** ReleaseAndGetAddressOf()
    {
        Reset();
        return &ptr;
    }

    // Boolean conversion
    explicit operator bool() const
    {
        return ptr != nullptr;
    }

    // Array indexing (useful for strings and arrays)
    T& operator[](size_t index) const
    {
        return ptr[index];
    }
};
