#include "d3d8to9.hpp"

Direct3DCache::Direct3DCache()
{
}

Direct3DCache::~Direct3DCache()
{
	// Direct3DSurface8
	while (SurfaceVector.size() > 0)
	{
		Direct3DSurface8* MySurface8 = SurfaceVector.back().Surface8;
		SurfaceVector.pop_back();
		if (MySurface8)
		{
			delete MySurface8;
		}
	}
	SurfaceVector.clear();

	// Direct3DTexture8
	while (TextureVector.size() > 0)
	{
		Direct3DTexture8* MyTexture8 = TextureVector.back().Texture8;
		TextureVector.pop_back();
		if (MyTexture8)
		{
			delete MyTexture8;
		}
	}
	TextureVector.clear();

	// Direct3DVolumeTexture8
	while (VolumeTextureVector.size() > 0)
	{
		Direct3DVolumeTexture8* MyVolumeTexture8 = VolumeTextureVector.back().Texture8;
		VolumeTextureVector.pop_back();
		if (MyVolumeTexture8)
		{
			delete MyVolumeTexture8;
		}
	}
	VolumeTextureVector.clear();

	// Direct3DCubeTexture8
	while (CubeTextureVector.size() > 0)
	{
		Direct3DCubeTexture8* MyCubeTexture8 = CubeTextureVector.back().Texture8;
		CubeTextureVector.pop_back();
		if (MyCubeTexture8)
		{
			delete MyCubeTexture8;
		}
	}
	CubeTextureVector.clear();

	// Direct3DVolume8
	while (VolumeVector.size() > 0)
	{
		Direct3DVolume8* MyVolume8 = VolumeVector.back().Volume8;
		VolumeVector.pop_back();
		if (MyVolume8)
		{
			delete MyVolume8;
		}
	}
	VolumeVector.clear();

	// Direct3DVertexBuffer8
	while (VertexBufferVector.size() > 0)
	{
		Direct3DVertexBuffer8* MyVertexBuffer8 = VertexBufferVector.back().VertexBuffer8;
		VertexBufferVector.pop_back();
		if (MyVertexBuffer8)
		{
			delete MyVertexBuffer8;
		}
	}
	VertexBufferVector.clear();

	// Direct3DIndexBuffer8
	while (IndexBufferVector.size() > 0)
	{
		Direct3DIndexBuffer8* MyIndexBuffer8 = IndexBufferVector.back().IndexBuffer8;
		IndexBufferVector.pop_back();
		if (MyIndexBuffer8)
		{
			delete MyIndexBuffer8;
		}
	}
	IndexBufferVector.clear();
}


// Direct3DSurface8
Direct3DSurface8* Direct3DCache::GetDirect3D(IDirect3DSurface9* pSurface9)
{
	if (pSurface9 == nullptr)
	{
		return nullptr;
	}

	for (UINT i = 0; i < SurfaceVector.size(); i++)
	{
		if (SurfaceVector[i].Surface9 == pSurface9)
		{
			return SurfaceVector[i].Surface8;
		}
	}
	return nullptr;
}

void Direct3DCache::SetDirect3D(Direct3DSurface8* pSurface8, IDirect3DSurface9* pSurface9)
{
	if (pSurface8 && pSurface9)
	{
		Direct3DSurfaceInfo MySurface;
		MySurface.Surface8 = pSurface8;
		MySurface.Surface9 = pSurface9;
		SurfaceVector.push_back(MySurface);
	}
}

void Direct3DCache::DeleteDirect3D(Direct3DSurface8* pSurface8)
{
	if (pSurface8)
	{
		Direct3DSurfaceInfo MySurface;
		for (UINT i = 0; i < SurfaceVector.size(); i++)
		{
			MySurface = SurfaceVector[i];
			if (MySurface.Surface8 == pSurface8)
			{
				std::swap(SurfaceVector[i], SurfaceVector.back());
				SurfaceVector.pop_back();
				return;
			}
		}
	}
}


// Direct3DTexture8
Direct3DTexture8* Direct3DCache::GetDirect3D(IDirect3DTexture9* pTexture9)
{
	if (pTexture9 == nullptr)
	{
		return nullptr;
	}

	for (UINT i = 0; i < TextureVector.size(); i++)
	{
		if (TextureVector[i].Texture9 == pTexture9)
		{
			return TextureVector[i].Texture8;
		}
	}
	return nullptr;
}

