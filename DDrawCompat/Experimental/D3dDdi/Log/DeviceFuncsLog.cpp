#include "DDrawLog.h"
#include "D3dDdi/Log/DeviceFuncsLog.h"

std::ostream& operator<<(std::ostream& os, const D3DDDI_RATIONAL& val)
{
	return Compat::LogStruct(os)
		<< val.Numerator
		<< val.Denominator;
}

std::ostream& operator<<(std::ostream& os, const D3DDDI_SURFACEINFO& val)
{
	return Compat::LogStruct(os)
		<< val.Width
		<< val.Height
		<< val.Depth
		<< val.pSysMem
		<< val.SysMemPitch
		<< val.SysMemSlicePitch;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CLEAR& val)
{
	return Compat::LogStruct(os)
		<< Compat::hex(val.Flags)
		<< Compat::hex(val.FillColor)
		<< val.FillDepth
		<< Compat::hex(val.FillStencil);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_COLORFILL& val)
{
	return Compat::LogStruct(os)
		<< val.hResource
		<< val.SubResourceIndex
		<< val.DstRect
		<< Compat::hex(val.Color)
		<< Compat::hex(val.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATERESOURCE& val)
{
	return Compat::LogStruct(os)
		<< val.Format
		<< val.Pool
		<< val.MultisampleType
		<< val.MultisampleQuality
		<< Compat::array(val.pSurfList, val.SurfCount)
		<< val.SurfCount
		<< val.MipLevels
		<< val.Fvf
		<< val.VidPnSourceId
		<< val.RefreshRate
		<< val.hResource
		<< Compat::hex(val.Flags.Value)
		<< val.Rotation;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_CREATERESOURCE2& val)
{
	return Compat::LogStruct(os)
		<< val.Format
		<< val.Pool
		<< val.MultisampleType
		<< val.MultisampleQuality
		<< Compat::array(val.pSurfList, val.SurfCount)
		<< val.SurfCount
		<< val.MipLevels
		<< val.Fvf
		<< val.VidPnSourceId
		<< val.RefreshRate
		<< val.hResource
		<< Compat::hex(val.Flags.Value)
		<< val.Rotation
		<< Compat::hex(val.Flags2.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_LOCK& val)
{
	return Compat::LogStruct(os)
		<< val.hResource
		<< val.SubResourceIndex
		<< val.Box
		<< val.pSurfData
		<< val.Pitch
		<< val.SlicePitch
		<< Compat::hex(val.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_OPENRESOURCE& val)
{
	return Compat::LogStruct(os)
		<< val.NumAllocations
		<< Compat::array(val.pOpenAllocationInfo, val.NumAllocations)
		<< Compat::hex(val.hKMResource)
		<< val.pPrivateDriverData
		<< val.PrivateDriverDataSize
		<< val.hResource
		<< val.Rotation
		<< Compat::hex(val.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_PRESENT& val)
{
	return Compat::LogStruct(os)
		<< val.hSrcResource
		<< val.SrcSubResourceIndex
		<< val.hDstResource
		<< val.DstSubResourceIndex
		<< Compat::hex(val.Flags.Value)
		<< val.FlipInterval;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_PRESENT1& val)
{
	return Compat::LogStruct(os)
		<< Compat::array(val.phSrcResources, val.SrcResources)
		<< val.SrcResources
		<< val.hDstResource
		<< val.DstSubResourceIndex
		<< Compat::hex(val.Flags.Value)
		<< val.FlipInterval
		<< val.Reserved
		<< Compat::array(val.pDirtyRects, val.DirtyRects)
		<< val.DirtyRects;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_PRESENTSURFACE& val)
{
	return Compat::LogStruct(os)
		<< val.hResource
		<< val.SubResourceIndex;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_RENDERSTATE& val)
{
	return Compat::LogStruct(os)
		<< val.State
		<< val.Value;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_UNLOCK& val)
{
	return Compat::LogStruct(os)
		<< val.hResource
		<< val.SubResourceIndex
		<< Compat::hex(val.Flags.Value);
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_WINFO& val)
{
	return Compat::LogStruct(os)
		<< val.WNear
		<< val.WFar;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIARG_ZRANGE& val)
{
	return Compat::LogStruct(os)
		<< val.MinZ
		<< val.MaxZ;
}

std::ostream& operator<<(std::ostream& os, const D3DDDIBOX& box)
{
	return Compat::LogStruct(os)
		<< box.Left
		<< box.Top
		<< box.Right
		<< box.Bottom
		<< box.Front
		<< box.Back;
}
