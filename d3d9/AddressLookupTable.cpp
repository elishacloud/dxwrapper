#include "d3d9.h"
#include "Libraries\ComPtr.h"

// Specializations for CreateInterface methods
template <>
m_IDirect3DCubeTexture9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DCubeTexture9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DCubeTexture9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DCubeTexture9(static_cast<m_IDirect3DCubeTexture9*>(Proxy), Device);
}

template <>
m_IDirect3DIndexBuffer9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DIndexBuffer9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DIndexBuffer9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DIndexBuffer9(static_cast<m_IDirect3DIndexBuffer9*>(Proxy), Device);
}

template <>
m_IDirect3DPixelShader9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DPixelShader9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DPixelShader9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DPixelShader9(static_cast<m_IDirect3DPixelShader9*>(Proxy), Device);
}

template <>
m_IDirect3DQuery9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DQuery9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DQuery9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DQuery9(static_cast<m_IDirect3DQuery9*>(Proxy), Device);
}

template <>
m_IDirect3DStateBlock9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DStateBlock9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DStateBlock9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DStateBlock9(static_cast<m_IDirect3DStateBlock9*>(Proxy), Device);
}

template <>
m_IDirect3DSurface9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DSurface9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DSurface9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DSurface9(static_cast<m_IDirect3DSurface9*>(Proxy), Device);
}

template <>
m_IDirect3DSwapChain9Ex* AddressLookupTableD3d9::CreateInterface<m_IDirect3DSwapChain9Ex, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DSwapChain9Ex* Proxy, m_IDirect3DDevice9Ex* Device, REFIID riid, void*)
{
	return new m_IDirect3DSwapChain9Ex(static_cast<m_IDirect3DSwapChain9Ex*>(Proxy), Device, riid);
}

template <>
m_IDirect3DTexture9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DTexture9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DTexture9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DTexture9(static_cast<m_IDirect3DTexture9*>(Proxy), Device);
}

template <>
m_IDirect3DVertexBuffer9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVertexBuffer9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVertexBuffer9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVertexBuffer9(static_cast<m_IDirect3DVertexBuffer9*>(Proxy), Device);
}

template <>
m_IDirect3DVertexDeclaration9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVertexDeclaration9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVertexDeclaration9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVertexDeclaration9(static_cast<m_IDirect3DVertexDeclaration9*>(Proxy), Device);
}

template <>
m_IDirect3DVertexShader9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVertexShader9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVertexShader9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVertexShader9(static_cast<m_IDirect3DVertexShader9*>(Proxy), Device);
}

template <>
m_IDirect3DVolume9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVolume9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVolume9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVolume9(static_cast<m_IDirect3DVolume9*>(Proxy), Device);
}

template <>
m_IDirect3DVolumeTexture9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVolumeTexture9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVolumeTexture9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVolumeTexture9(static_cast<m_IDirect3DVolumeTexture9*>(Proxy), Device);
}

template <>
m_IDirect3DVideoDevice9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DVideoDevice9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DVideoDevice9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DVideoDevice9(static_cast<m_IDirect3DVideoDevice9*>(Proxy), Device);
}

template <>
m_IDirect3DDXVADevice9* AddressLookupTableD3d9::CreateInterface<m_IDirect3DDXVADevice9, m_IDirect3DDevice9Ex, LPVOID>(
	m_IDirect3DDXVADevice9* Proxy, m_IDirect3DDevice9Ex* Device, REFIID, void*)
{
	return new m_IDirect3DDXVADevice9(static_cast<m_IDirect3DDXVADevice9*>(Proxy), Device);
}

StateBlockCache::~StateBlockCache()
{
	while (stateBlocks.size())
	{
		m_IDirect3DStateBlock9* StateBlockX = stateBlocks.back();
		RemoveStateBlock(StateBlockX);
		delete StateBlockX;
	}
}

void StateBlockCache::AddStateBlock(m_IDirect3DStateBlock9* stateBlock)
{
	if (stateBlock == nullptr) return;

	// Check if the state block already exists
	auto it = std::find(stateBlocks.begin(), stateBlocks.end(), stateBlock);
	if (it == stateBlocks.end())
	{
		stateBlocks.push_back(stateBlock);

		// If we exceed the max allowed state blocks, remove the oldest ones
		if (Config.LimitStateBlocks && stateBlocks.size() > MAX_STATE_BLOCKS)
		{
			m_IDirect3DStateBlock9* StateBlockX = stateBlocks.front();
			StateBlockX->Release();
		}
	}
}

void StateBlockCache::RemoveStateBlock(m_IDirect3DStateBlock9* stateBlock)
{
	auto it = std::find(stateBlocks.begin(), stateBlocks.end(), stateBlock);
	if (it != stateBlocks.end())
	{
		stateBlocks.erase(it);
	}
}
