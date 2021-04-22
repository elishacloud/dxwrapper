#pragma once

#include <type_traits>

#include <d3d.h>

#include <Common/CompatVtable.h>

namespace Compat30
{
	template <typename Intf>
	struct GetBaseIntf
	{
		typedef Intf Type;
	};

	template <typename SrcIntf, typename DestIntf>
	struct IsConvertible : std::integral_constant<bool,
		std::is_same<typename GetBaseIntf<SrcIntf>::Type, typename GetBaseIntf<DestIntf>::Type>::value>
	{
	};

	template <typename SrcIntf> struct IsConvertible<SrcIntf, IUnknown> : std::true_type {};
	template <typename DestIntf> struct IsConvertible<IUnknown, DestIntf> : std::true_type {};

	template<> struct IsConvertible<IDirect3D, IDirect3D7> : std::false_type {};
	template<> struct IsConvertible<IDirect3D2, IDirect3D7> : std::false_type {};
	template<> struct IsConvertible<IDirect3D3, IDirect3D7> : std::false_type {};

	template<> struct IsConvertible<IDirect3D7, IDirect3D> : std::false_type {};
	template<> struct IsConvertible<IDirect3D7, IDirect3D2> : std::false_type {};
	template<> struct IsConvertible<IDirect3D7, IDirect3D3> : std::false_type {};

#define DEFINE_BASE_INTF(Intf, BaseIntf) \
	template<> struct GetBaseIntf<Intf> { typedef BaseIntf Type; }

	DEFINE_BASE_INTF(IDirectDraw2, IDirectDraw);
	DEFINE_BASE_INTF(IDirectDraw4, IDirectDraw);
	DEFINE_BASE_INTF(IDirectDraw7, IDirectDraw);
	DEFINE_BASE_INTF(IDirectDrawSurface2, IDirectDrawSurface);
	DEFINE_BASE_INTF(IDirectDrawSurface3, IDirectDrawSurface);
	DEFINE_BASE_INTF(IDirectDrawSurface4, IDirectDrawSurface);
	DEFINE_BASE_INTF(IDirectDrawSurface7, IDirectDrawSurface);
	DEFINE_BASE_INTF(IDirectDrawGammaControl, IDirectDrawSurface);

	DEFINE_BASE_INTF(IDirect3D, IDirectDraw);
	DEFINE_BASE_INTF(IDirect3D2, IDirectDraw);
	DEFINE_BASE_INTF(IDirect3D3, IDirectDraw);
	DEFINE_BASE_INTF(IDirect3D7, IDirectDraw);
	DEFINE_BASE_INTF(IDirect3DDevice2, IDirect3DDevice);
	DEFINE_BASE_INTF(IDirect3DDevice3, IDirect3DDevice);
	DEFINE_BASE_INTF(IDirect3DMaterial2, IDirect3DMaterial);
	DEFINE_BASE_INTF(IDirect3DMaterial3, IDirect3DMaterial);
	DEFINE_BASE_INTF(IDirect3DTexture, IDirectDrawSurface);
	DEFINE_BASE_INTF(IDirect3DTexture2, IDirectDrawSurface);
	DEFINE_BASE_INTF(IDirect3DViewport2, IDirect3DViewport);
	DEFINE_BASE_INTF(IDirect3DViewport3, IDirect3DViewport);

#undef DEFINE_BASE_INTF

	template <typename Intf>
	const IID& getIntfId();
	
#define DEFINE_INTF_ID(Intf) \
	template<> inline const IID& getIntfId<Intf>() { return IID_##Intf; }

	DEFINE_INTF_ID(IUnknown);

	DEFINE_INTF_ID(IDirectDraw);
	DEFINE_INTF_ID(IDirectDraw2);
	DEFINE_INTF_ID(IDirectDraw4);
	DEFINE_INTF_ID(IDirectDraw7);
	DEFINE_INTF_ID(IDirectDrawSurface);
	DEFINE_INTF_ID(IDirectDrawSurface2);
	DEFINE_INTF_ID(IDirectDrawSurface3);
	DEFINE_INTF_ID(IDirectDrawSurface4);
	DEFINE_INTF_ID(IDirectDrawSurface7);
	DEFINE_INTF_ID(IDirectDrawPalette);
	DEFINE_INTF_ID(IDirectDrawClipper);
	DEFINE_INTF_ID(IDirectDrawColorControl);
	DEFINE_INTF_ID(IDirectDrawGammaControl);

	DEFINE_INTF_ID(IDirect3D);
	DEFINE_INTF_ID(IDirect3D2);
	DEFINE_INTF_ID(IDirect3D3);
	DEFINE_INTF_ID(IDirect3D7);
	DEFINE_INTF_ID(IDirect3DDevice);
	DEFINE_INTF_ID(IDirect3DDevice2);
	DEFINE_INTF_ID(IDirect3DDevice3);
	DEFINE_INTF_ID(IDirect3DDevice7);
	DEFINE_INTF_ID(IDirect3DMaterial);
	DEFINE_INTF_ID(IDirect3DMaterial2);
	DEFINE_INTF_ID(IDirect3DMaterial3);
	DEFINE_INTF_ID(IDirect3DTexture);
	DEFINE_INTF_ID(IDirect3DTexture2);
	DEFINE_INTF_ID(IDirect3DViewport);
	DEFINE_INTF_ID(IDirect3DViewport2);
	DEFINE_INTF_ID(IDirect3DViewport3);

#undef DEFINE_INTF_ID

	template <typename NewIntf, typename OrigIntf>
	std::enable_if_t<IsConvertible<OrigIntf, NewIntf>::value>
	queryInterface(OrigIntf& origIntf, NewIntf*& newIntf)
	{
		getOrigVtable(&origIntf).QueryInterface(&origIntf, getIntfId<NewIntf>(), reinterpret_cast<void**>(&newIntf));
	}

	template <typename Intf>
	void queryInterface(Intf& origIntf, Intf*& newIntf)
	{
		newIntf = &origIntf;
		getOrigVtable(newIntf).AddRef(newIntf);
	}

	template <typename NewIntf, typename OrigIntf>
	NewIntf* queryInterface(OrigIntf* origIntf)
	{
		if (!origIntf)
		{
			return nullptr;
		}

		NewIntf* newIntf = nullptr;
		queryInterface(*origIntf, newIntf);
		return newIntf;
	}
}
