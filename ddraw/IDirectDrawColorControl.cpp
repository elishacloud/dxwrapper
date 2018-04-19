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
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, IID_IDirectDrawColorControl, this);
}

ULONG m_IDirectDrawColorControl::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawColorControl::Release()
{
	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

HRESULT m_IDirectDrawColorControl::GetColorControls(LPDDCOLORCONTROL lpColorControl)
{
	return ProxyInterface->GetColorControls(lpColorControl);
}

HRESULT m_IDirectDrawColorControl::SetColorControls(LPDDCOLORCONTROL lpColorControl)
{
	return ProxyInterface->SetColorControls(lpColorControl);
}
