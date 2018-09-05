#pragma once

struct ENUMDISPLAYMODES
{
	LPVOID lpContext = nullptr;
	LPDDENUMMODESCALLBACK lpCallback = nullptr;
};

class m_IDirectDrawEnumDisplayModes
{
public:
	m_IDirectDrawEnumDisplayModes() {}
	~m_IDirectDrawEnumDisplayModes() {}

	static HRESULT CALLBACK ConvertCallback(LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext);
};

struct ENUMSURFACE
{
	LPVOID lpContext = nullptr;
	LPDDENUMSURFACESCALLBACK lpCallback = nullptr;
	LPDDENUMSURFACESCALLBACK7 lpCallback7 = nullptr;
	DWORD DirectXVersion;
	bool ConvertSurfaceDescTo2;
};

class m_IDirectDrawEnumSurface
{
public:
	m_IDirectDrawEnumSurface() {}
	~m_IDirectDrawEnumSurface() {}

	static HRESULT CALLBACK ConvertCallback(LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext);
	static HRESULT CALLBACK ConvertCallback2(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext);
};
