#pragma once

#define INITGUID

#include <d3d9.h>
#include <ddraw.h>
#include <ddrawex.h>
#include <d3d.h>
#include <d3dhal.h>
#include <memory>

class m_IDirect3D;
class m_IDirect3D2;
class m_IDirect3D3;
class m_IDirect3D7;
class m_IDirect3DX;
class m_IDirect3DDevice;
class m_IDirect3DDevice2;
class m_IDirect3DDevice3;
class m_IDirect3DDevice7;
class m_IDirect3DDeviceX;
class m_IDirect3DMaterial;
class m_IDirect3DMaterial2;
class m_IDirect3DMaterial3;
class m_IDirect3DMaterialX;
class m_IDirect3DTexture;
class m_IDirect3DTexture2;
class m_IDirect3DTextureX;
class m_IDirect3DVertexBuffer;
class m_IDirect3DVertexBuffer7;
class m_IDirect3DVertexBufferX;
class m_IDirect3DViewport;
class m_IDirect3DViewport2;
class m_IDirect3DViewport3;
class m_IDirect3DViewportX;
class m_IDirectDraw;
class m_IDirectDraw2;
class m_IDirectDraw3;
class m_IDirectDraw4;
class m_IDirectDraw7;
class m_IDirectDrawX;
class m_IDirectDrawSurface;
class m_IDirectDrawSurface2;
class m_IDirectDrawSurface3;
class m_IDirectDrawSurface4;
class m_IDirectDrawSurface7;
class m_IDirectDrawSurfaceX;
class m_IDirectDrawPalette;
class m_IDirectDrawColorControl;
class m_IDirectDrawGammaControl;

#include "AddressLookupTable.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

#define DDWRAPPER_TYPEX 0x80

// Indicates surface was created using CreateSurface()
#define DDSCAPS4_CREATESURFACE  0x0001
// Indicates surface is a primary surface or a backbuffer of a primary surface
#define DDSCAPS4_PRIMARYSURFACE 0x0200

DEFINE_GUID(IID_GetRealInterface,    0x254e1ffd, 0x10f9, 0x10c0, 0xc1, 0xb8, 0x21, 0x7d, 0x59, 0xd1, 0xe5, 0xb2);
DEFINE_GUID(IID_GetInterfaceX,       0x254e1ffd, 0x10f9, 0x10c0, 0xc1, 0xb8, 0x21, 0x7d, 0x59, 0xd1, 0xe5, 0xb4);

