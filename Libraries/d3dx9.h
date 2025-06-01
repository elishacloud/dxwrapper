#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d3d9.h>
#include <d3dcommon.h>

#ifndef __ID3D10Blob_FWD_DEFINED__
typedef struct _D3D_SHADER_MACRO
{
	LPCSTR Name;
	LPCSTR Definition;
} 	D3D_SHADER_MACRO;
typedef struct _D3D_SHADER_MACRO* LPD3D_SHADER_MACRO;

MIDL_INTERFACE("8BA5FB08-5195-40e2-AC58-0D989C3A0102")
ID3DBlob : public IUnknown
{
public:
	virtual LPVOID STDMETHODCALLTYPE GetBufferPointer(void) = 0;

	virtual SIZE_T STDMETHODCALLTYPE GetBufferSize(void) = 0;

};

typedef
enum _D3D_INCLUDE_TYPE
{
	D3D_INCLUDE_LOCAL = 0,
	D3D_INCLUDE_SYSTEM = (D3D_INCLUDE_LOCAL + 1),
	D3D10_INCLUDE_LOCAL = D3D_INCLUDE_LOCAL,
	D3D10_INCLUDE_SYSTEM = D3D_INCLUDE_SYSTEM,
	D3D_INCLUDE_FORCE_DWORD = 0x7fffffff
} 	D3D_INCLUDE_TYPE;

typedef interface ID3DInclude ID3DInclude;
#undef INTERFACE
#define INTERFACE ID3DInclude
DECLARE_INTERFACE(ID3DInclude)
{
	STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID * ppData, UINT * pBytes) PURE;
	STDMETHOD(Close)(THIS_ LPCVOID pData) PURE;
};
typedef ID3DInclude* LPD3DINCLUDE;
#endif // !__ID3D10Blob_FWD_DEFINED__

#define D3DX_FILTER_NONE             (1 << 0)
#define D3DX_FILTER_POINT            (2 << 0)
#define D3DX_FILTER_LINEAR           (3 << 0)
#define D3DX_FILTER_TRIANGLE         (4 << 0)
#define D3DX_FILTER_BOX              (5 << 0)

#define D3DX_FILTER_MIRROR_U         (1 << 16)
#define D3DX_FILTER_MIRROR_V         (2 << 16)
#define D3DX_FILTER_MIRROR_W         (4 << 16)
#define D3DX_FILTER_MIRROR           (7 << 16)

#define D3DXASM_DEBUG 0x0001
#define D3DXASM_SKIPVALIDATION  0x0010

#define D3DCOMPILE_OPTIMIZATION_LEVEL0            (1 << 14)
#define D3DCOMPILE_OPTIMIZATION_LEVEL1            0
#define D3DCOMPILE_OPTIMIZATION_LEVEL2            ((1 << 14) | (1 << 15))
#define D3DCOMPILE_OPTIMIZATION_LEVEL3            (1 << 15)

#define D3D_DISASM_ENABLE_COLOR_CODE            0x00000001
#define D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS  0x00000002
#define D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING 0x00000004
#define D3D_DISASM_ENABLE_INSTRUCTION_CYCLE     0x00000008
#define D3D_DISASM_DISABLE_DEBUG_INFO           0x00000010

using D3DXMACRO = D3D_SHADER_MACRO;

using D3DXMATRIX = D3DMATRIX;

using ID3DXBuffer = ID3DBlob;
using LPD3DXBUFFER = ID3DXBuffer*;

using ID3DXInclude = ID3DInclude;
using LPD3DXINCLUDE = ID3DXInclude*;

typedef struct D3DXVECTOR3 {
	FLOAT x;
	FLOAT y;
	FLOAT z;
} D3DXVECTOR3, * LPD3DXVECTOR3;

