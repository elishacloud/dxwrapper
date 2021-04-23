#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.0/D3dDdi/Log/DeviceFuncsLog.h>

std::ostream& operator<<(std::ostream& os, D3DDDI_POOL val)
{
	switch (val)
	{
		LOG_CONST_CASE(D3DDDIPOOL_SYSTEMMEM);
		LOG_CONST_CASE(D3DDDIPOOL_VIDEOMEMORY);
		LOG_CONST_CASE(D3DDDIPOOL_LOCALVIDMEM);
		LOG_CONST_CASE(D3DDDIPOOL_NONLOCALVIDMEM);
		LOG_CONST_CASE(D3DDDIPOOL_STAGINGMEM);
	}

	return os << "D3DDDIPOOL_" << static_cast<DWORD>(val);
}

std::ostream& operator<<(std::ostream& os, const D3DDDI_SURFACEINFO& val)
{
	return Compat30::LogStruct(os)
		<< val.Width
		<< val.Height
		<< val.Depth
		<< val.pSysMem
		<< val.SysMemPitch
		<< val.SysMemSlicePitch;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_BLT& val)
{
	return Compat30::LogStruct(os)
		<< val.hSrcResource
		<< val.SrcSubResourceIndex
		<< val.SrcRect
		<< val.hDstResource
		<< val.DstSubResourceIndex
		<< val.DstRect
		<< Compat30::hex(val.ColorKey)
		<< Compat30::hex(val.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CLEAR& val)
{
	return Compat30::LogStruct(os)
		<< Compat30::hex(val.Flags)
		<< Compat30::hex(val.FillColor)
		<< val.FillDepth
		<< Compat30::hex(val.FillStencil);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_COLORFILL& val)
{
	return Compat30::LogStruct(os)
		<< val.hResource
		<< val.SubResourceIndex
		<< val.DstRect
		<< Compat30::hex(val.Color)
		<< Compat30::hex(val.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATERESOURCE& val)
{
	return Compat30::LogStruct(os)
		<< val.Format
		<< val.Pool
		<< val.MultisampleType
		<< val.MultisampleQuality
		<< Compat30::array(val.pSurfList, val.SurfCount)
		<< val.SurfCount
		<< val.MipLevels
		<< val.Fvf
		<< val.VidPnSourceId
		<< val.RefreshRate
		<< val.hResource
		<< Compat30::hex(val.Flags.Value)
		<< val.Rotation;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATERESOURCE2& val)
{
	return Compat30::LogStruct(os)
		<< val.Format
		<< val.Pool
		<< val.MultisampleType
		<< val.MultisampleQuality
		<< Compat30::array(val.pSurfList, val.SurfCount)
		<< val.SurfCount
		<< val.MipLevels
		<< val.Fvf
		<< val.VidPnSourceId
		<< val.RefreshRate
		<< val.hResource
		<< Compat30::hex(val.Flags.Value)
		<< val.Rotation
		<< Compat30::hex(val.Flags2.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATEVERTEXSHADERDECL& val)
{
	return Compat30::LogStruct(os)
		<< val.NumVertexElements
		<< val.ShaderHandle;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_DRAWINDEXEDPRIMITIVE& val)
{
	return Compat30::LogStruct(os)
		<< val.PrimitiveType
		<< val.BaseVertexIndex
		<< val.MinIndex
		<< val.NumVertices
		<< val.StartIndex
		<< val.PrimitiveCount;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_DRAWINDEXEDPRIMITIVE2& val)
{
	return Compat30::LogStruct(os)
		<< val.PrimitiveType
		<< val.BaseVertexOffset
		<< val.MinIndex
		<< val.NumVertices
		<< val.StartIndexOffset
		<< val.PrimitiveCount;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_DRAWPRIMITIVE& val)
{
	return Compat30::LogStruct(os)
		<< val.PrimitiveType
		<< val.VStart
		<< val.PrimitiveCount;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_DRAWPRIMITIVE2& val)
{
	return Compat30::LogStruct(os)
		<< val.PrimitiveType
		<< val.FirstVertexOffset
		<< val.PrimitiveCount;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_LOCK& val)
{
	return Compat30::LogStruct(os)
		<< val.hResource
		<< val.SubResourceIndex
		<< val.Box
		<< val.pSurfData
		<< val.Pitch
		<< val.SlicePitch
		<< Compat30::hex(val.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_OPENRESOURCE& val)
{
	return Compat30::LogStruct(os)
		<< val.NumAllocations
		<< Compat30::array(val.pOpenAllocationInfo, val.NumAllocations)
		<< Compat30::hex(val.hKMResource)
		<< val.pPrivateDriverData
		<< val.PrivateDriverDataSize
		<< val.hResource
		<< val.Rotation
		<< Compat30::hex(val.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_PRESENT& val)
{
	return Compat30::LogStruct(os)
		<< val.hSrcResource
		<< val.SrcSubResourceIndex
		<< val.hDstResource
		<< val.DstSubResourceIndex
		<< Compat30::hex(val.Flags.Value)
		<< val.FlipInterval;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_PRESENT1& val)
{
	return Compat30::LogStruct(os)
		<< Compat30::array(val.phSrcResources, val.SrcResources)
		<< val.SrcResources
		<< val.hDstResource
		<< val.DstSubResourceIndex
		<< Compat30::hex(val.Flags.Value)
		<< val.FlipInterval
		<< val.Reserved
		<< Compat30::array(val.pDirtyRects, val.DirtyRects)
		<< val.DirtyRects;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_PRESENTSURFACE& val)
{
	return Compat30::LogStruct(os)
		<< val.hResource
		<< val.SubResourceIndex;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_RENDERSTATE& val)
{
	return Compat30::LogStruct(os)
		<< val.State
		<< val.Value;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_SETPIXELSHADERCONST& val)
{
	return Compat30::LogStruct(os)
		<< val.Register
		<< val.Count;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_SETRENDERTARGET& val)
{
	return Compat30::LogStruct(os)
		<< val.RenderTargetIndex
		<< val.hRenderTarget
		<< val.SubResourceIndex;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_SETSTREAMSOURCE& val)
{
	return Compat30::LogStruct(os)
		<< val.Stream
		<< val.hVertexBuffer
		<< val.Offset
		<< val.Stride;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_SETSTREAMSOURCEUM& val)
{
	return Compat30::LogStruct(os)
		<< val.Stream
		<< val.Stride;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_SETVERTEXSHADERCONST& val)
{
	return Compat30::LogStruct(os)
		<< val.Register
		<< val.Count;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_TEXTURESTAGESTATE& val)
{
	return Compat30::LogStruct(os)
		<< val.Stage
		<< val.State
		<< val.Value;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_UNLOCK& val)
{
	return Compat30::LogStruct(os)
		<< val.hResource
		<< val.SubResourceIndex
		<< Compat30::hex(val.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_WINFO& val)
{
	return Compat30::LogStruct(os)
		<< val.WNear
		<< val.WFar;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_ZRANGE& val)
{
	return Compat30::LogStruct(os)
		<< val.MinZ
		<< val.MaxZ;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIBOX& box)
{
	return Compat30::LogStruct(os)
		<< box.Left
		<< box.Top
		<< box.Right
		<< box.Bottom
		<< box.Front
		<< box.Back;
}

std::ostream& operator<<(std::ostream& os, D3DDDIRENDERSTATETYPE val)
{
	switch (val)
	{
		LOG_CONST_CASE(D3DDDIRS_ZENABLE);
		LOG_CONST_CASE(D3DDDIRS_FILLMODE);
		LOG_CONST_CASE(D3DDDIRS_SHADEMODE);
		LOG_CONST_CASE(D3DDDIRS_LINEPATTERN);
		LOG_CONST_CASE(D3DDDIRS_ZWRITEENABLE);
		LOG_CONST_CASE(D3DDDIRS_ALPHATESTENABLE);
		LOG_CONST_CASE(D3DDDIRS_LASTPIXEL);
		LOG_CONST_CASE(D3DDDIRS_SRCBLEND);
		LOG_CONST_CASE(D3DDDIRS_DESTBLEND);
		LOG_CONST_CASE(D3DDDIRS_CULLMODE);
		LOG_CONST_CASE(D3DDDIRS_ZFUNC);
		LOG_CONST_CASE(D3DDDIRS_ALPHAREF);
		LOG_CONST_CASE(D3DDDIRS_ALPHAFUNC);
		LOG_CONST_CASE(D3DDDIRS_DITHERENABLE);
		LOG_CONST_CASE(D3DDDIRS_ALPHABLENDENABLE);
		LOG_CONST_CASE(D3DDDIRS_FOGENABLE);
		LOG_CONST_CASE(D3DDDIRS_SPECULARENABLE);
		LOG_CONST_CASE(D3DDDIRS_ZVISIBLE);
		LOG_CONST_CASE(D3DDDIRS_FOGCOLOR);
		LOG_CONST_CASE(D3DDDIRS_FOGTABLEMODE);
		LOG_CONST_CASE(D3DDDIRS_FOGSTART);
		LOG_CONST_CASE(D3DDDIRS_FOGEND);
		LOG_CONST_CASE(D3DDDIRS_FOGDENSITY);
		LOG_CONST_CASE(D3DDDIRS_EDGEANTIALIAS);
		LOG_CONST_CASE(D3DDDIRS_COLORKEYENABLE);
		LOG_CONST_CASE(D3DDDIRS_OLDALPHABLENDENABLE);
		LOG_CONST_CASE(D3DDDIRS_ZBIAS);
		LOG_CONST_CASE(D3DDDIRS_RANGEFOGENABLE);
		LOG_CONST_CASE(D3DDDIRS_TRANSLUCENTSORTINDEPENDENT);
		LOG_CONST_CASE(D3DDDIRS_STENCILENABLE);
		LOG_CONST_CASE(D3DDDIRS_STENCILFAIL);
		LOG_CONST_CASE(D3DDDIRS_STENCILZFAIL);
		LOG_CONST_CASE(D3DDDIRS_STENCILPASS);
		LOG_CONST_CASE(D3DDDIRS_STENCILFUNC);
		LOG_CONST_CASE(D3DDDIRS_STENCILREF);
		LOG_CONST_CASE(D3DDDIRS_STENCILMASK);
		LOG_CONST_CASE(D3DDDIRS_STENCILWRITEMASK);
		LOG_CONST_CASE(D3DDDIRS_TEXTUREFACTOR);
		LOG_CONST_CASE(D3DDDIRS_SCENECAPTURE);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN00);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN01);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN02);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN03);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN04);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN05);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN06);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN07);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN08);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN09);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN10);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN11);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN12);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN13);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN14);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN15);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN16);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN17);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN18);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN19);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN20);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN21);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN22);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN23);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN24);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN25);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN26);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN27);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN28);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN29);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN30);
		LOG_CONST_CASE(D3DDDIRS_STIPPLEPATTERN31);
		LOG_CONST_CASE(D3DDDIRS_WRAP0);
		LOG_CONST_CASE(D3DDDIRS_WRAP1);
		LOG_CONST_CASE(D3DDDIRS_WRAP2);
		LOG_CONST_CASE(D3DDDIRS_WRAP3);
		LOG_CONST_CASE(D3DDDIRS_WRAP4);
		LOG_CONST_CASE(D3DDDIRS_WRAP5);
		LOG_CONST_CASE(D3DDDIRS_WRAP6);
		LOG_CONST_CASE(D3DDDIRS_WRAP7);
		LOG_CONST_CASE(D3DDDIRS_CLIPPING);
		LOG_CONST_CASE(D3DDDIRS_LIGHTING);
		LOG_CONST_CASE(D3DDDIRS_AMBIENT);
		LOG_CONST_CASE(D3DDDIRS_FOGVERTEXMODE);
		LOG_CONST_CASE(D3DDDIRS_COLORVERTEX);
		LOG_CONST_CASE(D3DDDIRS_LOCALVIEWER);
		LOG_CONST_CASE(D3DDDIRS_NORMALIZENORMALS);
		LOG_CONST_CASE(D3DDDIRS_COLORKEYBLENDENABLE);
		LOG_CONST_CASE(D3DDDIRS_DIFFUSEMATERIALSOURCE);
		LOG_CONST_CASE(D3DDDIRS_SPECULARMATERIALSOURCE);
		LOG_CONST_CASE(D3DDDIRS_AMBIENTMATERIALSOURCE);
		LOG_CONST_CASE(D3DDDIRS_EMISSIVEMATERIALSOURCE);
		LOG_CONST_CASE(D3DDDIRS_VERTEXBLEND);
		LOG_CONST_CASE(D3DDDIRS_CLIPPLANEENABLE);
		LOG_CONST_CASE(D3DDDIRS_SOFTWAREVERTEXPROCESSING);
		LOG_CONST_CASE(D3DDDIRS_POINTSIZE);
		LOG_CONST_CASE(D3DDDIRS_POINTSIZE_MIN);
		LOG_CONST_CASE(D3DDDIRS_POINTSPRITEENABLE);
		LOG_CONST_CASE(D3DDDIRS_POINTSCALEENABLE);
		LOG_CONST_CASE(D3DDDIRS_POINTSCALE_A);
		LOG_CONST_CASE(D3DDDIRS_POINTSCALE_B);
		LOG_CONST_CASE(D3DDDIRS_POINTSCALE_C);
		LOG_CONST_CASE(D3DDDIRS_MULTISAMPLEANTIALIAS);
		LOG_CONST_CASE(D3DDDIRS_MULTISAMPLEMASK);
		LOG_CONST_CASE(D3DDDIRS_PATCHEDGESTYLE);
		LOG_CONST_CASE(D3DDDIRS_PATCHSEGMENTS);
		LOG_CONST_CASE(D3DDDIRS_DEBUGMONITORTOKEN);
		LOG_CONST_CASE(D3DDDIRS_POINTSIZE_MAX);
		LOG_CONST_CASE(D3DDDIRS_INDEXEDVERTEXBLENDENABLE);
		LOG_CONST_CASE(D3DDDIRS_COLORWRITEENABLE);
		LOG_CONST_CASE(D3DDDIRS_DELETERTPATCH);
		LOG_CONST_CASE(D3DDDIRS_TWEENFACTOR);
		LOG_CONST_CASE(D3DDDIRS_BLENDOP);
		LOG_CONST_CASE(D3DDDIRS_POSITIONDEGREE);
		LOG_CONST_CASE(D3DDDIRS_NORMALDEGREE);
		LOG_CONST_CASE(D3DDDIRS_SCISSORTESTENABLE);
		LOG_CONST_CASE(D3DDDIRS_SLOPESCALEDEPTHBIAS);
		LOG_CONST_CASE(D3DDDIRS_ANTIALIASEDLINEENABLE);
		LOG_CONST_CASE(D3DDDIRS_MINTESSELLATIONLEVEL);
		LOG_CONST_CASE(D3DDDIRS_MAXTESSELLATIONLEVEL);
		LOG_CONST_CASE(D3DDDIRS_ADAPTIVETESS_X);
		LOG_CONST_CASE(D3DDDIRS_ADAPTIVETESS_Y);
		LOG_CONST_CASE(D3DDDIRS_ADAPTIVETESS_Z);
		LOG_CONST_CASE(D3DDDIRS_ADAPTIVETESS_W);
		LOG_CONST_CASE(D3DDDIRS_ENABLEADAPTIVETESSELLATION);
		LOG_CONST_CASE(D3DDDIRS_TWOSIDEDSTENCILMODE);
		LOG_CONST_CASE(D3DDDIRS_CCW_STENCILFAIL);
		LOG_CONST_CASE(D3DDDIRS_CCW_STENCILZFAIL);
		LOG_CONST_CASE(D3DDDIRS_CCW_STENCILPASS);
		LOG_CONST_CASE(D3DDDIRS_CCW_STENCILFUNC);
		LOG_CONST_CASE(D3DDDIRS_COLORWRITEENABLE1);
		LOG_CONST_CASE(D3DDDIRS_COLORWRITEENABLE2);
		LOG_CONST_CASE(D3DDDIRS_COLORWRITEENABLE3);
		LOG_CONST_CASE(D3DDDIRS_BLENDFACTOR);
		LOG_CONST_CASE(D3DDDIRS_SRGBWRITEENABLE);
		LOG_CONST_CASE(D3DDDIRS_DEPTHBIAS);
		LOG_CONST_CASE(D3DDDIRS_WRAP8);
		LOG_CONST_CASE(D3DDDIRS_WRAP9);
		LOG_CONST_CASE(D3DDDIRS_WRAP10);
		LOG_CONST_CASE(D3DDDIRS_WRAP11);
		LOG_CONST_CASE(D3DDDIRS_WRAP12);
		LOG_CONST_CASE(D3DDDIRS_WRAP13);
		LOG_CONST_CASE(D3DDDIRS_WRAP14);
		LOG_CONST_CASE(D3DDDIRS_WRAP15);
		LOG_CONST_CASE(D3DDDIRS_SEPARATEALPHABLENDENABLE);
		LOG_CONST_CASE(D3DDDIRS_SRCBLENDALPHA);
		LOG_CONST_CASE(D3DDDIRS_DESTBLENDALPHA);
		LOG_CONST_CASE(D3DDDIRS_BLENDOPALPHA);
	}

	return os << "D3DDDIRS_" << static_cast<DWORD>(val);
}

