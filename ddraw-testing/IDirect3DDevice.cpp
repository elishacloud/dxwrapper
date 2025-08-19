#include "ddraw-testing.h"
#include "testing-harness.h"


template <typename DDType, typename D3DDType>
void TestExecuteBuffer(DDType* pDDraw, D3DDType* pDirect3DDevice)
{
    IDirect3DExecuteBuffer* pBuffer = nullptr;

    D3DEXECUTEBUFFERDESC eDesc = {};
    eDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    eDesc.dwFlags = D3DDEB_BUFSIZE;
    eDesc.dwBufferSize = 15600;

    // ****  900  ****
    DWORD TestID = 900;
    HRESULT hr = pDirect3DDevice->CreateExecuteBuffer(&eDesc, &pBuffer, nullptr);
    if (FAILED(hr))
    {
        LOG_TEST_RESULT(TestID, "Failed to create ExecuteBuffer. Error: ", (DDERR)hr, TEST_FAILED);
        return;
    }

    LOG_TEST_RESULT(TestID, "ExecuteBuffer created. Ref count: ", GetRefCount(pBuffer), GetResults<DDType>(TestID));

    // ****  901  ****
    TestID = 901;
    LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

    // ****  902  ****
    TestID = 902;
    LOG_TEST_RESULT(TestID, "Direct3DDevice Ref count: ", GetRefCount(pDirect3DDevice), GetResults<DDType>(TestID));

    pBuffer->Release();

    // ****  903  ****
    TestID = 903;
    LOG_TEST_RESULT(TestID, "After ExecuteBuffer release. Direct3DDevice Ref count: ", GetRefCount(pDirect3DDevice), GetResults<DDType>(TestID));
}

template <typename D3DDType>
void TestRenderState(D3DDType* pDevice, const DWORD DefaultValue[], const DWORD Unchangeable[], size_t ArraySize)
{
    for (UINT x = 0; x < D3D_MAXRENDERSTATES; x++)
    {
        DWORD rsValue = 0;
        pDevice->GetRenderState((D3DRENDERSTATETYPE)x, &rsValue);

        if (rsValue != DefaultValue[x])
        {
            LOG_TEST_RESULT(x, "Failed to get correct Render State. Error: ", rsValue, DefaultValue[x]);
        }

        bool ChangeValue = true;
        for (UINT y = 0; y < ArraySize; y++)
        {
            if (Unchangeable[y] == x)
            {
                ChangeValue = false;
            }
        }

        if (ChangeValue)
        {
            // Enable setting
            DWORD NewValue = FALSE;
            pDevice->SetRenderState((D3DRENDERSTATETYPE)x, NewValue);
            pDevice->GetRenderState((D3DRENDERSTATETYPE)x, &rsValue);

            if (rsValue != NewValue)
            {
                LOG_TEST_RESULT(x, "Failed to set Render State to FALSE. Error: ", rsValue, NewValue);
            }

            // Disable setting
            NewValue = TRUE;
            pDevice->SetRenderState((D3DRENDERSTATETYPE)x, NewValue);
            pDevice->GetRenderState((D3DRENDERSTATETYPE)x, &rsValue);

            if (rsValue != NewValue)
            {
                LOG_TEST_RESULT(x, "Failed to set Render State to TRUE. Error: ", rsValue, NewValue);
            }
        }
        // Check if setting can be modified
        else
        {
            DWORD NewValue = rsValue ? FALSE : TRUE;
            pDevice->SetRenderState((D3DRENDERSTATETYPE)x, NewValue);
            pDevice->GetRenderState((D3DRENDERSTATETYPE)x, &rsValue);

            if (rsValue == NewValue)
            {
                LOG_TEST_RESULT(x, "Failed! Render State shouldn't be able to be changed. Error: ", rsValue, NewValue);
            }
        }
    }
}

