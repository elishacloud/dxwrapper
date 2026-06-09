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

		DWORD Flags = (dwFlags & (DDLOCK_READONLY | DDLOCK_DISCARDCONTENTS | DDLOCK_NOSYSLOCK)) |
			(IsVBEmulated || (Config.DdrawVertexLockDiscard && !(dwFlags & DDLOCK_READONLY)) ? D3DLOCK_DISCARD : NULL) |
			(Config.DdrawNoDrawBufferSysLock ? D3DLOCK_NOSYSLOCK : NULL);

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
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
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

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

		m_IDirect3DVertexBufferX* pSrcVertexBufferX = nullptr;
		lpSrcBuffer->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pSrcVertexBufferX);

		if (!pSrcVertexBufferX)
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

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		// Get FVF
		DWORD dwSrcVertexTypeDesc = pSrcVertexBufferX->VB.Desc.dwFVF;
		DWORD dwDestVertexTypeDesc = VB.Desc.dwFVF;

		// Validate destination range
		DWORD DestNumVertices = VB.Desc.dwNumVertices;
		if (dwDestIndex >= DestNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: destination vertex index is too large: " << DestNumVertices << " -> " << dwDestIndex);
			return D3DERR_INVALIDVERTEXTYPE;
		}
		dwCount = min(dwCount, DestNumVertices - dwDestIndex);

		// Check the dwSrcIndex and dwCount to make sure they won't cause an overload
		DWORD SrcNumVertices = pSrcVertexBufferX->VB.Desc.dwNumVertices;
		if (dwSrcIndex > SrcNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: source vertex index is too large: " << SrcNumVertices << " -> " << dwSrcIndex);
			return DDERR_INVALIDPARAMS;
		}
		dwCount = min(dwCount, SrcNumVertices - dwSrcIndex);

		void* pSrcVertices = nullptr;

		// Lock the source vertex buffer
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

		D3DRECT drExtent = { LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN };

		HRESULT hr = ProcessVerticesUP(dwVertexOp, pDestVertices, dwDestVertexTypeDesc, dwDestIndex, dwCount, pSrcVertices, dwSrcVertexTypeDesc, dwSrcIndex, drExtent, lpD3DDevice, dwFlags);

		// Unlock destination vertex buffer
		Unlock();

		// Unlock the source vertex buffer
		pSrcVertexBufferX->Unlock();

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
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

		// Check for device interface
		if (FAILED(CheckInterface(__FUNCTION__, true, true)))
		{
			return DDERR_GENERIC;
		}

#ifdef ENABLE_PROFILING
		auto startTime = std::chrono::high_resolution_clock::now();
#endif

		// Get FVF
		DWORD dwDestVertexTypeDesc = VB.Desc.dwFVF;

		// Validate destination range
		DWORD DestNumVertices = VB.Desc.dwNumVertices;
		if (dwDestIndex >= DestNumVertices)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: destination vertex index is too large: " << DestNumVertices << " -> " << dwDestIndex);
			return D3DERR_INVALIDVERTEXTYPE;
		}
		dwCount = min(dwCount, DestNumVertices - dwDestIndex);

		// Setup vars
		DWORD dwVertexTypeDesc = VB.Desc.dwFVF;
		std::vector<BYTE, aligned_allocator<BYTE, 4>> VertexCache;

		// Process strided data
		if (FAILED(InterleaveStridedVertexData(VertexCache, lpVertexArray, dwSrcIndex, dwCount, dwVertexTypeDesc)))
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

		D3DRECT drExtent = { LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN };

		HRESULT hr = ProcessVerticesUP(dwVertexOp, pDestVertices, dwDestVertexTypeDesc, dwDestIndex, dwCount, VertexCache.data(), dwVertexTypeDesc, dwSrcIndex, drExtent, lpD3DDevice, dwFlags);

		// Unlock destination vertex buffer
		Unlock();

