#include "ddraw-testing.h"
#include "testing-harness.h"

template <typename DDType, typename T>
void TestCreateGammaControl(DDType* pDDraw, T* pInterface, const char* ParentName, DWORD TestIDBase)
{
    IDirectDrawGammaControl* pGammaControl = nullptr;
    HRESULT hr = pInterface->QueryInterface(IID_IDirectDrawGammaControl, reinterpret_cast<LPVOID*>(&pGammaControl));

    // ****  Base + 0  ****
    DWORD TestID = TestIDBase + 0;
    if (SUCCEEDED(hr))
    {
        LOG_TEST_RESULT(TestID, "GammaControl created from " << ParentName << ". Ref count: ", GetRefCount(pGammaControl), GetResults<DDType>(TestID));

        // ****  Base + 1  ****
        TestID = TestIDBase + 1;
        LOG_TEST_RESULT(TestID, "Base interface Ref count: ", GetRefCount(pInterface), GetResults<DDType>(TestID));

        // ****  Base + 2  ****
        TestID = TestIDBase + 2;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

        IDirectDrawGammaControl* pGammaControl2 = nullptr;
        hr = pDDraw->QueryInterface(IID_IDirectDrawGammaControl, reinterpret_cast<LPVOID*>(&pGammaControl2));

        // ****  Base + 3  ****
        TestID = TestIDBase + 3;
        if (SUCCEEDED(hr))
        {
            LOG_TEST_RESULT(TestID, "Query GammaControl from DirectDraw. Ref count: ", GetRefCount(pGammaControl2), GetResults<DDType>(TestID));

            // ****  Base + 4  ****
            TestID = TestIDBase + 4;
            LOG_TEST_RESULT(TestID, "GammaControl Ref count: ", GetRefCount(pGammaControl), GetResults<DDType>(TestID));

            pGammaControl2->Release();
        }
        else
        {
            LOG_TEST_RESULT(TestID, "Failed to query IDirectDrawGammaControl from DirectDraw. Error: ", (DDERR)hr, GetResults<DDType>(TestID));
        }

        pGammaControl->Release();
    }
    else
    {
        LOG_TEST_RESULT(TestID, "Failed to create IDirectDrawGammaControl from " << ParentName << ". Error: ", (DDERR)hr, GetResults<DDType>(TestID));
    }
}

template <typename DDType, typename DSType>
void TestCreateTexture(DDType* pDDraw, DSType* pSurface)
{
    IDirect3DTexture* pTexture = nullptr;
    HRESULT hr = pSurface->QueryInterface(IID_IDirect3DTexture, reinterpret_cast<LPVOID*>(&pTexture));

    // ****  300  ****
    DWORD TestID = 300;
    if (SUCCEEDED(hr))
    {
        LOG_TEST_RESULT(TestID, "Texture created. Ref count: ", GetRefCount(pTexture), GetResults<DDType>(TestID));

        // ****  301  ****
        TestID = 301;
        LOG_TEST_RESULT(TestID, "Surface Ref count: ", GetRefCount(pSurface), GetResults<DDType>(TestID));

        // ****  302  ****
        TestID = 302;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

        // ****  303  ****
        TestID = 303;
        pSurface->AddRef();
        LOG_TEST_RESULT(TestID, "AddRef Surface.  Ref count: ", GetRefCount(pSurface), GetResults<DDType>(TestID));

        // ****  304  ****
        TestID = 304;
        LOG_TEST_RESULT(TestID, "Texture Ref count: ", GetRefCount(pTexture), GetResults<DDType>(TestID));

        // ****  305  ****
        TestID = 305;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
        pSurface->Release();

        // ****  306  ****
        TestID = 306;
        pTexture->AddRef();
        LOG_TEST_RESULT(TestID, "AddRef Texture. Ref count: ", GetRefCount(pTexture), GetResults<DDType>(TestID));

        // ****  307  ****
        TestID = 307;
        LOG_TEST_RESULT(TestID, "Surface Ref count: ", GetRefCount(pSurface), GetResults<DDType>(TestID));

        // ****  308  ****
        TestID = 308;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
        pTexture->Release();

        IDirect3DTexture2* pTexture2 = nullptr;
        hr = pSurface->QueryInterface(IID_IDirect3DTexture2, reinterpret_cast<LPVOID*>(&pTexture2));
        if (SUCCEEDED(hr))
        {
            // ****  309  ****
            TestID = 309;
            LOG_TEST_RESULT(TestID, "Texture2 Ref count: ", GetRefCount(pTexture2), GetResults<DDType>(TestID));

            // ****  310  ****
            TestID = 310;
            LOG_TEST_RESULT(TestID, "Texture Ref count: ", GetRefCount(pTexture), GetResults<DDType>(TestID));

            // ****  311  ****
            TestID = 311;
            LOG_TEST_RESULT(TestID, "Surface Ref count: ", GetRefCount(pSurface), GetResults<DDType>(TestID));

            // ****  312  ****
            TestID = 312;
            LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

            // ****  313  ****
            TestID = 313;
            pTexture2->AddRef();
            LOG_TEST_RESULT(TestID, "AddRef Texture2. Ref count: ", GetRefCount(pTexture2), GetResults<DDType>(TestID));

            // ****  314  ****
            TestID = 314;
            LOG_TEST_RESULT(TestID, "Texture Ref count: ", GetRefCount(pTexture), GetResults<DDType>(TestID));

            // ****  315  ****
            TestID = 315;
            LOG_TEST_RESULT(TestID, "Surface Ref count: ", GetRefCount(pSurface), GetResults<DDType>(TestID));

            // ****  316  ****
            TestID = 316;
            LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
            pTexture2->Release();

            pTexture2->Release();
        }
        else
        {
            LOG_TEST_RESULT(TestID, "Failed to create IDirect3DTexture2. Error: ", (DDERR)hr, GetResults<DDType>(TestID));
        }

        pTexture->Release();
    }
    else
    {
        LOG_TEST_RESULT(TestID, "Failed to create IDirect3DTexture. Error: ", (DDERR)hr, GetResults<DDType>(TestID));
    }
}

