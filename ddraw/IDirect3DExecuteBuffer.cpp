/**
* Copyright (C) 2024 Elisha Riedlinger
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

// Cached wrapper interface
namespace {
	m_IDirect3DExecuteBuffer* WrapperInterfaceBackup = nullptr;
}

inline static void SaveInterfaceAddress(m_IDirect3DExecuteBuffer* Interface, m_IDirect3DExecuteBuffer*& InterfaceBackup)
{
	if (Interface)
	{
		SetCriticalSection();
		Interface->SetProxy(nullptr, nullptr);
		if (InterfaceBackup)
		{
			InterfaceBackup->DeleteMe();
			InterfaceBackup = nullptr;
		}
		InterfaceBackup = Interface;
		ReleaseCriticalSection();
	}
}

m_IDirect3DExecuteBuffer* CreateDirect3DExecuteBuffer(IDirect3DExecuteBuffer* aOriginal, m_IDirect3DDeviceX* NewD3DDInterface, LPD3DEXECUTEBUFFERDESC lpDesc)
{
	SetCriticalSection();
	m_IDirect3DExecuteBuffer* Interface = nullptr;
	if (WrapperInterfaceBackup)
	{
		Interface = WrapperInterfaceBackup;
		WrapperInterfaceBackup = nullptr;
		Interface->SetProxy(aOriginal, NewD3DDInterface);
	}
	else
	{
		if (aOriginal)
		{
			Interface = new m_IDirect3DExecuteBuffer(aOriginal);
		}
		else
		{
			Interface = new m_IDirect3DExecuteBuffer(NewD3DDInterface, lpDesc);
		}
	}
	ReleaseCriticalSection();
	return Interface;
}

HRESULT m_IDirect3DExecuteBuffer::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ") " << riid;

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		if (ppvObj)
		{
			*ppvObj = nullptr;
		}
		return E_NOINTERFACE;
	}

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

	if (riid == IID_IDirect3DExecuteBuffer || riid == IID_IUnknown)
	{
		AddRef();

		*ppvObj = this;

		return D3D_OK;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID);
}

ULONG m_IDirect3DExecuteBuffer::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return 0;
	}

	if (!ProxyInterface)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DExecuteBuffer::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return 0;
	}

	LONG ref;

	if (!ProxyInterface)
	{
		ref = InterlockedDecrement(&RefCount);
	}
	else
	{
		ref = ProxyInterface->Release();
	}

	if (ref == 0)
	{
		SaveInterfaceAddress(this, WrapperInterfaceBackup);

		ReleaseInterface();
	}

	return ref;
}

HRESULT m_IDirect3DExecuteBuffer::Initialize(LPDIRECT3DDEVICE lpDirect3DDevice, LPD3DEXECUTEBUFFERDESC lpDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		// The method returns DDERR_ALREADYINITIALIZED because the Direct3DExecuteBuffer object is initialized when it is created.
		return DDERR_ALREADYINITIALIZED;
	}

	if (lpDirect3DDevice)
	{
		lpDirect3DDevice->QueryInterface(IID_GetRealInterface, (LPVOID*)&lpDirect3DDevice);
	}

	return ProxyInterface->Initialize(lpDirect3DDevice, lpDesc);
}

HRESULT m_IDirect3DExecuteBuffer::Lock(LPD3DEXECUTEBUFFERDESC lpDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->Lock(lpDesc);
}

HRESULT m_IDirect3DExecuteBuffer::Unlock()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->Unlock();
}

HRESULT m_IDirect3DExecuteBuffer::SetExecuteData(LPD3DEXECUTEDATA lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->SetExecuteData(lpData);
}

HRESULT m_IDirect3DExecuteBuffer::GetExecuteData(LPD3DEXECUTEDATA lpData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->GetExecuteData(lpData);
}

HRESULT m_IDirect3DExecuteBuffer::Validate(LPDWORD lpdwOffset, LPD3DVALIDATECALLBACK lpFunc, LPVOID lpUserArg, DWORD dwReserved)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: Not Implemented");
		return DDERR_UNSUPPORTED;
	}

	return ProxyInterface->Validate(lpdwOffset, lpFunc, lpUserArg, dwReserved);
}

HRESULT m_IDirect3DExecuteBuffer::Optimize(DWORD dwDummy)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!ProxyInterface && !D3DDeviceInterface)
	{
		return DDERR_INVALIDOBJECT;
	}

	if (!ProxyInterface)
	{
		// The Optimize function doesn't exist in Direct3D9 because it manages vertex buffer optimizations internally
		return D3D_OK;
	}

	return ProxyInterface->Optimize(dwDummy);
}

/************************/
/*** Helper functions ***/
/************************/

void m_IDirect3DExecuteBuffer::InitInterface(LPD3DEXECUTEBUFFERDESC lpDesc)
{
	if (lpDesc && lpDesc->dwSize == sizeof(D3DEXECUTEBUFFERDESC))
	{
		Desc = *lpDesc;
	}
}

void m_IDirect3DExecuteBuffer::ReleaseInterface()
{
	if (D3DDeviceInterface)
	{
		D3DDeviceInterface->ReleaseExecuteBuffer(this);
	}
}
