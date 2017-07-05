#pragma once

#include <iostream>
#include <vector>

class Direct3DCache
{
private:
	struct Direct3DSurfaceInfo
	{
		Direct3DSurface8* Surface8 = nullptr;
		IDirect3DSurface9* Surface9 = nullptr;
	};

	struct Direct3DTextureInfo
	{
		Direct3DTexture8* Texture8 = nullptr;
		IDirect3DTexture9* Texture9 = nullptr;
	};

	struct Direct3DVolumeTextureInfo
	{
		Direct3DVolumeTexture8* Texture8 = nullptr;
		IDirect3DVolumeTexture9* Texture9 = nullptr;
	};

	struct Direct3DCubeTextureInfo
	{
		Direct3DCubeTexture8* Texture8 = nullptr;
		IDirect3DCubeTexture9* Texture9 = nullptr;
	};

	struct Direct3DVolumeInfo
	{
		Direct3DVolume8* Volume8 = nullptr;
		IDirect3DVolume9* Volume9 = nullptr;
	};

	struct Direct3DVertexBufferInfo
	{
		Direct3DVertexBuffer8* VertexBuffer8 = nullptr;
		IDirect3DVertexBuffer9* VertexBuffer9 = nullptr;
	};

	struct Direct3DIndexBufferInfo
	{
		Direct3DIndexBuffer8* IndexBuffer8 = nullptr;
		IDirect3DIndexBuffer9* IndexBuffer9 = nullptr;
	};

	std::vector<Direct3DSurfaceInfo> SurfaceVector;
	std::vector<Direct3DTextureInfo> TextureVector;
	std::vector<Direct3DVolumeTextureInfo> VolumeTextureVector;
	std::vector<Direct3DCubeTextureInfo> CubeTextureVector;
	std::vector<Direct3DVolumeInfo> VolumeVector;
	std::vector<Direct3DVertexBufferInfo> VertexBufferVector;
	std::vector<Direct3DIndexBufferInfo> IndexBufferVector;
public:
	Direct3DCache();
	~Direct3DCache();

	// Direct3DSurface8
	void SetDirect3D(Direct3DSurface8*, IDirect3DSurface9*);
	Direct3DSurface8* GetDirect3D(IDirect3DSurface9*);
	void DeleteDirect3D(Direct3DSurface8*);

	// Direct3DTexture8
	void SetDirect3D(Direct3DTexture8*, IDirect3DTexture9*);
	Direct3DTexture8* GetDirect3D(IDirect3DTexture9*);
	void DeleteDirect3D(Direct3DTexture8*);

	// Direct3DVolumeTexture8
	void SetDirect3D(Direct3DVolumeTexture8*, IDirect3DVolumeTexture9*);
	Direct3DVolumeTexture8* GetDirect3D(IDirect3DVolumeTexture9*);
	void DeleteDirect3D(Direct3DVolumeTexture8*);

	// Direct3DCubeTexture8
	void SetDirect3D(Direct3DCubeTexture8*, IDirect3DCubeTexture9*);
	Direct3DCubeTexture8* GetDirect3D(IDirect3DCubeTexture9*);
	void DeleteDirect3D(Direct3DCubeTexture8*);

	// Direct3DVolume8
	void SetDirect3D(Direct3DVolume8*, IDirect3DVolume9*);
	Direct3DVolume8* GetDirect3D(IDirect3DVolume9*);
	void DeleteDirect3D(Direct3DVolume8*);

	// Direct3DVertexBuffer8
	void SetDirect3D(Direct3DVertexBuffer8*, IDirect3DVertexBuffer9*);
	Direct3DVertexBuffer8* GetDirect3D(IDirect3DVertexBuffer9*);
	void DeleteDirect3D(Direct3DVertexBuffer8*);

	// Direct3DIndexBuffer8
	void SetDirect3D(Direct3DIndexBuffer8*, IDirect3DIndexBuffer9*);
	Direct3DIndexBuffer8* GetDirect3D(IDirect3DIndexBuffer9*);
	void DeleteDirect3D(Direct3DIndexBuffer8*);
};