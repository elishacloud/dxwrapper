#pragma once

#include "d3d9External.h"
#include <deque>
#include "GDI\GDI.h"

class m_IDirect3D9Ex;
class m_IDirect3DDevice9Ex;
class m_IDirect3DCubeTexture9;
class m_IDirect3DIndexBuffer9;
class m_IDirect3DPixelShader9;
class m_IDirect3DQuery9;
class m_IDirect3DStateBlock9;
class m_IDirect3DSurface9;
class m_IDirect3DSwapChain9Ex;
class m_IDirect3DTexture9;
class m_IDirect3DVertexBuffer9;
class m_IDirect3DVertexDeclaration9;
class m_IDirect3DVertexShader9;
class m_IDirect3DVolume9;
class m_IDirect3DVolumeTexture9;

#include "AddressLookupTable.h"
#include "IClassFactory\IClassFactory.h"
#include "Utils\Utils.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

struct D3DDISPLAYMODEEX_CONVERT : public D3DDISPLAYMODEEX {
	D3DDISPLAYMODEEX_CONVERT() {
		Size = sizeof(D3DDISPLAYMODEEX);
		Width = 0;
		Height = 0;
		RefreshRate = 0;
		Format = D3DFMT_UNKNOWN;
		ScanLineOrdering = D3DSCANLINEORDERING_UNKNOWN;
	}

	explicit D3DDISPLAYMODEEX_CONVERT(const D3DDISPLAYMODE& mode) {
		Size = sizeof(D3DDISPLAYMODEEX);
		Width = mode.Width;
		Height = mode.Height;
		RefreshRate = mode.RefreshRate;
		Format = mode.Format;
		ScanLineOrdering = D3DSCANLINEORDERING_UNKNOWN;
	}

	explicit D3DDISPLAYMODEEX_CONVERT(const D3DDISPLAYMODEEX& mode) {
		Size = mode.Size;
		Width = mode.Width;
		Height = mode.Height;
		RefreshRate = mode.RefreshRate;
		Format = mode.Format;
		ScanLineOrdering = mode.ScanLineOrdering;
	}

	D3DDISPLAYMODEEX_CONVERT& operator=(const D3DDISPLAYMODE& mode) {
		Size = sizeof(D3DDISPLAYMODEEX);
		Width = mode.Width;
		Height = mode.Height;
		RefreshRate = mode.RefreshRate;
		Format = mode.Format;
		ScanLineOrdering = D3DSCANLINEORDERING_UNKNOWN;
		return *this;
	}

	// Returns a pointer to the D3DDISPLAYMODE part (offset 4 bytes into the struct)
	D3DDISPLAYMODE* Ptr() {
		return reinterpret_cast<D3DDISPLAYMODE*>(reinterpret_cast<uint8_t*>(this) + sizeof(Size));
	}

	// Returns a pointer to the full D3DDISPLAYMODEEX struct
	D3DDISPLAYMODEEX* PtrEx() {
		return this;
	}

	// Returns the D3DDISPLAYMODE data
	D3DDISPLAYMODE Data() {
		return *this->Ptr();
	}

	// Returns the full D3DDISPLAYMODEEX data
	D3DDISPLAYMODEEX DataEx() {
		return *this;
	}
};

typedef int(WINAPI* D3DPERF_BeginEventProc)(D3DCOLOR, LPCWSTR);
typedef int(WINAPI* D3DPERF_EndEventProc)();
typedef DWORD(WINAPI* D3DPERF_GetStatusProc)();
typedef BOOL(WINAPI* D3DPERF_QueryRepeatFrameProc)();
typedef void(WINAPI* D3DPERF_SetMarkerProc)(D3DCOLOR, LPCWSTR);
typedef void(WINAPI* D3DPERF_SetOptionsProc)(DWORD);
typedef void(WINAPI* D3DPERF_SetRegionProc)(D3DCOLOR, LPCWSTR);
typedef IDirect3D9* (WINAPI* Direct3DCreate9Proc)(UINT);
typedef HRESULT(WINAPI* Direct3DCreate9ExProc)(UINT, IDirect3D9Ex**);
typedef IDirect3D9* (WINAPI* Direct3DCreate9On12Proc)(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries);
typedef HRESULT(WINAPI* Direct3DCreate9On12ExProc)(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries, IDirect3D9Ex** ppOutputInterface);

DWORD UpdateBehaviorFlags(DWORD BehaviorFlags);

namespace D3d9Wrapper
{
	void WINAPI genericQueryInterface(REFIID riid, LPVOID* ppvObj, m_IDirect3DDevice9Ex* m_pDeviceEx);
}

#include "IDirect3DDevice9Ex.h"
#include "IDirect3DCubeTexture9.h"
#include "IDirect3DIndexBuffer9.h"
#include "IDirect3DPixelShader9.h"
#include "IDirect3DQuery9.h"
#include "IDirect3DStateBlock9.h"
#include "IDirect3DSurface9.h"
#include "IDirect3DSwapChain9Ex.h"
#include "IDirect3DTexture9.h"
#include "IDirect3DVertexBuffer9.h"
#include "IDirect3DVertexDeclaration9.h"
#include "IDirect3DVertexShader9.h"
#include "IDirect3DVolume9.h"
#include "IDirect3DVolumeTexture9.h"
