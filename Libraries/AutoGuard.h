#pragma once

struct AutoSetFlag {
    bool& flag;

    // Constructor sets the flag to true
    AutoSetFlag(bool& setflag) : flag(setflag) {
        flag = true;
    }

    // Destructor sets the flag back to false
    ~AutoSetFlag() {
        flag = false;
    }
};

struct AutoCriticalSection {
    CRITICAL_SECTION* cs;

    // Constructor enters critical section
    AutoCriticalSection(CRITICAL_SECTION* cs) : cs(cs) {
        EnterCriticalSection(cs);
    }

    // Destructor leaves critical section
    ~AutoCriticalSection() {
        LeaveCriticalSection(cs);
    }
};
