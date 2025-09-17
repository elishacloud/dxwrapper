#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "External\Logging\Logging.h"

namespace Logging
{
	void InitLog();
}

template <typename T>
inline std::string GetTypeName() { return "Unknown"; }

template <>
inline std::string GetTypeName<DWORD>() { return "DWORD"; }

#if(WINVER < 0x0602)
typedef struct tagTOUCH_HIT_TESTING_INPUT
{
    UINT32 pointerId;
    POINT point;
    RECT boundingBox;
    RECT nonOccludedBoundingBox;
    UINT32 orientation;
} TOUCH_HIT_TESTING_INPUT, * PTOUCH_HIT_TESTING_INPUT;
#endif

#pragma warning (disable: 26812)
typedef enum _DDFOURCC {} DDFOURCC;
typedef enum _DDERR {} DDERR;
typedef enum _D3DERR {} D3DERR;
#ifndef DIERR_SET
#define DIERR_SET
typedef enum _DIERR {} DIERR;
#endif
typedef enum _DSERR {} DSERR;
typedef enum _WMMSG {} WMMSG;

struct FLOAT4 {
    union {
        float m[4];
        float Plane[4];
    };
};

std::ostream& operator<<(std::ostream& os, const FLOAT4& data);
std::ostream& operator<<(std::ostream& os, const DDFOURCC& dwFourCC);
std::ostream& operator<<(std::ostream& os, const DDERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const D3DERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const DIERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const DSERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const WMMSG& Id);

