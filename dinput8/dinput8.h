#pragma once

#define INITGUID

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class m_IDirectInput8;
class m_IDirectInputDevice8;
class m_IDirectInputEffect8;

#include "AddressLookupTable.h"
#include "External\dinputto8\ModuleObjectCount.h"
#include "Settings\Settings.h"
#include "Logging\Logging.h"

typedef HRESULT(WINAPI *DirectInput8CreateProc)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
typedef HRESULT(WINAPI *DllCanUnloadNowProc)();
typedef	HRESULT(WINAPI *DllGetClassObjectProc)(REFCLSID, REFIID, LPVOID *);
typedef HRESULT(WINAPI *DllRegisterServerProc)();
typedef HRESULT(WINAPI *DllUnregisterServerProc)();
typedef	LPCDIDATAFORMAT(WINAPI *GetdfDIJoystickProc)();

extern AddressLookupTableDinput8 ProxyAddressLookupTableDinput8;

#include "IDirectInput8.h"
#include "IDirectInputDevice8.h"
#include "IDirectInputEffect8.h"
#include "External\dinputto8\ClassFactory.h"