#ifdef ENABLE_PROFILING
		Logging::Log() << __FUNCTION__ << " (" << this << ") hr = " << (D3DERR)hr << " Timing = " << Logging::GetTimeLapseInMS(startTime);
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

	if (ddrawParent)
	{
		ddrawParent->ClearVertexBuffer(this);
	}

	if (D3DInterface)
	{
		D3DInterface->ClearVertexBuffer(this);
	}

	// Don't delete wrapper interface
	SaveInterfaceAddress(WrapperInterface);
	SaveInterfaceAddress(WrapperInterface7);

	ReleaseD9Buffer(false, false);
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

	// ToDo: implement D3DVBCAPS_OPTIMIZED

	IsVBEmulated = (VB.Desc.dwFVF == D3DFVF_LVERTEX) || (Config.DdrawClampVertexZDepth && (VB.Desc.dwFVF && D3DFVF_XYZRHW));

	d3d9VBDesc.FVF = (VB.Desc.dwFVF == D3DFVF_LVERTEX) ? D3DFVF_LVERTEX9 : VB.Desc.dwFVF;
	d3d9VBDesc.Size = GetVertexStride(d3d9VBDesc.FVF) * VB.Desc.dwNumVertices;
	d3d9VBDesc.Usage = D3DUSAGE_DYNAMIC |
		((VB.Desc.dwCaps & D3DVBCAPS_WRITEONLY) || IsVBEmulated ? D3DUSAGE_WRITEONLY : 0) |
		((VB.Desc.dwCaps & D3DVBCAPS_DONOTCLIP) ? D3DUSAGE_DONOTCLIP : 0);
	d3d9VBDesc.Pool = (VB.Desc.dwCaps & D3DVBCAPS_SYSTEMMEMORY) ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;

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

