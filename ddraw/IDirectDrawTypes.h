#pragma once

void ConvertColorControl(DDCOLORCONTROL &ColorControl, DDCOLORCONTROL &ColorControl2);
void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC &Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc, DDSURFACEDESC2 &Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC &Desc, DDSURFACEDESC2 &Desc2);
void ConvertSurfaceDesc(DDSURFACEDESC2 &Desc2, DDSURFACEDESC &Desc);
void ConvertPixelFormat(DDPIXELFORMAT &Format, DDPIXELFORMAT &Format2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID, DDDEVICEIDENTIFIER2 &DeviceID2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER &DeviceID, DDDEVICEIDENTIFIER2 &DeviceID2);
void ConvertDeviceIdentifier(DDDEVICEIDENTIFIER2 &DeviceID2, DDDEVICEIDENTIFIER &DeviceID);
void ConvertCaps(DDSCAPS2 &Caps, DDSCAPS2 &Caps2);
bool ConvertCaps(DDCAPS_DX7 &Caps, DDCAPS_DX7 &Caps7);
void ConvertCaps(DDSCAPS &Caps, DDSCAPS2 &Caps2);
void ConvertCaps(DDSCAPS2 &Caps2, DDSCAPS &Caps);
void ConvertCaps(DDCAPS_DX1 &Caps1, DDCAPS_DX7 &Caps7);
void ConvertCaps(DDCAPS_DX3 &Caps3, DDCAPS_DX7 &Caps7);
void ConvertCaps(DDCAPS_DX5 &Caps5, DDCAPS_DX7 &Caps7);