template <typename DDType, typename DSType, typename DSDesc, typename D3DType, typename D3DDType>
void TestCreate3DDeviceT(DDType* pDDraw, D3DType* pDirect3D)
{
    // Create primary surface with backbuffer
    DSDesc ddsd = {};
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;

    DSType* pSurface1 = nullptr;
    HRESULT hr = pDDraw->CreateSurface(&ddsd, &pSurface1, nullptr);

    // ****  800  ****
    DWORD TestID = 800;
    if (FAILED(hr))
    {
        LOG_TEST_RESULT(TestID, "Failed to create 3D Surface1. Error: ", (DDERR)hr, TEST_FAILED);
        return;
    }

    LOG_TEST_RESULT(TestID, "3D Surface1 created. Ref count: ", GetRefCount(pSurface1), GetResults<DDType>(TestID));

    // ****  801  ****
    TestID = 801;
    LOG_TEST_RESULT(TestID, "Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

    // ****  802  ****
    TestID = 802;
    LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

    // Create Z-buffer surface
    DSDesc zddsd = {};
    zddsd.dwSize = sizeof(zddsd);
    zddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    zddsd.dwWidth = 640;
    zddsd.dwHeight = 480;
    zddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;

    // Set pixel format for z-buffer (example: 16-bit Z-buffer)
    if constexpr (std::is_same_v<D3DType, IDirect3D> || std::is_same_v<D3DType, IDirect3D2>)
    {
        zddsd.dwFlags |= DDSD_ZBUFFERBITDEPTH;
        zddsd.dwZBufferBitDepth = 16;
    }
    else
    {
        zddsd.dwFlags |= DDSD_PIXELFORMAT;
        zddsd.ddpfPixelFormat.dwSize = sizeof(zddsd.ddpfPixelFormat);
        zddsd.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
        zddsd.ddpfPixelFormat.dwZBufferBitDepth = 16;
        zddsd.ddpfPixelFormat.dwZBitMask = 0xFFFF;
    }

    DSType* pZBuffer = nullptr;
    hr = pDDraw->CreateSurface(&zddsd, &pZBuffer, nullptr);

    // ****  838  ****
    TestID = 838;
    if (FAILED(hr))
    {
        LOG_TEST_RESULT(TestID, "Failed to create Z-buffer surface", (DDERR)hr, TEST_FAILED);
        pSurface1->Release();
        return;
    }

    LOG_TEST_RESULT(TestID, "zBuffer Surface created. Ref count: ", GetRefCount(pZBuffer), GetResults<DDType>(TestID));

    // ****  839  ****
    TestID = 839;
    LOG_TEST_RESULT(TestID, "3D Surface1 Ref count: ", GetRefCount(pSurface1), GetResults<DDType>(TestID));

    // ****  840  ****
    TestID = 840;
    LOG_TEST_RESULT(TestID, "Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

    // ****  841  ****
    TestID = 841;
    LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

    // Attach Z-buffer to the back buffer
    hr = pSurface1->AddAttachedSurface(pZBuffer);

    // ****  842  ****
    TestID = 842;
    if (FAILED(hr))
    {
        LOG_TEST_RESULT(TestID, "Failed to attach Z-buffer", (DDERR)hr, TEST_FAILED);
        pZBuffer->Release();
        pSurface1->Release();
        return;
    }

    LOG_TEST_RESULT(TestID, "zBuffer Attached. Ref count: ", GetRefCount(pZBuffer), GetResults<DDType>(TestID));

    // ****  843  ****
    TestID = 843;
    LOG_TEST_RESULT(TestID, "3D Surface1 Ref count: ", GetRefCount(pSurface1), GetResults<DDType>(TestID));

    // ****  844  ****
    TestID = 844;
    LOG_TEST_RESULT(TestID, "Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

    // ****  845  ****
    TestID = 845;
    LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

    D3DDType* pD3DDevice1 = nullptr;
    hr = DDERR_GENERIC;

    if constexpr (std::is_same_v<D3DType, IDirect3D>)
    {
        // The Direct3DDevice object is obtained through the appropriate call to the IDirect3DDevice::QueryInterface
        // method from a DirectDrawSurface object that was created as a 3D-capable surface.
        hr = pSurface1->QueryInterface(IID_IDirect3DHALDevice, reinterpret_cast<LPVOID*>(&pD3DDevice1));
    }
    else if constexpr (std::is_same_v<D3DType, IDirect3D2>)
    {
        hr = pDirect3D->CreateDevice(IID_IDirect3DHALDevice, pSurface1, &pD3DDevice1);

        if (SUCCEEDED(hr))
        {
            TestRenderState<IDirect3DDevice2>(pD3DDevice1, DefaultRenderTargetDX5, UnchangeableRenderTarget, sizeof(UnchangeableRenderTarget) / sizeof(UnchangeableRenderTarget[0]));
        }
    }
    else if constexpr (std::is_same_v<D3DType, IDirect3D3>)
    {
        hr = pDirect3D->CreateDevice(IID_IDirect3DHALDevice, pSurface1, &pD3DDevice1, nullptr);

        if (SUCCEEDED(hr))
        {
            TestRenderState<IDirect3DDevice3>(pD3DDevice1, DefaultRenderTargetDX6, UnchangeableRenderTarget, sizeof(UnchangeableRenderTarget) / sizeof(UnchangeableRenderTarget[0]));
        }
    }
    else if constexpr (std::is_same_v<D3DType, IDirect3D7>)
    {
        hr = pDirect3D->CreateDevice(IID_IDirect3DHALDevice, pSurface1, &pD3DDevice1);

        if (SUCCEEDED(hr))
        {
            TestRenderState<IDirect3DDevice7>(pD3DDevice1, DefaultRenderTargetDX7, UnchangeableRenderTargetDX7, sizeof(UnchangeableRenderTargetDX7) / sizeof(UnchangeableRenderTargetDX7[0]));
        }
    }

    // ****  803  ****
    TestID = 803;
    if (SUCCEEDED(hr))
    {
        LOG_TEST_RESULT(TestID, "3D Device1 created. Ref count: ", GetRefCount(pD3DDevice1), GetResults<DDType>(TestID));

        // ****  804  ****
        TestID = 804;
        LOG_TEST_RESULT(TestID, "3D Surface1 Ref count ", GetRefCount(pSurface1), GetResults<DDType>(TestID));

        // ****  805  ****
        TestID = 805;
        LOG_TEST_RESULT(TestID, "Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

        // ****  806  ****
        TestID = 806;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

        // Test execute buffer
        if constexpr (std::is_same_v<D3DDType, IDirect3DDevice>)
        {
            TestExecuteBuffer(pDDraw, pD3DDevice1);
        }

        // Create surface2
        ddsd = {};
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN;
        ddsd.dwWidth = 640;
        ddsd.dwHeight = 480;

        DSType* pSurface2 = nullptr;
        hr = pDDraw->CreateSurface(&ddsd, &pSurface2, nullptr);

        // ****  807  ****
        TestID = 807;
        if (SUCCEEDED(hr))
        {
            LOG_TEST_RESULT(TestID, "3D surface2 created. Ref count: ", GetRefCount(pSurface2), GetResults<DDType>(TestID));

            // ****  808  ****
            TestID = 808;
            LOG_TEST_RESULT(TestID, "Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

            // ****  809  ****
            TestID = 809;
            LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

            D3DDType* pD3DDevice2 = nullptr;
            hr = DDERR_GENERIC;

            if constexpr (std::is_same_v<D3DType, IDirect3D>)
            {
                // The Direct3DDevice object is obtained through the appropriate call to the IDirect3DDevice::QueryInterface
                // method from a DirectDrawSurface object that was created as a 3D-capable surface.
                hr = pSurface2->QueryInterface(IID_IDirect3DHALDevice, reinterpret_cast<LPVOID*>(&pD3DDevice2));
            }
            else if constexpr (std::is_same_v<D3DType, IDirect3D2>)
            {
                hr = pDirect3D->CreateDevice(IID_IDirect3DHALDevice, pSurface2, &pD3DDevice2);
            }
            else if constexpr (std::is_same_v<D3DType, IDirect3D3>)
            {
                hr = pDirect3D->CreateDevice(IID_IDirect3DHALDevice, pSurface2, &pD3DDevice2, nullptr);
            }
            else if constexpr (std::is_same_v<D3DType, IDirect3D7>)
            {
                hr = pDirect3D->CreateDevice(IID_IDirect3DHALDevice, pSurface2, &pD3DDevice2);
            }

            // ****  810  ****
            TestID = 810;
            if (SUCCEEDED(hr))
            {
                LOG_TEST_RESULT(TestID, "3D Device2 created. Do they match: ", (BOOL)(pD3DDevice1 == pD3DDevice2), GetResults<DDType>(TestID));

                // ****  811  ****
                TestID = 811;
                LOG_TEST_RESULT(TestID, "3D Device2 Ref count: ", GetRefCount(pD3DDevice2), GetResults<DDType>(TestID));

                // ****  812  ****
                TestID = 812;
                LOG_TEST_RESULT(TestID, "3D Surface2 Ref count: ", GetRefCount(pSurface2), GetResults<DDType>(TestID));

                // ****  813  ****
                TestID = 813;
                LOG_TEST_RESULT(TestID, "3D Device1 Ref count: ", GetRefCount(pD3DDevice1), GetResults<DDType>(TestID));

                // ****  814  ****
                TestID = 814;
                LOG_TEST_RESULT(TestID, "3D Surface1 Ref count ", GetRefCount(pSurface1), GetResults<DDType>(TestID));

                // ****  815  ****
                TestID = 815;
                LOG_TEST_RESULT(TestID, "Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

                // ****  816  ****
                TestID = 816;
                LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

                pD3DDevice2->Release();

                // ****  817  ****
                TestID = 817;
                LOG_TEST_RESULT(TestID, "3D Surface2 created. Ref count: ", GetRefCount(pSurface2), GetResults<DDType>(TestID));

                // ****  818  ****
                TestID = 818;
                LOG_TEST_RESULT(TestID, "3D Device1 Ref count: ", GetRefCount(pD3DDevice1), GetResults<DDType>(TestID));

                // ****  819  ****
                TestID = 819;
                LOG_TEST_RESULT(TestID, "3D Surface1 Ref count ", GetRefCount(pSurface1), GetResults<DDType>(TestID));

                // ****  820  ****
                TestID = 820;
                LOG_TEST_RESULT(TestID, "After 3D Device2 release. Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

                // ****  821  ****
                TestID = 821;
                LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
            }
            else
            {
                LOG_TEST_RESULT(TestID, "Failed to create 3D Device2. Error: ", (DDERR)hr, TEST_FAILED);
            }

            pSurface2->Release();

            // ****  822  ****
            TestID = 822;
            LOG_TEST_RESULT(TestID, "After 3D Surface2 release. Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

            // ****  823  ****
            TestID = 823;
            LOG_TEST_RESULT(TestID, "3D Device1 Ref count: ", GetRefCount(pD3DDevice1), GetResults<DDType>(TestID));

            // ****  824  ****
            TestID = 824;
            LOG_TEST_RESULT(TestID, "3D Surface1 Ref count ", GetRefCount(pSurface1), GetResults<DDType>(TestID));

            // ****  825  ****
            TestID = 825;
            LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
        }
        else
        {
            LOG_TEST_RESULT(TestID, "Failed to create 3D Surface2. Error: ", (DDERR)hr, TEST_FAILED);
        }

        pD3DDevice1->AddRef();

        // ****  830  ****
        TestID = 830;
        LOG_TEST_RESULT(TestID, "AddRef to 3D Device1. Ref count: ", GetRefCount(pD3DDevice1), GetResults<DDType>(TestID));

        // ****  831  ****
        TestID = 831;
        LOG_TEST_RESULT(TestID, "3D Surface1 Ref count ", GetRefCount(pSurface1), GetResults<DDType>(TestID));

        // ****  832  ****
        TestID = 832;
        LOG_TEST_RESULT(TestID, "Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

        // ****  833  ****
        TestID = 833;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

        pD3DDevice1->Release();

        pSurface1->AddRef();

        // ****  834  ****
        TestID = 834;
        LOG_TEST_RESULT(TestID, "AddRef to 3D Surface1. Ref count ", GetRefCount(pSurface1), GetResults<DDType>(TestID));

        // ****  835  ****
        TestID = 835;
        LOG_TEST_RESULT(TestID, "3D Device1 Ref count: ", GetRefCount(pD3DDevice1), GetResults<DDType>(TestID));

        // ****  836  ****
        TestID = 836;
        LOG_TEST_RESULT(TestID, "Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

        // ****  837  ****
        TestID = 837;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

        pZBuffer->Release();

        pSurface1->Release();

        pD3DDevice1->Release();

        // ****  826  ****
        TestID = 826;
        LOG_TEST_RESULT(TestID, "After 3D Device1 release. Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

        // ****  827  ****
        TestID = 827;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
    }
    else
    {
        LOG_TEST_RESULT(TestID, "Failed to create 3D Device1. Error: ", (DDERR)hr, TEST_FAILED);
    }

    pSurface1->Release();

    // ****  828  ****
    TestID = 828;
    LOG_TEST_RESULT(TestID, "After 3D Surface1 release. Direct3D Ref count ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

    // ****  829  ****
    TestID = 829;
    LOG_TEST_RESULT(TestID, "DirectDraw Ref count ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
}

template void TestCreate3DDevice<IDirectDraw, IDirect3D>(IDirectDraw*, IDirect3D*);
template void TestCreate3DDevice<IDirectDraw2, IDirect3D2>(IDirectDraw2*, IDirect3D2*);
template void TestCreate3DDevice<IDirectDraw3, IDirect3D3>(IDirectDraw3*, IDirect3D3*);
template void TestCreate3DDevice<IDirectDraw4, IDirect3D3>(IDirectDraw4*, IDirect3D3*);
template void TestCreate3DDevice<IDirectDraw7, IDirect3D7>(IDirectDraw7*, IDirect3D7*);
template void TestCreate3DDevice<IDirectDraw7Ex, IDirect3D7>(IDirectDraw7Ex*, IDirect3D7*);
template <typename DDType, typename D3DType>
void TestCreate3DDevice(DDType* pDDraw, D3DType* pDirect3D)
{
    UNREFERENCED_PARAMETER(pDDraw);
    UNREFERENCED_PARAMETER(pDirect3D);

    // Testing 3D Device
    if constexpr (std::is_same_v<D3DType, IDirect3D>)
    {
        TestCreate3DDeviceT<DDType, IDirectDrawSurface, DDSURFACEDESC, D3DType, IDirect3DDevice>(pDDraw, pDirect3D);
    }
    else if constexpr (std::is_same_v<D3DType, IDirect3D2>)
    {
        TestCreate3DDeviceT<DDType, IDirectDrawSurface, DDSURFACEDESC, D3DType, IDirect3DDevice2>(pDDraw, pDirect3D);
    }
    else if constexpr (std::is_same_v<D3DType, IDirect3D3> && std::is_same_v<DDType, IDirectDraw4>)
    {
        TestCreate3DDeviceT<DDType, IDirectDrawSurface4, DDSURFACEDESC2, D3DType, IDirect3DDevice3>(pDDraw, pDirect3D);
    }
    else if constexpr (std::is_same_v<D3DType, IDirect3D7>)
    {
        TestCreate3DDeviceT<DDType, IDirectDrawSurface7, DDSURFACEDESC2, D3DType, IDirect3DDevice7>(pDDraw, pDirect3D);
    }
}
