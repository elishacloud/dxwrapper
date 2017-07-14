#include "d3d8to9.hpp"

AddressLookupTable::AddressLookupTable(Direct3DDevice8 *Device) :
	Device(Device)
{
	// Do nothing
}

AddressLookupTable::~AddressLookupTable()
{
	for (int x = 0; x < SizeOfVector; x++)
	{
		while (AddressVector[x].size() > 0)
		{
			if (AddressVector[x].back().Address8)
			{
				switch (x)
				{
				case SurfaceVector:
					reinterpret_cast<Direct3DSurface8*>(AddressVector[x].back().Address8)->DeleteMe(false);
					break;
				case TextureVector:
					reinterpret_cast<Direct3DTexture8*>(AddressVector[x].back().Address8)->DeleteMe(false);
					break;
				case VolumeTextureVector:
					reinterpret_cast<Direct3DVolumeTexture8*>(AddressVector[x].back().Address8)->DeleteMe(false);
					break;
				case CubeTextureVector:
					reinterpret_cast<Direct3DCubeTexture8*>(AddressVector[x].back().Address8)->DeleteMe(false);
					break;
				case VolumeVector:
					reinterpret_cast<Direct3DVolume8*>(AddressVector[x].back().Address8)->DeleteMe(false);
					break;
				case VertexBufferVector:
					reinterpret_cast<Direct3DVertexBuffer8*>(AddressVector[x].back().Address8)->DeleteMe(false);
					break;
				case IndexBufferVector:
					reinterpret_cast<Direct3DIndexBuffer8*>(AddressVector[x].back().Address8)->DeleteMe(false);
					break;
				case SwapChainVector:
					reinterpret_cast<Direct3DSwapChain8*>(AddressVector[x].back().Address8)->DeleteMe(false);
					break;
				}
			}
			AddressVector[x].pop_back();
		}
		AddressVector[x].clear();
	}
}


// Function templates
void* AddressLookupTable::FindAddress(void* pAddress9, DWORD VectorNum)
{
	if (pAddress9)
	{
		for (UINT i = 0; i < AddressVector[VectorNum].size(); i++)
		{
			if (AddressVector[VectorNum][i].Address9 == pAddress9)
			{
				return AddressVector[VectorNum][i].Address8;
			}
		}
	}
	return nullptr;
}

void AddressLookupTable::SaveAddress(void* pAddress8, void* pAddress9, DWORD VectorNum)
{
	if (pAddress8 && pAddress8)
	{
		AddrStruct MyNewVector;
		MyNewVector.Address8 = pAddress8;
		MyNewVector.Address9 = pAddress9;
		AddressVector[VectorNum].push_back(MyNewVector);
	}
}

void AddressLookupTable::DeleteAddress(void* pAddress8, DWORD VectorNum)
{
	if (pAddress8)
	{
		for (UINT i = 0; i < AddressVector[VectorNum].size(); i++)
		{
			if (AddressVector[VectorNum][i].Address8 == pAddress8)
			{
				std::swap(AddressVector[VectorNum][i], AddressVector[VectorNum].back());
				AddressVector[VectorNum].pop_back();
				return;
			}
		}
	}
}


// Direct3DSurface8
Direct3DSurface8* AddressLookupTable::FindAddress(IDirect3DSurface9* pSurface9)
{
	Direct3DSurface8* pSurface8 = nullptr;
	if (pSurface9)
	{
		pSurface8 = reinterpret_cast<Direct3DSurface8*>(FindAddress(reinterpret_cast<void*>(pSurface9), SurfaceVector));
		if (pSurface8 == nullptr)
		{
			pSurface8 = new Direct3DSurface8(Device, pSurface9);
		}
	}
	return pSurface8;
}
void AddressLookupTable::SaveAddress(Direct3DSurface8* pSurface8, IDirect3DSurface9* pSurface9)
{
	SaveAddress(reinterpret_cast<void*>(pSurface8), reinterpret_cast<void*>(pSurface9), SurfaceVector);
}
void AddressLookupTable::DeleteAddress(Direct3DSurface8* pSurface8)
{
	DeleteAddress(reinterpret_cast<void*>(pSurface8), SurfaceVector);
}


