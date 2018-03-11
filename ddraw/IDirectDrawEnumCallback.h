#pragma once

// IDirectDraw EnumDisplayModes
class m_IDirectDrawEnumDisplayModes
{
private:
	static LPDDENUMMODESCALLBACK2 lpCallback;
	static DWORD DirectXVersion;
	static DWORD ProxyDirectXVersion;

public:
	m_IDirectDrawEnumDisplayModes() {}
	~m_IDirectDrawEnumDisplayModes() {}

	static void SetCallback(LPDDENUMMODESCALLBACK2, DWORD Version, DWORD ProxyVersion);
	static void ReleaseCallback();
	static HRESULT CALLBACK ConvertCallback(LPDDSURFACEDESC2 lpDDSurfaceDesc2, LPVOID lpContext);
};

// IDirectDraw EnumSurface
class m_IDirectDrawEnumSurface
{
private:
	static LPDDENUMSURFACESCALLBACK7 lpCallback;
	static DWORD DirectXVersion;
	static DWORD ProxyDirectXVersion;

public:
	m_IDirectDrawEnumSurface() {}
	~m_IDirectDrawEnumSurface() {}

	static void SetCallback(LPDDENUMSURFACESCALLBACK7, DWORD Version, DWORD ProxyVersion);
	static void ReleaseCallback();
	static HRESULT CALLBACK ConvertCallback(LPDIRECTDRAWSURFACE7 lpDDSurface, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext);
};
