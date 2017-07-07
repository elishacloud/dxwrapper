#pragma once

#include <iostream>
#include <vector>

class Direct3DCache
{
private:
	struct AddrStruct
	{
		void* Address8 = nullptr;
		void* Address9 = nullptr;
	};

	static constexpr DWORD SurfaceVector = 0;
	static constexpr DWORD TextureVector = 1;
	static constexpr DWORD CubeTextureVector = 2;
	static constexpr DWORD VolumeTextureVector = 3;
	static constexpr DWORD VolumeVector = 4;
	static constexpr DWORD VertexBufferVector = 5;
	static constexpr DWORD IndexBufferVector = 6;
	static constexpr DWORD SwapChainVector = 7;

	static constexpr DWORD SizeOfVector = 8;

	std::vector<AddrStruct> AddressVector[SizeOfVector];

	void SetDirect3D(void*, void*, const DWORD);
	void* GetDirect3D(void*, const DWORD);
	void DeleteDirect3D(void*, const DWORD);

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

	// Direct3DSwapChain8
	void SetDirect3D(Direct3DSwapChain8*, IDirect3DSwapChain9*);
	Direct3DSwapChain8* GetDirect3D(IDirect3DSwapChain9*);
	void DeleteDirect3D(Direct3DSwapChain8*);
};
