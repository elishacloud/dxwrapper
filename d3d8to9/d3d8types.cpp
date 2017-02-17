/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 *
 * Updated 2017 by Elisha Riedlinger
 */

#include "d3d8types.hpp"
#include <assert.h>

static UINT calc_texture_size(UINT width, UINT height, UINT depth, D3DFORMAT format)
{
	switch (static_cast<DWORD>(format))
	{
		default:
		case D3DFMT_UNKNOWN:
			return 0;
		case D3DFMT_R3G3B2:
		case D3DFMT_A8:
		case D3DFMT_P8:
		case D3DFMT_L8:
		case D3DFMT_A4L4:
			return width * height * depth;
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
			return width * 2 * height * depth;
		case D3DFMT_R8G8B8:
			return width * 3 * height * depth;
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
			return width * 4 * height * depth;
		case D3DFMT_DXT1:
			assert(depth <= 1);
			return ((width + 3) >> 2) * ((height + 3) >> 2) * 8;
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
			assert(depth <= 1);
			return ((width + 3) >> 2) * ((height + 3) >> 2) * 16;
	}
}

void convert_caps(D3DCAPS9 &input, D3DCAPS8 &output)
{
	CopyMemory(&output, &input, sizeof(output));

	output.Caps2 |= D3DCAPS2_CANRENDERWINDOWED;								// Tell application that windows mode is supported
	output.RasterCaps |= D3DPRASTERCAPS_ZBIAS;								// Tell application that z-bias is supported
	output.StencilCaps &= ~D3DSTENCILCAPS_TWOSIDED;							// Remove unsupported StencilCaps type
	output.PixelShaderVersion = D3DPS_VERSION(1, 4);						// Set defaul PixelShaderVersion to 1.4 for D3D8 compatibility
	output.VertexShaderVersion = D3DVS_VERSION(1, 1);						// Set defaul VertexShaderVersion to 1.1 for D3D8 compatibility
	output.MaxVertexShaderConst = min(256, input.MaxVertexShaderConst);		// D3D8 can only handle upto 256 for MaxVertexShaderConst
}
void convert_volume_desc(D3DVOLUME_DESC &input, D3DVOLUME_DESC8 &output)
{
	output.Format = input.Format;
	output.Type = input.Type;
	output.Usage = input.Usage;
	output.Pool = input.Pool;
	output.Size = calc_texture_size(input.Width, input.Height, input.Depth, input.Format);
	output.Width = input.Width;
	output.Height = input.Height;
	output.Depth = input.Depth;
}
void convert_surface_desc(D3DSURFACE_DESC &input, D3DSURFACE_DESC8 &output)
{
	output.Format = input.Format;
	output.Type = input.Type;
	output.Usage = input.Usage;
	output.Pool = input.Pool;
	output.Size = calc_texture_size(input.Width, input.Height, 1, input.Format);
	output.Width = input.Width;
	output.Height = input.Height;

	// Check for D3DMULTISAMPLE_NONMASKABLE and change it to D3DMULTISAMPLE_NONE for best D3D8 compatibility.
	if (input.MultiSampleType == D3DMULTISAMPLE_NONMASKABLE)
	{
		output.MultiSampleType = D3DMULTISAMPLE_NONE;
	}
	else
	{
		output.MultiSampleType = input.MultiSampleType;
	}
}
void convert_present_parameters(D3DPRESENT_PARAMETERS8 &input, D3DPRESENT_PARAMETERS &output)
{
	output.BackBufferWidth = input.BackBufferWidth;
	output.BackBufferHeight = input.BackBufferHeight;
	output.BackBufferFormat = input.BackBufferFormat;
	output.BackBufferCount = input.BackBufferCount;
	output.MultiSampleQuality = 0;
	output.hDeviceWindow = input.hDeviceWindow;
	output.Windowed = input.Windowed;
	output.EnableAutoDepthStencil = input.EnableAutoDepthStencil;
	output.AutoDepthStencilFormat = input.AutoDepthStencilFormat;
	output.Flags = input.Flags;
	output.FullScreen_RefreshRateInHz = input.FullScreen_RefreshRateInHz;

	// MultiSampleType must be D3DMULTISAMPLE_NONE unless SwapEffect has been set to D3DSWAPEFFECT_DISCARD.
	// https://msdn.microsoft.com/en-us/library/windows/desktop/bb172588(v=vs.85).aspx
	// Check for D3DMULTISAMPLE_NONMASKABLE and change it to D3DMULTISAMPLE_NONE for best D3D8 compatibility.
	if (input.SwapEffect != D3DSWAPEFFECT_DISCARD || input.MultiSampleType == D3DMULTISAMPLE_NONMASKABLE)
	{
		output.MultiSampleType = D3DMULTISAMPLE_NONE;
	}
	else
	{
		output.MultiSampleType = input.MultiSampleType;
	}

	// D3DPRESENT_RATE_UNLIMITED is no longer supported in D3D9
	// Update PresentationInterval when SwapEffect = D3DSWAPEFFECT_COPY_VSYNC and
	// application is not windowed for best D3D8 compatibility
	if (input.FullScreen_PresentationInterval == D3DPRESENT_RATE_UNLIMITED || (input.SwapEffect == D3DSWAPEFFECT_COPY_VSYNC && !input.Windowed))
	{
		output.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}
	else
	{
		output.PresentationInterval = input.FullScreen_PresentationInterval;
	}

	// D3DSWAPEFFECT_COPY_VSYNC is no longer supported in D3D9
	if (input.SwapEffect == D3DSWAPEFFECT_COPY_VSYNC)
	{
		output.SwapEffect = D3DSWAPEFFECT_COPY;
	}
	else
	{
		output.SwapEffect = input.SwapEffect;
	}
}
void convert_adapter_identifier(D3DADAPTER_IDENTIFIER9 &input, D3DADAPTER_IDENTIFIER8 &output)
{
	CopyMemory(output.Driver, input.Driver, MAX_DEVICE_IDENTIFIER_STRING);
	CopyMemory(output.Description, input.Description, MAX_DEVICE_IDENTIFIER_STRING);
	output.DriverVersion = input.DriverVersion;
	output.VendorId = input.VendorId;
	output.DeviceId = input.DeviceId;
	output.SubSysId = input.SubSysId;
	output.Revision = input.Revision;
	output.DeviceIdentifier = input.DeviceIdentifier;
	output.WHQLLevel = input.WHQLLevel;
}
