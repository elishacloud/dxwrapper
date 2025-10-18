/**
* Copyright (C) 2025 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "d3d9.h"
#include "ddraw\Shaders\GammaPixelShader.h"

template<typename CreateFunc>
static void TestDeviceRefChange(IDirect3DDevice9* device, const char* name, CreateFunc createFunc, DWORD ExpectedRefCount)
{
    // Get reference count before creation
    ULONG before = device->AddRef();
    device->Release();

    // Create the object
    IUnknown* obj = nullptr;
    HRESULT hr = createFunc(&obj);

    // Get reference count after creation
    ULONG after = device->AddRef();
    device->Release();

    if (SUCCEEDED(hr) && obj)
    {
        obj->Release(); // release the created object
    }

    ULONG ref = after - before;
    if (ExpectedRefCount != ref)
    {
        Logging::Log() << __FUNCTION__ << " Error: " << name << " has incorrect reference count. " << ref << " -> " << ExpectedRefCount << " hr = " << (D3DERR)hr;
    }
}

void D3d9Wrapper::TestAllDeviceRefs(IDirect3DDevice9* device)
{
    TestDeviceRefChange(device, "Texture", [&](IUnknown** out) {
        IDirect3DTexture9* tex = nullptr;
        HRESULT hr = device->CreateTexture(64, 64, 1, 0, D3DFMT_DXT1, D3DPOOL_DEFAULT, &tex, nullptr);
        *out = tex;
        return hr;
        }, 1);

    TestDeviceRefChange(device, "StateBlock", [&](IUnknown** out) {
        IDirect3DStateBlock9* sb = nullptr;
        HRESULT hr = device->CreateStateBlock(D3DSBT_ALL, &sb);
        *out = sb;
        return hr;
        }, 1);

    if (Config.WindowModeGammaShader)
    {
        TestDeviceRefChange(device, "Pixel Shader", [&](IUnknown** out) {
            IDirect3DPixelShader9* ps = nullptr;
            HRESULT hr = device->CreatePixelShader((DWORD*)GammaPixelShaderSrc, &ps);
            *out = ps;
            return hr;
            }, 1);
    }

    if (Config.ShowFPSCounter)
    {
        TestDeviceRefChange(device, "D3DXFont", [&](IUnknown** out) {
            ID3DXFont* font = nullptr;
            HRESULT hr = D3DXCreateFontW(device, 16, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                L"Arial", &font);
            *out = font;
            return hr;
            }, 1);

        TestDeviceRefChange(device, "D3DXSprite", [&](IUnknown** out) {
            ID3DXSprite* sprite = nullptr;
            HRESULT hr = D3DXCreateSprite(device, &sprite);
            *out = sprite;
            return hr;
            }, 2);
    }
}
