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
        Log& operator<<(const T& value)
        {
            return *this; // Return Log object to support chaining
        }
    };
    static const char* hex(int value)
    {
        return "";
    }
    static void LogFormat(const char* fmt, ...)
    {}
    static void LogFormat(const wchar_t* fmt, ...)
    {}
};
