/**
* Copyright (C) 2018 Elisha Riedlinger
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

HRESULT m_IDirectDrawColorControl::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	if (ProxyInterface == nullptr)
	{
		if ((riid == IID_IDirectDrawColorControl || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}

		return E_NOINTERFACE;
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, this);
}

ULONG m_IDirectDrawColorControl::AddRef()
{
	if (ProxyInterface == nullptr)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawColorControl::Release()
{
	if (ProxyInterface == nullptr)
	{
		LONG ref = InterlockedDecrement(&RefCount);

		if (ref == 0)
		{
			delete this;
			return 0;
		}

		return ref;
	}

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

HRESULT m_IDirectDrawColorControl::GetColorControls(LPDDCOLORCONTROL lpColorControl)
{
	if (ProxyInterface == nullptr)
	{
		if (!lpColorControl)
		{
			return DDERR_INVALIDPARAMS;
		}

		ConvertColorControl(*lpColorControl, ColorControl);

		return DD_OK;
	}

	return ProxyInterface->GetColorControls(lpColorControl);
}

HRESULT m_IDirectDrawColorControl::SetColorControls(LPDDCOLORCONTROL lpColorControl)
{
	if (ProxyInterface == nullptr)
	{
		if (!lpColorControl)
		{
			return DDERR_INVALIDPARAMS;
		}

		ConvertColorControl(ColorControl, *lpColorControl);

		return DD_OK;
	}

	return ProxyInterface->SetColorControls(lpColorControl);
}
