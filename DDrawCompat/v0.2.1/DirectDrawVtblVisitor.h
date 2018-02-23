#pragma once

#include "DDrawVtableVisitor.h"

namespace Compat21
{
	template <>
	struct DDrawVtableForEach<IDirectDrawVtbl>
	{
		template <typename Vtable, typename Visitor>
		static void forEach(Visitor& visitor)
		{
			DDrawVtableForEach<IUnknownVtbl>::forEach<Vtable>(visitor);

			DD_VISIT(Compact);
			DD_VISIT(CreateClipper);
			DD_VISIT(CreatePalette);
			DD_VISIT(CreateSurface);
			DD_VISIT(DuplicateSurface);
			DD_VISIT(EnumDisplayModes);
			DD_VISIT(EnumSurfaces);
			DD_VISIT(FlipToGDISurface);
			DD_VISIT(GetCaps);
			DD_VISIT(GetDisplayMode);
			DD_VISIT(GetFourCCCodes);
			DD_VISIT(GetGDISurface);
			DD_VISIT(GetMonitorFrequency);
			DD_VISIT(GetScanLine);
			DD_VISIT(GetVerticalBlankStatus);
			DD_VISIT(Initialize);
			DD_VISIT(RestoreDisplayMode);
			DD_VISIT(SetCooperativeLevel);
			DD_VISIT(SetDisplayMode);
			DD_VISIT(WaitForVerticalBlank);
		}
	};

	template <>
	struct DDrawVtableForEach<IDirectDraw2Vtbl>
	{
		template <typename Vtable, typename Visitor>
		static void forEach(Visitor& visitor)
		{
			DDrawVtableForEach<IDirectDrawVtbl>::forEach<Vtable>(visitor);

			DD_VISIT(GetAvailableVidMem);
		}
	};

	template <>
	struct DDrawVtableForEach<IDirectDraw4Vtbl>
	{
		template <typename Vtable, typename Visitor>
		static void forEach(Visitor& visitor)
		{
			DDrawVtableForEach<IDirectDraw2Vtbl>::forEach<Vtable>(visitor);

			DD_VISIT(GetSurfaceFromDC);
			DD_VISIT(RestoreAllSurfaces);
			DD_VISIT(TestCooperativeLevel);
			DD_VISIT(GetDeviceIdentifier);
		}
	};

	template <>
	struct DDrawVtableForEach<IDirectDraw7Vtbl>
	{
		template <typename Vtable, typename Visitor>
		static void forEach(Visitor& visitor)
		{
			DDrawVtableForEach<IDirectDraw4Vtbl>::forEach<Vtable>(visitor);

			DD_VISIT(StartModeTest);
			DD_VISIT(EvaluateMode);
		}
	};
}
