#pragma once

#include <iostream>
#include <vector>

class AddressLookupTable
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

	void SaveAddress(void*, void*, const DWORD);
	void* FindAddress(void*, const DWORD);
	void DeleteAddress(void*, const DWORD);

	Direct3DDevice8 *const Device;

public:
	AddressLookupTable(Direct3DDevice8 *Device);
	~AddressLookupTable();

	// Direct3DSurface8
	void SaveAddress(Direct3DSurface8*, IDirect3DSurface9*);
	Direct3DSurface8* FindAddress(IDirect3DSurface9*);
	void DeleteAddress(Direct3DSurface8*);

	// Direct3DTexture8
	void SaveAddress(Direct3DTexture8*, IDirect3DTexture9*);
	Direct3DTexture8* FindAddress(IDirect3DTexture9*);
	void DeleteAddress(Direct3DTexture8*);

	// Direct3DVolumeTexture8
	void SaveAddress(Direct3DVolumeTexture8*, IDirect3DVolumeTexture9*);
	Direct3DVolumeTexture8* FindAddress(IDirect3DVolumeTexture9*);
	void DeleteAddress(Direct3DVolumeTexture8*);

	// Direct3DCubeTexture8
	void SaveAddress(Direct3DCubeTexture8*, IDirect3DCubeTexture9*);
	Direct3DCubeTexture8* FindAddress(IDirect3DCubeTexture9*);
	void DeleteAddress(Direct3DCubeTexture8*);

	// Direct3DVolume8
	void SaveAddress(Direct3DVolume8*, IDirect3DVolume9*);
	Direct3DVolume8* FindAddress(IDirect3DVolume9*);
	void DeleteAddress(Direct3DVolume8*);

	// Direct3DVertexBuffer8
	void SaveAddress(Direct3DVertexBuffer8*, IDirect3DVertexBuffer9*);
	Direct3DVertexBuffer8* FindAddress(IDirect3DVertexBuffer9*);
	void DeleteAddress(Direct3DVertexBuffer8*);

	// Direct3DIndexBuffer8
	void SaveAddress(Direct3DIndexBuffer8*, IDirect3DIndexBuffer9*);
	Direct3DIndexBuffer8* FindAddress(IDirect3DIndexBuffer9*);
	void DeleteAddress(Direct3DIndexBuffer8*);

	// Direct3DSwapChain8
	void SaveAddress(Direct3DSwapChain8*, IDirect3DSwapChain9*);
	Direct3DSwapChain8* FindAddress(IDirect3DSwapChain9*);
	void DeleteAddress(Direct3DSwapChain8*);
};

class AddressLookupTableObject
{
public:
	virtual ~AddressLookupTableObject() { }

	void DeleteMe(bool CleanUp = true)
	{
		CleanUpFlag = CleanUp;
		delete this;
	}

protected:
	bool CleanUpFlag = true;
};