#pragma once

#ifndef D3DTSS_ADDRESS
#define D3DTSS_ADDRESS 12
#endif

void ConvertMaterial(D3DMATERIAL &Material, D3DMATERIAL &Material2);
void ConvertMaterial(D3DMATERIAL7 &Material, D3DMATERIAL7 &Material2);
void ConvertMaterial(D3DMATERIAL &Material, D3DMATERIAL7 &Material7);
void ConvertMaterial(D3DMATERIAL7 &Material7, D3DMATERIAL &Material);
void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT &ViewPort2);
void ConvertViewport(D3DVIEWPORT2 &ViewPort, D3DVIEWPORT2 &ViewPort2);
void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT2 &ViewPort2);
void ConvertViewport(D3DVIEWPORT2 &ViewPort2, D3DVIEWPORT &ViewPort);
void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT7 &ViewPort7);
void ConvertViewport(D3DVIEWPORT2 &ViewPort2, D3DVIEWPORT7 &ViewPort7);
void ConvertViewport(D3DVIEWPORT7 &ViewPort7, D3DVIEWPORT &ViewPort);
void ConvertViewport(D3DVIEWPORT7 &ViewPort7, D3DVIEWPORT2 &ViewPort2);
void ConvertViewport(D3DVIEWPORT7 &ViewPort, D3DVIEWPORT7 &ViewPort7);
void ConvertViewport(D3DVIEWPORT7 &ViewPort7, D3DVIEWPORT9 &ViewPort9);
void ConvertViewport(D3DVIEWPORT9 &ViewPort9, D3DVIEWPORT7 &ViewPort7);
void ConvertCaps(D3DPRIMCAPS &PrimCaps, D3DPRIMCAPS &PrimCaps2);
void ConvertDeviceDesc(D3DDEVICEDESC &Desc, D3DDEVICEDESC7 &Desc7);
void ConvertDeviceDescSoft(D3DDEVICEDESC &Desc);
void ConvertDeviceDesc(D3DDEVICEDESC7 &Desc7, D3DCAPS9 &Caps9);
void GetBufferFormat(DDPIXELFORMAT &ddpfPixelFormat, DWORD Num);
bool CheckTextureStageStateType(D3DTEXTURESTAGESTATETYPE dwState);
bool CheckRenderStateType(D3DRENDERSTATETYPE dwRenderStateType);
