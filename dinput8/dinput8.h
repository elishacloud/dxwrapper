#pragma once

#define INITGUID

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <objbase.h>
#include <strsafe.h>
#include "dinput8.h"
#include "hid.h"

class m_IDirectInput8A;
class m_IDirectInput8W;
class m_IDirectInputDevice8A;
class m_IDirectInputDevice8W;
class m_IDirectInputEffect8;
class CDirectInput8Globals;

#include "AddressLookupTable.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"
#include "Dllmain\Dllmain.h"

typedef HRESULT(WINAPI *DirectInput8CreateProc)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
typedef HRESULT(WINAPI *DllCanUnloadNowProc)();
typedef	HRESULT(WINAPI *DllGetClassObjectProc)(REFCLSID, REFIID, LPVOID *);
typedef HRESULT(WINAPI *DllRegisterServerProc)();
typedef HRESULT(WINAPI *DllUnregisterServerProc)();
typedef	LPCDIDATAFORMAT(WINAPI *GetdfDIJoystickProc)();

namespace Dinput8Wrapper
{
	void WINAPI genericQueryInterface(REFIID CalledID, LPVOID * ppvObj);
}

extern AddressLookupTableDinput8<void> ProxyAddressLookupTableDinput8;
extern CDirectInput8Globals* diGlobalsInstance;

using namespace Dinput8Wrapper;

#include "dinput8type.h"

#include "CDirectInput8Globals.h"
#include "CDirectInputDeviceMouse8.h"
#include "CDirectInputDeviceMouse8A.h"
#include "CDirectInputDeviceMouse8W.h"

#include "IDirectInput8A.h"
#include "IDirectInput8W.h"
#include "IDirectInputDevice8A.h"
#include "IDirectInputDevice8W.h"
#include "IDirectInputEffect.h"
