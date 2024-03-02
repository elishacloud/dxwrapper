#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/DDrawLog.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Device.h>
#include <DDrawCompat/v0.3.1/D3dDdi/DeviceState.h>
#include <DDrawCompat/v0.3.1/D3dDdi/DrawPrimitive.h>
#include <DDrawCompat/v0.3.1/D3dDdi/Log/DeviceFuncsLog.h>

namespace
{
	const UINT UNINITIALIZED_STATE = 0xBAADBAAD;
	const HANDLE UNINITIALIZED_HANDLE = reinterpret_cast<HANDLE>(0xBAADBAAD);

	bool operator==(const D3DDDIARG_ZRANGE& lhs, const D3DDDIARG_ZRANGE& rhs)
	{
		return lhs.MinZ == rhs.MinZ && lhs.MaxZ == rhs.MaxZ;
	}

	bool operator==(const D3DDDIARG_WINFO& lhs, const D3DDDIARG_WINFO& rhs)
	{
		return lhs.WNear == rhs.WNear && lhs.WFar == rhs.WFar;
	}
}

namespace D3dDdi
{
	DeviceState::DeviceState(Device& device)
		: m_device(device)
		, m_pixelShader(UNINITIALIZED_HANDLE)
		, m_vertexShaderDecl(UNINITIALIZED_HANDLE)
		, m_vertexShaderFunc(UNINITIALIZED_HANDLE)
		, m_wInfo{ NAN, NAN }
		, m_zRange{ NAN, NAN }
	{
		m_renderState.fill(UNINITIALIZED_STATE);
		m_textures.fill(UNINITIALIZED_HANDLE);
		for (UINT i = 0; i < m_textureStageState.size(); ++i)
		{
			m_textureStageState[i].fill(UNINITIALIZED_STATE);
		}
	}

	HRESULT DeviceState::pfnCreateVertexShaderDecl(
		D3DDDIARG_CREATEVERTEXSHADERDECL* data,
		const D3DDDIVERTEXELEMENT* vertexElements)
	{
		LOG_DEBUG << Compat32::array(vertexElements, data->NumVertexElements);
		HRESULT result = m_device.getOrigVtable().pfnCreateVertexShaderDecl(m_device, data, vertexElements);
		if (SUCCEEDED(result))
		{
			m_vertexShaderDecls[data->ShaderHandle].assign(vertexElements, vertexElements + data->NumVertexElements);
		}
		return result;
	}

	HRESULT DeviceState::pfnDeletePixelShader(HANDLE shader)
	{
		return deleteShader(shader, m_pixelShader, m_device.getOrigVtable().pfnDeletePixelShader);
	}

	HRESULT DeviceState::pfnDeleteVertexShaderDecl(HANDLE shader)
	{
		const bool isCurrentShader = shader == m_vertexShaderDecl;
		HRESULT result = deleteShader(shader, m_vertexShaderDecl, m_device.getOrigVtable().pfnDeleteVertexShaderDecl);
		if (SUCCEEDED(result))
		{
			m_vertexShaderDecls.erase(shader);
			if (isCurrentShader)
			{
				m_device.getDrawPrimitive().setVertexShaderDecl({});
			}
		}
		return result;
	}

	HRESULT DeviceState::pfnDeleteVertexShaderFunc(HANDLE shader)
	{
		return deleteShader(shader, m_vertexShaderFunc, m_device.getOrigVtable().pfnDeleteVertexShaderFunc);
	}

	HRESULT DeviceState::pfnSetPixelShader(HANDLE shader)
	{
		return setShader(shader, m_pixelShader, m_device.getOrigVtable().pfnSetPixelShader);
	}

	HRESULT DeviceState::pfnSetPixelShaderConst(const D3DDDIARG_SETPIXELSHADERCONST* data, const FLOAT* registers)
	{
		return setShaderConst(data, registers, m_pixelShaderConst, m_device.getOrigVtable().pfnSetPixelShaderConst);
	}

	HRESULT DeviceState::pfnSetPixelShaderConstB(const D3DDDIARG_SETPIXELSHADERCONSTB* data, const BOOL* registers)
	{
		return setShaderConst(data, registers, m_pixelShaderConstB, m_device.getOrigVtable().pfnSetPixelShaderConstB);
	}

