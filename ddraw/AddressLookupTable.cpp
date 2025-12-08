/**
* Copyright (C) 2025 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "ddraw.h"

// Define the static member for the template
template <typename T>
std::vector<T*> AddressInterfaceList<T>::InterfaceList;

// Explicitly instantiate for the types
template std::vector<m_IDirectDraw*> AddressInterfaceList<m_IDirectDraw>::InterfaceList;
template std::vector<m_IDirectDraw2*> AddressInterfaceList<m_IDirectDraw2>::InterfaceList;
template std::vector<m_IDirectDraw3*> AddressInterfaceList<m_IDirectDraw3>::InterfaceList;
template std::vector<m_IDirectDraw4*> AddressInterfaceList<m_IDirectDraw4>::InterfaceList;
template std::vector<m_IDirectDraw7*> AddressInterfaceList<m_IDirectDraw7>::InterfaceList;
template std::vector<m_IDirectDrawSurface*> AddressInterfaceList<m_IDirectDrawSurface>::InterfaceList;
template std::vector<m_IDirectDrawSurface2*> AddressInterfaceList<m_IDirectDrawSurface2>::InterfaceList;
template std::vector<m_IDirectDrawSurface3*> AddressInterfaceList<m_IDirectDrawSurface3>::InterfaceList;
template std::vector<m_IDirectDrawSurface4*> AddressInterfaceList<m_IDirectDrawSurface4>::InterfaceList;
template std::vector<m_IDirectDrawSurface7*> AddressInterfaceList<m_IDirectDrawSurface7>::InterfaceList;
template std::vector<m_IDirectDrawPalette*> AddressInterfaceList<m_IDirectDrawPalette>::InterfaceList;
template std::vector<m_IDirectDrawClipper*> AddressInterfaceList<m_IDirectDrawClipper>::InterfaceList;
template std::vector<m_IDirectDrawColorControl*> AddressInterfaceList<m_IDirectDrawColorControl>::InterfaceList;
template std::vector<m_IDirectDrawGammaControl*> AddressInterfaceList<m_IDirectDrawGammaControl>::InterfaceList;
template std::vector<m_IDirect3D*> AddressInterfaceList<m_IDirect3D>::InterfaceList;
template std::vector<m_IDirect3D2*> AddressInterfaceList<m_IDirect3D2>::InterfaceList;
template std::vector<m_IDirect3D3*> AddressInterfaceList<m_IDirect3D3>::InterfaceList;
template std::vector<m_IDirect3D7*> AddressInterfaceList<m_IDirect3D7>::InterfaceList;
template std::vector<m_IDirect3DDevice*> AddressInterfaceList<m_IDirect3DDevice>::InterfaceList;
template std::vector<m_IDirect3DDevice2*> AddressInterfaceList<m_IDirect3DDevice2>::InterfaceList;
template std::vector<m_IDirect3DDevice3*> AddressInterfaceList<m_IDirect3DDevice3>::InterfaceList;
template std::vector<m_IDirect3DDevice7*> AddressInterfaceList<m_IDirect3DDevice7>::InterfaceList;
template std::vector<m_IDirect3DMaterial*> AddressInterfaceList<m_IDirect3DMaterial>::InterfaceList;
template std::vector<m_IDirect3DMaterial2*> AddressInterfaceList<m_IDirect3DMaterial2>::InterfaceList;
template std::vector<m_IDirect3DMaterial3*> AddressInterfaceList<m_IDirect3DMaterial3>::InterfaceList;
template std::vector<m_IDirect3DTexture*> AddressInterfaceList<m_IDirect3DTexture>::InterfaceList;
template std::vector<m_IDirect3DTexture2*> AddressInterfaceList<m_IDirect3DTexture2>::InterfaceList;
template std::vector<m_IDirect3DVertexBuffer*> AddressInterfaceList<m_IDirect3DVertexBuffer>::InterfaceList;
template std::vector<m_IDirect3DVertexBuffer7*> AddressInterfaceList<m_IDirect3DVertexBuffer7>::InterfaceList;
template std::vector<m_IDirect3DViewport*> AddressInterfaceList<m_IDirect3DViewport>::InterfaceList;
template std::vector<m_IDirect3DViewport2*> AddressInterfaceList<m_IDirect3DViewport2>::InterfaceList;
template std::vector<m_IDirect3DViewport3*> AddressInterfaceList<m_IDirect3DViewport3>::InterfaceList;
template std::vector<m_IDirect3DExecuteBuffer*> AddressInterfaceList<m_IDirect3DExecuteBuffer>::InterfaceList;
template std::vector<m_IDirect3DLight*> AddressInterfaceList<m_IDirect3DLight>::InterfaceList;
