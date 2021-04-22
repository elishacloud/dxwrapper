#pragma once

#include <ddraw.h>

namespace DDraw
{
	struct Types1
	{
		typedef IDirectDrawSurface TSurface;
		typedef IDirectDrawSurface TCreatedSurface;
		typedef DDSURFACEDESC TSurfaceDesc;
		typedef DDSCAPS TDdsCaps;
		typedef LPDDENUMSURFACESCALLBACK TEnumSurfacesCallbackPtr;
		typedef LPVOID TUnlockParam;
	};

	struct Types2
	{
		typedef IDirectDrawSurface2 TSurface;
		typedef IDirectDrawSurface TCreatedSurface;
		typedef DDSURFACEDESC TSurfaceDesc;
		typedef DDSCAPS TDdsCaps;
		typedef LPDDENUMSURFACESCALLBACK TEnumSurfacesCallbackPtr;
		typedef LPVOID TUnlockParam;
	};

	struct Types3
	{
		typedef IDirectDrawSurface3 TSurface;
		typedef IDirectDrawSurface3 TCreatedSurface;
		typedef DDSURFACEDESC TSurfaceDesc;
		typedef DDSCAPS TDdsCaps;
		typedef LPDDENUMSURFACESCALLBACK TEnumSurfacesCallbackPtr;
		typedef LPVOID TUnlockParam;
	};

	struct Types4
	{
		typedef IDirectDrawSurface4 TSurface;
		typedef IDirectDrawSurface4 TCreatedSurface;
		typedef DDSURFACEDESC2 TSurfaceDesc;
		typedef DDSCAPS2 TDdsCaps;
		typedef LPDDENUMSURFACESCALLBACK2 TEnumSurfacesCallbackPtr;
		typedef LPRECT TUnlockParam;
	};

	struct Types7
	{
		typedef IDirectDrawSurface7 TSurface;
		typedef IDirectDrawSurface7 TCreatedSurface;
		typedef DDSURFACEDESC2 TSurfaceDesc;
		typedef DDSCAPS2 TDdsCaps;
		typedef LPDDENUMSURFACESCALLBACK7 TEnumSurfacesCallbackPtr;
		typedef LPRECT TUnlockParam;
	};

	template <typename Interface>
	struct Types;

	template <> struct Types<IDirectDraw> : Types1 {};
	template <> struct Types<IDirectDraw2> : Types2 {};
	template <> struct Types<IDirectDraw4> : Types4 {};
	template <> struct Types<IDirectDraw7> : Types7 {};

	template <> struct Types<IDirectDrawSurface> : Types1 {};
	template <> struct Types<IDirectDrawSurface2> : Types2 {};
	template <> struct Types<IDirectDrawSurface3> : Types3 {};
	template <> struct Types<IDirectDrawSurface4> : Types4 {};
	template <> struct Types<IDirectDrawSurface7> : Types7 {};
}
