#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d3d9.h>

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

#ifdef NDEBUG
#define D3DXASM_FLAGS  0
#else
#define D3DXASM_FLAGS D3DXASM_DEBUG
#endif // NDEBUG

struct D3DXMACRO
{
	LPCSTR Name;
	LPCSTR Definition;
};
typedef struct D3DXMACRO D3D_SHADER_MACRO;

typedef interface ID3DXBuffer *LPD3DXBUFFER;
typedef interface ID3DInclude *LPD3DINCLUDE;
typedef interface ID3DXInclude *LPD3DXINCLUDE;

DECLARE_INTERFACE_(ID3DXBuffer, IUnknown)
{
	// IUnknown
	STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	// ID3DXBuffer
	STDMETHOD_(LPVOID, GetBufferPointer)(THIS) PURE;
	STDMETHOD_(DWORD, GetBufferSize)(THIS) PURE;
};

typedef ID3DXBuffer ID3DBlob;

HRESULT WINAPI D3DXCreateTexture(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE9* ppTexture);
HRESULT WINAPI D3DXAssembleShader(LPCSTR pSrcData, UINT SrcDataLen, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER *ppShader, LPD3DXBUFFER *ppErrorMsgs);
HRESULT WINAPI D3DXDisassembleShader(const DWORD *pShader, BOOL EnableColorCode, LPCSTR pComments, LPD3DXBUFFER *ppDisassembly);
HRESULT WINAPI D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE9 pDestSurface, const PALETTEENTRY *pDestPalette, const RECT *pDestRect, LPDIRECT3DSURFACE9 pSrcSurface, const PALETTEENTRY *pSrcPalette, const RECT *pSrcRect, DWORD Filter, D3DCOLOR ColorKey);
HRESULT WINAPI D3DAssemble(const void *pSrcData, SIZE_T SrcDataSize, const char *pFileName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude, UINT Flags, ID3DBlob **ppShader, ID3DBlob **ppErrorMsgs);
HRESULT WINAPI D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob **ppCode, ID3DBlob **ppErrorMsgs);
HRESULT WINAPI D3DDisassemble(LPCVOID pSrcData, SIZE_T SrcDataSize, UINT Flags, LPCSTR szComments, ID3DBlob **ppDisassembly);
