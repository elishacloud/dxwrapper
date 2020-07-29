#pragma once

#include <ddraw.h>

static constexpr DWORD FourCCTypes[]
{
	//0x026CFB68, // MAKEFOURCC('N', 'V', '1', '2')
	//0x026CFB6C, // MAKEFOURCC('P', '0', '1', '0')
	//0x026CFB70, // MAKEFOURCC('N', 'V', '2', '4')
	//0x026CFB74, // MAKEFOURCC('Y', 'V', '1', '2')
	0x026CFB78, // MAKEFOURCC('U', 'Y', 'V', 'Y')
	0x026CFB7C, // MAKEFOURCC('Y', 'U', 'Y', '2')
	//0x026CFB80, // MAKEFOURCC('A', 'I', '4', '4')
	//0x026CFB84, // MAKEFOURCC('A', 'Y', 'U', 'V')
	//0x026CFB88, // MAKEFOURCC('A', 'I', 'P', '8')
	//0x026CFB8C, // MAKEFOURCC('A', 'V', '1', '2')
	//0x026CFB90, // MAKEFOURCC('P', 'L', 'F', 'F')
	//0x026CFB94, // MAKEFOURCC('N', 'V', 'M', 'D')
	//0x026CFB98, // MAKEFOURCC('N', 'V', 'D', 'P')
	//0x026CFB9C, // MAKEFOURCC('N', 'V', 'D', 'B')
	//0x026CFBA0, // MAKEFOURCC('S', 'S', 'A', 'A')
	//0x026CFBA4, // MAKEFOURCC('A', 'T', 'O', 'C')
	//0x026CFBA8, // MAKEFOURCC('3', 'x', '1', '1')
	//0x026CFBAC, // MAKEFOURCC('3', 'x', '1', '6')
};
static constexpr int NumFourCCs = (sizeof(FourCCTypes) / sizeof(*FourCCTypes));

void ConvertColorControl(DDCOLORCONTROL &ColorControl, DDCOLORCONTROL &ColorControl2);
void ConvertGammaRamp(DDGAMMARAMP &RampData, DDGAMMARAMP &RampData2);
void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC &Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc, DDSURFACEDESC2 &Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC2 &Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc2, DDSURFACEDESC &Desc);
void ConvertPixelFormat(DDPIXELFORMAT &Format, DDPIXELFORMAT &Format2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID, DDDEVICEIDENTIFIER2 &DeviceID2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER &DeviceID, DDDEVICEIDENTIFIER2 &DeviceID2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID2, DDDEVICEIDENTIFIER &DeviceID);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID, D3DADAPTER_IDENTIFIER9 &Identifier9);
void ConvertCaps(DDSCAPS2 &Caps, DDSCAPS2 &Caps2);
void ConvertCaps(DDSCAPS &Caps, DDSCAPS2 &Caps2);
void ConvertCaps(DDSCAPS2 &Caps2, DDSCAPS &Caps);
void ConvertCaps(DDCAPS &Caps, DDCAPS &Caps2);
void ConvertCaps(DDCAPS &Caps7, D3DCAPS9 &Caps9);
DWORD GetBitCount(DDPIXELFORMAT ddpfPixelFormat);
DWORD GetBitCount(D3DFORMAT Format);
D3DFORMAT GetDisplayFormat(DDPIXELFORMAT ddpfPixelFormat);
void SetPixelDisplayFormat(D3DFORMAT Format, DDPIXELFORMAT &lpPixelFormat);
HRESULT SetDisplayFormat(DWORD BPP, DDPIXELFORMAT &ddpfPixelFormat);
DWORD ComputePitch(DWORD Width, DWORD BitCount);
