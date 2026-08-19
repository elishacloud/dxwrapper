/**
* Copyright (C) 2026 Elisha Riedlinger
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

#include "ddraw.h"

using namespace DdrawWrapper;

// ******************************
// IUnknown functions
// ******************************

HRESULT m_IDirect3DVertexBufferX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ppvObj)
	{
		return E_POINTER;
	}
	*ppvObj = nullptr;

	if (riid == IID_GetRealInterface)
	{
		*ppvObj = ProxyInterface;
		return D3D_OK;
	}
	if (riid == IID_GetInterfaceX)
	{
		*ppvObj = this;
		return D3D_OK;
	}

	DWORD DxVersion = (Config.Dd7to9 && CheckWrapperType(riid)) ? GetGUIDVersion(riid) : DirectXVersion;

	if (riid == GetWrapperType(DxVersion) || riid == IID_IUnknown)
	{
		*ppvObj = GetWrapperInterfaceX(DxVersion);

		AddRef(DxVersion);

		return D3D_OK;
	}

	if (GetWrapperType(DirectXVersion) == IID_IUnknown)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: DirectXVersion is unsupported version: " << DirectXVersion);
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, GetWrapperType(DirectXVersion));
}

ULONG m_IDirect3DVertexBufferX::AddRef(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	if (Config.Dd7to9)
	{
		switch (DirectXVersion)
		{
		case 1:
			return _InterlockedIncrement(&RefCount1);
		case 7:
			return _InterlockedIncrement(&RefCount7);
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			return 0;
		}
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DVertexBufferX::Release(DWORD DirectXVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") v" << DirectXVersion;

	if (Config.Dd7to9)
	{
		ULONG ref;

		switch (DirectXVersion)
		{
		case 1:
			ref = InterlockedDecrementIfPositive(&RefCount1);
			break;
		case 7:
			ref = InterlockedDecrementIfPositive(&RefCount7);
			break;
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
			ref = 0;
		}

		if (AtomicRead(RefCount1) + AtomicRead(RefCount7) == 0)
		{
			delete this;
		}

		return ref;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

// ******************************
// IDirect3DVertexBuffer v1 functions
// ******************************

HRESULT m_IDirect3DVertexBufferX::Lock(DWORD dwFlags, LPVOID* lplpData, LPDWORD lpdwSize)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lplpData)
		{
			return DDERR_INVALIDPARAMS;
		}

		*lplpData = nullptr;

		if (lpdwSize)
		{
			*lpdwSize = 0;
		}

		if (LastLock.IsLocked)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: locking vertex buffer when buffer is already locked!");
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, true)))
		{
			return DDERR_GENERIC;
		}

		// Non-implemented dwFlags:
		// DDLOCK_WAIT and DDLOCK_WRITEONLY can be ignored safely

		// If using write only and discard than discard is ignored
		dwFlags = (dwFlags & DDLOCK_WRITEONLY) || (d3d9VBDesc.Usage & D3DUSAGE_WRITEONLY) ? (dwFlags & ~DDLOCK_DISCARDCONTENTS) : dwFlags;

		// If using read only and not write only
		dwFlags = (dwFlags & DDLOCK_READONLY) && !(dwFlags & DDLOCK_WRITEONLY) ? dwFlags : (dwFlags & ~(DDLOCK_READONLY | DDLOCK_WRITEONLY));

		DWORD Flags =
			((dwFlags & (DDLOCK_READONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_NOSYSLOCK)) |
				(IsVBEmulated || (Config.DdrawVertexLockDiscard && !(dwFlags & DDLOCK_READONLY)) ? D3DLOCK_DISCARD : NULL) |
				(Config.DdrawNoDrawBufferSysLock ? D3DLOCK_NOSYSLOCK : NULL)) &
			~(d3d9VBDesc.Pool == D3DPOOL_MANAGED ? DDLOCK_DISCARDCONTENTS : 0);

		// Handle emulated readonly
		if (IsVBEmulated && (Flags & D3DLOCK_READONLY))
		{
			LastLock.IsLocked = true;
			LastLock.Addr = nullptr;
			LastLock.Flags = Flags;

			*lplpData = VertexData.data();

			if (lpdwSize)
			{
				*lpdwSize = VB.Size;
			}

			return D3D_OK;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		// Lock vertex
		void* pData = nullptr;
		HRESULT hr = d3d9VertexBuffer->Lock(0, 0, &pData, Flags);
		if (FAILED(hr) && (Flags & D3DLOCK_NOSYSLOCK))
		{
			hr = d3d9VertexBuffer->Lock(0, 0, &pData, Flags & ~D3DLOCK_NOSYSLOCK);
		}

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock vertex buffer: " << (D3DERR)hr);
			return hr;
		}

		LastLock.IsLocked = true;
		LastLock.Addr = pData;
		LastLock.Flags = Flags;

		// Handle emulated vertex
		if (IsVBEmulated)
		{
			*lplpData = VertexData.data();

			if (lpdwSize)
			{
				*lpdwSize = VB.Size;
			}

			//if (dwFlags & DDLOCK_DISCARDCONTENTS)
			//{
			//	ZeroMemory(VertexData.data(), VB.Size);
			//}
		}
		else
		{
			*lplpData = pData;

			if (lpdwSize)
			{
				*lpdwSize = VB.Size;
			}
		}

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInUS(startTime);
#endif

		return D3D_OK;
	}

	return ProxyInterface->Lock(dwFlags, lplpData, lpdwSize);
}

HRESULT m_IDirect3DVertexBufferX::Unlock()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, false, true)))
		{
			return DDERR_GENERIC;
		}

		// Handle emulated readonly
		if (IsVBEmulated && (LastLock.Flags & D3DLOCK_READONLY))
		{
			LastLock.IsLocked = false;
			LastLock.Addr = nullptr;
			LastLock.Flags = 0;

			return D3D_OK;
		}

		// Handle emulated vertex
		if (IsVBEmulated && LastLock.Addr)
		{
			if (VB.Desc.dwFVF == D3DFVF_LVERTEX)
			{
				LOG_LIMIT(100, __FUNCTION__ << " Warning: converting vertex buffer, may cause slowdowns!");

				ConvertLVertex((DXLVERTEX9*)LastLock.Addr, (DXLVERTEX7*)VertexData.data(), VB.Desc.dwNumVertices);
			}
			else
			{
				DWORD stride = GetVertexStride(VB.Desc.dwFVF);
				memcpy(LastLock.Addr, VertexData.data(), VB.Desc.dwNumVertices * stride);

				if (Config.DdrawClampVertexZDepth && (VB.Desc.dwFVF & D3DFVF_XYZRHW))
				{
					ClampVertices(VertexData.data(), stride, VB.Desc.dwNumVertices);
				}
			}
		}

		HRESULT hr = d3d9VertexBuffer->Unlock();

		if (FAILED(hr))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to unlock vertex buffer: " << (D3DERR)hr);
			return hr;
		}
		
		LastLock.IsLocked = false;
		LastLock.Addr = nullptr;
		LastLock.Flags = 0;

		return D3D_OK;
	}

	return ProxyInterface->Unlock();
}

HRESULT m_IDirect3DVertexBufferX::ProcessVertices(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPDIRECT3DVERTEXBUFFER7 lpSrcBuffer, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check if there are no vertices to process
		if (dwCount == 0)
		{
			return D3D_OK;
		}

		// Always include the D3DVOP_TRANSFORM flag in the dwVertexOp parameter. If you do not, the method fails, returning DDERR_INVALIDPARAMS.
		if (!lpSrcBuffer || !lpD3DDevice || !(dwVertexOp & D3DVOP_TRANSFORM))
		{
			return DDERR_INVALIDPARAMS;
		}

		// Docs state that D3DVOP_CLIP cannot be used with a vertex buffer created with the D3DVBCAPS_DONOTCLIP flag
		if ((dwVertexOp & D3DVOP_CLIP) && !(VB.Desc.dwCaps & D3DVBCAPS_DONOTCLIP))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: D3DVOP_CLIP specified when vertex buffer was created with D3DVBCAPS_DONOTCLIP!");
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		// Get our wrapper device
		m_IDirect3DDeviceX* pDirect3DDeviceX = nullptr;
		if (FAILED(lpD3DDevice->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pDirect3DDeviceX)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3DDeviceX interface!");
			return DDERR_GENERIC;
		}

		m_IDirect3DVertexBufferX* pSrcVertexBufferX = nullptr;
		if (FAILED(lpSrcBuffer->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pSrcVertexBufferX)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get source vertex buffer wrapper!");
			return DDERR_GENERIC;
		}

		LPDIRECT3DVERTEXBUFFER9 d3d9SrcVertexBuffer = pSrcVertexBufferX->GetCurrentD9VertexBuffer();
		if (!d3d9SrcVertexBuffer)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get d3d9 source vertex buffer!");
			return DDERR_GENERIC;
		}

		// Check source vertex buffer for device interface
		if (FAILED(pSrcVertexBufferX->CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		// Get FVF
		const DWORD dwSrcVertexTypeDesc = pSrcVertexBufferX->VB.Desc.dwFVF;
		const DWORD dwDestVertexTypeDesc = VB.Desc.dwFVF;

		// Validate destination range
		const DWORD DestNumVertices = VB.Desc.dwNumVertices;
		if (dwDestIndex >= DestNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: destination vertex index is too large: " << DestNumVertices << " -> " << dwDestIndex);
			return D3DERR_INVALIDVERTEXTYPE;
		}
		dwCount = min(dwCount, DestNumVertices - dwDestIndex);

		// Check the dwSrcIndex and dwCount to make sure they won't cause an overload
		const DWORD SrcNumVertices = pSrcVertexBufferX->VB.Desc.dwNumVertices;
		if (dwSrcIndex > SrcNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: source vertex index is too large: " << SrcNumVertices << " -> " << dwSrcIndex);
			return DDERR_INVALIDPARAMS;
		}
		dwCount = min(dwCount, SrcNumVertices - dwSrcIndex);

		const BOOL doLighting = (dwVertexOp & D3DVOP_LIGHT) && (dwSrcVertexTypeDesc & D3DFVF_NORMAL) && pDirect3DDeviceX->IsMaterialSet() ? TRUE : FALSE;
		if (!doLighting)
		{
			dwVertexOp &= ~D3DVOP_LIGHT;
		}

		const BOOL doClipping = (dwVertexOp & D3DVOP_CLIP) ? TRUE : FALSE;

		IDirect3DVertexBuffer9* pSrcBuffer = pSrcVertexBufferX->GetCurrentD9VertexBuffer();
		IDirect3DVertexBuffer9* pDestBuffer = GetCurrentD9VertexBuffer();

		// Try using d3d9 device for vertex processing
		HRESULT hr = pDirect3DDeviceX->ProcessVertices(dwSrcIndex, dwDestIndex, dwCount, pSrcBuffer, pDestBuffer, dwSrcVertexTypeDesc, doLighting, doClipping, (dwFlags & D3DPV_DONOTCOPYDATA));

		if (SUCCEEDED(hr))
		{
			return D3D_OK;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		// Lock the source vertex buffer
		void* pSrcVertices = nullptr;
		if (FAILED(pSrcVertexBufferX->Lock(D3DLOCK_READONLY, &pSrcVertices, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock source vertex");
			return DDERR_GENERIC;
		}

		// Lock destination buffer
		void* pDestVertices = nullptr;
		if (FAILED(Lock(0, &pDestVertices, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock destination vertex");
			pSrcVertexBufferX->Unlock();
			return DDERR_GENERIC;
		}

		hr = ProcessVerticesUP(dwVertexOp, pDestVertices, dwDestVertexTypeDesc, dwDestIndex, dwCount, pSrcVertices, dwSrcVertexTypeDesc, dwSrcIndex, pDirect3DDeviceX, dwFlags);

		// Unlock destination vertex buffer
		Unlock();

		// Unlock the source vertex buffer
		pSrcVertexBufferX->Unlock();

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInUS(startTime);
#endif

		return hr;
	}

	if (lpSrcBuffer)
	{
		lpSrcBuffer->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpSrcBuffer);
	}
	if (lpD3DDevice)
	{
		lpD3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DDevice);
	}

	return ProxyInterface->ProcessVertices(dwVertexOp, dwDestIndex, dwCount, lpSrcBuffer, dwSrcIndex, lpD3DDevice, dwFlags);
}

HRESULT m_IDirect3DVertexBufferX::GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC lpVBDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		if (!lpVBDesc)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (VB.Desc.dwSize != sizeof(D3DVERTEXBUFFERDESC))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid vertex buffer desc: " << VB.Desc);
			return DDERR_GENERIC;
		}

		lpVBDesc->dwSize = sizeof(D3DVERTEXBUFFERDESC);
		lpVBDesc->dwCaps = VB.Desc.dwCaps;
		lpVBDesc->dwFVF = VB.Desc.dwFVF;
		lpVBDesc->dwNumVertices = VB.Desc.dwNumVertices;

		return D3D_OK;
	}

	return ProxyInterface->GetVertexBufferDesc(lpVBDesc);
}

HRESULT m_IDirect3DVertexBufferX::Optimize(LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// The Optimize function doesn't exist in Direct3D9 because it manages vertex buffer optimizations internally

		if (!lpD3DDevice)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (LastLock.IsLocked)
		{
			return D3DERR_VERTEXBUFFERLOCKED;
		}

		if (VB.Desc.dwCaps & D3DVBCAPS_OPTIMIZED)
		{
			return D3DERR_VERTEXBUFFEROPTIMIZED;
		}

		VB.Desc.dwCaps |= D3DVBCAPS_OPTIMIZED;

		return D3D_OK;
	}

	if (lpD3DDevice)
	{
		lpD3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DDevice);
	}

	return ProxyInterface->Optimize(lpD3DDevice, dwFlags);
}

// ******************************
// IDirect3DVertexBuffer v7 functions
// ******************************

HRESULT m_IDirect3DVertexBufferX::ProcessVerticesStrided(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (Config.Dd7to9)
	{
		// Check if there are no vertices to process
		if (dwCount == 0)
		{
			return D3D_OK;
		}

		// Always include the D3DVOP_TRANSFORM flag in the dwVertexOp parameter. If you do not, the method fails, returning DDERR_INVALIDPARAMS.
		if (!lpVertexArray || !lpD3DDevice || !(dwVertexOp & D3DVOP_TRANSFORM))
		{
			return DDERR_INVALIDPARAMS;
		}

		// Docs state that D3DVOP_CLIP cannot be used with a vertex buffer created with the D3DVBCAPS_DONOTCLIP flag
		if ((dwVertexOp & D3DVOP_CLIP) && !(VB.Desc.dwCaps & D3DVBCAPS_DONOTCLIP))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: D3DVOP_CLIP specified when vertex buffer was created with D3DVBCAPS_DONOTCLIP!");
			return DDERR_INVALIDPARAMS;
		}

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		// Get our wrapper device
		m_IDirect3DDeviceX* pDirect3DDeviceX = nullptr;
		if (FAILED(lpD3DDevice->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pDirect3DDeviceX)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3DDeviceX interface!");
			return DDERR_GENERIC;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		// Get FVF
		const DWORD dwDestVertexTypeDesc = VB.Desc.dwFVF;

		// Validate destination range
		const DWORD DestNumVertices = VB.Desc.dwNumVertices;
		if (dwDestIndex >= DestNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: destination vertex index is too large: " << DestNumVertices << " -> " << dwDestIndex);
			return D3DERR_INVALIDVERTEXTYPE;
		}
		dwCount = min(dwCount, DestNumVertices - dwDestIndex);

		// Get source vars
		const DWORD dwSrcVertexTypeDesc = GetStridedVertexTypeDesc(lpVertexArray);
		std::vector<BYTE, aligned_allocator<BYTE, 4>> SrcVertexCache;

		if (D3DFVF_TEXCOUNT(dwSrcVertexTypeDesc) != D3DFVF_TEXCOUNT(dwDestVertexTypeDesc))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: source and destination FVF texture counts don't match: " << D3DFVF_TEXCOUNT(dwSrcVertexTypeDesc) << " -> " << D3DFVF_TEXCOUNT(dwDestVertexTypeDesc));
		}

		// Process strided data
		if (FAILED(InterleaveStridedVertexData(SrcVertexCache, lpVertexArray, dwSrcIndex, dwCount, dwSrcVertexTypeDesc)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid StridedVertexData!");
			return DDERR_INVALIDPARAMS;
		}

		// Lock destination buffer
		void* pDestVertices = nullptr;
		if (FAILED(Lock(0, &pDestVertices, 0)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to lock destination vertex");
			return DDERR_GENERIC;
		}

		HRESULT hr = ProcessVerticesUP(dwVertexOp, pDestVertices, dwDestVertexTypeDesc, dwDestIndex, dwCount, SrcVertexCache.data(), dwSrcVertexTypeDesc, dwSrcIndex, pDirect3DDeviceX, dwFlags);

		// Unlock destination vertex buffer
		Unlock();

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInUS(startTime);
#endif

		return hr;
	}

	if (lpD3DDevice)
	{
		lpD3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpD3DDevice);
	}

	switch (ProxyDirectXVersion)
	{
	case 1:
		return DDERR_GENERIC;
	case 7:
		return ProxyInterface->ProcessVerticesStrided(dwVertexOp, dwDestIndex, dwCount, lpVertexArray, dwSrcIndex, lpD3DDevice, dwFlags);
	default:
		return DDERR_GENERIC;
	}
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirect3DVertexBufferX::InitInterface(DWORD DirectXVersion)
{
	if (ddrawParent)
	{
		ddrawParent->AddVertexBuffer(this);
	}

	if (D3DInterface)
	{
		D3DInterface->AddVertexBuffer(this);
	}

	if (Config.Dd7to9)
	{
		if (ddrawParent)
		{
			d3d9Device = ddrawParent->GetDirectD9Device();
		}

		AddRef(DirectXVersion);
	}
}

void m_IDirect3DVertexBufferX::ReleaseInterface()
{
	if (Config.Exiting)
	{
		return;
	}

	// Don't delete wrapper interface
	SaveInterfaceAddress(WrapperInterface);
	SaveInterfaceAddress(WrapperInterface7);

	ReleaseD9Buffer(false, false);

	if (ddrawParent)
	{
		ddrawParent->ClearVertexBuffer(this);
	}

	if (D3DInterface)
	{
		D3DInterface->ClearVertexBuffer(this);
	}
}

HRESULT m_IDirect3DVertexBufferX::CheckInterface(char* FunctionName, bool CheckD3DDevice, bool CheckD3DVertexBuffer)
{
	// Check for ddraw parent
	if (!ddrawParent)
	{
		LOG_LIMIT(100, FunctionName << " Error: no ddraw parent!");
		return DDERR_INVALIDOBJECT;
	}

	// Check d3d9 device
	if (CheckD3DDevice)
	{
		if (!ddrawParent->CheckD9Device(FunctionName) || !d3d9Device || !*d3d9Device)
		{
			LOG_LIMIT(100, FunctionName << " Error: d3d9 device not setup!");
			return DDERR_INVALIDOBJECT;
		}
	}

	// Check for vertex buffer, if not then create it
	if (CheckD3DVertexBuffer && !d3d9VertexBuffer)
	{
		// Create d3d9 vertex buffer
		if (FAILED(CreateD3D9VertexBuffer()))
		{
			LOG_LIMIT(100, FunctionName << " Error: could not create d3d9 vertex buffer!");
			return DDERR_GENERIC;
		}
	}

	return D3D_OK;
}

void* m_IDirect3DVertexBufferX::GetWrapperInterfaceX(DWORD DirectXVersion)
{
	switch (DirectXVersion)
	{
	case 0:
		if (WrapperInterface7) return WrapperInterface7;
		if (WrapperInterface) return WrapperInterface;
		break;
	case 1:
		return GetInterfaceAddress(WrapperInterface, (LPDIRECT3DVERTEXBUFFER)ProxyInterface, this);
	case 7:
		return GetInterfaceAddress(WrapperInterface7, (LPDIRECT3DVERTEXBUFFER7)ProxyInterface, this);
	}
	LOG_LIMIT(100, __FUNCTION__ << " Error: wrapper interface version not found: " << DirectXVersion);
	return nullptr;
}

HRESULT m_IDirect3DVertexBufferX::CreateD3D9VertexBuffer()
{
	// Release existing buffer
	ReleaseD3D9VertexBuffer();

	// Check for device interface
	if (FAILED(CheckInterface(__FUNCTION__, true, false)))
	{
		return DDERR_GENERIC;
	}

	IsVBEmulated = (VB.Desc.dwFVF == D3DFVF_LVERTEX) || (Config.DdrawClampVertexZDepth && (VB.Desc.dwFVF && D3DFVF_XYZRHW));

	d3d9VBDesc.FVF = (VB.Desc.dwFVF == D3DFVF_LVERTEX) ? D3DFVF_LVERTEX9 : VB.Desc.dwFVF;
	d3d9VBDesc.Size = GetVertexStride(d3d9VBDesc.FVF) * VB.Desc.dwNumVertices;
	d3d9VBDesc.Pool = (VB.Desc.dwCaps & D3DVBCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_MANAGED;
	d3d9VBDesc.Usage =
		(d3d9VBDesc.Pool != D3DPOOL_MANAGED ? D3DUSAGE_DYNAMIC : 0) |
		((VB.Desc.dwCaps & D3DVBCAPS_WRITEONLY) || IsVBEmulated ? D3DUSAGE_WRITEONLY : 0) |
		((VB.Desc.dwCaps & D3DVBCAPS_DONOTCLIP) ? D3DUSAGE_DONOTCLIP : 0);

	HRESULT hr = (*d3d9Device)->CreateVertexBuffer(d3d9VBDesc.Size, d3d9VBDesc.Usage, d3d9VBDesc.FVF, d3d9VBDesc.Pool, &d3d9VertexBuffer, nullptr);
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create vertex buffer: " << (D3DERR)hr <<
			" Size: " << d3d9VBDesc.Size << " Usage: " << Logging::hex(d3d9VBDesc.Usage) << " FVF: " << Logging::hex(d3d9VBDesc.FVF) <<
			" Pool: " << Logging::hex(d3d9VBDesc.Pool) << VB.Desc);
		return DDERR_GENERIC;
	}

	VB.Size = GetVertexStride(VB.Desc.dwFVF) * VB.Desc.dwNumVertices;

	if (IsVBEmulated)
	{
		// ToDo: restore vertex buffer data
		if (VertexData.size() < VB.Size)
		{
			VertexData.resize(VB.Size);
		}
	}

	LastLock.IsLocked = false;
	LastLock.Addr = nullptr;
	LastLock.Flags = 0;

	return D3D_OK;
}

void m_IDirect3DVertexBufferX::ReleaseD3D9VertexBuffer()
{
	// Release vertex buffer
	if (d3d9VertexBuffer)
	{
		ULONG ref = d3d9VertexBuffer->Release();
		if (ref)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ")" << " Error: there is still a reference to 'd3d9VertexBuffer' " << ref;
		}
		d3d9VertexBuffer = nullptr;
	}
}

void m_IDirect3DVertexBufferX::ReleaseD9Buffer(bool BackupData, bool ResetBuffer)
{
	if (BackupData && VB.Desc.dwFVF != D3DFVF_LVERTEX)
	{
		// ToDo: backup vertex buffer data
	}

	if (!ResetBuffer || d3d9VBDesc.Pool == D3DPOOL_DEFAULT)
	{
		ReleaseD3D9VertexBuffer();
	}
}

DWORD m_IDirect3DVertexBufferX::GetStridedVertexTypeDesc(const D3DDRAWPRIMITIVESTRIDEDDATA* sd)
{
	DWORD TypeData = 0;

	// Position
	if (sd->position.lpvData)
	{
		TypeData |= D3DFVF_XYZ;
	}

	// Normal
	if (sd->normal.lpvData)
	{
		TypeData |= D3DFVF_NORMAL;
	}

	// Diffuse
	if (sd->diffuse.lpvData)
	{
		TypeData |= D3DFVF_DIFFUSE;
	}

	// Specular
	if (sd->specular.lpvData)
	{
		TypeData |= D3DFVF_SPECULAR;
	}

	// Texture coordinate size
	DWORD texCount = 0;
	for (DWORD t = 0; t < D3DDP_MAXTEXCOORD; ++t)
	{
		if (sd->textureCoords[t].lpvData)
		{
			texCount++;
			switch (sd->textureCoords[t].dwStride)
			{
				case sizeof(float) :
					TypeData |= D3DFVF_TEXCOORDSIZE1(t);
					break;

				default:
				case sizeof(float) * 2:
					TypeData |= D3DFVF_TEXCOORDSIZE2(t);
					break;

				case sizeof(float) * 3:
					TypeData |= D3DFVF_TEXCOORDSIZE3(t);
					break;

				case sizeof(float) * 4:
					TypeData |= D3DFVF_TEXCOORDSIZE4(t);
					break;
			}
		}
		else
		{
			break;
		}
	}

	// Texture count
	TypeData |= texCount << D3DFVF_TEXCOUNT_SHIFT;

	return TypeData;
}

HRESULT m_IDirect3DVertexBufferX::InterleaveStridedVertexData(std::vector<BYTE, aligned_allocator<BYTE, 4>>& outputBuffer, const D3DDRAWPRIMITIVESTRIDEDDATA* sd, const DWORD dwVertexStart, const DWORD dwNumVertices, const DWORD dwVertexTypeDesc)
{
	if (!sd)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: missing D3DDRAWPRIMITIVESTRIDEDDATA!");
		return DDERR_INVALIDPARAMS;
	}

	const DWORD Stride = GetVertexStride(dwVertexTypeDesc);

	const bool hasPosition = (dwVertexTypeDesc & D3DFVF_POSITION_MASK);
	const bool hasReserved = (dwVertexTypeDesc & D3DFVF_RESERVED1);
	const bool hasNormal = (dwVertexTypeDesc & D3DFVF_NORMAL);
	const bool hasDiffuse = (dwVertexTypeDesc & D3DFVF_DIFFUSE);
	const bool hasSpecular = (dwVertexTypeDesc & D3DFVF_SPECULAR);
	const DWORD texCount = min(D3DFVF_TEXCOUNT(dwVertexTypeDesc), D3DDP_MAXTEXCOORD);

	if (texCount > D3DDP_MAXTEXCOORD)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: texCount " << texCount << " exceeds D3DDP_MAXTEXCOORD!");
		return DDERR_INVALIDPARAMS;
	}

	UINT posStride = GetVertexPositionStride(dwVertexTypeDesc);
	UINT texStride[D3DDP_MAXTEXCOORD] = {};

	// Check data and compute stride
	if (hasPosition)
	{
		if (!sd->position.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: position data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}
	}
	if (hasNormal)
	{
		if (!sd->normal.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: normal data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}
	}
	if (hasDiffuse)
	{
		if (!sd->diffuse.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: diffuse data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}
	}
	if (hasSpecular)
	{
		if (!sd->specular.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: specular data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}
	}
	for (DWORD t = 0; t < texCount; ++t)
	{
		if (!sd->textureCoords[t].lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: textureCoords " << t << " data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}
		texStride[t] = GetTexStride(dwVertexTypeDesc, t);
	}

	outputBuffer.resize((dwVertexStart + dwNumVertices) * Stride);

	BYTE* cursor = outputBuffer.data() + dwVertexStart * Stride;
	BYTE* posCursor = reinterpret_cast<BYTE*>(sd->position.lpvData) + dwVertexStart * sd->position.dwStride;
	BYTE* normalCursor = reinterpret_cast<BYTE*>(sd->normal.lpvData) + dwVertexStart * sd->normal.dwStride;
	BYTE* diffCursor = reinterpret_cast<BYTE*>(sd->diffuse.lpvData) + dwVertexStart * sd->diffuse.dwStride;
	BYTE* specCursor = reinterpret_cast<BYTE*>(sd->specular.lpvData) + dwVertexStart * sd->specular.dwStride;
	BYTE* texCursor[D3DDP_MAXTEXCOORD] = {};
	for (DWORD t = 0; t < texCount; ++t)
	{
		texCursor[t] = reinterpret_cast<BYTE*>(sd->textureCoords[t].lpvData) + dwVertexStart * sd->textureCoords[t].dwStride;
	}

	for (DWORD i = 0; i < dwNumVertices; ++i)
	{
		if (hasPosition)
		{
			memcpy(cursor, posCursor, posStride);
			cursor += posStride;
			posCursor += sd->position.dwStride;
		}

		if (hasReserved)
		{
			*(DWORD*)cursor = 0;
			cursor += sizeof(DWORD);
		}

		if (hasNormal)
		{
			memcpy(cursor, normalCursor, sizeof(D3DXVECTOR3));
			cursor += sizeof(D3DXVECTOR3);
			normalCursor += sd->normal.dwStride;
		}

		if (hasDiffuse)
		{
			memcpy(cursor, diffCursor, sizeof(D3DCOLOR));
			cursor += sizeof(D3DCOLOR);
			diffCursor += sd->diffuse.dwStride;
		}

		if (hasSpecular)
		{
			memcpy(cursor, specCursor, sizeof(D3DCOLOR));
			cursor += sizeof(D3DCOLOR);
			specCursor += sd->specular.dwStride;
		}

		for (DWORD t = 0; t < texCount; ++t)
		{
			memcpy(cursor, texCursor[t], texStride[t]);
			cursor += texStride[t];
			texCursor[t] += sd->textureCoords[t].dwStride;
		}
	}

	return D3D_OK;
}

template HRESULT m_IDirect3DVertexBufferX::TransformVertexUP<XYZ>(m_IDirect3DDeviceX*, const DWORD, LPD3DTRANSFORMDATA, DWORD, const VIEWPORTINFO&, LPDWORD);
template HRESULT m_IDirect3DVertexBufferX::TransformVertexUP<D3DLVERTEX>(m_IDirect3DDeviceX*, const DWORD, LPD3DTRANSFORMDATA, DWORD, const VIEWPORTINFO&, LPDWORD);
template <typename T>
HRESULT m_IDirect3DVertexBufferX::TransformVertexUP(m_IDirect3DDeviceX* pDirect3DDeviceX, const DWORD dwCount, LPD3DTRANSFORMDATA lpData, DWORD dwFlags, const VIEWPORTINFO& Viewport, LPDWORD lpOffscreen)
{
	if (!lpData || !pDirect3DDeviceX)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Check dwSize parameters
	if (lpData->dwSize != sizeof(D3DTRANSFORMDATA))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: dwSize doesn't match: " << sizeof(D3DTRANSFORMDATA) << " -> " << lpData->dwSize);
		return DDERR_INVALIDPARAMS;
	}

	if (!lpData->lpIn || !lpData->lpOut)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (lpData->dwInSize < sizeof(T))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: dwInSize is too small: " << sizeof(T) << " -> " << lpData->dwInSize);
		return DDERR_INVALIDPARAMS;
	}

	if (lpData->dwOutSize < sizeof(D3DTLVERTEX))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: dwOutSize is too small: " << sizeof(D3DTLVERTEX) << " -> " << lpData->dwOutSize);
		return DDERR_INVALIDPARAMS;
	}

	if ((dwFlags & (D3DTRANSFORM_CLIPPED | D3DTRANSFORM_UNCLIPPED)) == 0)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: invalid dwFlags: " << Logging::hex(dwFlags));
		return DDERR_INVALIDPARAMS;
	}

	const bool IsClipped = (dwFlags & D3DTRANSFORM_CLIPPED) && !(dwFlags & D3DTRANSFORM_UNCLIPPED);

	if (IsClipped && !lpData->lpHOut)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: lpHOut is null when clipping!");
		return DDERR_INVALIDPARAMS;
	}

	D3DMATRIX matWorld, matView, matProj;
	if (FAILED(pDirect3DDeviceX->GetD9Transform(D3DTS_WORLD, &matWorld)) ||
		FAILED(pDirect3DDeviceX->GetD9Transform(D3DTS_VIEW, &matView)) ||
		FAILED(pDirect3DDeviceX->GetD9Transform(D3DTS_PROJECTION, &matProj)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get transform matrices");
		return DDERR_GENERIC;
	}

	matProj = UpdateProjectionMatrix(matProj, Viewport.Scale, Viewport.Clip, IsClipped);

	D3DMATRIX matWorldView = {}, matWorldViewProj = {};
	D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
	D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &matProj);

	// Get viewport
	const D3DVIEWPORT9& vp = Viewport.Data9;
	const D3DVECTOR& legacyClip = Viewport.Clip;
	const D3DVECTOR& legacyScale = Viewport.Scale;

	// Precalculate a few static viewport factors, to save on per-vertex cycles
	const float viewportHalfWidth = static_cast<float>(vp.Width) * 0.5f;
	const float viewportHalfHeight = static_cast<float>(vp.Height) * 0.5f;
	const float viewportZDelta = vp.MaxZ - vp.MinZ;

	bool allOffscreen = true;
	DWORD clipIntersection = UINT_MAX;
	DWORD clipUnion = 0;

	D3DHVERTEX* pHOut = reinterpret_cast<D3DHVERTEX*>(lpData->lpHOut);

	for (DWORD i = 0; i < dwCount; ++i)
	{
		// Source position (can have arbitrary stride set by application and defined via dwInSize)
		T& src = *(reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(lpData->lpIn) + lpData->dwInSize * i));

		// Projection-space position
		D3DXVECTOR4 h = TransformVector4(src.x, src.y, src.z, 1.0f, matWorldViewProj);

		// Output vertex (can have arbitrary stride set by application and defined via dwOutSize)
		D3DTLVERTEX& dst = *(reinterpret_cast<D3DTLVERTEX*>(reinterpret_cast<uint8_t*>(lpData->lpOut) + lpData->dwOutSize * i));

		if (IsClipped)
		{
			DWORD clipFlags =
				(h.x > h.w) * D3DCLIP_RIGHT |
				(h.x < -h.w) * D3DCLIP_LEFT |
				(h.y > h.w) * D3DCLIP_TOP |
				(h.y < -h.w) * D3DCLIP_BOTTOM |
				(h.z < 0.0f) * D3DCLIP_FRONT |
				(h.z > h.w) * D3DCLIP_BACK;

			allOffscreen &= (clipFlags != 0);
			clipIntersection &= clipFlags;
			clipUnion |= clipFlags;

			if (clipFlags)
			{
				// Fill homogeneous out if the vertex is clipped
				D3DHVERTEX& hdst = pHOut[i];

				// Store pre-divide homogeneous coords (applying legacyClip/legacyScale here seems to be what native Windows does)
				hdst.hx = (h.x - legacyClip.x * h.w) / legacyScale.x;
				hdst.hy = (h.y - legacyClip.y * h.w) / legacyScale.y;
				hdst.hz = (h.z - legacyClip.z * h.w) / legacyScale.z;
				hdst.dwFlags = clipFlags;

				// Native windows seems to do this
				dst.sx = h.x;
				dst.sy = h.y;
				dst.sz = h.z;
				dst.rhw = h.w;
				continue;
			}
		}

		// Preserve INF/NAN behavior
		dst.rhw = 1.0f / h.w;

		// Convert to screen-space TL coords
		dst.sx = vp.X + (h.x * dst.rhw + 1.0f) * viewportHalfWidth;
		dst.sy = vp.Y + (1.0f - h.y * dst.rhw) * viewportHalfHeight;
		dst.sz = vp.MinZ + (h.z * dst.rhw) * viewportZDelta;

		// Set extent contains for visible vertices (disable this for now unless it is needed by some game)
		//lpData->drExtent.x1 = min(lpData->drExtent.x1, static_cast<LONG>(floorf(dst.sx)));
		//lpData->drExtent.y1 = min(lpData->drExtent.y1, static_cast<LONG>(floorf(dst.sy)));
		//lpData->drExtent.x2 = max(lpData->drExtent.x2, static_cast<LONG>(ceilf(dst.sx)));
		//lpData->drExtent.y2 = max(lpData->drExtent.y2, static_cast<LONG>(ceilf(dst.sy)));

		// Default values: set for XYZ or copy for detailed vertex
		if constexpr (std::is_same_v<T, XYZ>)
		{
			dst.color = 0xFFFFFFFF;	// Default color to white
			dst.specular = 0;
			dst.tu = 0.0f;
			dst.tv = 0.0f;
		}
		else if constexpr (std::is_same_v<T, D3DLVERTEX>)
		{
			dst.color = src.color;
			dst.specular = src.specular;
			dst.tu = src.tu;
			dst.tv = src.tv;
		}
		else
		{
			static_assert(false);
		}
	}

	// Address of a variable that is set to a nonzero value if the resulting vertices are all off-screen.
	if (lpOffscreen)
	{
		*lpOffscreen = IsClipped && allOffscreen ? clipIntersection | D3DSTATUS_ZNOTVISIBLE : FALSE;
	}
	lpData->dwClipIntersection = IsClipped ? clipIntersection << 12 : 0;
	lpData->dwClipUnion = IsClipped ? clipUnion : 0;

	return D3D_OK;
}

HRESULT m_IDirect3DVertexBufferX::ProcessVerticesUP(DWORD dwVertexOp, LPVOID lpDestBuffer, DWORD dwDestVertexTypeDesc, DWORD dwDestIndex, DWORD dwCount, LPVOID lpSrcBuffer, DWORD dwSrcVertexTypeDesc, DWORD dwSrcIndex, m_IDirect3DDeviceX* pDirect3DDeviceX, DWORD dwFlags)
{
	if (!lpDestBuffer || !lpSrcBuffer || !pDirect3DDeviceX)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Get and verify FVF
	const DWORD SrcFVF = dwSrcVertexTypeDesc;
	const DWORD DestFVF = dwDestVertexTypeDesc;
	const UINT SrcStride = GetVertexStride(SrcFVF);
	const UINT DestStride = GetVertexStride(DestFVF);
	const DWORD SrcPosFVF = SrcFVF & D3DFVF_POSITION_MASK;
	const DWORD DestPosFVF = DestFVF & D3DFVF_POSITION_MASK;

	// Cannot transform RHW vertices or convert to non-RHW TL format
	if ((SrcPosFVF & D3DFVF_XYZRHW) || !(DestPosFVF & D3DFVF_XYZRHW))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid FVF conversion: Cannot transform from D3DFVF_XYZRHW or to non-D3DFVF_XYZRHW format: " << Logging::hex(SrcFVF) << " -> " << Logging::hex(DestFVF));
		return D3DERR_INVALIDVERTEXTYPE;
	}

	// Just ignore D3DVOP_CLIP ans D3DVOP_EXTENTS in dwVertexOp

	// D3DVOP_TRANSFORM is inherently handled by ProcessVertices() as it performs vertex transformations based on the current world, view, and projection matrices.
	if (!(dwVertexOp & D3DVOP_TRANSFORM))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: D3DVOP_TRANSFORM not set, forcing transform");
		dwVertexOp |= D3DVOP_TRANSFORM;
	}

	// If the rendering device does not have a material assigned to it, the Direct3D lighting engine is disabled.
	bool bLighting = (dwVertexOp & D3DVOP_LIGHT) && (SrcFVF & D3DFVF_NORMAL) && pDirect3DDeviceX->IsMaterialSet();
	bool DoNotCopyData = (dwFlags & D3DPV_DONOTCOPYDATA) != 0;

	// Check lighting state
	if (bLighting)
	{
		DWORD rsLighting = FALSE;
		if (FAILED(pDirect3DDeviceX->GetD9RenderState(D3DRS_LIGHTING, &rsLighting)) || rsLighting == FALSE)
		{
			bLighting = false;
		}
	}

	// Get lights
	std::vector<DXLIGHT7> cachedLights;
	if (bLighting)
	{
		pDirect3DDeviceX->GetEnabledLightList(cachedLights);
		if (cachedLights.empty())
		{
			bLighting = false;
		}
	}

	DWORD PosSizeSrc = GetVertexPositionStride(SrcFVF);
	DWORD PosSizeDest = GetVertexPositionStride(DestFVF);

	DWORD NormalSrcOffset = 0;
	DWORD DiffuseSrcOffset = 0;
	DWORD SpecularSrcOffset = 0;
	DWORD DiffuseDestOffset = 0;
	DWORD SpecularDestOffset = 0;

	// Only compute offsets if lighting is enabled
	if (bLighting)
	{
		// Get dest offsets
		DWORD offset = PosSizeDest;
		if (DestFVF & D3DFVF_NORMAL)
		{
			offset += sizeof(float) * 3;
		}
		if (DestFVF & D3DFVF_RESERVED1)
		{
			offset += sizeof(DWORD);
		}
		if (DestFVF & D3DFVF_DIFFUSE)
		{
			DiffuseDestOffset = offset;
			offset += sizeof(DWORD);
		}
		if (DestFVF & D3DFVF_SPECULAR)
		{
			SpecularDestOffset = offset;
			offset += sizeof(DWORD);
		}

		// Get src offsets
		offset = PosSizeSrc;
		if (SrcFVF & D3DFVF_NORMAL)
		{
			NormalSrcOffset = offset;
			offset += sizeof(float) * 3;
		}
		if (SrcFVF & D3DFVF_RESERVED1)
		{
			offset += sizeof(DWORD);
		}
		if (SrcFVF & D3DFVF_DIFFUSE)
		{
			DiffuseSrcOffset = offset;
			offset += sizeof(DWORD);
		}
		if (SrcFVF & D3DFVF_SPECULAR)
		{
			SpecularSrcOffset = offset;
			offset += sizeof(DWORD);
		}
	}

	// Check for lighiting, must have source normals and dest diffuse or specular
	if (bLighting && (!NormalSrcOffset || (!(DestFVF & D3DFVF_DIFFUSE) && !(DestFVF & D3DFVF_SPECULAR))))
	{
		bLighting = false;
		LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_LIGHT' is specified but verticies don't support it: " << Logging::hex(SrcFVF) << " -> " << Logging::hex(DestFVF));
	}

	// Get transformation matrices
	D3DMATRIX matWorld, matView, matProj;
	if (FAILED(pDirect3DDeviceX->GetD9Transform(D3DTS_WORLD, &matWorld)) ||
		FAILED(pDirect3DDeviceX->GetD9Transform(D3DTS_VIEW, &matView)) ||
		FAILED(pDirect3DDeviceX->GetD9Transform(D3DTS_PROJECTION, &matProj)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get transform matrices");
		return DDERR_GENERIC;
	}

	matProj = pDirect3DDeviceX->GetUpdatedProjectionMatrix(matProj, true);

	D3DMATRIX matWorldView = {}, matWorldViewProj = {};
	D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
	D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &matProj);

	// Get viewport
	D3DVIEWPORT9 vp = {};
	if (FAILED(pDirect3DDeviceX->GetD9Viewport(&vp)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get viewport");
		return DDERR_GENERIC;
	}

	// Cache specular, ambient, material and lights if needed
	LightingState lsState = {};
	D3DMATRIX matNormal = {};

	if (DWORD rsColorVertex = 0;
		(DiffuseSrcOffset || SpecularSrcOffset) &&
		SUCCEEDED(pDirect3DDeviceX->GetD9RenderState(D3DRS_COLORVERTEX, &rsColorVertex)))
	{
		lsState.ColorVertex = rsColorVertex != FALSE;
	}

	// Get lighting data
	if (bLighting)
	{
		lsState.DiffuseMaterialSource = D3DMCS_COLOR1;
		lsState.SpecularMaterialSource = D3DMCS_COLOR2;
		lsState.AmbientMaterialSource = D3DMCS_MATERIAL;
		lsState.EmissiveMaterialSource = D3DMCS_MATERIAL;

		lsState.ViewMatrix = matView;

		if (DWORD rsSpecular = 0;
			SUCCEEDED(pDirect3DDeviceX->GetD9RenderState(D3DRS_SPECULARENABLE, &rsSpecular)))
		{
			lsState.UseSpecular = rsSpecular != FALSE;
		}

		if (DWORD rsLocalViewer = 0;
			SUCCEEDED(pDirect3DDeviceX->GetD9RenderState(D3DRS_LOCALVIEWER, &rsLocalViewer)))
		{
			lsState.LocalViewer = rsLocalViewer != FALSE;
		}

		if (DWORD rsNormalizeNormals = 0;
			SUCCEEDED(pDirect3DDeviceX->GetD9RenderState(D3DRS_NORMALIZENORMALS, &rsNormalizeNormals)))
		{
			lsState.NormalizeNormals = rsNormalizeNormals != FALSE;
		}

		pDirect3DDeviceX->GetD9RenderState(D3DRS_AMBIENT, &lsState.AmbientRenderState);

		pDirect3DDeviceX->GetD9RenderState(D3DRS_DIFFUSEMATERIALSOURCE, &lsState.DiffuseMaterialSource);
		pDirect3DDeviceX->GetD9RenderState(D3DRS_SPECULARMATERIALSOURCE, &lsState.SpecularMaterialSource);
		pDirect3DDeviceX->GetD9RenderState(D3DRS_AMBIENTMATERIALSOURCE, &lsState.AmbientMaterialSource);
		pDirect3DDeviceX->GetD9RenderState(D3DRS_EMISSIVEMATERIALSOURCE, &lsState.EmissiveMaterialSource);

		pDirect3DDeviceX->GetD9Material(&lsState.Material);

		D3DXMatrixInverse(&matNormal, nullptr, &matWorld);
		D3DXMatrixTranspose(&matNormal, &matNormal);
	}

	// Precalculate a few static viewport factors, to save on per-vertex cycles
	const float viewportHalfWidth = static_cast<float>(vp.Width) * 0.5f;
	const float viewportHalfHeight = static_cast<float>(vp.Height) * 0.5f;
	const float viewportZDelta = vp.MaxZ - vp.MinZ;

	BYTE* pSrcVertex = (BYTE*)lpSrcBuffer + (dwSrcIndex * SrcStride);
	BYTE* pDestVertex = (BYTE*)lpDestBuffer + (dwDestIndex * DestStride);

	const bool SimpleCopy = (SrcFVF & ~D3DFVF_POSITION_MASK) == (DestFVF & ~D3DFVF_POSITION_MASK) && (SrcStride - PosSizeSrc) == (DestStride - PosSizeDest);

	// Copy vertex data
	if (!DoNotCopyData)
	{
		if (SrcFVF == DestFVF || ((SrcFVF & ~(D3DFVF_XYZ | D3DFVF_RESERVED1)) == (DestFVF & ~D3DFVF_XYZRHW) && SrcStride == DestStride))
		{
			DoNotCopyData = true;
			memcpy(pDestVertex, pSrcVertex, dwCount * DestStride);
		}
	}

	for (UINT i = 0; i < dwCount; ++i)
	{
		// Copy or convert vertex data
		if (!DoNotCopyData)
		{
			if (SimpleCopy)
			{
				memcpy(pDestVertex + PosSizeDest, pSrcVertex + PosSizeSrc, SrcStride - PosSizeSrc);
			}
			else
			{
				ConvertVertex(pDestVertex, DestFVF, pSrcVertex, SrcFVF);
			}
		}

		// Source position
		D3DXVECTOR3& src = *reinterpret_cast<D3DXVECTOR3*>(pSrcVertex);

		// Projection-space position
		D3DXVECTOR4 h = TransformVector4(src.x, src.y, src.z, 1.0f, matWorldViewProj);

		// Output vertex
		D3DVERTEX4& dst = *reinterpret_cast<D3DVERTEX4*>(pDestVertex);

		// Preserve INF/NAN behavior
		dst.rhw = 1.0f / h.w;

		// Convert to screen-space TL coords
		dst.sx = vp.X + (h.x * dst.rhw + 1.0f) * viewportHalfWidth;
		dst.sy = vp.Y + (1.0f - h.y * dst.rhw) * viewportHalfHeight;
		dst.sz = vp.MinZ + (h.z * dst.rhw) * viewportZDelta;

		D3DCOLOR Diffuse = 0xFFFFFFFF, Specular = 0;	// Default diffuse to white

		// Get source diffuse and specular
		if (lsState.ColorVertex)
		{
			if (DiffuseSrcOffset)
			{
				Diffuse = *reinterpret_cast<D3DCOLOR*>(pSrcVertex + DiffuseSrcOffset);
			}
			if (SpecularSrcOffset)
			{
				Specular = *reinterpret_cast<D3DCOLOR*>(pSrcVertex + SpecularSrcOffset);
			}
		}

		// Lighting
		if (bLighting)
		{
			// View-space position
			D3DXVECTOR4 viewPos4 = TransformVector4(src.x, src.y, src.z, 1.0f, matWorldView);

			D3DXVECTOR3 transformedPos =
			{
				viewPos4.x / viewPos4.w,
				viewPos4.y / viewPos4.w,
				viewPos4.z / viewPos4.w
			};

			// Transform normal
			D3DXVECTOR3 normal = *reinterpret_cast<D3DXVECTOR3*>(pSrcVertex + NormalSrcOffset);
			D3DXVECTOR3 transformedNormal;
			D3DXVec3TransformNormal(&transformedNormal, &normal, &matNormal);

			ComputeLighting(transformedPos, transformedNormal, cachedLights, &lsState, Diffuse, Specular);
		}

		// Set diffuse and specular
		if (DiffuseDestOffset)
		{
			*reinterpret_cast<D3DCOLOR*>(pDestVertex + DiffuseDestOffset) = Diffuse;
		}
		if (SpecularDestOffset)
		{
			*reinterpret_cast<D3DCOLOR*>(pDestVertex + SpecularDestOffset) = Specular;
		}

		// Move to the next vertex
		pSrcVertex += SrcStride;
		pDestVertex += DestStride;
	}

	return D3D_OK;
}

void m_IDirect3DVertexBufferX::ComputeLighting(const D3DVECTOR& Position, const D3DVECTOR& Normal, const std::vector<DXLIGHT7>& lights, const LightingState* s, D3DCOLOR& inoutColor, D3DCOLOR& inoutSpecular)
{
	// Should never happen
	if (!s)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: missing LightingState structure!");
		return;
	}

	// Helpers
	auto ToColor = [](D3DCOLOR c)
	{
		D3DCOLORVALUE v;
		v.r = ((c >> 16) & 0xFF) / 255.0f;
		v.g = ((c >> 8) & 0xFF) / 255.0f;
		v.b = ((c >> 0) & 0xFF) / 255.0f;
		v.a = ((c >> 24) & 0xFF) / 255.0f;
		return v;
	};

	auto Src = [&](DWORD src, const D3DCOLORVALUE& mat, const D3DCOLORVALUE& diffuse, const D3DCOLORVALUE& specular)
	{
		switch (src)
		{
		case D3DMCS_COLOR1:
			return diffuse;

		case D3DMCS_COLOR2:
			return specular;

		case D3DMCS_MATERIAL:
		default:
			return mat;
		}
	};

	// Position
	D3DXVECTOR3 VWPosition(Position.x, Position.y, Position.z);

	// Normal
	D3DXVECTOR3 normals(Normal.x, Normal.y, Normal.z);

	if (s->NormalizeNormals &&
		D3DXVec3LengthSq(&normals) > 1e-12f)
	{
		D3DXVec3Normalize(&normals, &normals);
	}

	// Viewer vector
	D3DXVECTOR3 NVWPosition = VWPosition;

	if (D3DXVec3LengthSq(&NVWPosition) > 1e-12f)
	{
		D3DXVec3Normalize(&NVWPosition, &NVWPosition);
	}

	// Vertex colors
	D3DCOLORVALUE inDiffuse = ToColor(inoutColor);
	D3DCOLORVALUE inSpecular = ToColor(inoutSpecular);

	// Initial accumulators
	D3DCOLORVALUE diffuse = { 0, 0, 0, 1 };
	D3DCOLORVALUE specular = { 0, 0, 0, 1 };

	D3DCOLORVALUE ambient = ToColor(s->AmbientRenderState);

	const float materialPower = s->UseSpecular ? s->Material.Power : 0.0f;

	// Light loop
	for (const auto& light : lights)
	{
		D3DXVECTOR3 hitDirection;
		float attenuation = 1.0f;

		switch ((DWORD)light.dltType)
		{
		case D3DLIGHT_DIRECTIONAL:
		{
			D3DXVECTOR3 dir(-light.dvDirection.x, -light.dvDirection.y, -light.dvDirection.z);

			D3DXVec3TransformNormal(&hitDirection, &dir, &s->ViewMatrix);

			D3DXVec3Normalize(&hitDirection, &hitDirection);

			attenuation = 1.0f;

			break;
		}

		case D3DLIGHT_POINT:
		case D3DLIGHT_SPOT:
		{
			D3DXVECTOR3 worldPos(light.dvPosition.x, light.dvPosition.y, light.dvPosition.z);
			D3DXVECTOR3 lightPos;

			D3DXVec3TransformCoord(&lightPos, &worldPos, &s->ViewMatrix);

			hitDirection = lightPos - VWPosition;

			float distSq = D3DXVec3LengthSq(&hitDirection);
			float dist = sqrtf(distSq);

			if (dist <= 1e-12f)
			{
				continue;
			}

			D3DXVec3Normalize(&hitDirection, &hitDirection);

			if (light.dwLightVersion != 7)
			{
				// legacy attenuation
				float d = (light.dvRange - dist) / light.dvRange;

				if (d <= 0.0f)
				{
					continue;
				}

				distSq = d * d;
				dist = d;
			}

			attenuation = (light.dvAttenuation0) + (light.dvAttenuation1 * dist) + (light.dvAttenuation2 * distSq);

			if (light.dwLightVersion == 7)
			{
				if (attenuation != 0.0f)
				{
					attenuation = 1.0f / attenuation;
				}
			}

			// Spot
			if (light.dltType == D3DLIGHT_SPOT)
			{
				D3DXVECTOR3 dir(light.dvDirection.x, light.dvDirection.y, light.dvDirection.z);
				D3DXVECTOR3 lightDir;

				D3DXVec3TransformNormal(&lightDir, &dir, &s->ViewMatrix);

				D3DXVec3Normalize(&lightDir, &lightDir);

				D3DXVECTOR3 nhitDirection = -hitDirection;
				float rho = D3DXVec3Dot(&nhitDirection, &lightDir);

				float cosHalfPhi = cosf(light.dvPhi * 0.5f);
				float cosHalfTheta = cosf(light.dvTheta * 0.5f);

				if (rho <= cosHalfPhi)
				{
					attenuation = 0.0f;
				}
				else if (rho <= cosHalfTheta)
				{
					attenuation *= powf((rho - cosHalfPhi) / (cosHalfTheta - cosHalfPhi), light.dvFalloff);
				}
			}

			break;
		}

		case D3DLIGHT_PARALLELPOINT:
		default:
			LOG_LIMIT(100, __FUNCTION__ << " Warning: Unsupported light type: " << light.dltType);
			continue;
		}

		// Ambient accumulation
		ambient.r += light.dcvAmbient.r * attenuation;
		ambient.g += light.dcvAmbient.g * attenuation;
		ambient.b += light.dcvAmbient.b * attenuation;

		// Diffuse
		float NdotL = D3DXVec3Dot(&normals, &hitDirection);
		float NdotLClamped = CLAMP(NdotL, 0.0f, 1.0f);

		if (NdotLClamped > 0.0f)
		{
			diffuse.r += light.dcvDiffuse.r * NdotLClamped * attenuation;
			diffuse.g += light.dcvDiffuse.g * NdotLClamped * attenuation;
			diffuse.b += light.dcvDiffuse.b * NdotLClamped * attenuation;

			// Specular
			if (materialPower > 0.0f && !(light.dwFlags & D3DLIGHT_NO_SPECULAR))
			{
				D3DXVECTOR3 H;

				if (s->LocalViewer)
				{
					H = hitDirection - NVWPosition;
				}
				else
				{
					H = hitDirection;
					H.z -= 1.0f;
				}

				if (D3DXVec3LengthSq(&H) > 1e-12f)
				{
					D3DXVec3Normalize(&H, &H);

					const float NdotH = max(0.0f, D3DXVec3Dot(&normals, &H));

					if (NdotH > 0.0f)
					{
						const float spec = powf(NdotH, materialPower) * attenuation;

						specular.r += light.dcvSpecular.r * spec;
						specular.g += light.dcvSpecular.g * spec;
						specular.b += light.dcvSpecular.b * spec;
					}
				}
			}
		}
	}

	// Material sourcing
	D3DCOLORVALUE materialDiffuse = Src(s->DiffuseMaterialSource, s->Material.Diffuse, inDiffuse, inSpecular);
	D3DCOLORVALUE materialSpecular = Src(s->SpecularMaterialSource, s->Material.Specular, inDiffuse, inSpecular);
	D3DCOLORVALUE materialAmbient = Src(s->AmbientMaterialSource, s->Material.Ambient, inDiffuse, inSpecular);
	D3DCOLORVALUE materialEmissive = Src(s->EmissiveMaterialSource, s->Material.Emissive, inDiffuse, inSpecular);

	// Final combine
	diffuse.r = (ambient.r * materialAmbient.r) + (diffuse.r * materialDiffuse.r) + materialEmissive.r;
	diffuse.g = (ambient.g * materialAmbient.g) + (diffuse.g * materialDiffuse.g) + materialEmissive.g;
	diffuse.b = (ambient.b * materialAmbient.b) + (diffuse.b * materialDiffuse.b) + materialEmissive.b;
	diffuse.a = s->Material.Diffuse.a;

	specular.r *= materialSpecular.r;
	specular.g *= materialSpecular.g;
	specular.b *= materialSpecular.b;
	specular.a = materialSpecular.a;

	// Output
	inoutColor = D3DCOLOR_COLORVALUE(
		CLAMP(diffuse.r, 0.0f, 1.0f),
		CLAMP(diffuse.g, 0.0f, 1.0f),
		CLAMP(diffuse.b, 0.0f, 1.0f),
		CLAMP(diffuse.a, 0.0f, 1.0f));

	inoutSpecular = s->UseSpecular
		? D3DCOLOR_COLORVALUE(
			CLAMP(specular.r, 0.0f, 1.0f),
			CLAMP(specular.g, 0.0f, 1.0f),
			CLAMP(specular.b, 0.0f, 1.0f),
			CLAMP(specular.a, 0.0f, 1.0f))
		: 0;
}
