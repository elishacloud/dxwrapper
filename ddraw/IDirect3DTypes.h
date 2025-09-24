#pragma once

#ifndef DX3DTYPE_H
#define DX3DTYPE_H
#endif

#include "IDirect3DTypesDefault.h"

#define D3DSTATE D3DSTATE7

#define D3DRENDERSTATE_NONE (D3DRENDERSTATETYPE)0

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
#define D3DTSS_ADDRESS (D3DTEXTURESTAGESTATETYPE)12
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

constexpr D3DRENDERSTATETYPE D9RenderStateList[] = {
    D3DRS_ZENABLE,
    D3DRS_FILLMODE,
    D3DRS_SHADEMODE,
    D3DRS_ZWRITEENABLE,
    D3DRS_ALPHATESTENABLE,
    D3DRS_LASTPIXEL,
    D3DRS_SRCBLEND,
    D3DRS_DESTBLEND,
    D3DRS_CULLMODE,
    D3DRS_ZFUNC,
    D3DRS_ALPHAREF,
    D3DRS_ALPHAFUNC,
    D3DRS_DITHERENABLE,
    D3DRS_ALPHABLENDENABLE,
    D3DRS_FOGENABLE,
    D3DRS_SPECULARENABLE,
    D3DRS_FOGCOLOR,
    D3DRS_FOGTABLEMODE,
    D3DRS_FOGSTART,
    D3DRS_FOGEND,
    D3DRS_FOGDENSITY,
    D3DRS_RANGEFOGENABLE,
    D3DRS_STENCILENABLE,
    D3DRS_STENCILFAIL,
    D3DRS_STENCILZFAIL,
    D3DRS_STENCILPASS,
    D3DRS_STENCILFUNC,
    D3DRS_STENCILREF,
    D3DRS_STENCILMASK,
    D3DRS_STENCILWRITEMASK,
    D3DRS_TEXTUREFACTOR,
    D3DRS_WRAP0,
    D3DRS_WRAP1,
    D3DRS_WRAP2,
    D3DRS_WRAP3,
    D3DRS_WRAP4,
    D3DRS_WRAP5,
    D3DRS_WRAP6,
    D3DRS_WRAP7,
    D3DRS_CLIPPING,
    D3DRS_LIGHTING,
    D3DRS_AMBIENT,
    D3DRS_FOGVERTEXMODE,
    D3DRS_COLORVERTEX,
    D3DRS_LOCALVIEWER,
    D3DRS_NORMALIZENORMALS,
    D3DRS_DIFFUSEMATERIALSOURCE,
    D3DRS_SPECULARMATERIALSOURCE,
    D3DRS_AMBIENTMATERIALSOURCE,
    D3DRS_EMISSIVEMATERIALSOURCE,
    D3DRS_VERTEXBLEND,
    D3DRS_CLIPPLANEENABLE,
    D3DRS_POINTSIZE,
    D3DRS_POINTSIZE_MIN,
    D3DRS_POINTSPRITEENABLE,
    D3DRS_POINTSCALEENABLE,
    D3DRS_POINTSCALE_A,
    D3DRS_POINTSCALE_B,
    D3DRS_POINTSCALE_C,
    D3DRS_MULTISAMPLEANTIALIAS,
    D3DRS_MULTISAMPLEMASK,
    D3DRS_PATCHEDGESTYLE,
    D3DRS_DEBUGMONITORTOKEN,
    D3DRS_POINTSIZE_MAX,
    D3DRS_INDEXEDVERTEXBLENDENABLE,
    D3DRS_COLORWRITEENABLE,
    D3DRS_TWEENFACTOR,
    D3DRS_BLENDOP,
    D3DRS_POSITIONDEGREE,
    D3DRS_NORMALDEGREE,
    D3DRS_SCISSORTESTENABLE,
    D3DRS_SLOPESCALEDEPTHBIAS,
    D3DRS_ANTIALIASEDLINEENABLE,
    D3DRS_MINTESSELLATIONLEVEL,
    D3DRS_MAXTESSELLATIONLEVEL,
    D3DRS_ADAPTIVETESS_X,
    D3DRS_ADAPTIVETESS_Y,
    D3DRS_ADAPTIVETESS_Z,
    D3DRS_ADAPTIVETESS_W,
    D3DRS_ENABLEADAPTIVETESSELLATION,
    D3DRS_TWOSIDEDSTENCILMODE,
    D3DRS_CCW_STENCILFAIL,
    D3DRS_CCW_STENCILZFAIL,
    D3DRS_CCW_STENCILPASS,
    D3DRS_CCW_STENCILFUNC,
    D3DRS_COLORWRITEENABLE1,
    D3DRS_COLORWRITEENABLE2,
    D3DRS_COLORWRITEENABLE3,
    D3DRS_BLENDFACTOR,
    D3DRS_SRGBWRITEENABLE,
    D3DRS_DEPTHBIAS,
    D3DRS_WRAP8,
    D3DRS_WRAP9,
    D3DRS_WRAP10,
    D3DRS_WRAP11,
    D3DRS_WRAP12,
    D3DRS_WRAP13,
    D3DRS_WRAP14,
    D3DRS_WRAP15,
    D3DRS_SEPARATEALPHABLENDENABLE,
    D3DRS_SRCBLENDALPHA,
    D3DRS_DESTBLENDALPHA,
    D3DRS_BLENDOPALPHA
};

