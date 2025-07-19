#pragma once

#ifndef DX3DTYPE_H
#define DX3DTYPE_H
#endif

constexpr UINT MaxDeviceStates = 256;	    // Devices can have up to 256 states.
constexpr UINT MaxTextureStageStates = 33;  // Devices have up to 33 types.
constexpr UINT MaxSamplerStates = 14;	    // Devices can have up to 14 sampler states.
constexpr UINT MaxTextureStages = 8;	    // Devices can have up to eight set textures.
constexpr UINT MaxClipPlaneIndex = 6;       // Devices can have up to six clip planes.

#define D3DSTATE D3DSTATE7

#define D3DDP_FORCE_DWORD               0x0000001Fl
#define D3DDP_DXW_COLORKEYENABLE        0x00000020l
#define D3DDP_DXW_ALPHACOLORKEY         0x00000040l

#define D3DDEVICEDESC1_SIZE 172
#define D3DDEVICEDESC5_SIZE 204
#define D3DDEVICEDESC6_SIZE 252

#define MAX_EXECUTE_BUFFER_SIZE (256 * 1024) // 256 KB

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

typedef enum _DX_D3DDEVTYPE { D3DDEVTYPE_TNLHAL = (D3DDEVTYPE)(D3DDEVTYPE_HAL + 0x10) } DX_D3DDEVTYPE;

typedef struct _D3DSTATE7 {
    union {
        D3DTRANSFORMSTATETYPE   dtstTransformStateType;
        D3DLIGHTSTATETYPE   dlstLightStateType;
        D3DRENDERSTATETYPE  drstRenderStateType;
    };
    union {
        DWORD           dwArg[1];
        D3DVALUE        dvArg[1];
    };
} D3DSTATE7, * LPD3DSTATE7;

typedef struct _DXLIGHT7 {
    /* D3DLIGHT7 Begin */
    D3DLIGHTTYPE    dltType;
    D3DCOLORVALUE   dcvDiffuse;
    D3DCOLORVALUE   dcvSpecular;
    D3DCOLORVALUE   dcvAmbient;
    D3DVECTOR       dvPosition;
    D3DVECTOR       dvDirection;
    D3DVALUE        dvRange;
    D3DVALUE        dvFalloff;
    D3DVALUE        dvAttenuation0;
    D3DVALUE        dvAttenuation1;
    D3DVALUE        dvAttenuation2;
    D3DVALUE        dvTheta;
    D3DVALUE        dvPhi;
    /* D3DLIGHT7 End */
    DWORD           dwFlags;        // D3DLIGHT2 flags
    DWORD           dwLightVersion; // D3DLIGHT version (1, 2 or 7)
} DXLIGHT7;

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
#define D3DFVF_POSITION_MASK    0x00E
#define D3DFVF_POSITION_MASK_9  0x400E

#define D3DFVF_LVERTEX9 (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

#ifndef D3DFVF_TEXCOUNT
#define D3DFVF_TEXCOUNT(fvf) (((fvf) >> D3DFVF_TEXCOUNT_SHIFT) & 0xF)
#endif

#define D3DFVF_TEXCOORDSIZE_ALL (0xFFFF0000) // bits 16-31 cover up to 8 texture stage format sizes

#define D3DFVF_SUPPORTED_BIT_MASK \
    (D3DFVF_POSITION_MASK | \
    D3DFVF_RESERVED1 | \
    D3DFVF_NORMAL | \
    D3DFVF_DIFFUSE | \
    D3DFVF_SPECULAR | \
    D3DFVF_TEXCOUNT_MASK | \
    D3DFVF_TEXCOORDSIZE_ALL)

struct XYZ
{
    float x, y, z;
};

typedef struct {
	FLOAT    x, y, z;
	D3DCOLOR diffuse, specular;
	FLOAT    tu, tv;
} D3DLVERTEX9, *LPD3DLVERTEX9;

// Custom vertex
const DWORD TLVERTEXFVF = (D3DFVF_XYZRHW | D3DFVF_TEX1);
struct TLVERTEX
{
    float x, y, z, rhw;
    float u, v;
};

typedef enum _D3DSURFACETYPE {
    D3DTYPE_NONE = 0,
    D3DTYPE_OFFPLAINSURFACE = 1,
    D3DTYPE_TEXTURE = 2,
    D3DTYPE_RENDERTARGET = 3,
    D3DTYPE_DEPTHSTENCIL = 4
} D3DSURFACETYPE;

#define CLAMP(val,zmin,zmax) (max((zmin),min((zmax),(val))))

void ConvertLight(D3DLIGHT7& Light7, const D3DLIGHT& Light);
void ConvertMaterial(D3DMATERIAL& Material, const D3DMATERIAL7& Material7);
void ConvertMaterial(D3DMATERIAL7& Material7, const D3DMATERIAL& Material);
void ConvertViewport(D3DVIEWPORT& ViewPort, const D3DVIEWPORT2& ViewPort2);
void ConvertViewport(D3DVIEWPORT2& ViewPort2, const D3DVIEWPORT& ViewPort);
void ConvertViewport(D3DVIEWPORT& ViewPort, const D3DVIEWPORT7& ViewPort7);
void ConvertViewport(D3DVIEWPORT2& ViewPort2, const D3DVIEWPORT7& ViewPort7);
void ConvertViewport(D3DVIEWPORT7& ViewPort7, const D3DVIEWPORT& ViewPort);
void ConvertViewport(D3DVIEWPORT7& ViewPort7, const D3DVIEWPORT2& ViewPort2);
void ConvertViewport(D3DVIEWPORT7& ViewPort, const D3DVIEWPORT7& ViewPort7);
void ConvertDeviceDesc(D3DDEVICEDESC& Desc, const D3DDEVICEDESC7& Desc7);
void ConvertDeviceDesc(D3DDEVICEDESC7& Desc7, const D3DCAPS9& Caps9);
void ConvertLVertex(D3DLVERTEX* lFVF, const D3DLVERTEX9* lFVF9, DWORD NumVertices);
void ConvertLVertex(D3DLVERTEX9* lFVF9, const D3DLVERTEX* lFVF, DWORD NumVertices);
bool CheckTextureStageStateType(D3DTEXTURESTAGESTATETYPE dwState);
bool CheckRenderStateType(D3DRENDERSTATETYPE dwRenderStateType);
void ClampVertices(BYTE* pVertexData, DWORD Stride, DWORD dwNumVertices);
void ConvertVertex(BYTE* pDestVertex, DWORD DestFVF, const BYTE* pSrcVertex, DWORD SrcFVF);
DWORD ConvertVertexTypeToFVF(D3DVERTEXTYPE d3dVertexType);
bool IsValidFVF(DWORD dwVertexTypeDesc);
UINT GetBlendCount(DWORD dwVertexTypeDesc);
UINT GetVertexPositionStride(DWORD dwVertexTypeDesc);
UINT GetTexStride(DWORD dwVertexTypeDesc, DWORD t);
UINT GetVertexTextureStride(DWORD dwVertexTypeDesc);
UINT GetVertexStride(DWORD dwVertexTypeDesc);
UINT GetNumberOfPrimitives(D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexCount);