std::ostream& operator<<(std::ostream& os, D3DDDITEXTURESTAGESTATETYPE val)
{
	switch (val)
	{
		LOG_CONST_CASE(D3DDDITSS_TEXTUREMAP);
		LOG_CONST_CASE(D3DDDITSS_COLOROP);
		LOG_CONST_CASE(D3DDDITSS_COLORARG1);
		LOG_CONST_CASE(D3DDDITSS_COLORARG2);
		LOG_CONST_CASE(D3DDDITSS_ALPHAOP);
		LOG_CONST_CASE(D3DDDITSS_ALPHAARG1);
		LOG_CONST_CASE(D3DDDITSS_ALPHAARG2);
		LOG_CONST_CASE(D3DDDITSS_BUMPENVMAT00);
		LOG_CONST_CASE(D3DDDITSS_BUMPENVMAT01);
		LOG_CONST_CASE(D3DDDITSS_BUMPENVMAT10);
		LOG_CONST_CASE(D3DDDITSS_BUMPENVMAT11);
		LOG_CONST_CASE(D3DDDITSS_TEXCOORDINDEX);
		LOG_CONST_CASE(D3DDDITSS_ADDRESSU);
		LOG_CONST_CASE(D3DDDITSS_ADDRESSV);
		LOG_CONST_CASE(D3DDDITSS_BORDERCOLOR);
		LOG_CONST_CASE(D3DDDITSS_MAGFILTER);
		LOG_CONST_CASE(D3DDDITSS_MINFILTER);
		LOG_CONST_CASE(D3DDDITSS_MIPFILTER);
		LOG_CONST_CASE(D3DDDITSS_MIPMAPLODBIAS);
		LOG_CONST_CASE(D3DDDITSS_MAXMIPLEVEL);
		LOG_CONST_CASE(D3DDDITSS_MAXANISOTROPY);
		LOG_CONST_CASE(D3DDDITSS_BUMPENVLSCALE);
		LOG_CONST_CASE(D3DDDITSS_BUMPENVLOFFSET);
		LOG_CONST_CASE(D3DDDITSS_TEXTURETRANSFORMFLAGS);
		LOG_CONST_CASE(D3DDDITSS_ADDRESSW);
		LOG_CONST_CASE(D3DDDITSS_COLORARG0);
		LOG_CONST_CASE(D3DDDITSS_ALPHAARG0);
		LOG_CONST_CASE(D3DDDITSS_RESULTARG);
		LOG_CONST_CASE(D3DDDITSS_SRGBTEXTURE);
		LOG_CONST_CASE(D3DDDITSS_ELEMENTINDEX);
		LOG_CONST_CASE(D3DDDITSS_DMAPOFFSET);
		LOG_CONST_CASE(D3DDDITSS_CONSTANT);
		LOG_CONST_CASE(D3DDDITSS_DISABLETEXTURECOLORKEY);
		LOG_CONST_CASE(D3DDDITSS_TEXTURECOLORKEYVAL);
	}

	return os << "D3DDDITSS_" << static_cast<DWORD>(val);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIVERTEXELEMENT& val)
{
	return Compat30::LogStruct(os)
		<< val.Stream
		<< val.Offset
		<< static_cast<UINT>(val.Type)
		<< static_cast<UINT>(val.Method)
		<< static_cast<UINT>(val.Usage)
		<< static_cast<UINT>(val.UsageIndex);
}
