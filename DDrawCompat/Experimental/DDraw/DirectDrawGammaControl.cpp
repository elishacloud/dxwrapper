#include "DDraw/DirectDrawGammaControl.h"
#include "DDraw/RealPrimarySurface.h"
#include "DDraw/Surfaces/PrimarySurface.h"

namespace
{
	bool isPrimaryGamma(IDirectDrawGammaControl* gamma)
	{
		return CompatPtr<IDirectDrawSurface7>::from(gamma) == DDraw::PrimarySurface::getPrimary();
	}

	HRESULT STDMETHODCALLTYPE getGammaRamp(
		IDirectDrawGammaControl* This, DWORD dwFlags, LPDDGAMMARAMP lpRampData)
	{
		if (0 != dwFlags || !lpRampData || !isPrimaryGamma(This))
		{
			return DDraw::DirectDrawGammaControl::s_origVtable.GetGammaRamp(This, dwFlags, lpRampData);
		}

		return DDraw::RealPrimarySurface::getGammaRamp(lpRampData);
	}

	HRESULT STDMETHODCALLTYPE setGammaRamp(
		IDirectDrawGammaControl* This, DWORD dwFlags, LPDDGAMMARAMP lpRampData)
	{
		if ((0 != dwFlags && DDSGR_CALIBRATE != dwFlags) || !isPrimaryGamma(This))
		{
			return DDraw::DirectDrawGammaControl::s_origVtable.SetGammaRamp(This, dwFlags, lpRampData);
		}

		return DDraw::RealPrimarySurface::setGammaRamp(lpRampData);
	}
}

namespace DDraw
{
	void DirectDrawGammaControl::setCompatVtable(IDirectDrawGammaControlVtbl& vtable)
	{
		vtable.GetGammaRamp = &getGammaRamp;
		vtable.SetGammaRamp = &setGammaRamp;
	}
}
