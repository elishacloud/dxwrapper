#pragma once

#define CINTERFACE

#include <d3d.h>

namespace Direct3d
{
	struct Types1
	{
		typedef IDirect3D TDirect3d;
		typedef IDirect3D3 TDirect3dHighest;
		typedef IDirect3DDevice TDirect3dDevice;
		typedef D3DDEVICEDESC TD3dDeviceDesc;
		typedef LPD3DENUMDEVICESCALLBACK TD3dEnumDevicesCallback;
	};

	struct Types2
	{
		typedef IDirect3D2 TDirect3d;
		typedef IDirect3D3 TDirect3dHighest;
		typedef IDirect3DDevice2 TDirect3dDevice;
		typedef D3DDEVICEDESC TD3dDeviceDesc;
		typedef LPD3DENUMDEVICESCALLBACK TD3dEnumDevicesCallback;
	};

	struct Types3
	{
		typedef IDirect3D3 TDirect3d;
		typedef IDirect3D3 TDirect3dHighest;
		typedef IDirect3DDevice3 TDirect3dDevice;
		typedef D3DDEVICEDESC TD3dDeviceDesc;
		typedef LPD3DENUMDEVICESCALLBACK TD3dEnumDevicesCallback;
	};

	struct Types7
	{
		typedef IDirect3D7 TDirect3d;
		typedef IDirect3D7 TDirect3dHighest;
		typedef IDirect3DDevice7 TDirect3dDevice;
		typedef D3DDEVICEDESC7 TD3dDeviceDesc;
		typedef LPD3DENUMDEVICESCALLBACK7 TD3dEnumDevicesCallback;
	};

	template <typename Interface>
	struct Types;

	template <> struct Types<IDirect3D> : Types1 {};
	template <> struct Types<IDirect3D2> : Types2 {};
	template <> struct Types<IDirect3D3> : Types3 {};
	template <> struct Types<IDirect3D7> : Types7 {};

	template <> struct Types<IDirect3DDevice> : Types1 {};
	template <> struct Types<IDirect3DDevice2> : Types2 {};
	template <> struct Types<IDirect3DDevice3> : Types3 {};
	template <> struct Types<IDirect3DDevice7> : Types7 {};
}