constexpr D3DTEXTURESTAGESTATETYPE D9TextureStateList[] = {
    D3DTSS_COLOROP,
    D3DTSS_COLORARG1,
    D3DTSS_COLORARG2,
    D3DTSS_ALPHAOP,
    D3DTSS_ALPHAARG1,
    D3DTSS_ALPHAARG2,
    D3DTSS_BUMPENVMAT00,
    D3DTSS_BUMPENVMAT01,
    D3DTSS_BUMPENVMAT10,
    D3DTSS_BUMPENVMAT11,
    D3DTSS_TEXCOORDINDEX,
    D3DTSS_BUMPENVLSCALE,
    D3DTSS_BUMPENVLOFFSET,
    D3DTSS_TEXTURETRANSFORMFLAGS,
    D3DTSS_COLORARG0,
    D3DTSS_ALPHAARG0,
    D3DTSS_RESULTARG,
    D3DTSS_CONSTANT
};

constexpr D3DSAMPLERSTATETYPE D9SamplerStateList[] = {
    D3DSAMP_ADDRESSU,
    D3DSAMP_ADDRESSV,
    D3DSAMP_ADDRESSW,
    D3DSAMP_BORDERCOLOR,
    D3DSAMP_MAGFILTER,
    D3DSAMP_MINFILTER,
    D3DSAMP_MIPFILTER,
    D3DSAMP_MIPMAPLODBIAS,
    D3DSAMP_MAXMIPLEVEL,
    D3DSAMP_MAXANISOTROPY,
    D3DSAMP_SRGBTEXTURE,
    D3DSAMP_ELEMENTINDEX,
    D3DSAMP_DMAPOFFSET
};

constexpr D3DRENDERSTATETYPE StateBlockPixelRenderStates[] = {
    D3DRENDERSTATE_ALPHABLENDENABLE,
    D3DRENDERSTATE_ALPHAFUNC,
    D3DRENDERSTATE_ALPHAREF,
    D3DRENDERSTATE_ALPHATESTENABLE,
    D3DRENDERSTATE_ANTIALIAS,
    D3DRENDERSTATE_COLORKEYENABLE,
    D3DRENDERSTATE_DESTBLEND,
    D3DRENDERSTATE_DITHERENABLE,
    D3DRENDERSTATE_EDGEANTIALIAS,
    D3DRENDERSTATE_FILLMODE,
    D3DRENDERSTATE_FOGDENSITY,
    D3DRENDERSTATE_FOGEND,
    D3DRENDERSTATE_FOGSTART,
    D3DRENDERSTATE_LASTPIXEL,
    D3DRENDERSTATE_LINEPATTERN,
    D3DRENDERSTATE_SHADEMODE,
    D3DRENDERSTATE_SRCBLEND,
    D3DRENDERSTATE_STENCILENABLE,
    D3DRENDERSTATE_STENCILFAIL,
    D3DRENDERSTATE_STENCILFUNC,
    D3DRENDERSTATE_STENCILMASK,
    D3DRENDERSTATE_STENCILPASS,
    D3DRENDERSTATE_STENCILREF,
    D3DRENDERSTATE_STENCILWRITEMASK,
    D3DRENDERSTATE_STENCILZFAIL,
    D3DRENDERSTATE_STIPPLEDALPHA,
    D3DRENDERSTATE_TEXTUREFACTOR,
    D3DRENDERSTATE_TEXTUREPERSPECTIVE,
    D3DRENDERSTATE_WRAP0,
    D3DRENDERSTATE_WRAP1,
    D3DRENDERSTATE_WRAP2,
    D3DRENDERSTATE_WRAP3,
    D3DRENDERSTATE_WRAP4,
    D3DRENDERSTATE_WRAP5,
    D3DRENDERSTATE_WRAP6,
    D3DRENDERSTATE_WRAP7,
    D3DRENDERSTATE_ZBIAS,
    D3DRS_DEPTHBIAS,
    D3DRENDERSTATE_ZENABLE,
    D3DRENDERSTATE_ZFUNC,
    D3DRENDERSTATE_ZWRITEENABLE
};

