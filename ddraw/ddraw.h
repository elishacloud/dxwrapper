#pragma once

#define INITGUID

#include <ddraw.h>
#include <ddrawex.h>
#include <d3d.h>
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
class m_IDirect3DMaterial;
class m_IDirect3DMaterial2;
class m_IDirect3DMaterial3;
class m_IDirect3DMaterialX;
class m_IDirect3DTexture;
class m_IDirect3DTexture2;
class m_IDirect3DTextureX;
class m_IDirect3DVertexBuffer;
class m_IDirect3DVertexBuffer7;
class m_IDirect3DViewport;
class m_IDirect3DViewport2;
class m_IDirect3DViewport3;
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

#include "AddressLookupTable.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

typedef void(WINAPI *AcquireDDThreadLockProc)();
typedef HRESULT(WINAPI *D3DParseUnknownCommandProc)(LPVOID lpCmd, LPVOID *lpRetCmd);
typedef HRESULT(WINAPI *DirectDrawCreateProc)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
typedef HRESULT(WINAPI *DirectDrawCreateClipperProc)(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, LPUNKNOWN pUnkOuter);
typedef HRESULT(WINAPI *DDrawEnumerateAProc)(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
typedef HRESULT(WINAPI *DDrawEnumerateExAProc)(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef HRESULT(WINAPI *DDrawEnumerateExWProc)(LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags);
typedef HRESULT(WINAPI *DDrawEnumerateWProc)(LPDDENUMCALLBACKW lpCallback, LPVOID lpContext);
typedef HRESULT(WINAPI *DDrawCreateExProc)(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID riid, IUnknown FAR *pUnkOuter);
typedef HRESULT(WINAPI *DllCanUnloadNowProc)();
typedef HRESULT(WINAPI *DllGetClassObjectProc)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
typedef HRESULT(WINAPI *GetSurfaceFromDCProc)(HDC hdc, LPDIRECTDRAWSURFACE7 *lpDDS);
typedef void(WINAPI *ReleaseDDThreadLockProc)();
typedef HRESULT(WINAPI *SetAppCompatDataProc)(DWORD, DWORD);

REFIID ConvertREFIID(REFIID riid);
HRESULT ProxyQueryInterface(LPVOID ProxyInterface, REFIID CalledID, LPVOID * ppvObj, REFIID CallerID, LPVOID WrapperInterface);
void genericQueryInterface(REFIID riid, LPVOID * ppvObj);
extern AddressLookupTableDdraw<void> ProxyAddressLookupTable;

// Direct3D
#include "IDirect3D.h"
#include "IDirect3D2.h"
#include "IDirect3D3.h"
#include "IDirect3D7.h"
#include "IDirect3DX.h"
#include "IDirect3DDevice.h"
#include "IDirect3DDevice2.h"
#include "IDirect3DDevice3.h"
#include "IDirect3DDevice7.h"
#include "IDirect3DEnumCallback.h"
#include "IDirect3DExecuteBuffer.h"
#include "IDirect3DLight.h"
#include "IDirect3DMaterial.h"
#include "IDirect3DMaterial2.h"
#include "IDirect3DMaterial3.h"
#include "IDirect3DMaterialX.h"
#include "IDirect3DTexture.h"
#include "IDirect3DTexture2.h"
#include "IDirect3DTextureX.h"
#include "IDirect3DTypes.h"
#include "IDirect3DVertexBuffer.h"
#include "IDirect3DVertexBuffer7.h"
#include "IDirect3DViewport.h"
#include "IDirect3DViewport2.h"
#include "IDirect3DViewport3.h"
// DirectDraw
#include "IDirectDraw.h"
#include "IDirectDraw2.h"
#include "IDirectDraw3.h"
#include "IDirectDraw4.h"
#include "IDirectDraw7.h"
#include "IDirectDrawX.h"
#include "IDirectDrawClipper.h"
#include "IDirectDrawColorControl.h"
#include "IDirectDrawEnumCallback.h"
#include "IDirectDrawFactory.h"
#include "IDirectDrawGammaControl.h"
#include "IDirectDrawPalette.h"
#include "IDirectDrawSurface.h"
#include "IDirectDrawSurface2.h"
#include "IDirectDrawSurface3.h"
#include "IDirectDrawSurface4.h"
#include "IDirectDrawSurface7.h"
#include "IDirectDrawSurfaceX.h"
#include "IDirectDrawTypes.h"