void Direct3DCache::SetDirect3D(Direct3DTexture8* pTexture8, IDirect3DTexture9* pTexture9)
{
	if (pTexture8 && pTexture9)
	{
		Direct3DTextureInfo MyTexture;
		MyTexture.Texture8 = pTexture8;
		MyTexture.Texture9 = pTexture9;
		TextureVector.push_back(MyTexture);
	}
}

void Direct3DCache::DeleteDirect3D(Direct3DTexture8* pTexture8)
{
	if (pTexture8)
	{
		Direct3DTextureInfo MyTexture;
		for (UINT i = 0; i < TextureVector.size(); i++)
		{
			MyTexture = TextureVector[i];
			if (MyTexture.Texture8 == pTexture8)
			{
				std::swap(TextureVector[i], TextureVector.back());
				TextureVector.pop_back();
				return;
			}
		}
	}
}


// Direct3DVolumeTexture8
Direct3DVolumeTexture8* Direct3DCache::GetDirect3D(IDirect3DVolumeTexture9* pVolumeTexture9)
{
	if (pVolumeTexture9 == nullptr)
	{
		return nullptr;
	}

	for (UINT i = 0; i < VolumeTextureVector.size(); i++)
	{
		if (VolumeTextureVector[i].Texture9 == pVolumeTexture9)
		{
			return VolumeTextureVector[i].Texture8;
		}
	}
	return nullptr;
}

void Direct3DCache::SetDirect3D(Direct3DVolumeTexture8* pVolumeTexture8, IDirect3DVolumeTexture9* pVolumeTexture9)
{
	if (pVolumeTexture8 && pVolumeTexture9)
	{
		Direct3DVolumeTextureInfo MyVolumeTexture;
		MyVolumeTexture.Texture8 = pVolumeTexture8;
		MyVolumeTexture.Texture9 = pVolumeTexture9;
		VolumeTextureVector.push_back(MyVolumeTexture);
	}
}

void Direct3DCache::DeleteDirect3D(Direct3DVolumeTexture8* pVolumeTexture8)
{
	if (pVolumeTexture8)
	{
		Direct3DVolumeTextureInfo MyVolumeTexture;
		for (UINT i = 0; i < VolumeTextureVector.size(); i++)
		{
			MyVolumeTexture = VolumeTextureVector[i];
			if (MyVolumeTexture.Texture8 == pVolumeTexture8)
			{
				std::swap(VolumeTextureVector[i], VolumeTextureVector.back());
				VolumeTextureVector.pop_back();
				return;
			}
		}
	}
}


// Direct3DCubeTexture8
Direct3DCubeTexture8* Direct3DCache::GetDirect3D(IDirect3DCubeTexture9* pCubeTexture9)
{
	if (pCubeTexture9 == nullptr)
	{
		return nullptr;
	}

	for (UINT i = 0; i < CubeTextureVector.size(); i++)
	{
		if (CubeTextureVector[i].Texture9 == pCubeTexture9)
		{
			return CubeTextureVector[i].Texture8;
		}
	}
	return nullptr;
}

void Direct3DCache::SetDirect3D(Direct3DCubeTexture8* pCubeTexture8, IDirect3DCubeTexture9* pCubeTexture9)
{
	if (pCubeTexture8 && pCubeTexture9)
	{
		Direct3DCubeTextureInfo MyCubeTexture;
		MyCubeTexture.Texture8 = pCubeTexture8;
		MyCubeTexture.Texture9 = pCubeTexture9;
		CubeTextureVector.push_back(MyCubeTexture);
	}
}

void Direct3DCache::DeleteDirect3D(Direct3DCubeTexture8* pCubeTexture8)
{
	if (pCubeTexture8)
	{
		Direct3DCubeTextureInfo MyCubeTexture;
		for (UINT i = 0; i < CubeTextureVector.size(); i++)
		{
			MyCubeTexture = CubeTextureVector[i];
			if (MyCubeTexture.Texture8 == pCubeTexture8)
			{
				std::swap(CubeTextureVector[i], CubeTextureVector.back());
				CubeTextureVector.pop_back();
				return;
			}
		}
	}
}


// Direct3DVolume8
Direct3DVolume8* Direct3DCache::GetDirect3D(IDirect3DVolume9* pVolume9)
{
	if (pVolume9 == nullptr)
	{
		return nullptr;
	}

	for (UINT i = 0; i < VolumeVector.size(); i++)
	{
		if (VolumeVector[i].Volume9 == pVolume9)
		{
			return VolumeVector[i].Volume8;
		}
	}
	return nullptr;
}