constexpr D3DRENDERSTATETYPE StateBlockVertexRenderStates[] = {
    D3DRENDERSTATE_AMBIENT,
    D3DRENDERSTATE_AMBIENTMATERIALSOURCE,
    D3DRENDERSTATE_ANTIALIAS,
    D3DRENDERSTATE_CLIPPING,
    D3DRENDERSTATE_CLIPPLANEENABLE,
    D3DRENDERSTATE_COLORVERTEX,
    D3DRENDERSTATE_CULLMODE,
    D3DRENDERSTATE_DIFFUSEMATERIALSOURCE,
    D3DRENDERSTATE_EDGEANTIALIAS,
    D3DRS_MULTISAMPLEANTIALIAS,
    D3DRENDERSTATE_EMISSIVEMATERIALSOURCE,
    D3DRENDERSTATE_EXTENTS,
    D3DRENDERSTATE_FOGCOLOR,
    D3DRENDERSTATE_FOGDENSITY,
    D3DRENDERSTATE_FOGENABLE,
    D3DRENDERSTATE_FOGEND,
    D3DRENDERSTATE_FOGSTART,
    D3DRENDERSTATE_FOGTABLEMODE,
    D3DRENDERSTATE_FOGVERTEXMODE,
    D3DRENDERSTATE_LIGHTING,
    D3DRENDERSTATE_LOCALVIEWER,
    D3DRENDERSTATE_NORMALIZENORMALS,
    D3DRENDERSTATE_RANGEFOGENABLE,
    D3DRENDERSTATE_SHADEMODE,
    D3DRENDERSTATE_SPECULARENABLE,
    D3DRENDERSTATE_SPECULARMATERIALSOURCE,
    D3DRENDERSTATE_VERTEXBLEND
};

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

struct CTUV
{
    D3DCOLOR diffuse, specular;
    float    tu, tv;
};

typedef struct {
    XYZ xyz;
    DWORD dwReserved;
    CTUV ctuv;
} DXLVERTEX7, * LPDXLVERTEX7;

typedef struct {
    XYZ xyz;
    CTUV ctuv;
} DXLVERTEX9, * LPDXLVERTEX9;

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

// Clamp rhw values
const float max_rhw = static_cast<float>(1u << 31);
const float min_rhw = 1.0f / max_rhw;

void ConvertLight(D3DLIGHT7& Light7, const D3DLIGHT& Light);
D3DLIGHT9 FixLight(const D3DLIGHT9& Light);
void ConvertMaterial(D3DMATERIAL& Material, const D3DMATERIAL7& Material7);
void ConvertMaterial(D3DMATERIAL7& Material7, const D3DMATERIAL& Material);
void ConvertViewport(D3DVIEWPORT& Viewport, const D3DVIEWPORT2& Viewport2);
void ConvertViewport(D3DVIEWPORT2& Viewport2, const D3DVIEWPORT& Viewport);
void ConvertViewport(D3DVIEWPORT& Viewport, const D3DVIEWPORT7& Viewport7);
void ConvertViewport(D3DVIEWPORT2& Viewport2, const D3DVIEWPORT7& Viewport7);
void ConvertViewport(D3DVIEWPORT7& Viewport7, const D3DVIEWPORT& Viewport);
void ConvertViewport(D3DVIEWPORT7& Viewport7, const D3DVIEWPORT2& Viewport2);
void ConvertViewport(D3DVIEWPORT7& Viewport, const D3DVIEWPORT7& Viewport7);
bool IsValidRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD DirectXVersion);
bool IsOutOfRangeRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD DirectXVersion);
DWORD GetDepthBias(DWORD ZBias, DWORD DepthBits);
DWORD FixSamplerState(D3DSAMPLERSTATETYPE Type, DWORD Value);
bool IsValidTransformState(D3DTRANSFORMSTATETYPE State);
D3DMATRIX FixMatrix(const D3DMATRIX& Matrix, D3DTRANSFORMSTATETYPE State, D3DVIEWPORT Viewport, bool ScaleMatrix);
void ConvertDeviceDesc(D3DDEVICEDESC& Desc, const D3DDEVICEDESC7& Desc7);
void ConvertDeviceDesc(D3DDEVICEDESC7& Desc7, const D3DCAPS9& Caps9);
void ConvertLVertex(DXLVERTEX7* lFVF7, const DXLVERTEX9* lFVF9, DWORD NumVertices);
void ConvertLVertex(DXLVERTEX9* lFVF9, const DXLVERTEX7* lFVF7, DWORD NumVertices);
bool CheckTextureStageStateType(D3DTEXTURESTAGESTATETYPE dwState);
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
