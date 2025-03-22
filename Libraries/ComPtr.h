#pragma once

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

    // Overload -> operator
    T* operator->() const
    {
        return ptr;
    }

    // Overload * operator
    T& operator*() const
    {
        return *ptr;
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
};
