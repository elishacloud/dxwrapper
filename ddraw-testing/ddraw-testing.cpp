#include "ddraw-testing.h"
#include "testing-harness.h"

std::ofstream LOG;

HWND DDhWnd = nullptr;

DirectDrawCreateProc pDirectDrawCreate = nullptr;
DirectDrawCreateExProc pDirectDrawCreateEx = nullptr;
DirectDrawEnumerateAProc pDirectDrawEnumerateA = nullptr;
DirectDrawEnumerateExAProc pDirectDrawEnumerateExA = nullptr;
DirectDrawEnumerateExWProc pDirectDrawEnumerateExW = nullptr;
DirectDrawEnumerateWProc pDirectDrawEnumerateW = nullptr;

template void TestQueryInterfaces<IDirectDraw>(IDirectDraw*, IUnknown*, const char*, DWORD);
template void TestQueryInterfaces<IDirectDraw2>(IDirectDraw2*, IUnknown*, const char*, DWORD);
template void TestQueryInterfaces<IDirectDraw3>(IDirectDraw3*, IUnknown*, const char*, DWORD);
template void TestQueryInterfaces<IDirectDraw4>(IDirectDraw4*, IUnknown*, const char*, DWORD);
template void TestQueryInterfaces<IDirectDraw7>(IDirectDraw7*, IUnknown*, const char*, DWORD);
template void TestQueryInterfaces<IDirectDraw7Ex>(IDirectDraw7Ex*, IUnknown*, const char*, DWORD);
template <typename DDType>
void TestQueryInterfaces(DDType* pDDraw, IUnknown* pInterface, const char* ParentName, DWORD TestIDBase)
{
    struct InterfaceEntry {
        const IID* iid;
        const char* name;
    };

    // List of interfaces to test
    const InterfaceEntry interfaces[] = {
        {&IID_IDirectDraw, "IDirectDraw"},
        {&IID_IDirectDraw2, "IDirectDraw2"},
        {&IID_IDirectDraw3, "IDirectDraw3"},
        {&IID_IDirectDraw4, "IDirectDraw4"},
        {&IID_IDirectDraw7, "IDirectDraw7"},
        {&IID_IDirectDrawSurface, "IDirectDrawSurface"},
        {&IID_IDirectDrawSurface2, "IDirectDrawSurface2"},
        {&IID_IDirectDrawSurface3, "IDirectDrawSurface3"},
        {&IID_IDirectDrawSurface4, "IDirectDrawSurface4"},
        {&IID_IDirectDrawSurface7, "IDirectDrawSurface7"},
        {&IID_IDirectDrawPalette, "IDirectDrawPalette"},
        {&IID_IDirectDrawClipper, "IDirectDrawClipper"},
        {&IID_IDirectDrawColorControl, "IDirectDrawColorControl"},
        {&IID_IDirectDrawGammaControl, "IDirectDrawGammaControl"},
        {&IID_IDirect3D, "IDirect3D"},
        {&IID_IDirect3D2, "IDirect3D2"},
        {&IID_IDirect3D3, "IDirect3D3"},
        {&IID_IDirect3D7, "IDirect3D7"},
        {&IID_IDirect3DDevice, "IDirect3DDevice"},
        {&IID_IDirect3DDevice2, "IDirect3DDevice2"},
        {&IID_IDirect3DDevice3, "IDirect3DDevice3"},
        {&IID_IDirect3DDevice7, "IDirect3DDevice7"},
        {&IID_IDirect3DTexture, "IDirect3DTexture"},
        {&IID_IDirect3DTexture2, "IDirect3DTexture2"},
        {&IID_IDirect3DLight, "IDirect3DLight"},
        {&IID_IDirect3DMaterial, "IDirect3DMaterial"},
        {&IID_IDirect3DMaterial2, "IDirect3DMaterial2"},
        {&IID_IDirect3DMaterial3, "IDirect3DMaterial3"},
        {&IID_IDirect3DExecuteBuffer, "IDirect3DExecuteBuffer"},
        {&IID_IDirect3DViewport, "IDirect3DViewport"},
        {&IID_IDirect3DViewport2, "IDirect3DViewport2"},
        {&IID_IDirect3DViewport3, "IDirect3DViewport3"},
        {&IID_IDirect3DVertexBuffer, "IDirect3DVertexBuffer"},
        {&IID_IDirect3DVertexBuffer7, "IDirect3DVertexBuffer7"},
    };

    for (size_t i = 0; i < std::size(interfaces); ++i)
    {
        IUnknown* pQueriedInterface = nullptr;
        HRESULT hr = pInterface->QueryInterface(*interfaces[i].iid, reinterpret_cast<LPVOID*>(&pQueriedInterface));

        // ****  Base + 0  ****
        DWORD TestID = TestIDBase + (DWORD)(i * 3) + 0;
        if (SUCCEEDED(hr))
        {
            LOG_TEST_RESULT(TestID, "Successfully queried " << interfaces[i].name << " from " << ParentName << ". Ref count: ", GetRefCount(pQueriedInterface), GetResults<DDType>(TestID));

            // ****  Base + 1  ****
            TestID = TestIDBase + (DWORD)(i * 3) + 1;
            LOG_TEST_RESULT(TestID, "Base interface Ref count: ", GetRefCount(pInterface), GetResults<DDType>(TestID));

            // ****  Base + 2  ****
            TestID = TestIDBase + (DWORD)(i * 3) + 2;
            LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

            pQueriedInterface->Release();
        }
        else
        {
            LOG_TEST_RESULT(TestID, "Failed to query " << interfaces[i].name << " from " << ParentName << ". Error: ", (DDERR)hr, GetResults<DDType>(TestID));
        }
    }
}