// ddraw proc typedefs
typedef void(WINAPI *AcquireDDThreadLockProc)();
typedef void(WINAPI *CompleteCreateSysmemSurfaceProc)();
typedef HRESULT(WINAPI *D3DParseUnknownCommandProc)(LPVOID lpCmd, LPVOID *lpRetCmd);
typedef void(WINAPI *DDGetAttachedSurfaceLclProc)();
typedef void(WINAPI *DDInternalLockProc)();
typedef void(WINAPI *DDInternalUnlockProc)();
typedef void(WINAPI *DSoundHelpProc)();
typedef HRESULT(WINAPI *DirectDrawCreateProc)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
typedef HRESULT(WINAPI *DirectDrawCreateClipperProc)(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, LPUNKNOWN pUnkOuter);
typedef HRESULT(WINAPI *DirectDrawEnumerateAProc)(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
typedef HRESULT(WINAPI *DirectDrawEnumerateExAProc)(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef HRESULT(WINAPI *DirectDrawEnumerateExWProc)(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef HRESULT(WINAPI *DirectDrawEnumerateWProc)(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);
typedef HRESULT(WINAPI *DirectDrawCreateExProc)(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter);
typedef HRESULT(WINAPI *DllCanUnloadNowProc)();
typedef HRESULT(WINAPI *DllGetClassObjectProc)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
typedef void(WINAPI *GetDDSurfaceLocalProc)();
typedef HANDLE(WINAPI *GetOLEThunkDataProc)(int i1);
typedef HRESULT(WINAPI *GetSurfaceFromDCProc)(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS);
typedef void(WINAPI *RegisterSpecialCaseProc)();
typedef void(WINAPI *ReleaseDDThreadLockProc)();
typedef HRESULT(WINAPI *SetAppCompatDataProc)(DWORD, DWORD);
typedef LPDIRECT3D9(WINAPI *PFN_Direct3DCreate9)(UINT SDKVersion);

// ddraw proc forward declaration
HRESULT WINAPI dd_DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
HRESULT WINAPI dd_DirectDrawCreateEx(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter);
HRESULT WINAPI dd_DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
HRESULT WINAPI dd_DirectDrawEnumerateW(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);

// Function and variable forward declarations
namespace DdrawWrapper
{
	DWORD GetGUIDVersion(REFIID CalledID);
	REFIID ReplaceIIDUnknown(REFIID riid, REFIID guid);
	REFCLSID ConvertREFCLSID(REFCLSID rclsid);
	REFIID ConvertREFIID(REFIID riid);
	void SetCriticalSection();
	void ReleaseCriticalSection();
	void AddRef(void *lpvObj);
	HRESULT ProxyQueryInterface(LPVOID ProxyInterface, REFIID CalledID, LPVOID * ppvObj, REFIID CallerID, LPVOID WrapperInterface);
	void WINAPI genericQueryInterface(REFIID riid, LPVOID *ppvObj);
}

extern AddressLookupTableDdraw<void> ProxyAddressLookupTable;

enum DirectDrawEnumerateTypes
{
	DDET_ENUMCALLBACKA,
	DDET_ENUMCALLBACKEXA,
	DDET_ENUMCALLBACKEXW,
	DDET_ENUMCALLBACKW,
};

using namespace DdrawWrapper;

// Direct3D Version Wrappers
#include "Versions\IDirect3D.h"
#include "Versions\IDirect3D2.h"
#include "Versions\IDirect3D3.h"
#include "Versions\IDirect3D7.h"
#include "Versions\IDirect3DDevice.h"
#include "Versions\IDirect3DDevice2.h"
#include "Versions\IDirect3DDevice3.h"
#include "Versions\IDirect3DDevice7.h"
#include "Versions\IDirect3DMaterial.h"
#include "Versions\IDirect3DMaterial2.h"
#include "Versions\IDirect3DMaterial3.h"
#include "Versions\IDirect3DTexture.h"
#include "Versions\IDirect3DTexture2.h"
#include "Versions\IDirect3DVertexBuffer.h"
#include "Versions\IDirect3DVertexBuffer7.h"
#include "Versions\IDirect3DViewport.h"
#include "Versions\IDirect3DViewport2.h"
#include "Versions\IDirect3DViewport3.h"
// DirectDraw Version Wrappers
#include "Versions\IDirectDraw.h"
#include "Versions\IDirectDraw2.h"
#include "Versions\IDirectDraw3.h"
#include "Versions\IDirectDraw4.h"
#include "Versions\IDirectDraw7.h"
#include "Versions\IDirectDrawSurface.h"
#include "Versions\IDirectDrawSurface2.h"
#include "Versions\IDirectDrawSurface3.h"
#include "Versions\IDirectDrawSurface4.h"
#include "Versions\IDirectDrawSurface7.h"
// Direct3D Helpers
#include "IDirect3DTypes.h"
#include "IDirect3DEnumCallback.h"
// DirectDraw Helpers
#include "IDirectDrawEnumCallback.h"
#include "IDirectDrawTypes.h"
// Direct3D Interfaces
#include "IDirect3DX.h"
#include "IDirect3DDeviceX.h"
#include "IDirect3DExecuteBuffer.h"
#include "IDirect3DLight.h"
#include "IDirect3DMaterialX.h"
#include "IDirect3DTextureX.h"
#include "IDirect3DVertexBufferX.h"
#include "IDirect3DViewportX.h"
// DirectDraw Interfaces
#include "IDirectDrawX.h"
#include "IDirectDrawClipper.h"
#include "IDirectDrawColorControl.h"
#include "IDirectDrawFactory.h"
#include "IDirectDrawGammaControl.h"
#include "IDirectDrawPalette.h"
#include "IDirectDrawSurfaceX.h"
