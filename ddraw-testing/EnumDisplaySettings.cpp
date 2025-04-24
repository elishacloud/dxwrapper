#include "ddraw-testing.h"
#include "testing-harness.h"

static BOOL WINAPI MyEnumCallbackA(GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{
    UNREFERENCED_PARAMETER(lpContext);

    Logging::Log() << __FUNCTION__ << " " << lpGUID << " " << lpDriverDescription << " " << lpDriverName;

    return DDENUMRET_OK;
}

static BOOL WINAPI MyEnumCallbackW(GUID FAR* lpGUID, LPWSTR lpDriverDescription, LPWSTR lpDriverName, LPVOID lpContext)
{
    UNREFERENCED_PARAMETER(lpContext);

    Logging::Log() << __FUNCTION__ << " " << lpGUID << " " << lpDriverDescription << " " << lpDriverName;

    return DDENUMRET_OK;
}

static BOOL WINAPI MyEnumCallbackExA(GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hMonitor)
{
    UNREFERENCED_PARAMETER(lpContext);

    Logging::Log() << __FUNCTION__ << " " << lpGUID << " " << lpDriverDescription << " " << lpDriverName << " " << hMonitor;

    return DDENUMRET_OK;
}

static BOOL WINAPI MyEnumCallbackExW(GUID FAR* lpGUID, LPWSTR lpDriverDescription, LPWSTR lpDriverName, LPVOID lpContext, HMONITOR hMonitor)
{
    UNREFERENCED_PARAMETER(lpContext);

    Logging::Log() << __FUNCTION__ << " " << lpGUID << " " << lpDriverDescription << " " << lpDriverName << " " << hMonitor;

    return DDENUMRET_OK;
}

void TestEnumDisplaySettings()
{
    Logging::Log() << "****";
    Logging::Log() << "**** Testing DirectDrawEnumerate";
    Logging::Log() << "****";

    HRESULT hr;

    // Basic callback
    hr = pDirectDrawEnumerateA(MyEnumCallbackA, nullptr);
    Logging::Log() << "Return value: " << (DDERR)hr;
    hr = pDirectDrawEnumerateW(MyEnumCallbackW, nullptr);
    Logging::Log() << "Return value: " << (DDERR)hr;

    // Extended callback for attached devices
    Logging::Log() << __FUNCTION__ << " Attached secondary devices";
    hr = pDirectDrawEnumerateExA(MyEnumCallbackExA, nullptr, DDENUM_ATTACHEDSECONDARYDEVICES);
    Logging::Log() << "Return value: " << (DDERR)hr;
    hr = pDirectDrawEnumerateExW(MyEnumCallbackExW, nullptr, DDENUM_ATTACHEDSECONDARYDEVICES);
    Logging::Log() << "Return value: " << (DDERR)hr;

    // Extended callback for detached devices
    Logging::Log() << __FUNCTION__ << " Detached secondary devices";
    hr = pDirectDrawEnumerateExA(MyEnumCallbackExA, nullptr, DDENUM_DETACHEDSECONDARYDEVICES);
    Logging::Log() << "Return value: " << (DDERR)hr;
    hr = pDirectDrawEnumerateExW(MyEnumCallbackExW, nullptr, DDENUM_DETACHEDSECONDARYDEVICES);
    Logging::Log() << "Return value: " << (DDERR)hr;

    // Extended callback for non-display devices
    Logging::Log() << __FUNCTION__ << " Non-display devices";
    hr = pDirectDrawEnumerateExA(MyEnumCallbackExA, nullptr, DDENUM_NONDISPLAYDEVICES);
    Logging::Log() << "Return value: " << (DDERR)hr;
    hr = pDirectDrawEnumerateExW(MyEnumCallbackExW, nullptr, DDENUM_NONDISPLAYDEVICES);
    Logging::Log() << "Return value: " << (DDERR)hr;
}
