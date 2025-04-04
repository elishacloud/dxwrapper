#include "ddraw-testing.h"
#include "testing-harness.h"

template <typename DDType>
void TestCreateClipper(DDType* pDDraw)
{
    IDirectDrawClipper* pClipper = nullptr;
    HRESULT hr = pDDraw->CreateClipper(0, &pClipper, nullptr);

    // ****  650  ****
    DWORD TestID = 650;
    if (FAILED(hr))
    {
        LOG_TEST_RESULT(TestID, "Failed to create clipper. Error: ", (DDERR)hr, TEST_FAILED);
        return;
    }
    LOG_TEST_RESULT(TestID, "Clipper created. Ref count: ", GetRefCount(pClipper), GetResults<DDType>(TestID));

    // ****  651  ****
    TestID = 651;
    LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

    pClipper->Release();

    // ****  652  ****
    TestID = 652;
    LOG_TEST_RESULT(TestID, "After clipper release. DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
}

template <typename DDType>
void TestCreatePalette(DDType* pDDraw)
{
    PALETTEENTRY DDColorArray[256] = {};

    IDirectDrawPalette* pPalette = nullptr;
    HRESULT hr = pDDraw->CreatePalette(DDPCAPS_8BIT, DDColorArray, &pPalette, nullptr);

    // ****  600  ****
    DWORD TestID = 600;
    if (FAILED(hr))
    {
        LOG_TEST_RESULT(TestID, "Failed to create palette. Error: ", (DDERR)hr, TEST_FAILED);
        return;
    }
    LOG_TEST_RESULT(TestID, "Palette created. Ref count: ", GetRefCount(pPalette), GetResults<DDType>(TestID));

    // ****  601  ****
    TestID = 601;
    LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

    pPalette->Release();

    // ****  602  ****
    TestID = 602;
    LOG_TEST_RESULT(TestID, "After palette release. DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
}

static HRESULT CALLBACK ConvertCallback(LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
    UNREFERENCED_PARAMETER(lpContext);

    if (lpDDSurfaceDesc)
    {
        Logging::Log() << __FUNCTION__ " " << lpDDSurfaceDesc->dwWidth << "x" << lpDDSurfaceDesc->dwHeight <<
            " " << lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount << " " << lpDDSurfaceDesc->dwRefreshRate;
    }
    return D3DENUMRET_OK;
}

static HRESULT CALLBACK ConvertCallback2(LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext)
{
    UNREFERENCED_PARAMETER(lpContext);

    if (lpDDSurfaceDesc)
    {
        Logging::Log() << __FUNCTION__ " " << lpDDSurfaceDesc->dwWidth << "x" << lpDDSurfaceDesc->dwHeight <<
            " " << lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount << " " << lpDDSurfaceDesc->dwRefreshRate;
    }
    return D3DENUMRET_OK;
}

template <typename DDType>
DDType* TestDirectDrawCreateT()
{
    DDType* pDDraw = nullptr;

    // ****  100  ****
    DWORD TestID = 100;
    if constexpr (std::is_same_v<DDType, IDirectDraw7Ex>)
    {
        if (!pDirectDrawCreateEx)
        {
            LOG_TEST_RESULT(TestID, "DirectDrawCreateEx function not found! ", 0, TEST_FAILED);
            return nullptr;
        }

        HRESULT hr = pDirectDrawCreateEx(nullptr, reinterpret_cast<LPVOID*>(&pDDraw), IID_IDirectDraw7, nullptr);
        if (FAILED(hr))
        {
            LOG_TEST_RESULT(TestID, "DirectDrawCreateEx failed with error: ", (DDERR)hr, TEST_FAILED);
            return nullptr;
        }
        LOG_TEST_RESULT(TestID, "DirectDrawCreateEx device created. Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
    }
    else
    {
        if (!pDirectDrawCreate)
        {
            LOG_TEST_RESULT(TestID, "DirectDrawCreate function not found! ", 0, TEST_FAILED);
            return nullptr;
        }

        HRESULT hr = pDirectDrawCreate(nullptr, reinterpret_cast<LPDIRECTDRAW*>(&pDDraw), nullptr);
        if (FAILED(hr))
        {
            LOG_TEST_RESULT(TestID, "DirectDrawCreate failed with error: ", (DDERR)hr, TEST_FAILED);
            return nullptr;
        }
        LOG_TEST_RESULT(TestID, "DirectDrawCreate device created. Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
    }

    // Get version interface
    REFIID riid =
        std::is_same_v<DDType, IDirectDraw> ? IID_IDirectDraw :
        std::is_same_v<DDType, IDirectDraw2> ? IID_IDirectDraw2 :
        std::is_same_v<DDType, IDirectDraw3> ? IID_IDirectDraw3 :
        std::is_same_v<DDType, IDirectDraw4> ? IID_IDirectDraw4 :
        std::is_same_v<DDType, IDirectDraw7> ||
        std::is_same_v<DDType, IDirectDraw7Ex> ? IID_IDirectDraw7 : IID_IUnknown;

    // ****  101  ****
    TestID = 101;
    DDType* pDDraw2 = nullptr;
    HRESULT hr = pDDraw->QueryInterface(riid, reinterpret_cast<LPVOID*>(&pDDraw2));
    if (SUCCEEDED(hr))
    {
        LOG_TEST_RESULT(TestID, riid << " Ref count: ", GetRefCount(pDDraw2), GetResults<DDType>(TestID));

        // ****  102  ****
        TestID = 102;
        LOG_TEST_RESULT(TestID, "IDirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

        // ****  103  ****
        TestID = 103;
        UINT ref = pDDraw->Release();
        LOG_TEST_RESULT(TestID, "Release IDirectDraw: ", ref, GetResults<DDType>(TestID));

        // ****  104  ****
        TestID = 104;
        LOG_TEST_RESULT(TestID, riid << " Ref count ", GetRefCount(pDDraw2), GetResults<DDType>(TestID));

        return pDDraw2;
    }
    LOG_TEST_RESULT(TestID, "Failed to QueryInterface " << riid << " ", (DDERR)hr, GetResults<DDType>(TestID));

    pDDraw->Release();

    return nullptr;
}

template void TestDirectDrawCreate<IDirectDraw>();
template void TestDirectDrawCreate<IDirectDraw2>();
template void TestDirectDrawCreate<IDirectDraw3>();
template void TestDirectDrawCreate<IDirectDraw4>();
template void TestDirectDrawCreate<IDirectDraw7>();
template void TestDirectDrawCreate<IDirectDraw7Ex>();
template <typename DDType>
void TestDirectDrawCreate()
{
    Logging::Log() << "****";
    Logging::Log() << "**** Testing version: " << typeid(DDType).name();
    Logging::Log() << "****";

    DDType* pDDraw = TestDirectDrawCreateT<DDType>();
    if (!pDDraw)
    {
        return;
    }

    // Test setting cooperative level
    HRESULT hr = pDDraw->SetCooperativeLevel(DDhWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);

    // ****  105  ****
    DWORD TestID = 105;
    LOG_TEST_RESULT(TestID, "SetCooperativeLevel result: ", (DDERR)hr, (DDERR)GetResults<DDType>(TestID));

    // For IDirectDraw4 and IDirectDraw7, set a display mode before creating surfaces
    if constexpr (std::is_same_v<DDType, IDirectDraw>)
    {
        hr = pDDraw->SetDisplayMode(640, 480, 32);
    }
    else
    {
        hr = pDDraw->SetDisplayMode(640, 480, 32, 0, 0);
    }

    // ****  106  ****
    TestID = 106;
    LOG_TEST_RESULT(TestID, "SetDisplayMode result: ", (DDERR)hr, (DDERR)GetResults<DDType>(TestID));

    bool EnableDisplayLog = false;
    if (EnableDisplayLog)
    {
        if constexpr (std::is_same_v<DDType, IDirectDraw7Ex> || std::is_same_v<DDType, IDirectDraw7> || std::is_same_v<DDType, IDirectDraw4>)
        {
            pDDraw->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, nullptr, ConvertCallback2);
        }
        else
        {
            pDDraw->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, nullptr, ConvertCallback);
        }
    }

    // Test creating surfaces
    TestCreateSurface(pDDraw);

    // Test creating palettes
    TestCreatePalette(pDDraw);

    // Test creating clippers
    TestCreateClipper(pDDraw);

    // Test creating a Direct3D interface
    TestCreateDirect3D(pDDraw);

    // Test All Interfaces
    TestQueryInterfaces<DDType>(pDDraw, reinterpret_cast<IUnknown*>(pDDraw), "IDirectDraw", 1000);

    // Cleanup
    pDDraw->Release();
}
