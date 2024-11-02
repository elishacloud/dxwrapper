#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.2/D3dDdi/Device.h>
#include <DDrawCompat/v0.3.2/D3dDdi/Resource.h>
#include <DDrawCompat/v0.3.2/D3dDdi/ScopedCriticalSection.h>
#include <DDrawCompat/v0.3.2/DDraw/RealPrimarySurface.h>
#include <DDrawCompat/v0.3.2/DDraw/Surfaces/PrimarySurface.h>
#include <DDrawCompat/v0.3.2/Gdi/CompatDc.h>
#include <DDrawCompat/v0.3.2/Gdi/Dc.h>

namespace Gdi
{
	CompatDc::CompatDc(HDC dc, bool isReadOnly)
		: m_origDc(dc)
		, m_compatDc(Gdi::Dc::getDc(dc))
		, m_isReadOnly(isReadOnly)
	{
		if (m_compatDc)
		{
			D3dDdi::ScopedCriticalSection lock;
			auto gdiResource = D3dDdi::Device::getGdiResource();
			if (gdiResource)
			{
				gdiResource->prepareForGdiRendering(isReadOnly);
			}
		}
		else
		{
			m_compatDc = m_origDc;
		}
	}

	CompatDc::~CompatDc()
	{
		if (m_compatDc != m_origDc)
		{
			D3dDdi::ScopedCriticalSection lock;
			auto gdiResource = D3dDdi::Device::getGdiResource();
			if (!m_isReadOnly && (!gdiResource || DDraw::PrimarySurface::getFrontResource() == *gdiResource))
			{
				DDraw::RealPrimarySurface::scheduleUpdate();
			}
			Gdi::Dc::releaseDc(m_origDc);
		}
	}
}
