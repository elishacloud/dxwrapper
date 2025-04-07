#pragma once

struct ScopedFlagSet
{
private:
    bool& flag;
public:
    // Constructor sets the flag to true
    ScopedFlagSet(bool& setflag) : flag(setflag)
    {
        flag = true;
    }
    // Destructor sets the flag back to false
    ~ScopedFlagSet()
    {
        flag = false;
    }
};

struct ScopedCriticalSection
{
private:
    CRITICAL_SECTION* cs;
public:
    // Constructor enters critical section
    ScopedCriticalSection(CRITICAL_SECTION* cs) : cs(cs)
    {
        if (cs)
        {
            EnterCriticalSection(cs);
        }
    }
    // Destructor leaves critical section
    ~ScopedCriticalSection()
    {
        if (cs)
        {
            LeaveCriticalSection(cs);
        }
    }
};
