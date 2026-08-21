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
		dwFlags = (dwFlags & DDLOCK_WRITEONLY) || (VB9.Usage & D3DUSAGE_WRITEONLY) ? (dwFlags & ~DDLOCK_DISCARDCONTENTS) : dwFlags;

		// If using read only and not write only
		dwFlags = (dwFlags & DDLOCK_READONLY) && !(dwFlags & DDLOCK_WRITEONLY) ? dwFlags : (dwFlags & ~(DDLOCK_READONLY | DDLOCK_WRITEONLY));

		DWORD Flags =
			((dwFlags & (DDLOCK_READONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_NOSYSLOCK)) |
				(IsVBEmulated || (Config.DdrawVertexLockDiscard && !(dwFlags & DDLOCK_READONLY)) ? D3DLOCK_DISCARD : NULL) |
				(Config.DdrawNoDrawBufferSysLock ? D3DLOCK_NOSYSLOCK : NULL)) &
			~(VB9.Pool == D3DPOOL_MANAGED ? DDLOCK_DISCARDCONTENTS : 0);

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
		CopyBufferFromEmulatedMem((BYTE*)LastLock.Addr);

		// Unlock vertex buffer
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
		// Always include the D3DVOP_TRANSFORM flag in the dwVertexOp parameter. If you do not, the method fails, returning DDERR_INVALIDPARAMS.
		if (!lpSrcBuffer || !lpD3DDevice || !(dwVertexOp & D3DVOP_TRANSFORM))
		{
			return DDERR_INVALIDPARAMS;
		}

		// Docs state that D3DVOP_CLIP cannot be used with a vertex buffer created with the D3DVBCAPS_DONOTCLIP flag
		if ((dwVertexOp & D3DVOP_CLIP) && (VB.Desc.dwCaps & D3DVBCAPS_DONOTCLIP))
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

		// Check source vertex buffer for device interface
		if (FAILED(pSrcVertexBufferX->CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		// Get FVF
		const DWORD SrcFVF = pSrcVertexBufferX->VB9.FVF;

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

		// Check if there are no vertices to process
		if (dwCount == 0)
		{
			return D3D_OK;
		}

		const BOOL doClipping = (dwVertexOp & D3DVOP_CLIP) ? TRUE : FALSE;
		const BOOL doLighting = (dwVertexOp & D3DVOP_LIGHT) && (SrcFVF & D3DFVF_NORMAL) && pDirect3DDeviceX->IsMaterialEnabled() ? TRUE : FALSE;

		IDirect3DVertexBuffer9* pSrcBuffer = pSrcVertexBufferX->GetCurrentD9VertexBuffer();
		IDirect3DVertexBuffer9* pDestBuffer = GetCurrentD9VertexBuffer();

		// Try using d3d9 device for vertex processing
		HRESULT hr = pDirect3DDeviceX->ProcessVertices(dwSrcIndex, dwDestIndex, dwCount, pSrcBuffer, pDestBuffer, SrcFVF, doLighting, doClipping, (dwFlags & D3DPV_DONOTCOPYDATA), nullptr);

		if (SUCCEEDED(hr))
		{
			hr = CopyBufferToEmulatedMem();
		}

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
		// Always include the D3DVOP_TRANSFORM flag in the dwVertexOp parameter. If you do not, the method fails, returning DDERR_INVALIDPARAMS.
		if (!lpVertexArray || !lpD3DDevice || !(dwVertexOp & D3DVOP_TRANSFORM))
		{
			return DDERR_INVALIDPARAMS;
		}

		// Docs state that D3DVOP_CLIP cannot be used with a vertex buffer created with the D3DVBCAPS_DONOTCLIP flag
		if ((dwVertexOp & D3DVOP_CLIP) && (VB.Desc.dwCaps & D3DVBCAPS_DONOTCLIP))
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

		// Get FVF
		const DWORD DestFVF = VB9.FVF;

		// Validate destination range
		const DWORD DestNumVertices = VB.Desc.dwNumVertices;
		if (dwDestIndex >= DestNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: destination vertex index is too large: " << DestNumVertices << " -> " << dwDestIndex);
			return D3DERR_INVALIDVERTEXTYPE;
		}
		dwCount = min(dwCount, DestNumVertices - dwDestIndex);

		// Check if there are no vertices to process
		if (dwCount == 0)
		{
			return D3D_OK;
		}

		// Get source vars
		DWORD SrcFVF = GetStridedVertexTypeDesc(*lpVertexArray);
		std::vector<BYTE, aligned_allocator<BYTE, 4>> SrcVertexCache;

		if (D3DFVF_TEXCOUNT(SrcFVF) != D3DFVF_TEXCOUNT(DestFVF))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: source and destination FVF texture counts don't match: " << D3DFVF_TEXCOUNT(SrcFVF) << " -> " << D3DFVF_TEXCOUNT(DestFVF));
		}

		// Process strided data
		if (FAILED(InterleaveStridedVertexData(SrcVertexCache, *lpVertexArray, dwSrcIndex, dwCount, SrcFVF)))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: invalid StridedVertexData!");
			return DDERR_INVALIDPARAMS;
		}

		const BOOL doClipping = (dwVertexOp & D3DVOP_CLIP) ? TRUE : FALSE;
		const BOOL doLighting = (dwVertexOp & D3DVOP_LIGHT) && (SrcFVF & D3DFVF_NORMAL) && pDirect3DDeviceX->IsMaterialEnabled() ? TRUE : FALSE;

		void* SrcVertices = SrcVertexCache.data();
		UINT SrcVertexSize = GetVertexStride(SrcFVF) * dwCount;

		IDirect3DVertexBuffer9* pDestBuffer = GetCurrentD9VertexBuffer();

		// Try using d3d9 device for vertex processing
		HRESULT hr = pDirect3DDeviceX->ProcessVerticesStrided(dwDestIndex, dwCount, SrcVertices, SrcVertexSize, pDestBuffer, SrcFVF, doLighting, doClipping, (dwFlags & D3DPV_DONOTCOPYDATA));

		if (SUCCEEDED(hr))
		{
			hr = CopyBufferToEmulatedMem();
		}

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

	VB9.FVF = (VB.Desc.dwFVF == D3DFVF_LVERTEX) ? D3DFVF_LVERTEX9 : VB.Desc.dwFVF;
	VB9.Stride = GetVertexStride(VB9.FVF);
	VB9.Size = VB9.Stride * VB.Desc.dwNumVertices;
	VB9.Pool = (VB.Desc.dwCaps & D3DVBCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_MANAGED;
	VB9.Usage =
		(VB9.Pool != D3DPOOL_MANAGED ? D3DUSAGE_DYNAMIC : 0) |
		((VB.Desc.dwCaps & D3DVBCAPS_WRITEONLY) || IsVBEmulated ? D3DUSAGE_WRITEONLY : 0) |
		((VB.Desc.dwCaps & D3DVBCAPS_DONOTCLIP) ? D3DUSAGE_DONOTCLIP : 0);

	HRESULT hr = (*d3d9Device)->CreateVertexBuffer(VB9.Size, VB9.Usage, VB9.FVF, VB9.Pool, &d3d9VertexBuffer, nullptr);
	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to create vertex buffer: " << (D3DERR)hr <<
			" Size: " << VB9.Size << " Usage: " << Logging::hex(VB9.Usage) << " FVF: " << Logging::hex(VB9.FVF) <<
			" Pool: " << Logging::hex(VB9.Pool) << VB.Desc);
		return DDERR_GENERIC;
	}

	VB.Stride = GetVertexStride(VB.Desc.dwFVF);
	VB.Size = VB.Stride * VB.Desc.dwNumVertices;

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

	if (!ResetBuffer || VB9.Pool == D3DPOOL_DEFAULT)
	{
		ReleaseD3D9VertexBuffer();
	}
}

