#pragma once

#include <ddraw.h>

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
void ConvertCaps(DDCAPS &Caps, DDCAPS &Caps7);
void ConvertCaps(DDCAPS &Caps7, D3DCAPS9 &Caps9);
DWORD GetBitCount(DDPIXELFORMAT ddpfPixelFormat);
DWORD GetBitCount(D3DFORMAT Format);
D3DFORMAT GetDisplayFormat(DDPIXELFORMAT ddpfPixelFormat);
void SetPixelDisplayFormat(D3DFORMAT Format, DDPIXELFORMAT &lpPixelFormat);
