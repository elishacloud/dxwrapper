#pragma once

struct ScopedFlagSet {
    bool& flag;

    // Constructor sets the flag to true
    ScopedFlagSet(bool& setflag) : flag(setflag) {
        flag = true;
    }

    // Destructor sets the flag back to false
    ~ScopedFlagSet() {
        flag = false;
    }
};

struct ScopedCriticalSection {
    CRITICAL_SECTION* cs;

    // Constructor enters critical section
    ScopedCriticalSection(CRITICAL_SECTION* cs) : cs(cs) {
        EnterCriticalSection(cs);
    }

    // Destructor leaves critical section
    ~ScopedCriticalSection() {
        LeaveCriticalSection(cs);
    }
};