//////////////////////////////////////////////////////////////////////////////
// D3DXSPRITE flags:
// -----------------
// D3DXSPRITE_DONOTSAVESTATE
//   Specifies device state is not to be saved and restored in Begin/End.
// D3DXSPRITE_DONOTMODIFY_RENDERSTATE
//   Specifies device render state is not to be changed in Begin.  The device
//   is assumed to be in a valid state to draw vertices containing POSITION0, 
//   TEXCOORD0, and COLOR0 data.
// D3DXSPRITE_OBJECTSPACE
//   The WORLD, VIEW, and PROJECTION transforms are NOT modified.  The 
//   transforms currently set to the device are used to transform the sprites 
//   when the batch is drawn (at Flush or End).  If this is not specified, 
//   WORLD, VIEW, and PROJECTION transforms are modified so that sprites are 
//   drawn in screenspace coordinates.
// D3DXSPRITE_BILLBOARD
//   Rotates each sprite about its center so that it is facing the viewer.
// D3DXSPRITE_ALPHABLEND
//   Enables ALPHABLEND(SRCALPHA, INVSRCALPHA) and ALPHATEST(alpha > 0).
//   ID3DXFont expects this to be set when drawing text.
// D3DXSPRITE_SORT_TEXTURE
//   Sprites are sorted by texture prior to drawing.  This is recommended when
//   drawing non-overlapping sprites of uniform depth.  For example, drawing
//   screen-aligned text with ID3DXFont.
// D3DXSPRITE_SORT_DEPTH_FRONTTOBACK
//   Sprites are sorted by depth front-to-back prior to drawing.  This is 
//   recommended when drawing opaque sprites of varying depths.
// D3DXSPRITE_SORT_DEPTH_BACKTOFRONT
//   Sprites are sorted by depth back-to-front prior to drawing.  This is 
//   recommended when drawing transparent sprites of varying depths.
// D3DXSPRITE_DO_NOT_ADDREF_TEXTURE
//   Disables calling AddRef() on every draw, and Release() on Flush() for
//   better performance.
//////////////////////////////////////////////////////////////////////////////

#define D3DXSPRITE_DONOTSAVESTATE               (1 << 0)
#define D3DXSPRITE_DONOTMODIFY_RENDERSTATE      (1 << 1)
#define D3DXSPRITE_OBJECTSPACE                  (1 << 2)
#define D3DXSPRITE_BILLBOARD                    (1 << 3)
#define D3DXSPRITE_ALPHABLEND                   (1 << 4)
#define D3DXSPRITE_SORT_TEXTURE                 (1 << 5)
#define D3DXSPRITE_SORT_DEPTH_FRONTTOBACK       (1 << 6)
#define D3DXSPRITE_SORT_DEPTH_BACKTOFRONT       (1 << 7)
#define D3DXSPRITE_DO_NOT_ADDREF_TEXTURE        (1 << 8)

typedef interface ID3DXSprite ID3DXSprite;
typedef interface ID3DXSprite* LPD3DXSPRITE;

// {BA0B762D-7D28-43ec-B9DC-2F84443B0614}
DEFINE_GUID(IID_ID3DXSprite,
	0xba0b762d, 0x7d28, 0x43ec, 0xb9, 0xdc, 0x2f, 0x84, 0x44, 0x3b, 0x6, 0x14);

#undef INTERFACE
#define INTERFACE ID3DXSprite

DECLARE_INTERFACE_(ID3DXSprite, IUnknown)
{
	// IUnknown
	STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID * ppv) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	// ID3DXSprite
	STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9 * ppDevice) PURE;

	STDMETHOD(GetTransform)(THIS_ D3DXMATRIX * pTransform) PURE;
	STDMETHOD(SetTransform)(THIS_ CONST D3DXMATRIX * pTransform) PURE;

	STDMETHOD(SetWorldViewRH)(THIS_ CONST D3DXMATRIX * pWorld, CONST D3DXMATRIX * pView) PURE;
	STDMETHOD(SetWorldViewLH)(THIS_ CONST D3DXMATRIX * pWorld, CONST D3DXMATRIX * pView) PURE;

	STDMETHOD(Begin)(THIS_ DWORD Flags) PURE;
	STDMETHOD(Draw)(THIS_ LPDIRECT3DTEXTURE9 pTexture, CONST RECT * pSrcRect, CONST D3DXVECTOR3 * pCenter, CONST D3DXVECTOR3 * pPosition, D3DCOLOR Color) PURE;
	STDMETHOD(Flush)(THIS) PURE;
	STDMETHOD(End)(THIS) PURE;

	STDMETHOD(OnLostDevice)(THIS) PURE;
	STDMETHOD(OnResetDevice)(THIS) PURE;
};

typedef struct _D3DXFONT_DESCA
{
	INT Height;
	UINT Width;
	UINT Weight;
	UINT MipLevels;
	BOOL Italic;
	BYTE CharSet;
	BYTE OutputPrecision;
	BYTE Quality;
	BYTE PitchAndFamily;
	CHAR FaceName[LF_FACESIZE];

} D3DXFONT_DESCA, * LPD3DXFONT_DESCA;

