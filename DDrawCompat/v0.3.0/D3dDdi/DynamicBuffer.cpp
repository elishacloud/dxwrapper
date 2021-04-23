#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include <DDrawCompat/v0.3.0/D3dDdi/Device.h>
#include <DDrawCompat/v0.3.0/D3dDdi/DynamicBuffer.h>

namespace
{
	D3DDDI_RESOURCEFLAGS getIndexBufferFlag()
	{
		D3DDDI_RESOURCEFLAGS flags = {};
		flags.IndexBuffer = 1;
		return flags;
	}

	D3DDDI_RESOURCEFLAGS getVertexBufferFlag()
	{
		D3DDDI_RESOURCEFLAGS flags = {};
		flags.VertexBuffer = 1;
		return flags;
	}
}

namespace D3dDdi
{
	DynamicBuffer::DynamicBuffer(Device& device, UINT size, D3DDDIFORMAT format, D3DDDI_RESOURCEFLAGS resourceFlag)
		: m_device(device)
		, m_resource(nullptr, [&](HANDLE vb) { device.getOrigVtable().pfnDestroyResource(device, vb); })
		, m_size(size)
		, m_format(format)
		, m_resourceFlag(resourceFlag)
		, m_stride(0)
		, m_pos(0)
	{
		resize(size);
	}

	void* DynamicBuffer::lock(UINT size)
	{
		D3DDDIARG_LOCK lock = {};
		lock.hResource = m_resource.get();
		lock.Range.Offset = m_pos;
		lock.Range.Size = size;
		lock.Flags.RangeValid = 1;

		if (0 == m_pos)
		{
			lock.Flags.Discard = 1;
		}
		else
		{
			lock.Flags.WriteOnly = 1;
			lock.Flags.NoOverwrite = 1;
		}

		HRESULT result = m_device.getOrigVtable().pfnLock(m_device, &lock);
		if (FAILED(result))
		{
			return nullptr;
		}
		return lock.pSurfData;
	}

	INT DynamicBuffer::load(const void* src, UINT count)
	{
		UINT size = count * m_stride;
		if (m_pos + size > m_size)
		{
			m_pos = 0;
		}

		UINT pos = m_pos;
		auto dst = lock(size);
		if (!dst)
		{
			return -1;
		}

		memcpy(dst, src, size);
		unlock();
		m_pos += size;
		return pos / m_stride;
	}

	void DynamicBuffer::resize(UINT size)
	{
		m_size = 0;
		if (0 == size)
		{
			m_resource.reset();
			return;
		}

		D3DDDI_SURFACEINFO surfaceInfo = {};
		surfaceInfo.Width = size;
		surfaceInfo.Height = 1;

		D3DDDIARG_CREATERESOURCE2 cr = {};
		cr.Format = m_format;
		cr.Pool = D3DDDIPOOL_VIDEOMEMORY;
		cr.pSurfList = &surfaceInfo;
		cr.SurfCount = 1;
		cr.Flags = m_resourceFlag;
		cr.Flags.Dynamic = 1;
		cr.Flags.WriteOnly = 1;
		cr.Rotation = D3DDDI_ROTATION_IDENTITY;

		m_resource.reset();
		if (SUCCEEDED(m_device.createPrivateResource(cr)))
		{
			m_resource.reset(cr.hResource);
			m_size = size;
		}
	}

	void DynamicBuffer::setStride(UINT stride)
	{
		m_stride = stride;
		m_pos = (m_pos + stride - 1) / stride * stride;
	}

	void DynamicBuffer::unlock()
	{
		D3DDDIARG_UNLOCK unlock = {};
		unlock.hResource = m_resource.get();
		m_device.getOrigVtable().pfnUnlock(m_device, &unlock);
	}

	DynamicIndexBuffer::DynamicIndexBuffer(Device& device, UINT size)
		: DynamicBuffer(device, size, D3DDDIFMT_INDEX16, getIndexBufferFlag())
	{
		m_stride = 2;
	}

	DynamicVertexBuffer::DynamicVertexBuffer(Device& device, UINT size)
		: DynamicBuffer(device, size, D3DDDIFMT_VERTEXDATA, getVertexBufferFlag())
	{
	}
}
