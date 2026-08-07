#define INITGUID

#define DIRECTINPUT_VERSION 0x0800
#include <ddraw.h>
#include <ddrawex.h>
#include <dinput.h>
#include <dsound.h>

#include "ddraw-testing.h"
#include "testing-harness.h"

TestEntry tests[] =
{
	{ CLSID_DirectDraw,          "CLSID_DirectDraw",          IID_IDirectDraw },
	{ CLSID_DirectDraw7,         "CLSID_DirectDraw7",         IID_IDirectDraw7 },
	{ CLSID_DirectDrawClipper,   "CLSID_DirectDrawClipper",   IID_IDirectDrawClipper },
	{ CLSID_DirectDrawFactory,   "CLSID_DirectDrawFactory",   IID_IDirectDrawFactory },

	{ CLSID_DirectInput,         "CLSID_DirectInput",         IID_IDirectInput },
	{ CLSID_DirectInputDevice,   "CLSID_DirectInputDevice",   IID_IDirectInputDevice },
	{ CLSID_DirectInput8,        "CLSID_DirectInput8",        IID_IDirectInput8 },
	{ CLSID_DirectInputDevice8,  "CLSID_DirectInputDevice8",  IID_IDirectInputDevice8 },

	{ CLSID_DirectSound,         "CLSID_DirectSound",         IID_IDirectSound },
	{ CLSID_DirectSound8,        "CLSID_DirectSound8",        IID_IDirectSound8 },
};

const char* GetTestIIDName(REFIID riid)
{
	if (IsEqualIID(riid, IID_IDirectDraw))
		return "IID_IDirectDraw";

	if (IsEqualIID(riid, IID_IDirectDraw7))
		return "IID_IDirectDraw7";

	if (IsEqualIID(riid, IID_IDirectDrawClipper))
		return "IID_IDirectDrawClipper";

	if (IsEqualIID(riid, IID_IDirectDrawFactory))
		return "IID_IDirectDrawFactory";

	if (IsEqualIID(riid, IID_IDirectInput))
		return "IID_IDirectInput";

	if (IsEqualIID(riid, IID_IDirectInputDevice))
		return "IID_IDirectInputDevice";

	if (IsEqualIID(riid, IID_IDirectInput8))
		return "IID_IDirectInput8";

	if (IsEqualIID(riid, IID_IDirectInputDevice8))
		return "IID_IDirectInputDevice8";

	if (IsEqualIID(riid, IID_IDirectSound))
		return "IID_IDirectSound";

	if (IsEqualIID(riid, IID_IDirectSound8))
		return "IID_IDirectSound8";

	return "Unknown IID";
}