typedef struct _D3DXFONT_DESCW
{
	INT Height;
	UINT Width;
	UINT Weight;
	UINT MipLevels;
	BOOL Italic;
	BYTE CharSet;
	BYTE OutputPrecision;
	BYTE Quality;
	BYTE PitchAndFamily;
	WCHAR FaceName[LF_FACESIZE];

} D3DXFONT_DESCW, * LPD3DXFONT_DESCW;

#ifdef UNICODE
typedef D3DXFONT_DESCW D3DXFONT_DESC;
typedef LPD3DXFONT_DESCW LPD3DXFONT_DESC;
#else
typedef D3DXFONT_DESCA D3DXFONT_DESC;
typedef LPD3DXFONT_DESCA LPD3DXFONT_DESC;
#endif

typedef interface ID3DXFont ID3DXFont;
typedef interface ID3DXFont* LPD3DXFONT;

// {D79DBB70-5F21-4d36-BBC2-FF525C213CDC}
DEFINE_GUID(IID_ID3DXFont,
	0xd79dbb70, 0x5f21, 0x4d36, 0xbb, 0xc2, 0xff, 0x52, 0x5c, 0x21, 0x3c, 0xdc);

#undef INTERFACE
#define INTERFACE ID3DXFont

DECLARE_INTERFACE_(ID3DXFont, IUnknown)
{
	// IUnknown
	STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID * ppv) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	// ID3DXFont
	STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9 * ppDevice) PURE;
	STDMETHOD(GetDescA)(THIS_ D3DXFONT_DESCA * pDesc) PURE;
	STDMETHOD(GetDescW)(THIS_ D3DXFONT_DESCW * pDesc) PURE;
	STDMETHOD_(BOOL, GetTextMetricsA)(THIS_ TEXTMETRICA * pTextMetrics) PURE;
	STDMETHOD_(BOOL, GetTextMetricsW)(THIS_ TEXTMETRICW * pTextMetrics) PURE;

	STDMETHOD_(HDC, GetDC)(THIS) PURE;
	STDMETHOD(GetGlyphData)(THIS_ UINT Glyph, LPDIRECT3DTEXTURE9 * ppTexture, RECT * pBlackBox, POINT * pCellInc) PURE;

	STDMETHOD(PreloadCharacters)(THIS_ UINT First, UINT Last) PURE;
	STDMETHOD(PreloadGlyphs)(THIS_ UINT First, UINT Last) PURE;
	STDMETHOD(PreloadTextA)(THIS_ LPCSTR pString, INT Count) PURE;
	STDMETHOD(PreloadTextW)(THIS_ LPCWSTR pString, INT Count) PURE;

	STDMETHOD_(INT, DrawTextA)(THIS_ LPD3DXSPRITE pSprite, LPCSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) PURE;
	STDMETHOD_(INT, DrawTextW)(THIS_ LPD3DXSPRITE pSprite, LPCWSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) PURE;

	STDMETHOD(OnLostDevice)(THIS) PURE;
	STDMETHOD(OnResetDevice)(THIS) PURE;

#ifdef __cplusplus
#ifdef UNICODE
	HRESULT GetDesc(D3DXFONT_DESCW * pDesc) { return GetDescW(pDesc); }
	HRESULT PreloadText(LPCWSTR pString, INT Count) { return PreloadTextW(pString, Count); }
#else
	HRESULT GetDesc(D3DXFONT_DESCA * pDesc) { return GetDescA(pDesc); }
	HRESULT PreloadText(LPCSTR pString, INT Count) { return PreloadTextA(pString, Count); }
#endif
#endif //__cplusplus
};

typedef enum D3DXIMAGE_FILEFORMAT {
	D3DXIFF_BMP = 0,
	D3DXIFF_JPG = 1,
	D3DXIFF_TGA = 2,
	D3DXIFF_PNG = 3,
	D3DXIFF_DDS = 4,
	D3DXIFF_PPM = 5,
	D3DXIFF_DIB = 6,
	D3DXIFF_HDR = 7,
	D3DXIFF_PFM = 8,
	D3DXIFF_FORCE_DWORD = 0x7fffffff
} D3DXIMAGE_FILEFORMAT, * LPD3DXIMAGE_FILEFORMAT;

typedef interface ID3DXConstantTable ID3DXConstantTable;
typedef interface ID3DXConstantTable* LPD3DXCONSTANTTABLE;

// Define the D3DXVECTOR4 structure
struct D3DXVECTOR4
{
	float x, y, z, w;

	D3DXVECTOR4() : x(0), y(0), z(0), w(0) {}
	D3DXVECTOR4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

// Define the D3DXVECTOR2 structure
struct D3DXVECTOR2
{
	float x, y;

