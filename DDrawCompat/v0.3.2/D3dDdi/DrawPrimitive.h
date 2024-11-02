#pragma once

#include <map>
#include <vector>

#include <d3d.h>
#include <d3dumddi.h>

#include <DDrawCompat/v0.3.2/D3dDdi/DynamicBuffer.h>

namespace D3dDdi
{
	class Device;

	class DrawPrimitive
	{
	public:
		DrawPrimitive(Device& device);

		void addSysMemVertexBuffer(HANDLE resource, BYTE* vertices);
		void removeSysMemVertexBuffer(HANDLE resource);

		HRESULT flushPrimitives(const UINT* flagBuffer = nullptr);

		HRESULT draw(D3DDDIARG_DRAWPRIMITIVE data, const UINT* flagBuffer);
		HRESULT drawIndexed(D3DDDIARG_DRAWINDEXEDPRIMITIVE2 data, const UINT16* indices, const UINT* flagBuffer);
		HRESULT setStreamSource(const D3DDDIARG_SETSTREAMSOURCE& data);
		HRESULT setStreamSourceUm(const D3DDDIARG_SETSTREAMSOURCEUM& data, const void* umBuffer);
		void setVertexShaderDecl(const std::vector<D3DDDIVERTEXELEMENT>& decl);

	private:
		struct BatchedPrimitives
		{
			D3DPRIMITIVETYPE primitiveType;
			UINT primitiveCount;
			INT baseVertexIndex;
			UINT minIndex;
			UINT maxIndex;
			std::vector<BYTE> vertices;
			std::vector<UINT16> indices;
		};

		struct StreamSource
		{
			const BYTE* vertices;
			UINT stride;
		};

		void appendIndexedVertices(const UINT16* indices, UINT count,
			INT baseVertexIndex, UINT minIndex, UINT maxIndex);
		void appendIndexedVerticesWithoutRebase(const UINT16* indices, UINT count,
			INT baseVertexIndex, UINT minIndex, UINT maxIndex);
		void appendIndexRange(UINT base, UINT count);
		void appendIndexRangeWithoutRebase(UINT base, UINT count);
		void appendIndices(const UINT16* indices, UINT count,
			INT baseVertexIndex, UINT minIndex, UINT maxIndex);
		void appendIndicesAndVertices(const UINT16* indices, UINT count,
			INT baseVertexIndex, UINT minIndex, UINT maxIndex);
		void appendLineOrTriangleList(INT baseVertexIndex, UINT primitiveCount, UINT vpp,
			const UINT16* indices, UINT minIndex, UINT maxIndex);
		bool appendPrimitives(D3DPRIMITIVETYPE primitiveType, INT baseVertexIndex, UINT primitiveCount,
			const UINT16* indices, UINT minIndex, UINT maxIndex);
		void appendTriangleFan(INT baseVertexIndex, UINT primitiveCount,
			const UINT16* indices, UINT minIndex, UINT maxIndex);
		void appendTriangleStrip(INT baseVertexIndex, UINT primitiveCount,
			const UINT16* indices, UINT minIndex, UINT maxIndex);
		void appendVertices(UINT base, UINT count);
		void clearBatchedPrimitives();
		void convertIndexedTriangleFanToList(UINT startPrimitive, UINT primitiveCount);
		void convertIndexedTriangleStripToList(UINT startPrimitive, UINT primitiveCount);
		void convertToTriangleList();
		HRESULT flush(const UINT* flagBuffer);
		HRESULT flushIndexed(const UINT* flagBuffer);
		INT loadIndices(const void* indices, UINT count);
		INT loadVertices(UINT count);
		UINT getBatchedVertexCount() const;
		void rebaseIndices();
		void repeatLastBatchedVertex();

		HRESULT setSysMemStreamSource(const BYTE* vertices, UINT stride);

		HANDLE m_device;
		const D3DDDI_DEVICEFUNCS& m_origVtable;
		DynamicVertexBuffer m_vertexBuffer;
		DynamicIndexBuffer m_indexBuffer;
		StreamSource m_streamSource;
		std::map<HANDLE, BYTE*> m_sysMemVertexBuffers;
		BatchedPrimitives m_batched;
		bool m_isHwVertexProcessingUsed;
	};
}
