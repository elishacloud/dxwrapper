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
    if constexpr (std::is_same_v<DDType, IDirectDrawCoC>)
    {
        HRESULT hr = CoCreateInstance(CLSID_DirectDraw, nullptr, CLSCTX_INPROC_SERVER, IID_IDirectDraw, reinterpret_cast<void**>(&pDDraw));
        if (FAILED(hr))
        {
            LOG_TEST_RESULT(TestID, "CoCreateInstance for DirectDraw function failed! ", 0, TEST_FAILED);
            return nullptr;
        }

        pDDraw->Initialize(nullptr);
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw7CoC>)
    {
        HRESULT hr = CoCreateInstance(CLSID_DirectDraw7, nullptr, CLSCTX_INPROC_SERVER, IID_IDirectDraw7, reinterpret_cast<void**>(&pDDraw));
        if (FAILED(hr))
        {
            LOG_TEST_RESULT(TestID, "CoCreateInstance for DirectDraw7 function failed! ", 0, TEST_FAILED);
            return nullptr;
        }

        pDDraw->Initialize(nullptr);
    }
    else if constexpr (std::is_same_v<DDType, IDirectDrawClass>)
    {
        IClassFactory* pFactory = nullptr;

        HRESULT hr = CoGetClassObject(CLSID_DirectDraw, CLSCTX_INPROC_SERVER, nullptr, IID_IClassFactory, reinterpret_cast<void**>(&pFactory));
        if (FAILED(hr))
        {
            LOG_TEST_RESULT(TestID, "CoGetClassObject for DirectDraw function failed! ", 0, TEST_FAILED);
            return nullptr;
        }

        hr = pFactory->CreateInstance(nullptr, IID_IDirectDraw, reinterpret_cast<void**>(&pDDraw));
        if (FAILED(hr))
        {
            LOG_TEST_RESULT(TestID, "CreateInstance for CoGetClassObject for DirectDraw function failed! ", 0, TEST_FAILED);
            return nullptr;
        }

        pDDraw->Initialize(nullptr);

        pFactory->Release();
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw7Class>)
    {
        IClassFactory* pFactory = nullptr;

        HRESULT hr = CoGetClassObject(CLSID_DirectDraw7, CLSCTX_INPROC_SERVER, nullptr, IID_IClassFactory, reinterpret_cast<void**>(&pFactory));
        if (FAILED(hr))
        {
            LOG_TEST_RESULT(TestID, "CoGetClassObject for DirectDraw7 function failed! ", 0, TEST_FAILED);
            return nullptr;
        }

        hr = pFactory->CreateInstance(nullptr, IID_IDirectDraw7, reinterpret_cast<void**>(&pDDraw));
        if (FAILED(hr))
        {
            LOG_TEST_RESULT(TestID, "CreateInstance for CoGetClassObject for DirectDraw7 function failed! ", 0, TEST_FAILED);
            return nullptr;
        }

        pDDraw->Initialize(nullptr);

        pFactory->Release();
    }
    else if constexpr (std::is_same_v<DDType, IDirectDrawDDF>)
    {
        IDirectDrawFactory* pFactory = nullptr;

        HRESULT hr = CoCreateInstance(CLSID_DirectDrawFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IDirectDrawFactory, reinterpret_cast<void**>(&pFactory));
        if (FAILED(hr))
        {
            LOG_TEST_RESULT(TestID, "CoCreateInstance for DirectDrawFactory function failed! ", 0, TEST_FAILED);
            return nullptr;
        }

        hr = pFactory->CreateDirectDraw(nullptr, DDhWnd, DDSCL_NORMAL, 0, nullptr, reinterpret_cast<IDirectDraw**>(&pDDraw));
        if (FAILED(hr))
        {
            LOG_TEST_RESULT(TestID, "CreateInstance for CoGetClassObject for DirectDrawFactory function failed! ", 0, TEST_FAILED);
            return nullptr;
        }

        pDDraw->Initialize(nullptr);

        pFactory->Release();
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw7Ex>)
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
        std::is_same_v<DDType, IDirectDrawCoC> ? IID_IDirectDraw :
        std::is_same_v<DDType, IDirectDrawClass> ? IID_IDirectDraw :
        std::is_same_v<DDType, IDirectDrawDDF> ? IID_IDirectDraw :
        std::is_same_v<DDType, IDirectDraw2> ? IID_IDirectDraw2 :
        std::is_same_v<DDType, IDirectDraw3> ? IID_IDirectDraw3 :
        std::is_same_v<DDType, IDirectDraw4> ? IID_IDirectDraw4 : IID_IDirectDraw7;

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