template <typename DDType, typename DSType, typename DSDesc>
void TestCreateSurfaceT(DDType* pDDraw)
{
    // Create primary surface
    DSDesc primaryDesc = {};
    primaryDesc.dwSize = sizeof(primaryDesc);
    primaryDesc.dwFlags = DDSD_CAPS;
    primaryDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;

    DSType* pPrimarySurface = nullptr;
    HRESULT hr = pDDraw->CreateSurface(&primaryDesc, &pPrimarySurface, nullptr);

    // ****  200  ****
    DWORD TestID = 200;
    if (FAILED(hr))
    {
        LOG_TEST_RESULT(TestID, "Failed to create primary surface. Error: ", (DDERR)hr, TEST_FAILED);
        return;
    }
    LOG_TEST_RESULT(TestID, "Primary surface created. Ref count: ", GetRefCount(pPrimarySurface), GetResults<DDType>(TestID));

    // ****  201  ****
    TestID = 201;
    LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

    // Test GammaControl
    TestCreateGammaControl<DDType, DSType>(pDDraw, pPrimarySurface, "Primary Surface", 400);

    // Test QueryInterface
    for (int x = 0; x < 5; x++)
    {
        // Get version interface
        REFIID riid =
            x == 0 ? IID_IDirectDrawSurface :
            x == 1 ? IID_IDirectDrawSurface2 :
            x == 2 ? IID_IDirectDrawSurface3 :
            x == 3 ? IID_IDirectDrawSurface4 : IID_IDirectDrawSurface7;

        // ****  210 + X  ****
        TestID = 210 + x;
        IUnknown* pInterface = nullptr;
        hr = pPrimarySurface->QueryInterface(riid, reinterpret_cast<LPVOID*>(&pInterface));
        if (SUCCEEDED(hr))
        {
            LOG_TEST_RESULT(TestID, riid << " Ref count: ", GetRefCount(pInterface), GetResults<DDType>(TestID));

            // ****  210 + 5 + x  ****
            TestID = 210 + 5 + x;
            LOG_TEST_RESULT(TestID, "Parent surface Ref count ", GetRefCount(pPrimarySurface), GetResults<DDType>(TestID));

            // ****  210 + 10 + x  ****
            TestID = 210 + 10 + x;
            LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

            // Test GammaControl
            TestCreateGammaControl<DDType, DSType>(pDDraw, reinterpret_cast<DSType*>(pInterface), "QueryInterface Surface", 410 + (x * 5));

            pInterface->Release();
        }
        else
        {
            LOG_TEST_RESULT(TestID, "Failed to query for surface " << riid << ". Error: ", (DDERR)hr, TEST_FAILED);
        }
    }


    // Create an offscreen surface
    DSDesc ddsd = {};
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd.dwWidth = 640;
    ddsd.dwHeight = 480;

    DSType* pSurface = nullptr;
    hr = pDDraw->CreateSurface(&ddsd, &pSurface, nullptr);

    // ****  202  ****
    TestID = 202;
    if (SUCCEEDED(hr))
    {
        LOG_TEST_RESULT(TestID, "3D surface created. Ref count: ", GetRefCount(pSurface), GetResults<DDType>(TestID));

        // ****  203  ****
        TestID = 203;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

        // Test Texture
        TestCreateTexture(pDDraw, pSurface);

        // Test GammaControl
        TestCreateGammaControl<DDType, DSType>(pDDraw, pPrimarySurface, "3D Surface", 405);

        pSurface->Release();

        // ****  204  ****
        TestID = 204;
        LOG_TEST_RESULT(TestID, "After 3D surface release. DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
    }
    else
    {
        LOG_TEST_RESULT(TestID, "Failed to create 3D surface. Error: ", (DDERR)hr, TEST_FAILED);
    }

    // Cleanup primary surface
    pPrimarySurface->Release();

    // ****  205  ****
    TestID = 205;
    LOG_TEST_RESULT(TestID, "After primary surface release. DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
}

template void TestCreateSurface<IDirectDraw>(IDirectDraw*);
template void TestCreateSurface<IDirectDraw2>(IDirectDraw2*);
template void TestCreateSurface<IDirectDraw3>(IDirectDraw3*);
template void TestCreateSurface<IDirectDraw4>(IDirectDraw4*);
template void TestCreateSurface<IDirectDraw7>(IDirectDraw7*);
template void TestCreateSurface<IDirectDraw7Ex>(IDirectDraw7Ex*);
template <typename DDType>
void TestCreateSurface(DDType* pDDraw)
{
    // Test creating a surface
    if constexpr (std::is_same_v<DDType, IDirectDraw> || std::is_same_v<DDType, IDirectDraw2> || std::is_same_v<DDType, IDirectDraw3>)
    {
        TestCreateSurfaceT<IDirectDraw, IDirectDrawSurface, DDSURFACEDESC>(reinterpret_cast<IDirectDraw*>(pDDraw));
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw4>)
    {
        TestCreateSurfaceT<DDType, IDirectDrawSurface4, DDSURFACEDESC2>(reinterpret_cast<DDType*>(pDDraw));
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw7> || std::is_same_v<DDType, IDirectDraw7Ex>)
    {
        TestCreateSurfaceT<DDType, IDirectDrawSurface7, DDSURFACEDESC2>(reinterpret_cast<DDType*>(pDDraw));
    }
}