HRESULT m_IDirect3DVertexBufferX::InterleaveStridedVertexData(std::vector<BYTE, aligned_allocator<BYTE, 4>>& outputBuffer, const D3DDRAWPRIMITIVESTRIDEDDATA* sd, const DWORD dwVertexStart, const DWORD dwNumVertices, const DWORD dwVertexTypeDesc)
{
	if (!sd)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: missing D3DDRAWPRIMITIVESTRIDEDDATA!");
		return DDERR_INVALIDPARAMS;
	}

	DWORD Stride = GetVertexStride(dwVertexTypeDesc);

	bool hasPosition = (dwVertexTypeDesc & D3DFVF_POSITION_MASK);
	bool hasReserved = (dwVertexTypeDesc & D3DFVF_RESERVED1);
	bool hasNormal = (dwVertexTypeDesc & D3DFVF_NORMAL);
	bool hasDiffuse = (dwVertexTypeDesc & D3DFVF_DIFFUSE);
	bool hasSpecular = (dwVertexTypeDesc & D3DFVF_SPECULAR);
	DWORD texCount = D3DFVF_TEXCOUNT(dwVertexTypeDesc);

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
		if (sd->position.dwStride && sd->position.dwStride < posStride)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: position stride does not match: " << posStride << " -> " << sd->position.dwStride << " FVF: " << Logging::hex(dwVertexTypeDesc));
		}
	}
	if (hasNormal)
	{
		if (!sd->normal.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: normal data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}
		if (sd->normal.dwStride && sd->normal.dwStride < sizeof(D3DXVECTOR3))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: normal stride does not match: " << sizeof(D3DXVECTOR3) << " -> " << sd->normal.dwStride << " FVF: " << Logging::hex(dwVertexTypeDesc));
		}
	}
	if (hasDiffuse)
	{
		if (!sd->diffuse.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: diffuse data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}
		if (sd->diffuse.dwStride && sd->diffuse.dwStride < sizeof(D3DCOLOR))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: diffuse stride does not match: " << sizeof(D3DCOLOR) << " -> " << sd->diffuse.dwStride << " FVF: " << Logging::hex(dwVertexTypeDesc));
		}
	}
	if (hasSpecular)
	{
		if (!sd->specular.lpvData)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: specular data missing! FVF: " << Logging::hex(dwVertexTypeDesc));
			return DDERR_INVALIDPARAMS;
		}
		if (sd->specular.dwStride && sd->specular.dwStride < sizeof(D3DCOLOR))
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: specular stride does not match: " << sizeof(D3DCOLOR) << " -> " << sd->specular.dwStride << " FVF: " << Logging::hex(dwVertexTypeDesc));
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
		if (sd->textureCoords[t].dwStride && sd->textureCoords[t].dwStride < texStride[t])
		{
			LOG_LIMIT(100, __FUNCTION__ << " Warning: texture stride does not match: " << texStride[t] << " -> " << sd->textureCoords[t].dwStride << " FVF: " << Logging::hex(dwVertexTypeDesc));
		}
	}

	outputBuffer.resize((dwVertexStart + dwNumVertices) * Stride);

	BYTE* cursor = outputBuffer.data() + dwVertexStart * Stride;
	BYTE* posCursor = reinterpret_cast<BYTE*>(sd->position.lpvData) + dwVertexStart * (sd->position.dwStride ? sd->position.dwStride : posStride);
	BYTE* normalCursor = reinterpret_cast<BYTE*>(sd->normal.lpvData) + dwVertexStart * (sd->normal.dwStride ? sd->normal.dwStride : sizeof(D3DXVECTOR3));
	BYTE* diffCursor = reinterpret_cast<BYTE*>(sd->diffuse.lpvData) + dwVertexStart * (sd->diffuse.dwStride ? sd->diffuse.dwStride : sizeof(D3DCOLOR));
	BYTE* specCursor = reinterpret_cast<BYTE*>(sd->specular.lpvData) + dwVertexStart * (sd->specular.dwStride ? sd->specular.dwStride : sizeof(D3DCOLOR));
	BYTE* texCursor[D3DDP_MAXTEXCOORD] = {};
	for (DWORD t = 0; t < texCount; ++t)
	{
		texCursor[t] = reinterpret_cast<BYTE*>(sd->textureCoords[t].lpvData) + dwVertexStart * (sd->textureCoords[t].dwStride ? sd->textureCoords[t].dwStride : texStride[t]);
	}

	for (DWORD i = 0; i < dwNumVertices; ++i)
	{
		if (hasPosition)
		{
			memcpy(cursor, posCursor, posStride);
			cursor += posStride;
			posCursor += sd->position.dwStride ? sd->position.dwStride : posStride;
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
			normalCursor += sd->normal.dwStride ? sd->normal.dwStride : sizeof(D3DXVECTOR3);
		}

		if (hasDiffuse)
		{
			memcpy(cursor, diffCursor, sizeof(D3DCOLOR));
			cursor += sizeof(D3DCOLOR);
			diffCursor += sd->diffuse.dwStride ? sd->diffuse.dwStride : sizeof(D3DCOLOR);
		}

		if (hasSpecular)
		{
			memcpy(cursor, specCursor, sizeof(D3DCOLOR));
			cursor += sizeof(D3DCOLOR);
			specCursor += sd->specular.dwStride ? sd->specular.dwStride : sizeof(D3DCOLOR);
		}

		for (DWORD t = 0; t < texCount; ++t)
		{
			memcpy(cursor, texCursor[t], texStride[t]);
			cursor += texStride[t];
			texCursor[t] += sd->textureCoords[t].dwStride ? sd->textureCoords[t].dwStride : texStride[t];
		}
	}

	return D3D_OK;
}

