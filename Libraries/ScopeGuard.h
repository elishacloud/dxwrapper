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
    bool flag;
    CRITICAL_SECTION* cs;
public:
    // Constructor enters critical section
    ScopedCriticalSection(CRITICAL_SECTION* cs, bool enable = true) : cs(cs), flag(enable)
    {
        if (flag && cs)
        {
            EnterCriticalSection(cs);
        }
    }
    // Destructor leaves critical section
    ~ScopedCriticalSection()
    {
        if (flag && cs)
        {
            LeaveCriticalSection(cs);
        }
    }
};

struct ScopedLeaveCriticalSection
{
private:
    bool flag;
    CRITICAL_SECTION* cs;
public:
    ScopedLeaveCriticalSection(CRITICAL_SECTION* cs, bool enable = true) : cs(cs), flag(enable) {}
    // Destructor leaves critical section
    ~ScopedLeaveCriticalSection()
    {
        if (flag && cs)
        {
            LeaveCriticalSection(cs);
        }
    }
};
