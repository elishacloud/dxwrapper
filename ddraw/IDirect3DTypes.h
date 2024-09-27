#pragma once

#ifndef DX3DTYPE_H
#define DX3DTYPE_H
#endif

constexpr UINT MaxTextureStages = 8;	// Devices can have up to eight set textures.
constexpr UINT MAX_LIGHTS = 8;          // Devices can have up to eight lights.

#define D3DDP_FORCE_DWORD               0x0000001Fl
#define D3DDP_DXW_COLORKEYENABLE        0x00000020l
#define D3DDP_DXW_ALPHACOLORKEY         0x00000040l

#define D3DDEVICEDESC1_SIZE 172
#define D3DDEVICEDESC5_SIZE 204
#define D3DDEVICEDESC6_SIZE 252

#define D3DLIGHTCAPS_PARALLELPOINT      0x00000008L /* Parallel point lights supported */
#define D3DLIGHTCAPS_GLSPOT             0x00000010L /* GL syle spot lights supported */

#ifndef D3DTSS_ADDRESS
#define D3DTSS_ADDRESS 12
#endif

#ifndef D3DDEVINFOID_TEXTUREMANAGER
#define D3DDEVINFOID_TEXTUREMANAGER    1
#endif
#ifndef D3DDEVINFOID_D3DTEXTUREMANAGER
#define D3DDEVINFOID_D3DTEXTUREMANAGER 2
#endif
#ifndef D3DDEVINFOID_TEXTURING
#define D3DDEVINFOID_TEXTURING         3
#endif

typedef struct _D3DDEVINFO_TEXTUREMANAGER {
    BOOL    bThrashing;             // Thrashing status. TRUE if thrashing occurred during the last frame, or FALSE otherwise.
    DWORD   dwNumEvicts;            // Number of textures that were removed during the last frame.
    DWORD   dwNumVidCreates;        // Number of textures that were created in video memory during the last frame.
    DWORD   dwNumTexturesUsed;      // Total number of textures used during the last frame.
    DWORD   dwNumUsedTexInVid;      // Number of video memory textures that were used during the last frame.
    DWORD   dwWorkingSet;           // Number of textures currently resident in video memory.
    DWORD   dwWorkingSetBytes;      // Number of bytes currently allocated by textures resident in video memory.
    DWORD   dwTotalManaged;         // Total number of managed textures.
    DWORD   dwTotalBytes;           // Total number of bytes allocated for managed textures.
    DWORD   dwLastPri;              // Priority of last evicted texture.
} D3DDEVINFO_TEXTUREMANAGER, * LPD3DDEVINFO_TEXTUREMANAGER;

typedef struct _D3DDEVINFO_TEXTURING {
    DWORD   dwNumLoads;             // Number of times a texture has been loaded by calling the IDirect3DDevice7::Load method.
    DWORD   dwApproxBytesLoaded;    // Approximate number of bytes loaded by calls to the IDirect3DDevice7::Load method.
    DWORD   dwNumPreLoads;          // Number of times managed textures have been explicitly loaded by calling the IDirect3DDevice7::PreLoad method.
    DWORD   dwNumSet;               // Number of times textures have been set to texture-blending stages by calling the IDirect3DDevice7::SetTexture method.
    DWORD   dwNumCreates;           // Number of texture surfaces created by the application.
    DWORD   dwNumDestroys;          // Number of textures destroyed (released) by the application.
    DWORD   dwNumSetPriorities;     // Number of times texture-management priority has been set by calling the IDirectDrawSurface7::SetPriority method.
    DWORD   dwNumSetLODs;           // Number of times the maximum mipmap level of detail has been set by calling the IDirectDrawSurface7::SetLOD method.
    DWORD   dwNumLocks;             // Number of times a texture surface has been locked by calling the IDirectDrawSurface7::Lock method.
    DWORD   dwNumGetDCs;            // Number of times a device context for a texture surface has been retrieved by calling the IDirectDrawSurface7::GetDC method.
} D3DDEVINFO_TEXTURING, * LPD3DDEVINFO_TEXTURING;

#define LVERTEX_SIZE 32

#undef D3DFVF_RESERVED2
#define D3DFVF_RESERVED2        0xf000  // 4 reserved bits
#define D3DFVF_RESERVED2_9      0x6000  // 2 reserved bits
#undef D3DFVF_POSITION_MASK
#define D3DFVF_POSITION_MASK    0x00e
#define D3DFVF_POSITION_MASK_9  0x400E

#define D3DFVF_LVERTEX9 (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

typedef struct {
	FLOAT    x, y, z;
	D3DCOLOR diffuse, specular;
	FLOAT    tu, tv;
} D3DLVERTEX9, *LPD3DLVERTEX9;

typedef enum _D3DSURFACETYPE {
    D3DTYPE_NONE = 0,
    D3DTYPE_OFFPLAINSURFACE = 1,
    D3DTYPE_TEXTURE = 2,
    D3DTYPE_RENDERTARGET = 3,
    D3DTYPE_DEPTHSTENCIL = 4
} D3DSURFACETYPE;

void ConvertLight(D3DLIGHT7& Light7, D3DLIGHT& Light);
void ConvertMaterial(D3DMATERIAL& Material, D3DMATERIAL7& Material7);
void ConvertMaterial(D3DMATERIAL7 &Material7, D3DMATERIAL &Material);
void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT2 &ViewPort2);
void ConvertViewport(D3DVIEWPORT2 &ViewPort2, D3DVIEWPORT &ViewPort);
void ConvertViewport(D3DVIEWPORT &ViewPort, D3DVIEWPORT7 &ViewPort7);
void ConvertViewport(D3DVIEWPORT2 &ViewPort2, D3DVIEWPORT7 &ViewPort7);
void ConvertViewport(D3DVIEWPORT7 &ViewPort7, D3DVIEWPORT &ViewPort);
void ConvertViewport(D3DVIEWPORT7 &ViewPort7, D3DVIEWPORT2 &ViewPort2);
void ConvertViewport(D3DVIEWPORT7 &ViewPort, D3DVIEWPORT7 &ViewPort7);
void ConvertDeviceDesc(D3DDEVICEDESC &Desc, D3DDEVICEDESC7 &Desc7);
void ConvertDeviceDesc(D3DDEVICEDESC7 &Desc7, D3DCAPS9 &Caps9);
void ConvertVertices(D3DLVERTEX* lFVF, D3DLVERTEX9* lFVF9, DWORD NumVertices);
void ConvertVertices(D3DLVERTEX9* lFVF9, D3DLVERTEX* lFVF, DWORD NumVertices);
bool CheckTextureStageStateType(D3DTEXTURESTAGESTATETYPE dwState);
bool CheckRenderStateType(D3DRENDERSTATETYPE dwRenderStateType);
void ConvertVertex(BYTE* pDestVertex, DWORD DestFVF, const BYTE* pSrcVertex, DWORD SrcFVF);
DWORD ConvertVertexTypeToFVF(D3DVERTEXTYPE d3dVertexType);
UINT GetVertexStride(DWORD dwVertexTypeDesc);
UINT GetNumberOfPrimitives(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexCount);