	HRESULT DeviceState::pfnSetPixelShaderConstI(const D3DDDIARG_SETPIXELSHADERCONSTI* data, const INT* registers)
	{
		return setShaderConst(data, registers, m_pixelShaderConstI, m_device.getOrigVtable().pfnSetPixelShaderConstI);
	}

	HRESULT DeviceState::pfnSetRenderState(const D3DDDIARG_RENDERSTATE* data)
	{
		if (data->State >= D3DDDIRS_WRAP0 && data->State <= D3DDDIRS_WRAP7)
		{
			auto d = *data;
			d.Value &= D3DWRAPCOORD_0 | D3DWRAPCOORD_1 | D3DWRAPCOORD_2 | D3DWRAPCOORD_3;
			return setStateArray(&d, m_renderState, m_device.getOrigVtable().pfnSetRenderState);
		}
		return setStateArray(data, m_renderState, m_device.getOrigVtable().pfnSetRenderState);
	}

	HRESULT DeviceState::pfnSetTexture(UINT stage, HANDLE texture)
	{
		if (stage >= m_textures.size())
		{
			m_device.flushPrimitives();
			return m_device.getOrigVtable().pfnSetTexture(m_device, stage, texture);
		}

		if (texture == m_textures[stage] &&
			texture != UNINITIALIZED_HANDLE)
		{
			return S_OK;
		}

		m_device.flushPrimitives();
		HRESULT result = m_device.getOrigVtable().pfnSetTexture(m_device, stage, texture);
		if (SUCCEEDED(result))
		{
			m_textures[stage] = texture;
			m_textureStageState[stage][D3DDDITSS_DISABLETEXTURECOLORKEY] = UNINITIALIZED_STATE;
			m_textureStageState[stage][D3DDDITSS_TEXTURECOLORKEYVAL] = UNINITIALIZED_STATE;
		}
		return result;
	}

	HRESULT DeviceState::pfnSetTextureStageState(const D3DDDIARG_TEXTURESTAGESTATE* data)
	{
		switch (data->State)
		{
		case D3DDDITSS_DISABLETEXTURECOLORKEY:
			m_textureStageState[data->Stage][D3DDDITSS_TEXTURECOLORKEYVAL] = UNINITIALIZED_STATE;
			break;

		case D3DDDITSS_TEXTURECOLORKEYVAL:
			m_textureStageState[data->Stage][D3DDDITSS_DISABLETEXTURECOLORKEY] = UNINITIALIZED_STATE;
			break;
		}
		return setStateArray(data, m_textureStageState[data->Stage], m_device.getOrigVtable().pfnSetTextureStageState);
	}

	HRESULT DeviceState::pfnSetVertexShaderConst(const D3DDDIARG_SETVERTEXSHADERCONST* data, const void* registers)
	{
		return setShaderConst(data, registers, m_vertexShaderConst, m_device.getOrigVtable().pfnSetVertexShaderConst);
	}

	HRESULT DeviceState::pfnSetVertexShaderConstB(const D3DDDIARG_SETVERTEXSHADERCONSTB* data, const BOOL* registers)
	{
		return setShaderConst(data, registers, m_vertexShaderConstB, m_device.getOrigVtable().pfnSetVertexShaderConstB);
	}

	HRESULT DeviceState::pfnSetVertexShaderConstI(const D3DDDIARG_SETVERTEXSHADERCONSTI* data, const INT* registers)
	{
		return setShaderConst(data, registers, m_vertexShaderConstI, m_device.getOrigVtable().pfnSetVertexShaderConstI);
	}

	HRESULT DeviceState::pfnSetVertexShaderDecl(HANDLE shader)
	{
		HRESULT result = setShader(shader, m_vertexShaderDecl, m_device.getOrigVtable().pfnSetVertexShaderDecl);
		if (SUCCEEDED(result))
		{
			auto it = m_vertexShaderDecls.find(shader);
			if (it != m_vertexShaderDecls.end())
			{
				m_device.getDrawPrimitive().setVertexShaderDecl(it->second);
			}
			else
			{
				m_device.getDrawPrimitive().setVertexShaderDecl({});
			}
		}
		return result;
	}

