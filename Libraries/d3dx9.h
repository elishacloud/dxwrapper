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

typedef struct D3DXVECTOR3 {
	FLOAT x;
	FLOAT y;
	FLOAT z;
} D3DXVECTOR3, * LPD3DXVECTOR3;

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

using D3DXMACRO = D3D_SHADER_MACRO;

using D3DXMATRIX = D3DMATRIX;

using ID3DXBuffer = ID3DBlob;
using LPD3DXBUFFER = ID3DXBuffer*;

using ID3DXInclude = ID3DInclude;
using LPD3DXINCLUDE = ID3DXInclude*;

HRESULT WINAPI D3DXCreateTexture(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE9* ppTexture);
HRESULT WINAPI D3DXLoadSurfaceFromMemory(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPCVOID pSrcMemory, D3DFORMAT SrcFormat, UINT SrcPitch, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey);
HRESULT WINAPI D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey);
HRESULT WINAPI D3DXSaveSurfaceToFileInMemory(LPD3DXBUFFER* ppDestBuf, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* SrcRect);
HRESULT WINAPI D3DXSaveTextureToFileInMemory(LPD3DXBUFFER* ppDestBuf, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DBASETEXTURE9 pSrcTexture, const PALETTEENTRY* pSrcPalette);

HRESULT WINAPI D3DXDeclaratorFromFVF(DWORD FVF, D3DVERTEXELEMENT9 pDeclarator[MAX_FVF_DECL_SIZE]);
D3DXMATRIX* WINAPI D3DXMatrixMultiply(_Inout_ D3DXMATRIX* pOut, _In_ const D3DXMATRIX* pM1, _In_ const D3DXMATRIX* pM2);
D3DXVECTOR3* WINAPI D3DXVec3TransformCoord(_Inout_ D3DXVECTOR3* pOut, _In_ const D3DXVECTOR3* pV, _In_ const D3DXMATRIX* pM);

HRESULT WINAPI D3DXCompileShaderFromFileA(LPCSTR pSrcFile, const D3DXMACRO* pDefines, LPD3DXINCLUDE pInclude, LPCSTR pFunctionName, LPCSTR pProfile, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs, LPD3DXCONSTANTTABLE* ppConstantTable);
HRESULT WINAPI D3DXCompileShaderFromFileW(LPCWSTR pSrcFile, const D3DXMACRO* pDefines, LPD3DXINCLUDE pInclude, LPCSTR pFunctionName, LPCSTR pProfile, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs, LPD3DXCONSTANTTABLE* ppConstantTable);
HRESULT WINAPI D3DXAssembleShader(LPCSTR pSrcData, UINT SrcDataLen, const D3DXMACRO* pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs);
HRESULT WINAPI D3DXDisassembleShader(const DWORD* pShader, BOOL EnableColorCode, LPCSTR pComments, LPD3DXBUFFER* ppDisassembly);

HRESULT WINAPI D3DAssemble(const void* pSrcData, SIZE_T SrcDataSize, const char* pFileName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, UINT Flags, ID3DBlob** ppShader, ID3DBlob** ppErrorMsgs);
HRESULT WINAPI D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);
HRESULT WINAPI D3DDisassemble(LPCVOID pSrcData, SIZE_T SrcDataSize, UINT Flags, LPCSTR szComments, ID3DBlob** ppDisassembly);

HRESULT WINAPI D3DXFillTexture(LPVOID pTexture, LPD3DXFILL3D pFunction, LPVOID pData);