HRESULT m_IDirect3DVertexBufferX::ProcessVerticesUP(DWORD dwVertexOp, LPVOID lpDestBuffer, DWORD dwDestVertexTypeDesc, DWORD dwDestIndex, DWORD dwCount, LPVOID lpSrcBuffer, DWORD dwSrcVertexTypeDesc, DWORD dwSrcIndex, D3DRECT& drExtent, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	if (dwCount == 0)
	{
		return D3D_OK;	// No vertices to process
	}

	if (!lpDestBuffer || !lpSrcBuffer || !lpD3DDevice)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Get our wrapper device
	m_IDirect3DDeviceX* pDirect3DDeviceX = nullptr;
	lpD3DDevice->QueryInterface(IID_GetInterfaceX, (LPVOID*)&pDirect3DDeviceX);
	if (!pDirect3DDeviceX)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: could not get Direct3DDeviceX interface!");
		return DDERR_GENERIC;
	}

	// Get and verify FVF
	DWORD SrcFVF = dwSrcVertexTypeDesc;
	DWORD DestFVF = dwDestVertexTypeDesc;
	UINT SrcStride = GetVertexStride(SrcFVF);
	UINT DestStride = GetVertexStride(DestFVF);
	DWORD SrcPosFVF = SrcFVF & D3DFVF_POSITION_MASK;
	DWORD DestPosFVF = DestFVF & D3DFVF_POSITION_MASK;

	// Cannot transform RHW vertices or convert to non-RHW TL format
	if (SrcPosFVF == D3DFVF_XYZRHW || DestPosFVF != D3DFVF_XYZRHW)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Invalid FVF conversion: Cannot transform from D3DFVF_XYZRHW or to non-D3DFVF_XYZRHW format: " << Logging::hex(SrcFVF) << " -> " << Logging::hex(DestFVF));
		return D3DERR_INVALIDVERTEXTYPE;
	}

	// Handle dwVertexOp
	if (dwVertexOp & D3DVOP_CLIP)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_CLIP' not handled!");
	}
	if (dwVertexOp & D3DVOP_EXTENTS)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: 'D3DVOP_EXTENTS' not handled!");
	}
	// D3DVOP_TRANSFORM is inherently handled by ProcessVertices() as it performs vertex transformations based on the current world, view, and projection matrices.
	if (!(dwVertexOp & D3DVOP_TRANSFORM))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Warning: D3DVOP_TRANSFORM not set, forcing transform");
		dwVertexOp |= D3DVOP_TRANSFORM;
	}

	bool DoNotCopyData = (dwFlags & D3DPV_DONOTCOPYDATA) != 0;
	bool bLighting = (dwVertexOp & D3DVOP_LIGHT);
	bool bUpdateExtents = (dwVertexOp & D3DVOP_EXTENTS);

	// Check lighting state
	if (bLighting)
	{
		DWORD rsLighting = FALSE;
		if (FAILED(pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_LIGHTING, &rsLighting)) || rsLighting == FALSE)
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
	if (FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld)) ||
		FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView)) ||
		FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get transform matrices");
		return DDERR_GENERIC;
	}

	D3DMATRIX matWorldView = {}, matWorldViewProj = {};
	D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
	D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &matProj);

	// Get viewport
	D3DVIEWPORT7 vp = {};
	if (FAILED(pDirect3DDeviceX->GetViewport(&vp)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get viewport");
		return DDERR_GENERIC;
	}

	// Cache specular, ambient, material and lights if needed
	LightingState lsState = {};
	D3DMATRIX matNormal = {};

	if (DWORD rsColorVertex = 0;
		(DiffuseSrcOffset || SpecularSrcOffset) &&
		SUCCEEDED(pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_COLORVERTEX, &rsColorVertex)))
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
			SUCCEEDED(pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_SPECULARENABLE, &rsSpecular)))
		{
			lsState.UseSpecular = rsSpecular != FALSE;
		}

		if (DWORD rsLocalViewer = 0;
			SUCCEEDED(pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_LOCALVIEWER, &rsLocalViewer)))
		{
			lsState.LocalViewer = rsLocalViewer != FALSE;
		}

		if (DWORD rsNormalizeNormals = 0;
			SUCCEEDED(pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, &rsNormalizeNormals)))
		{
			lsState.NormalizeNormals = rsNormalizeNormals != FALSE;
		}

		pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_AMBIENT, &lsState.AmbientRenderState);

		pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, &lsState.DiffuseMaterialSource);
		pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_SPECULARMATERIALSOURCE, &lsState.SpecularMaterialSource);
		pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_AMBIENTMATERIALSOURCE, &lsState.AmbientMaterialSource);
		pDirect3DDeviceX->GetRenderState(D3DRENDERSTATE_EMISSIVEMATERIALSOURCE, &lsState.EmissiveMaterialSource);

		pDirect3DDeviceX->GetMaterial(&lsState.Material);

		D3DXMatrixInverse(&matNormal, nullptr, &matWorld);
		D3DXMatrixTranspose(&matNormal, &matNormal);
	}

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

	D3DRECT newExtents = { LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN };

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
		D3DXVECTOR4 pos4(src.x, src.y, src.z, 1.0f);

		// View-space position
		D3DXVECTOR4 viewPos4;
		D3DXVec4Transform(&viewPos4, &pos4, &matWorldView);

		// Projection-space position
		D3DXVECTOR4 h;
		D3DXVec4Transform(&h, &pos4, &matWorldViewProj);

		// Output vertex
		D3DXVECTOR4& dst = *reinterpret_cast<D3DXVECTOR4*>(pDestVertex);

		// Preserve INF/NAN behavior
		float rhw = 1.0f / h.w;

		// Convert to screen-space TL coords
		dst.x = vp.dwX + ((h.x * rhw + 1.0f) * vp.dwWidth * 0.5f);
		dst.y = vp.dwY + ((1.0f - h.y * rhw) * vp.dwHeight * 0.5f);
		dst.z = vp.dvMinZ + ((h.z * rhw) * (vp.dvMaxZ - vp.dvMinZ));
		dst.w = rhw;

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
			// Transform normal
			D3DXVECTOR3 normal = *reinterpret_cast<D3DXVECTOR3*>(pSrcVertex + NormalSrcOffset);
			D3DXVECTOR3 transformedNormal;
			D3DXVec3TransformNormal(&transformedNormal, &normal, &matNormal);

			D3DXVECTOR3 transformedPos =
			{
				viewPos4.x / viewPos4.w,
				viewPos4.y / viewPos4.w,
				viewPos4.z / viewPos4.w
			};

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

		// Update extents
		if (bUpdateExtents)
		{
			// floor/ceil convert to integer extents
			newExtents.x1 = min(newExtents.x1, static_cast<LONG>(floorf(dst.x)));
			newExtents.y1 = min(newExtents.y1, static_cast<LONG>(floorf(dst.y)));
			newExtents.x2 = max(newExtents.x2, static_cast<LONG>(ceilf(dst.x)));
			newExtents.y2 = max(newExtents.y2, static_cast<LONG>(ceilf(dst.y)));
		}

		// Move to the next vertex
		pSrcVertex += SrcStride;
		pDestVertex += DestStride;
	}

	if (bUpdateExtents && newExtents.x1 != LONG_MAX)
	{
		if (!IsRectZero(drExtent))
		{
			// Merge with existing extents if valid
			drExtent.x1 = min(drExtent.x1, newExtents.x1);
			drExtent.y1 = min(drExtent.y1, newExtents.y1);
			drExtent.x2 = max(drExtent.x2, newExtents.x2);
			drExtent.y2 = max(drExtent.y2, newExtents.y2);
		}
		else
		{
			// First valid extents
			drExtent.x1 = newExtents.x1;
			drExtent.y1 = newExtents.y1;
			drExtent.x2 = newExtents.x2;
			drExtent.y2 = newExtents.y2;
		}
	}

	return D3D_OK;
}

