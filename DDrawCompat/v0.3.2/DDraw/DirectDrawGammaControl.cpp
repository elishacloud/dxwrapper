#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.2/Common/CompatVtable.h>
#include <DDrawCompat/v0.3.2/DDraw/DirectDrawGammaControl.h>
#include <DDrawCompat/v0.3.2/DDraw/RealPrimarySurface.h>
#include <DDrawCompat/v0.3.2/DDraw/ScopedThreadLock.h>
#include <DDrawCompat/v0.3.2/DDraw/Surfaces/PrimarySurface.h>
#include <DDrawCompat/v0.3.2/DDraw/Visitors/DirectDrawGammaControlVtblVisitor.h>

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
