#pragma once

class m_IDirect3DExecuteBuffer : public IDirect3DExecuteBuffer, public AddressLookupTableDdrawObject
{
private:
	IDirect3DExecuteBuffer *ProxyInterface = nullptr;
	REFIID WrapperID = IID_IDirect3DExecuteBuffer;
	ULONG RefCount = 1;

	// Convert Buffer
	m_IDirect3DDeviceX *D3DDeviceInterface = nullptr;
	D3DEXECUTEBUFFERDESC Desc = {};
	std::vector<BYTE> MemoryData;
	D3DEXECUTEDATA ExecuteData = {};
	bool IsLocked = false;
	bool IsDataValidated = false;

	// Instruction data 
	HRESULT ValidateInstructionData(LPD3DEXECUTEDATA lpExecuteData, LPDWORD lpdwOffset, LPD3DVALIDATECALLBACK lpFunc, LPVOID lpUserArg);

	// Interface initialization functions
	void InitInterface(LPD3DEXECUTEBUFFERDESC lpDesc);
	void ReleaseInterface();

public:
	m_IDirect3DExecuteBuffer(IDirect3DExecuteBuffer *aOriginal) : ProxyInterface(aOriginal)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		if (Config.Dd7to9)
		{
			Logging::Log() << __FUNCTION__ << " (" << this << ") Warning: created from non-dd7to9 interface!";
		}

		InitInterface(nullptr);

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	m_IDirect3DExecuteBuffer(m_IDirect3DDeviceX *D3DDInterface, LPD3DEXECUTEBUFFERDESC lpDesc) : D3DDeviceInterface(D3DDInterface)
	{
		LOG_LIMIT(3, "Creating interface " << __FUNCTION__ << " (" << this << ")");

		InitInterface(lpDesc);

		ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
	}
	~m_IDirect3DExecuteBuffer()
	{
		LOG_LIMIT(3, __FUNCTION__ << " (" << this << ")" << " deleting interface!");

		ReleaseInterface();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	void SetProxy(IDirect3DExecuteBuffer* NewProxyInterface, m_IDirect3DDeviceX* NewD3DDInterface, LPD3DEXECUTEBUFFERDESC lpDesc)
	{
		if (NewProxyInterface || NewD3DDInterface)
		{
			RefCount = 1;
			ProxyInterface = NewProxyInterface;
			D3DDeviceInterface = NewD3DDInterface;
			InitInterface(lpDesc);
			ProxyAddressLookupTable.SaveAddress(this, (ProxyInterface) ? ProxyInterface : (void*)this);
		}
		else
		{
			ReleaseInterface();
			ProxyAddressLookupTable.DeleteAddress(this);
			ProxyInterface = nullptr;
			D3DDeviceInterface = nullptr;
		}
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DExecuteBuffer methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3DDEVICE, LPD3DEXECUTEBUFFERDESC);
	STDMETHOD(Lock)(THIS_ LPD3DEXECUTEBUFFERDESC);
	STDMETHOD(Unlock)(THIS);
	STDMETHOD(SetExecuteData)(THIS_ LPD3DEXECUTEDATA);
	STDMETHOD(GetExecuteData)(THIS_ LPD3DEXECUTEDATA);
	STDMETHOD(Validate)(THIS_ LPDWORD, LPD3DVALIDATECALLBACK, LPVOID, DWORD);
	STDMETHOD(Optimize)(THIS_ DWORD);

	// Helper functions
	void ClearD3DDevice() { D3DDeviceInterface = nullptr; }
	HRESULT GetBuffer(LPVOID* lplpData, D3DEXECUTEDATA& CurrentExecuteData, LPD3DSTATUS* lplpStatus);
	static m_IDirect3DExecuteBuffer* CreateDirect3DExecuteBuffer(IDirect3DExecuteBuffer* aOriginal, m_IDirect3DDeviceX* NewD3DDInterface, LPD3DEXECUTEBUFFERDESC lpDesc);
};