#ifdef __DDRAW_INCLUDED__
std::ostream& operator<<(std::ostream& os, const DDCAPS& caps);
std::ostream& operator<<(std::ostream& os, const DDSCAPS& caps);
std::ostream& operator<<(std::ostream& os, const DDSCAPS2& caps);
std::ostream& operator<<(std::ostream& os, const DDPIXELFORMAT& pf);
std::ostream& operator<<(std::ostream& os, const DDCOLORKEY& ck);
std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC& sd);
std::ostream& operator<<(std::ostream& os, const DDSURFACEDESC2& sd);
#endif
#ifdef _D3DCAPS_H
std::ostream& operator<<(std::ostream& os, const D3DCOLORVALUE& data);
std::ostream& operator<<(std::ostream& os, const D3DDP_PTRSTRIDE& data);
std::ostream& operator<<(std::ostream& os, const D3DDRAWPRIMITIVESTRIDEDDATA& data);
std::ostream& operator<<(std::ostream& os, const D3DEXECUTEBUFFERDESC& data);
std::ostream& operator<<(std::ostream& os, const D3DEXECUTEDATA& data);
std::ostream& operator<<(std::ostream& os, const D3DLIGHT& data);
std::ostream& operator<<(std::ostream& os, const D3DLIGHT2& data);
std::ostream& operator<<(std::ostream& os, const D3DLIGHT7& data);
std::ostream& operator<<(std::ostream& os, const D3DMATERIAL& data);
std::ostream& operator<<(std::ostream& os, const D3DMATERIAL7& data);
std::ostream& operator<<(std::ostream& os, const D3DRECT& data);
std::ostream& operator<<(std::ostream& os, const D3DSTATUS& data);
std::ostream& operator<<(std::ostream& os, const D3DCLIPSTATUS& data);
std::ostream& operator<<(std::ostream& os, const D3DVERTEXBUFFERDESC& data);
std::ostream& operator<<(std::ostream& os, const D3DPRIMCAPS& pc);
std::ostream& operator<<(std::ostream& os, const D3DTRANSFORMCAPS& tc);
std::ostream& operator<<(std::ostream& os, const D3DLIGHTINGCAPS& lc);
std::ostream& operator<<(std::ostream& os, const D3DDEVICEDESC& dd);
std::ostream& operator<<(std::ostream& os, const D3DDEVICEDESC7& dd);
std::ostream& operator<<(std::ostream& os, const D3DVIEWPORT7& vp);
#endif
#ifdef _d3d9TYPES_H_
std::ostream& operator<<(std::ostream& os, const D3DFORMAT& format);
std::ostream& operator<<(std::ostream& os, const D3DRESOURCETYPE& Resource);
std::ostream& operator<<(std::ostream& os, const D3DPRESENT_PARAMETERS& pp);
std::ostream& operator<<(std::ostream& os, const D3DSURFACE_DESC& desc);
std::ostream& operator<<(std::ostream& os, const D3DVIEWPORT9& vp);
std::ostream& operator<<(std::ostream& os, const D3DLIGHT9& data);
std::ostream& operator<<(std::ostream& os, const D3DMATERIAL9& data);
std::ostream& operator<<(std::ostream& os, const D3DMATRIX& data);
#endif
#ifdef GUID_DEFINED
std::ostream& operator<<(std::ostream& os, REFIID riid);
#endif
std::ostream& operator<<(std::ostream& os, const COMPAREITEMSTRUCT& cis);
std::ostream& operator<<(std::ostream& os, const COPYDATASTRUCT& cds);
std::ostream& operator<<(std::ostream& os, const CREATESTRUCTA& cs);
std::ostream& operator<<(std::ostream& os, const CREATESTRUCTW& cs);
std::ostream& operator<<(std::ostream& os, const CWPSTRUCT& cwp);
std::ostream& operator<<(std::ostream& os, const CWPRETSTRUCT& cwrp);
std::ostream& operator<<(std::ostream& os, const DELETEITEMSTRUCT& dis);
std::ostream& operator<<(std::ostream& os, const DEVMODEA& dm);
std::ostream& operator<<(std::ostream& os, const DEVMODEW& dm);
std::ostream& operator<<(std::ostream& os, const DRAWITEMSTRUCT& dis);
std::ostream& operator<<(std::ostream& os, HDC__& dc);
std::ostream& operator<<(std::ostream& os, const HELPINFO& hi);
std::ostream& operator<<(std::ostream& os, HFONT font);
std::ostream& operator<<(std::ostream& os, HRGN rgn);
std::ostream& operator<<(std::ostream& os, HWND__& hwnd);
std::ostream& operator<<(std::ostream& os, const LOGFONT& lf);
std::ostream& operator<<(std::ostream& os, const MDICREATESTRUCTA& mcs);
std::ostream& operator<<(std::ostream& os, const MDICREATESTRUCTW& mcs);
std::ostream& operator<<(std::ostream& os, const MDINEXTMENU& mnm);
std::ostream& operator<<(std::ostream& os, const MEASUREITEMSTRUCT& mis);
std::ostream& operator<<(std::ostream& os, const MEMORYSTATUS& ms);
std::ostream& operator<<(std::ostream& os, const MENUGETOBJECTINFO& mgoi);
std::ostream& operator<<(std::ostream& os, const MINMAXINFO& mmi);
std::ostream& operator<<(std::ostream& os, const MSG& msg);
std::ostream& operator<<(std::ostream& os, const NCCALCSIZE_PARAMS& nccs);
std::ostream& operator<<(std::ostream& os, const NMHDR& nm);
std::ostream& operator<<(std::ostream& os, const POINT& p);
std::ostream& operator<<(std::ostream& os, const POINTS& p);
std::ostream& operator<<(std::ostream& os, const RECT& rect);
std::ostream& operator<<(std::ostream& os, const SIZE& size);
std::ostream& operator<<(std::ostream& os, const STYLESTRUCT& ss);
std::ostream& operator<<(std::ostream& os, const WINDOWPOS& wp);
#ifdef DDRAWCOMPAT
std::ostream& operator<<(std::ostream& os, const TITLEBARINFOEX& tbi);
std::ostream& operator<<(std::ostream& os, const GESTURENOTIFYSTRUCT& gns);
std::ostream& operator<<(std::ostream& os, const TOUCH_HIT_TESTING_INPUT& thti);
#endif // DDRAWCOMPAT
#ifdef DX3DTYPE_H
std::ostream& operator<<(std::ostream& os, const D3DSURFACETYPE& ddType);
#endif // DX3DTYPE_H