void m_IDirect3DVertexBufferX::CopyBufferFromEmulatedMem(BYTE* pVertexData)
{
	if (!pVertexData)
	{
		return;
	}

	const bool ShouldClampZ = (Config.DdrawClampVertexZDepth && (VB9.FVF & D3DFVF_XYZRHW));

	if (!IsVBEmulated && !ShouldClampZ)
	{
		return;
	}

	if (IsVBEmulated)
	{
		if (VB.Desc.dwFVF == D3DFVF_LVERTEX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: converting vertex buffer, may cause slowdowns!");

			ConvertLVertex((DXLVERTEX9*)pVertexData, (DXLVERTEX7*)VertexData.data(), VB.Desc.dwNumVertices);
		}
		else
		{
			memcpy(pVertexData, VertexData.data(), VB.Size);
		}
	}

	if (ShouldClampZ)
	{
		ClampVertices(pVertexData, VB9.Stride, VB.Desc.dwNumVertices);
	}
}

HRESULT m_IDirect3DVertexBufferX::CopyBufferToEmulatedMem()
{
	const bool ShouldClampZ = (Config.DdrawClampVertexZDepth && (VB9.FVF & D3DFVF_XYZRHW));

	if (!IsVBEmulated && !ShouldClampZ)
	{
		return D3D_OK;
	}

	BYTE* pVertexData = nullptr;
	HRESULT hr = d3d9VertexBuffer->Lock(0, 0, (void**)&pVertexData, D3DLOCK_READONLY);

	if (FAILED(hr))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not lock buffer!");
		return hr;
	}

	if (IsVBEmulated)
	{
		if (VB.Desc.dwFVF == D3DFVF_LVERTEX)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: converting vertex buffer, may cause slowdowns!");

			ConvertLVertex((DXLVERTEX7*)VertexData.data(), (DXLVERTEX9*)pVertexData, VB.Desc.dwNumVertices);
		}
		else
		{
			memcpy(VertexData.data(), pVertexData, VB.Size);
		}
	}

	if (ShouldClampZ)
	{
		ClampVertices(pVertexData, VB9.Stride, VB.Desc.dwNumVertices);
	}

	d3d9VertexBuffer->Unlock();

	return D3D_OK;
}