template <typename DDType>
void TestDirectDrawCreateContT(DDType* pDDraw)
{
    // Test setting cooperative level
    HRESULT hr = pDDraw->SetCooperativeLevel(DDhWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);

    // ****  105  ****
    DWORD TestID = 105;
    LOG_TEST_RESULT(TestID, "SetCooperativeLevel result: ", (DDERR)hr, (DDERR)GetResults<DDType>(TestID));

    // For IDirectDraw4 and IDirectDraw7, set a display mode before creating surfaces
    if constexpr (std::is_same_v<DDType, IDirectDraw> || std::is_same_v<DDType, IDirectDrawDDF> || std::is_same_v<DDType, IDirectDrawCoC> || std::is_same_v<DDType, IDirectDrawClass>)
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
        if constexpr (std::is_same_v<DDType, IDirectDraw> || std::is_same_v<DDType, IDirectDrawDDF> || std::is_same_v<DDType, IDirectDraw2> || std::is_same_v<DDType, IDirectDraw3>)
        {
            pDDraw->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, nullptr, ConvertCallback);
        }
        else
        {
            pDDraw->EnumDisplayModes(DDEDM_REFRESHRATES, nullptr, nullptr, ConvertCallback2);
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

    // Resetting cooperative level
    pDDraw->SetCooperativeLevel(DDhWnd, DDSCL_NORMAL);

    // Cleanup
    pDDraw->Release();
}

template void TestDirectDrawCreate<IDirectDraw>();
template void TestDirectDrawCreate<IDirectDrawDDF>();
template void TestDirectDrawCreate<IDirectDrawCoC>();
template void TestDirectDrawCreate<IDirectDrawClass>();
template void TestDirectDrawCreate<IDirectDraw2>();
template void TestDirectDrawCreate<IDirectDraw3>();
template void TestDirectDrawCreate<IDirectDraw4>();
template void TestDirectDrawCreate<IDirectDraw7>();
template void TestDirectDrawCreate<IDirectDraw7CoC>();
template void TestDirectDrawCreate<IDirectDraw7Class>();
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

    // Continue to create device
    if constexpr (std::is_same_v<DDType, IDirectDraw> || std::is_same_v<DDType, IDirectDrawCoC> || std::is_same_v<DDType, IDirectDrawClass>)
    {
        TestDirectDrawCreateContT<IDirectDraw>(reinterpret_cast<IDirectDraw*>(pDDraw));
    }
    else if constexpr (std::is_same_v<DDType, IDirectDrawDDF>)
    {
        TestDirectDrawCreateContT<DDType>(pDDraw);
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw2>)
    {
        TestDirectDrawCreateContT<DDType>(pDDraw);
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw3>)
    {
        TestDirectDrawCreateContT<DDType>(pDDraw);
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw4>)
    {
        TestDirectDrawCreateContT<DDType>(pDDraw);
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw7>)
    {
        TestDirectDrawCreateContT<DDType>(pDDraw);
    }
    else
    {
        TestDirectDrawCreateContT<IDirectDraw7Ex>(reinterpret_cast<IDirectDraw7Ex*>(pDDraw));
    }
}
