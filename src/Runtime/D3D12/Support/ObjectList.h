#pragma once

#include <D3D/SceneObject.h>

#include <d3d12.h>

#include <list>

class ObjectList
{
	public:
		uint32_t TotalVerticesSize();
		uint32_t TotalIndicesSize();
		uint32_t TotalSize();
		void Draw(ID3D12GraphicsCommandList* cmdList, Camera& camera);

		void CopyToUploadBuffer(ID3D12Resource* uploadBuffer, uint32_t destOffsetVertex = 0, uint32_t destOffsetIndex = 0, uint32_t destBufferOffset = 0);
		void CreateBufferViews(ID3D12Resource* vertexBuffer, ID3D12Resource* indexBuffer);

		std::list<SceneObject>& GetList()
		{
			return m_list;
		}
		inline uint32_t ListSize()
		{
			return (uint32_t)m_list.size();
		}

	private:
		std::list<SceneObject> m_list;
};