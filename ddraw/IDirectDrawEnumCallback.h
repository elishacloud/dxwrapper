#pragma once

struct ENUMDISPLAYMODES
{
	LPVOID lpContext;
	LPDDENUMMODESCALLBACK lpCallback;
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
	LPVOID lpContext;
	LPDDENUMSURFACESCALLBACK7 lpCallback;
	DWORD DirectXVersion;
	bool ConvertSurfaceDescTo2;
};

class m_IDirectDrawEnumSurface
{
public:
	m_IDirectDrawEnumSurface() {}
	~m_IDirectDrawEnumSurface() {}

	static HRESULT CALLBACK ConvertCallback(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext);
};