static HWND CreateTestWindow()
{
    WNDCLASSA wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "DirectDrawTestWindow";
    RegisterClassA(&wc);

    return CreateWindowA("DirectDrawTestWindow", "DDraw Test", WS_POPUPWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, wc.hInstance, nullptr);
}

// Run tests for all DirectDraw versions
static void RunAllTests()
{
    // Load dll
    HMODULE ddraw_dll = LoadLibraryA("ddraw.dll");
    if (!ddraw_dll)
    {
        Logging::Log() << "Failed to load ddraw.dll!";
        return;
    }
    pDirectDrawCreate = reinterpret_cast<DirectDrawCreateProc>(GetProcAddress(ddraw_dll, "DirectDrawCreate"));
    pDirectDrawCreateEx = reinterpret_cast<DirectDrawCreateExProc>(GetProcAddress(ddraw_dll, "DirectDrawCreateEx"));
    pDirectDrawEnumerateA = reinterpret_cast<DirectDrawEnumerateAProc>(GetProcAddress(ddraw_dll, "DirectDrawEnumerateA"));
    pDirectDrawEnumerateExA = reinterpret_cast<DirectDrawEnumerateExAProc>(GetProcAddress(ddraw_dll, "DirectDrawEnumerateExA"));
    pDirectDrawEnumerateExW = reinterpret_cast<DirectDrawEnumerateExWProc>(GetProcAddress(ddraw_dll, "DirectDrawEnumerateExW"));
    pDirectDrawEnumerateW = reinterpret_cast<DirectDrawEnumerateWProc>(GetProcAddress(ddraw_dll, "DirectDrawEnumerateW"));

    DDhWnd = CreateTestWindow();
    if (!DDhWnd)
    {
        Logging::Log() << "Failed to create window!";
        return;
    }

    TestEnumDisplaySettings();
    TestDirectDrawCreate<IDirectDraw>();
    TestDirectDrawCreate<IDirectDraw2>();
    TestDirectDrawCreate<IDirectDraw3>();
    TestDirectDrawCreate<IDirectDraw4>();
    TestDirectDrawCreate<IDirectDraw7>();
    TestDirectDrawCreate<IDirectDraw7Ex>();
}

// Main function
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);

	Logging::Open("ddraw-testing.log");
	Logging::LogVideoCard();
	Logging::LogOSVersion();

    RunAllTests();

    Logging::Log() << "Exiting...";
    
    return 0;
}
