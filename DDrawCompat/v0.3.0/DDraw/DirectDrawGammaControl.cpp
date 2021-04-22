#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <Common/CompatVtable.h>
#include <DDraw/DirectDrawGammaControl.h>
#include <DDraw/RealPrimarySurface.h>
#include <DDraw/ScopedThreadLock.h>
#include <DDraw/Surfaces/PrimarySurface.h>
#include <DDraw/Visitors/DirectDrawGammaControlVtblVisitor.h>

namespace
{
	bool isPrimaryGamma(IDirectDrawGammaControl* gamma)
	{
		return CompatPtr<IDirectDrawSurface7>::from(gamma) == DDraw::PrimarySurface::getPrimary();
	}

	HRESULT STDMETHODCALLTYPE GetGammaRamp(
		IDirectDrawGammaControl* This, DWORD dwFlags, LPDDGAMMARAMP lpRampData)
	{
		if (0 != dwFlags || !lpRampData || !isPrimaryGamma(This))
		{
			return getOrigVtable(This).GetGammaRamp(This, dwFlags, lpRampData);
		}

		return DDraw::RealPrimarySurface::getGammaRamp(lpRampData);
	}

	HRESULT STDMETHODCALLTYPE SetGammaRamp(
		IDirectDrawGammaControl* This, DWORD dwFlags, LPDDGAMMARAMP lpRampData)
	{
		if ((0 != dwFlags && DDSGR_CALIBRATE != dwFlags) || !isPrimaryGamma(This))
		{
			return getOrigVtable(This).SetGammaRamp(This, dwFlags, lpRampData);
		}

		return DDraw::RealPrimarySurface::setGammaRamp(lpRampData);
	}

	constexpr void setCompatVtable(IDirectDrawGammaControlVtbl& vtable)
	{
		vtable.GetGammaRamp = &GetGammaRamp;
		vtable.SetGammaRamp = &SetGammaRamp;
	}
}

namespace DDraw
{
	namespace DirectDrawGammaControl
	{
		void hookVtable(const IDirectDrawGammaControlVtbl& vtable)
		{
			CompatVtable<IDirectDrawGammaControlVtbl>::hookVtable<ScopedThreadLock>(vtable);
		}
	}
}