// Direct3DTexture8
Direct3DTexture8* AddressLookupTable::FindAddress(IDirect3DTexture9* pTexture9)
{
	Direct3DTexture8* pTexture8 = nullptr;
	if (pTexture9)
	{
		pTexture8 = reinterpret_cast<Direct3DTexture8*>(FindAddress(reinterpret_cast<void*>(pTexture9), TextureVector));
		if (pTexture8 == nullptr)
		{
			pTexture8 = new Direct3DTexture8(Device, pTexture9);
		}
	}
	return pTexture8;
}
void AddressLookupTable::SaveAddress(Direct3DTexture8* pTexture8, IDirect3DTexture9* pTexture9)
{
	SaveAddress(reinterpret_cast<void*>(pTexture8), reinterpret_cast<void*>(pTexture9), TextureVector);
}
void AddressLookupTable::DeleteAddress(Direct3DTexture8* pTexture8)
{
	DeleteAddress(reinterpret_cast<void*>(pTexture8), TextureVector);
}


// Direct3DVolumeTexture8
Direct3DVolumeTexture8* AddressLookupTable::FindAddress(IDirect3DVolumeTexture9* pVolumeTexture9)
{
	Direct3DVolumeTexture8* pVolumeTexture8 = nullptr;
	if (pVolumeTexture9)
	{
		pVolumeTexture8 = reinterpret_cast<Direct3DVolumeTexture8*>(FindAddress(reinterpret_cast<void*>(pVolumeTexture9), VolumeTextureVector));
		if (pVolumeTexture8 == nullptr)
		{
			pVolumeTexture8 = new Direct3DVolumeTexture8(Device, pVolumeTexture9);
		}
	}
	return pVolumeTexture8;
}
void AddressLookupTable::SaveAddress(Direct3DVolumeTexture8* pVolumeTexture8, IDirect3DVolumeTexture9* pVolumeTexture9)
{
	SaveAddress(reinterpret_cast<void*>(pVolumeTexture8), reinterpret_cast<void*>(pVolumeTexture9), VolumeTextureVector);
}
void AddressLookupTable::DeleteAddress(Direct3DVolumeTexture8* pVolumeTexture8)
{
	DeleteAddress(reinterpret_cast<void*>(pVolumeTexture8), VolumeTextureVector);
}


// Direct3DCubeTexture8
Direct3DCubeTexture8* AddressLookupTable::FindAddress(IDirect3DCubeTexture9* pCubeTexture9)
{
	Direct3DCubeTexture8* pCubeTexture8 = nullptr;
	if (pCubeTexture9)
	{
		pCubeTexture8 = reinterpret_cast<Direct3DCubeTexture8*>(FindAddress(reinterpret_cast<void*>(pCubeTexture9), CubeTextureVector));
		if (pCubeTexture8 == nullptr)
		{
			pCubeTexture8 = new Direct3DCubeTexture8(Device, pCubeTexture9);
		}
	}
	return pCubeTexture8;
}
void AddressLookupTable::SaveAddress(Direct3DCubeTexture8* pCubeTexture8, IDirect3DCubeTexture9* pCubeTexture9)
{
	SaveAddress(reinterpret_cast<void*>(pCubeTexture8), reinterpret_cast<void*>(pCubeTexture9), CubeTextureVector);
}
void AddressLookupTable::DeleteAddress(Direct3DCubeTexture8* pCubeTexture8)
{
	DeleteAddress(reinterpret_cast<void*>(pCubeTexture8), CubeTextureVector);
}


// Direct3DVolume8
Direct3DVolume8* AddressLookupTable::FindAddress(IDirect3DVolume9* pVolume9)
{
	Direct3DVolume8* pVolume8 = nullptr;
	if (pVolume9)
	{
		pVolume8 = reinterpret_cast<Direct3DVolume8*>(FindAddress(reinterpret_cast<void*>(pVolume9), VolumeVector));
		if (pVolume8 == nullptr)
		{
			pVolume8 = new Direct3DVolume8(Device, pVolume9);
		}
	}
	return pVolume8;
}
void AddressLookupTable::SaveAddress(Direct3DVolume8* pVolume8, IDirect3DVolume9* pVolume9)
{
	SaveAddress(reinterpret_cast<void*>(pVolume8), reinterpret_cast<void*>(pVolume9), VolumeVector);
}
void AddressLookupTable::DeleteAddress(Direct3DVolume8* pVolume8)
{
	DeleteAddress(reinterpret_cast<void*>(pVolume8), VolumeVector);
}


