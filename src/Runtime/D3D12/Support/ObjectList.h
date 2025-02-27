#pragma once

#include <D3D/SceneObject.h>
#include <D3D/Material.h>

#include <d3d12.h>

#include <list>
#include <vector>

class ObjectList
{
	public:
		uint32_t TotalTexturesSize();
		uint32_t TotalVerticesSize();
		uint32_t TotalIndicesSize();
		uint32_t TotalSize();
		uint32_t TextureCount();
		void CopyToUploadBuffer(ID3D12GraphicsCommandList* cmdList, D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, uint32_t destOffsetVertex = 0, uint32_t destOffsetIndex = 0, uint32_t destBufferOffset = 0);
		void CreateBufferViews(ID3D12Resource* vertexBuffer, ID3D12Resource* indexBuffer);
		void Draw(ID3D12GraphicsCommandList* cmdList, Camera& camera);


		inline std::list<SceneObject>& GetList()
		{
			return m_list;
		}
		inline uint32_t ListSize()
		{
			return (uint32_t)m_list.size();
		}

		inline std::vector<Material>& GetMaterials()
		{
			return m_materials;
		}
		inline void SetMaterials(std::vector<Material>& materials)
		{
			m_materials = materials;
		}
		inline uint32_t MaterialsCount()
		{
			return (uint32_t)m_materials.size();
		}

	private:
		std::list<SceneObject> m_list;
		std::vector<Material> m_materials;
		ComPointer<ID3D12DescriptorHeap> m_srvHeap;
};