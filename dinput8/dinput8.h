#pragma once

#define INITGUID

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <objbase.h>
#include <strsafe.h>
#include "dinput8.h"
#include "hid.h"

class m_IDirectInput8;
class m_IDirectInputDevice8;
class m_IDirectInputEffect8;
class CDirectInput8Globals;
class CDirectInputDeviceMouse8;

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

#include "CDirectInput8Globals.h"
#include "CDirectInputDeviceMouse8.h"

#include "IDirectInput8.h"
#include "IDirectInputDevice8.h"
#include "IDirectInputEffect8.h"