	HRESULT DeviceState::pfnSetVertexShaderFunc(HANDLE shader)
	{
		return setShader(shader, m_vertexShaderFunc, m_device.getOrigVtable().pfnSetVertexShaderFunc);
	}

	HRESULT DeviceState::pfnSetZRange(const D3DDDIARG_ZRANGE* data)
	{
		return setState(data, m_zRange, m_device.getOrigVtable().pfnSetZRange);
	}

	HRESULT DeviceState::pfnUpdateWInfo(const D3DDDIARG_WINFO* data)
	{
		D3DDDIARG_WINFO wInfo = *data;
		if (1.0f == wInfo.WNear && 1.0f == wInfo.WFar)
		{
			wInfo.WNear = 0.0f;
		}
		return setState(&wInfo, m_wInfo, m_device.getOrigVtable().pfnUpdateWInfo);
	}

	HRESULT DeviceState::deleteShader(HANDLE shader, HANDLE& currentShader,
		HRESULT(APIENTRY* origDeleteShaderFunc)(HANDLE, HANDLE))
	{
		if (shader == currentShader)
		{
			m_device.flushPrimitives();
		}

		HRESULT result = origDeleteShaderFunc(m_device, shader);
		if (SUCCEEDED(result) && shader == currentShader)
		{
			currentShader = UNINITIALIZED_HANDLE;
		}
		return result;
	}

	void DeviceState::removeTexture(HANDLE texture)
	{
		for (UINT i = 0; i < m_textures.size(); ++i)
		{
			if (m_textures[i] == texture)
			{
				m_textures[i] = UNINITIALIZED_HANDLE;
			}
		}
	}

	HRESULT DeviceState::setShader(HANDLE shader, HANDLE& currentShader,
		HRESULT(APIENTRY* origSetShaderFunc)(HANDLE, HANDLE))
	{
		if (shader == currentShader &&
			shader != UNINITIALIZED_HANDLE)
		{
			return S_OK;
		}

		m_device.flushPrimitives();
		HRESULT result = origSetShaderFunc(m_device, shader);
		if (SUCCEEDED(result))
		{
			currentShader = shader;
		}
		return result;
	}

	template <typename SetShaderConstData, typename ShaderConst, typename Registers>
	HRESULT DeviceState::setShaderConst(const SetShaderConstData* data, const Registers* registers,
		std::vector<ShaderConst>& shaderConst,
		HRESULT(APIENTRY* origSetShaderConstFunc)(HANDLE, const SetShaderConstData*, const Registers*))
	{
		if (data->Register + data->Count > shaderConst.size())
		{
			shaderConst.resize(data->Register + data->Count);
		}

		if (0 == memcmp(&shaderConst[data->Register], registers, data->Count * sizeof(ShaderConst)))
		{
			return S_OK;
		}

		m_device.flushPrimitives();
		HRESULT result = origSetShaderConstFunc(m_device, data, registers);
		if (SUCCEEDED(result))
		{
			memcpy(&shaderConst[data->Register], registers, data->Count * sizeof(ShaderConst));
		}
		return result;
	}

	template <typename StateData>
	HRESULT DeviceState::setState(const StateData* data, StateData& currentState,
		HRESULT(APIENTRY* origSetState)(HANDLE, const StateData*))
	{
		if (*data == currentState)
		{
			return S_OK;
		}

		m_device.flushPrimitives();
		HRESULT result = origSetState(m_device, data);
		if (SUCCEEDED(result))
		{
			currentState = *data;
		}
		return result;
	}

	template <typename StateData, UINT size>
	HRESULT DeviceState::setStateArray(const StateData* data, std::array<UINT, size>& currentState,
		HRESULT(APIENTRY* origSetState)(HANDLE, const StateData*))
	{
		if (data->State >= static_cast<INT>(currentState.size()))
		{
			m_device.flushPrimitives();
			return origSetState(m_device, data);
		}

		if (data->Value == currentState[data->State] &&
			data->Value != UNINITIALIZED_STATE)
		{
			return S_OK;
		}

		m_device.flushPrimitives();
		HRESULT result = origSetState(m_device, data);
		if (SUCCEEDED(result))
		{
			currentState[data->State] = data->Value;
		}
		return result;
	}
}
