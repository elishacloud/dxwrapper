#pragma once

struct ENUMDEVICES
{
	LPVOID lpContext = nullptr;
	LPD3DENUMDEVICESCALLBACK lpCallback = nullptr;
};

class m_IDirect3DEnumDevices
{
public:
	m_IDirect3DEnumDevices() {}
	~m_IDirect3DEnumDevices() {}

	static HRESULT CALLBACK ConvertCallback(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpDeviceDesc, LPVOID lpContext);
};

struct ENUMFINDDEVICES
{
	bool Found = false;
	GUID guid;
	D3DDEVICEDESC7 DeviceDesc7;
};

class m_IDirect3DEnumFindDevices
{
public:
	m_IDirect3DEnumFindDevices() {}
	~m_IDirect3DEnumFindDevices() {}

	static HRESULT CALLBACK ConvertCallback(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpDeviceDesc7, LPVOID lpContext);
};

struct ENUMPIXELFORMAT
{
	LPVOID lpContext = nullptr;
	LPD3DENUMPIXELFORMATSCALLBACK lpCallback = nullptr;
	LPD3DENUMTEXTUREFORMATSCALLBACK lpTextureCallback = nullptr;
};

class m_IDirect3DEnumPixelFormat
{
public:
	m_IDirect3DEnumPixelFormat() {}
	~m_IDirect3DEnumPixelFormat() {}

	static HRESULT CALLBACK ConvertCallback(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);
};
