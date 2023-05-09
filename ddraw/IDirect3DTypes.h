#pragma once

#define D3DDEVICEDESC1_SIZE 172
#define D3DDEVICEDESC5_SIZE 204
#define D3DDEVICEDESC6_SIZE 252

#define D3DLIGHTCAPS_PARALLELPOINT      0x00000008L /* Parallel point lights supported */
#define D3DLIGHTCAPS_GLSPOT             0x00000010L /* GL syle spot lights supported */

#ifndef D3DTSS_ADDRESS
#define D3DTSS_ADDRESS 12
#endif

#undef D3DFVF_RESERVED2
#define D3DFVF_RESERVED2        0xf000  // 4 reserved bits
#define D3DFVF_RESERVED2_9      0x6000  // 2 reserved bits
#undef D3DFVF_POSITION_MASK
#define D3DFVF_POSITION_MASK    0x00e
#define D3DFVF_POSITION_MASK_9  0x400E

#define D3DFVF_LVERTEX9 (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

typedef struct _D3DLVERTEX9 {
	FLOAT    x, y, z;
	D3DCOLOR diffuse, specular;
	FLOAT    tu, tv;
} D3DLVERTEX9, *LPD3DLVERTEX9;

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
void ConvertCaps(D3DPRIMCAPS &PrimCaps, D3DPRIMCAPS &PrimCaps2);
void ConvertDeviceDesc(D3DDEVICEDESC &Desc, D3DDEVICEDESC7 &Desc7);
void ConvertDeviceDescSoft(D3DDEVICEDESC &Desc);
void ConvertDeviceDesc(D3DDEVICEDESC7 &Desc7, D3DCAPS9 &Caps9);
bool CheckTextureStageStateType(D3DTEXTURESTAGESTATETYPE dwState);
bool CheckRenderStateType(D3DRENDERSTATETYPE dwRenderStateType);