// Direct3DVertexBuffer8
Direct3DVertexBuffer8* AddressLookupTable::FindAddress(IDirect3DVertexBuffer9* pVertexBuffer9)
{
	Direct3DVertexBuffer8* pVertexBuffer8 = nullptr;
	if (pVertexBuffer9)
	{
		pVertexBuffer8 = reinterpret_cast<Direct3DVertexBuffer8*>(FindAddress(reinterpret_cast<void*>(pVertexBuffer9), VertexBufferVector));
		if (pVertexBuffer8 == nullptr)
		{
			pVertexBuffer8 = new Direct3DVertexBuffer8(Device, pVertexBuffer9);
		}
	}
	return pVertexBuffer8;
}
void AddressLookupTable::SaveAddress(Direct3DVertexBuffer8* pVertexBuffer8, IDirect3DVertexBuffer9* pVertexBuffer9)
{
	SaveAddress(reinterpret_cast<void*>(pVertexBuffer8), reinterpret_cast<void*>(pVertexBuffer9), VertexBufferVector);
}
void AddressLookupTable::DeleteAddress(Direct3DVertexBuffer8* pVertexBuffer8)
{
	DeleteAddress(reinterpret_cast<void*>(pVertexBuffer8), VertexBufferVector);
}


// Direct3DIndexBuffer8
Direct3DIndexBuffer8* AddressLookupTable::FindAddress(IDirect3DIndexBuffer9* pIndexBuffer9)
{
	Direct3DIndexBuffer8* pIndexBuffer8 = nullptr;
	if (pIndexBuffer9)
	{
		pIndexBuffer8 = reinterpret_cast<Direct3DIndexBuffer8*>(FindAddress(reinterpret_cast<void*>(pIndexBuffer9), IndexBufferVector));
		if (pIndexBuffer8 == nullptr)
		{
			pIndexBuffer8 = new Direct3DIndexBuffer8(Device, pIndexBuffer9);
		}
	}
	return pIndexBuffer8;
}
void AddressLookupTable::SaveAddress(Direct3DIndexBuffer8* pIndexBuffer8, IDirect3DIndexBuffer9* pIndexBuffer9)
{
	SaveAddress(reinterpret_cast<void*>(pIndexBuffer8), reinterpret_cast<void*>(pIndexBuffer9), IndexBufferVector);
}
void AddressLookupTable::DeleteAddress(Direct3DIndexBuffer8* pIndexBuffer8)
{
	DeleteAddress(reinterpret_cast<void*>(pIndexBuffer8), IndexBufferVector);
}


// Direct3DSwapChain8
Direct3DSwapChain8* AddressLookupTable::FindAddress(IDirect3DSwapChain9* pSwapChain9)
{
	Direct3DSwapChain8* pSwapChain8 = nullptr;
	if (pSwapChain9)
	{
		pSwapChain8 = reinterpret_cast<Direct3DSwapChain8*>(FindAddress(reinterpret_cast<void*>(pSwapChain9), SwapChainVector));
		if (pSwapChain8 == nullptr)
		{
			pSwapChain8 = new Direct3DSwapChain8(Device, pSwapChain9);
		}
	}
	return pSwapChain8;
}
void AddressLookupTable::SaveAddress(Direct3DSwapChain8* pSwapChain8, IDirect3DSwapChain9* pSwapChain9)
{
	SaveAddress(reinterpret_cast<void*>(pSwapChain8), reinterpret_cast<void*>(pSwapChain9), SwapChainVector);
}
void AddressLookupTable::DeleteAddress(Direct3DSwapChain8* pSwapChain8)
{
	DeleteAddress(reinterpret_cast<void*>(pSwapChain8), SwapChainVector);
}