	D3DXVECTOR2() : x(0), y(0) {}
	D3DXVECTOR2(float x_, float y_) : x(x_), y(y_) {}
};

#define MAX_FVF_DECL_SIZE MAXD3DDECLLENGTH + 1

// Define the D3DXFillTexture function prototype
typedef HRESULT(WINAPI* LPD3DXFILL3D)(D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord,
	const D3DXVECTOR2* pTexelSize, LPVOID pData);

#ifdef NDEBUG
#define D3DXASM_FLAGS  0
#else
#define D3DXASM_FLAGS D3DXASM_DEBUG
#endif // NDEBUG

HRESULT WINAPI D3DXCreateTexture(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE9* ppTexture);
HRESULT WINAPI D3DXLoadSurfaceFromMemory(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPCVOID pSrcMemory, D3DFORMAT SrcFormat, UINT SrcPitch, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey);
HRESULT WINAPI D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey);
HRESULT WINAPI D3DXSaveSurfaceToFileInMemory(LPD3DXBUFFER* ppDestBuf, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* SrcRect);
HRESULT WINAPI D3DXSaveTextureToFileInMemory(LPD3DXBUFFER* ppDestBuf, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DBASETEXTURE9 pSrcTexture, const PALETTEENTRY* pSrcPalette);

HRESULT WINAPI D3DXDeclaratorFromFVF(DWORD FVF, D3DVERTEXELEMENT9 pDeclarator[MAX_FVF_DECL_SIZE]);
D3DXMATRIX* WINAPI D3DXMatrixMultiply(_Inout_ D3DXMATRIX* pOut, _In_ const D3DXMATRIX* pM1, _In_ const D3DXMATRIX* pM2);
D3DXVECTOR3* WINAPI D3DXVec3TransformCoord(_Inout_ D3DXVECTOR3* pOut, _In_ const D3DXVECTOR3* pV, _In_ const D3DXMATRIX* pM);
D3DXVECTOR4* WINAPI D3DXVec4Transform(_Inout_ D3DXVECTOR4* pOut, _In_ const D3DXVECTOR4* pV, _In_ const D3DXMATRIX* pM);

HRESULT WINAPI D3DXCompileShaderFromFileA(LPCSTR pSrcFile, const D3DXMACRO* pDefines, LPD3DXINCLUDE pInclude, LPCSTR pFunctionName, LPCSTR pProfile, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs, LPD3DXCONSTANTTABLE* ppConstantTable);
HRESULT WINAPI D3DXCompileShaderFromFileW(LPCWSTR pSrcFile, const D3DXMACRO* pDefines, LPD3DXINCLUDE pInclude, LPCSTR pFunctionName, LPCSTR pProfile, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs, LPD3DXCONSTANTTABLE* ppConstantTable);
HRESULT WINAPI D3DXAssembleShader(LPCSTR pSrcData, UINT SrcDataLen, const D3DXMACRO* pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs);
HRESULT WINAPI D3DXDisassembleShader(const DWORD* pShader, BOOL EnableColorCode, LPCSTR pComments, LPD3DXBUFFER* ppDisassembly);

HRESULT WINAPI D3DAssemble(const void* pSrcData, SIZE_T SrcDataSize, const char* pFileName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, UINT Flags, ID3DBlob** ppShader, ID3DBlob** ppErrorMsgs);
HRESULT WINAPI D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);
HRESULT WINAPI D3DDisassemble(LPCVOID pSrcData, SIZE_T SrcDataSize, UINT Flags, LPCSTR szComments, ID3DBlob** ppDisassembly);

HRESULT WINAPI D3DXFillTexture(LPVOID pTexture, LPD3DXFILL3D pFunction, LPVOID pData);

HRESULT WINAPI D3DXCreateFontA(LPDIRECT3DDEVICE9 pDevice, INT Height, UINT Width, UINT Weight, UINT MipLevels, BOOL Italic, DWORD CharSet, DWORD OutputPrecision, DWORD Quality, DWORD PitchAndFamily, LPCSTR pFaceName, LPD3DXFONT* ppFont);
HRESULT WINAPI D3DXCreateFontW(LPDIRECT3DDEVICE9 pDevice, INT Height, UINT Width, UINT Weight, UINT MipLevels, BOOL Italic, DWORD CharSet, DWORD OutputPrecision, DWORD Quality, DWORD PitchAndFamily, LPCWSTR pFaceName, LPD3DXFONT* ppFont);

HRESULT WINAPI D3DXCreateSprite(LPDIRECT3DDEVICE9 pDevice, LPD3DXSPRITE* ppSprite);
