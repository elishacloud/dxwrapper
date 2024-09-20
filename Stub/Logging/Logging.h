#pragma once

#define LogDebug Log

#define LOG_ONCE(msg) \
	{}

#define LOG_LIMIT(num, msg) \
	{}

class Logging
{
public:
    class Log
    {
    public:
        template <typename T>
        Log& operator<<(const T&)
        {
            return *this; // Return Log object to support chaining
        }
    };
    static const char* hex(int)
    {
        return "";
    }
    static const char* hexDump(const void*, const unsigned long)
    {
        return "";
    }
    static void LogFormat(const char*, ...)
    {}
    static void LogFormat(const wchar_t*, ...)
    {}
};