template HRESULT m_IDirect3DVertexBufferX::TransformVertexUP<XYZ>(m_IDirect3DDeviceX* , XYZ*, D3DTLVERTEX*, D3DHVERTEX*, const DWORD, D3DRECT&);
template HRESULT m_IDirect3DVertexBufferX::TransformVertexUP<D3DLVERTEX>(m_IDirect3DDeviceX* , D3DLVERTEX*, D3DTLVERTEX*, D3DHVERTEX*, const DWORD, D3DRECT&);
template <typename T>
HRESULT m_IDirect3DVertexBufferX::TransformVertexUP(m_IDirect3DDeviceX* pDirect3DDeviceX, T* srcVertex, D3DTLVERTEX* destVertex, D3DHVERTEX* pHOut, const DWORD dwCount, D3DRECT& drExtent)
{
	D3DMATRIX matWorld, matView, matProj;
	if (FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld)) ||
		FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView)) ||
		FAILED(pDirect3DDeviceX->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get transform matrices");
		return DDERR_GENERIC;
	}

	D3DMATRIX matWorldView = {}, matWorldViewProj = {};
	D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
	D3DXMatrixMultiply(&matWorldViewProj, &matWorldView, &matProj);

	// Get viewport
	D3DVIEWPORT7 vp = {};
	if (FAILED(pDirect3DDeviceX->GetViewport(&vp)))
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Failed to get viewport");
		return DDERR_GENERIC;
	}

	D3DRECT newExtents = { LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN };

	for (DWORD i = 0; i < dwCount; ++i)
	{
		// Source position
		T& src = srcVertex[i];
		D3DXVECTOR4 pos4(src.x, src.y, src.z, 1.0f);

		// View-space position
		D3DXVECTOR4 viewPos4;
		D3DXVec4Transform(&viewPos4, &pos4, &matWorldView);

		// Projection-space position
		D3DXVECTOR4 h;
		D3DXVec4Transform(&h, &pos4, &matWorldViewProj);

		// Output vertex
		D3DTLVERTEX& dst = destVertex[i];

		// Preserve INF/NAN behavior
		float rhw = 1.0f / h.w;

		// Convert to screen-space TL coords
		dst.sx = vp.dwX + ((h.x * rhw + 1.0f) * vp.dwWidth * 0.5f);
		dst.sy = vp.dwY + ((1.0f - h.y * rhw) * vp.dwHeight * 0.5f);
		dst.sz = vp.dvMinZ + ((h.z * rhw) * (vp.dvMaxZ - vp.dvMinZ));
		dst.rhw = rhw;

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

		// Fill homogeneous out if requested
		if (pHOut)
		{
			D3DHVERTEX& hdst = pHOut[i];
			// Store pre-divide homogeneous coords
			hdst.hx = h.x;
			hdst.hy = h.y;
			hdst.hz = h.z;
			hdst.dwFlags = 0; // Clip flags not computed here (TransformVertices only sets them if clipping performed upstream)
		}

		// floor/ceil convert to integer extents
		newExtents.x1 = min(newExtents.x1, static_cast<LONG>(floorf(dst.sx)));
		newExtents.y1 = min(newExtents.y1, static_cast<LONG>(floorf(dst.sy)));
		newExtents.x2 = max(newExtents.x2, static_cast<LONG>(ceilf(dst.sx)));
		newExtents.y2 = max(newExtents.y2, static_cast<LONG>(ceilf(dst.sy)));
	}

	if (newExtents.x1 != LONG_MAX)
	{
		if (!IsRectZero(drExtent))
		{
			// Merge with existing extents if valid
			drExtent.x1 = min(drExtent.x1, newExtents.x1);
			drExtent.y1 = min(drExtent.y1, newExtents.y1);
			drExtent.x2 = max(drExtent.x2, newExtents.x2);
			drExtent.y2 = max(drExtent.y2, newExtents.y2);
		}
		else
		{
			// First valid extents
			drExtent.x1 = newExtents.x1;
			drExtent.y1 = newExtents.y1;
			drExtent.x2 = newExtents.x2;
			drExtent.y2 = newExtents.y2;
		}
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

	const float materialPower = s->UseSpecular ? s->Material.power : 0.0f;

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
	D3DCOLORVALUE materialDiffuse = Src(s->DiffuseMaterialSource, s->Material.diffuse, inDiffuse, inSpecular);
	D3DCOLORVALUE materialSpecular = Src(s->SpecularMaterialSource, s->Material.specular, inDiffuse, inSpecular);
	D3DCOLORVALUE materialAmbient = Src(s->AmbientMaterialSource, s->Material.ambient, inDiffuse, inSpecular);
	D3DCOLORVALUE materialEmissive = Src(s->EmissiveMaterialSource, s->Material.emissive, inDiffuse, inSpecular);

	// Final combine
	diffuse.r = (ambient.r * materialAmbient.r) + (diffuse.r * materialDiffuse.r) + materialEmissive.r;
	diffuse.g = (ambient.g * materialAmbient.g) + (diffuse.g * materialDiffuse.g) + materialEmissive.g;
	diffuse.b = (ambient.b * materialAmbient.b) + (diffuse.b * materialDiffuse.b) + materialEmissive.b;
	diffuse.a = s->Material.diffuse.a;

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
