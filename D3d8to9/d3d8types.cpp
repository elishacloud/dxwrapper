/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "d3d8types.hpp"
#include <assert.h>

static UINT CalcTextureSize(UINT Width, UINT Height, UINT Depth, D3DFORMAT Format)
{
	switch (static_cast<DWORD>(Format))
	{
		default:
		case D3DFMT_UNKNOWN:
			return 0;
		case D3DFMT_R3G3B2:
		case D3DFMT_A8:
		case D3DFMT_P8:
		case D3DFMT_L8:
		case D3DFMT_A4L4:
			return Width * Height * Depth;
		case D3DFMT_R5G6B5:
		case D3DFMT_X1R5G5B5:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_A4R4G4B4:
		case D3DFMT_A8R3G3B2:
		case D3DFMT_X4R4G4B4:
		case D3DFMT_A8P8:
		case D3DFMT_A8L8:
		case D3DFMT_V8U8:
		case D3DFMT_L6V5U5:
		case D3DFMT_D16_LOCKABLE:
		case D3DFMT_D15S1:
		case D3DFMT_D16:
		case D3DFMT_UYVY:
		case D3DFMT_YUY2:
			return Width * 2 * Height * Depth;
		case D3DFMT_R8G8B8:
			return Width * 3 * Height * Depth;
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A2B10G10R10:
		case D3DFMT_A8B8G8R8:
		case D3DFMT_X8B8G8R8:
		case D3DFMT_G16R16:
		case D3DFMT_X8L8V8U8:
		case D3DFMT_Q8W8V8U8:
		case D3DFMT_V16U16:
		case D3DFMT_W11V11U10:
		case D3DFMT_A2W10V10U10:
		case D3DFMT_D32:
		case D3DFMT_D24S8:
		case D3DFMT_D24X8:
		case D3DFMT_D24X4S4:
			return Width * 4 * Height * Depth;
		case D3DFMT_DXT1:
			assert(Depth <= 1);
			return ((Width + 3) >> 2) * ((Height + 3) >> 2) * 8;
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
			assert(Depth <= 1);
			return ((Width + 3) >> 2) * ((Height + 3) >> 2) * 16;
	}
}

void ConvertCaps(D3DCAPS9 &Input, D3DCAPS8 &Output)
{
	CopyMemory(&Output, &Input, sizeof(Output));

	// Tell application that window mode is supported
	Output.Caps2 |= D3DCAPS2_CANRENDERWINDOWED;
	// Tell application that z-bias is supported
	Output.RasterCaps |= D3DPRASTERCAPS_ZBIAS;
	// Remove unsupported stencil capability
	Output.StencilCaps &= ~D3DSTENCILCAPS_TWOSIDED;
	// Set default pixel shader version to 1.4 for D3D8 compatibility
	Output.PixelShaderVersion = D3DPS_VERSION(1, 4);
	// Set default vertex shader version to 1.1 for D3D8 compatibility
	Output.VertexShaderVersion = D3DVS_VERSION(1, 1);
	// D3D8 can only handle up to 256 for MaxVertexShaderConst
	Output.MaxVertexShaderConst = min(256, Input.MaxVertexShaderConst);
}

void ConvertVolumeDesc(D3DVOLUME_DESC &Input, D3DVOLUME_DESC8 &Output)
{
	Output.Format = Input.Format;
	Output.Type = Input.Type;
	Output.Usage = Input.Usage;
	Output.Pool = Input.Pool;
	Output.Size = CalcTextureSize(Input.Width, Input.Height, Input.Depth, Input.Format);
	Output.Width = Input.Width;
	Output.Height = Input.Height;
	Output.Depth = Input.Depth;
}
void ConvertSurfaceDesc(D3DSURFACE_DESC &Input, D3DSURFACE_DESC8 &Output)
{
	Output.Format = Input.Format;
	Output.Type = Input.Type;
	Output.Usage = Input.Usage;
	Output.Pool = Input.Pool;
	Output.Size = CalcTextureSize(Input.Width, Input.Height, 1, Input.Format);
	Output.MultiSampleType = Input.MultiSampleType;
	Output.Width = Input.Width;
	Output.Height = Input.Height;

	// Check for D3DMULTISAMPLE_NONMASKABLE and change it to D3DMULTISAMPLE_NONE for best D3D8 compatibility.
	if (Output.MultiSampleType == D3DMULTISAMPLE_NONMASKABLE)
	{
		Output.MultiSampleType = D3DMULTISAMPLE_NONE;
	}
}

void ConvertPresentParameters(D3DPRESENT_PARAMETERS8 &Input, D3DPRESENT_PARAMETERS &Output)
{
	Output.BackBufferWidth = Input.BackBufferWidth;
	Output.BackBufferHeight = Input.BackBufferHeight;
	Output.BackBufferFormat = Input.BackBufferFormat;
	Output.BackBufferCount = Input.BackBufferCount;
	Output.MultiSampleType = Input.MultiSampleType;
	Output.MultiSampleQuality = 0;
	Output.SwapEffect = Input.SwapEffect;
	Output.hDeviceWindow = Input.hDeviceWindow;
	Output.Windowed = Input.Windowed;
	Output.EnableAutoDepthStencil = Input.EnableAutoDepthStencil;
	Output.AutoDepthStencilFormat = Input.AutoDepthStencilFormat;
	Output.Flags = Input.Flags;
	Output.FullScreen_RefreshRateInHz = Input.FullScreen_RefreshRateInHz;
	Output.PresentationInterval = Input.FullScreen_PresentationInterval;

	// MultiSampleType must be D3DMULTISAMPLE_NONE unless SwapEffect has been set to D3DSWAPEFFECT_DISCARD
	if (Output.SwapEffect != D3DSWAPEFFECT_DISCARD)
	{
		Output.MultiSampleType = D3DMULTISAMPLE_NONE;
	}

	// Remove Flags that are not compatible with multisampling
	if (Output.MultiSampleType != D3DMULTISAMPLE_NONE)
	{
		Output.Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	}

	// D3DPRESENT_RATE_UNLIMITED is no longer supported in D3D9
	if (Output.PresentationInterval == D3DPRESENT_RATE_UNLIMITED)
	{
		Output.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	// D3DSWAPEFFECT_COPY_VSYNC is no longer supported in D3D9
	if (Output.SwapEffect == D3DSWAPEFFECT_COPY_VSYNC)
	{
		Output.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		Output.SwapEffect = D3DSWAPEFFECT_COPY;
	}
}
void ConvertAdapterIdentifier(D3DADAPTER_IDENTIFIER9 &Input, D3DADAPTER_IDENTIFIER8 &Output)
{
	CopyMemory(Output.Driver, Input.Driver, MAX_DEVICE_IDENTIFIER_STRING);
	CopyMemory(Output.Description, Input.Description, MAX_DEVICE_IDENTIFIER_STRING);
	Output.DriverVersion = Input.DriverVersion;
	Output.VendorId = Input.VendorId;
	Output.DeviceId = Input.DeviceId;
	Output.SubSysId = Input.SubSysId;
	Output.Revision = Input.Revision;
	Output.DeviceIdentifier = Input.DeviceIdentifier;
	Output.WHQLLevel = Input.WHQLLevel;
}
bool SupportsPalettes()
{
	HDC hDC = GetDC(nullptr);
	bool hasPalette = (GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE) != 0;
	ReleaseDC(nullptr, hDC);
	return hasPalette;
}
