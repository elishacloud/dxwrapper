#pragma once

typedef struct IDirectSound *LPDIRECTSOUND;
typedef struct IDirectSound8 *LPDIRECTSOUND8;

BOOL APIENTRY DllMain_DSoundCtrl(HMODULE, DWORD, LPVOID);
HRESULT STDMETHODCALLTYPE _DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
HRESULT STDMETHODCALLTYPE _DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
STDAPI STDMETHODCALLTYPE _DllGetClassObject_DSoundCtrl(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv);

namespace DSoundCtrl
{
	constexpr FARPROC DirectSoundCreate = (FARPROC)*_DirectSoundCreate;
	constexpr FARPROC DirectSoundCreate8 = (FARPROC)*_DirectSoundCreate8;
	constexpr FARPROC DllGetClassObject = (FARPROC)*_DllGetClassObject_DSoundCtrl;
}
