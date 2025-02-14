#include "ddraw-testing.h"
#include "testing-harness.h"

template <typename DDType, typename D3DType>
void TestCreateDirect3DT(DDType* pDDraw)
{
    // Get version interface
    REFIID riid =
        std::is_same_v<DDType, IDirectDraw> ? IID_IDirect3D :
        std::is_same_v<DDType, IDirectDraw2> ? IID_IDirect3D2 :
        std::is_same_v<DDType, IDirectDraw3> || std::is_same_v<DDType, IDirectDraw4> ? IID_IDirect3D3 : IID_IDirect3D7;

    D3DType* pDirect3D = nullptr;
    HRESULT hr = pDDraw->QueryInterface(riid, reinterpret_cast<LPVOID*>(&pDirect3D));

    // ****  500  ****
    DWORD TestID = 500;
    if (SUCCEEDED(hr))
    {
        LOG_TEST_RESULT(TestID, "Successfully created " << riid << ". Ref count: ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

        // ****  501  ****
        TestID = 501;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

        // ****  502  ****
        TestID = 502;
        pDirect3D->AddRef();
        LOG_TEST_RESULT(TestID, "AddRef " << riid << " Ref count: ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));

        // ****  503  ****
        TestID = 503;
        LOG_TEST_RESULT(TestID, "DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));
        pDirect3D->Release();

        // ****  504  ****
        TestID = 504;
        pDDraw->AddRef();
        LOG_TEST_RESULT(TestID, "AddRef DirectDraw Ref count: ", GetRefCount(pDDraw), GetResults<DDType>(TestID));

        // ****  505  ****
        TestID = 505;
        LOG_TEST_RESULT(TestID, riid << " Ref count: ", GetRefCount(pDirect3D), GetResults<DDType>(TestID));
        pDDraw->Release();

        pDirect3D->Release();
    }
    else
    {
        LOG_TEST_RESULT(TestID, "Failed to created " << riid << ". Error: ", (DDERR)hr, GetResults<DDType>(TestID));
    }
}

template void TestCreateDirect3D<IDirectDraw>(IDirectDraw*);
template void TestCreateDirect3D<IDirectDraw2>(IDirectDraw2*);
template void TestCreateDirect3D<IDirectDraw3>(IDirectDraw3*);
template void TestCreateDirect3D<IDirectDraw4>(IDirectDraw4*);
template void TestCreateDirect3D<IDirectDraw7>(IDirectDraw7*);
template void TestCreateDirect3D<IDirectDraw7Ex>(IDirectDraw7Ex*);
template <typename DDType>
void TestCreateDirect3D(DDType* pDDraw)
{
    // Test creating a surface
    if constexpr (std::is_same_v<DDType, IDirectDraw>)
    {
        TestCreateDirect3DT<DDType, IDirect3D>(pDDraw);
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw2>)
    {
        TestCreateDirect3DT<DDType, IDirect3D2>(pDDraw);
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw3> || std::is_same_v<DDType, IDirectDraw4>)
    {
        TestCreateDirect3DT<DDType, IDirect3D3>(pDDraw);
    }
    else if constexpr (std::is_same_v<DDType, IDirectDraw7> || std::is_same_v<DDType, IDirectDraw7Ex>)
    {
        TestCreateDirect3DT<DDType, IDirect3D7>(pDDraw);
    }
}