void Direct3DCache::SetDirect3D(Direct3DVolume8* pVolume8, IDirect3DVolume9* pVolume9)
{
	if (pVolume8 && pVolume9)
	{
		Direct3DVolumeInfo MyVolume;
		MyVolume.Volume8 = pVolume8;
		MyVolume.Volume9 = pVolume9;
		VolumeVector.push_back(MyVolume);
	}
}

void Direct3DCache::DeleteDirect3D(Direct3DVolume8* pVolume8)
{
	if (pVolume8)
	{
		Direct3DVolumeInfo MyVolume;
		for (UINT i = 0; i < VolumeVector.size(); i++)
		{
			MyVolume = VolumeVector[i];
			if (MyVolume.Volume8 == pVolume8)
			{
				std::swap(VolumeVector[i], VolumeVector.back());
				VolumeVector.pop_back();
				return;
			}
		}
	}
}


// Direct3DVertexBuffer8
Direct3DVertexBuffer8* Direct3DCache::GetDirect3D(IDirect3DVertexBuffer9* pVertexBuffer9)
{
	if (pVertexBuffer9 == nullptr)
	{
		return nullptr;
	}

	for (UINT i = 0; i < VertexBufferVector.size(); i++)
	{
		if (VertexBufferVector[i].VertexBuffer9 == pVertexBuffer9)
		{
			return VertexBufferVector[i].VertexBuffer8;
		}
	}
	return nullptr;
}

void Direct3DCache::SetDirect3D(Direct3DVertexBuffer8* pVertexBuffer8, IDirect3DVertexBuffer9* pVertexBuffer9)
{
	if (pVertexBuffer8 && pVertexBuffer9)
	{
		Direct3DVertexBufferInfo MyVertexBuffer;
		MyVertexBuffer.VertexBuffer8 = pVertexBuffer8;
		MyVertexBuffer.VertexBuffer9 = pVertexBuffer9;
		VertexBufferVector.push_back(MyVertexBuffer);
	}
}

void Direct3DCache::DeleteDirect3D(Direct3DVertexBuffer8* pVertexBuffer8)
{
	if (pVertexBuffer8)
	{
		Direct3DVertexBufferInfo MyVertexBuffer;
		for (UINT i = 0; i < VertexBufferVector.size(); i++)
		{
			MyVertexBuffer = VertexBufferVector[i];
			if (MyVertexBuffer.VertexBuffer8 == pVertexBuffer8)
			{
				std::swap(VertexBufferVector[i], VertexBufferVector.back());
				VertexBufferVector.pop_back();
				return;
			}
		}
	}
}


// Direct3DIndexBuffer8
Direct3DIndexBuffer8* Direct3DCache::GetDirect3D(IDirect3DIndexBuffer9* pIndexBuffer9)
{
	if (pIndexBuffer9 == nullptr)
	{
		return nullptr;
	}

	for (UINT i = 0; i < IndexBufferVector.size(); i++)
	{
		if (IndexBufferVector[i].IndexBuffer9 == pIndexBuffer9)
		{
			return IndexBufferVector[i].IndexBuffer8;
		}
	}
	return nullptr;
}

void Direct3DCache::SetDirect3D(Direct3DIndexBuffer8* pIndexBuffer8, IDirect3DIndexBuffer9* pIndexBuffer9)
{
	if (pIndexBuffer8 && pIndexBuffer9)
	{
		Direct3DIndexBufferInfo MyIndexBuffer;
		MyIndexBuffer.IndexBuffer8 = pIndexBuffer8;
		MyIndexBuffer.IndexBuffer9 = pIndexBuffer9;
		IndexBufferVector.push_back(MyIndexBuffer);
	}
}

void Direct3DCache::DeleteDirect3D(Direct3DIndexBuffer8* pIndexBuffer8)
{
	if (pIndexBuffer8)
	{
		Direct3DIndexBufferInfo MyIndexBuffer;
		for (UINT i = 0; i < IndexBufferVector.size(); i++)
		{
			MyIndexBuffer = IndexBufferVector[i];
			if (MyIndexBuffer.IndexBuffer8 == pIndexBuffer8)
			{
				std::swap(IndexBufferVector[i], IndexBufferVector.back());
				IndexBufferVector.pop_back();
				return;
			}
		}
	}
}
