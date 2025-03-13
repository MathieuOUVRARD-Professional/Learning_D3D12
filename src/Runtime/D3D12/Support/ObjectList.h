#pragma once

#include <D3D/SceneObject.h>
#include <D3D/Material.h>

#include <d3d12.h>

#include <list>
#include <vector>

class ObjectList
{
	public:
		bool hasDefaultTexture = false;
		bool hasDefaultNormalTexture = false;

		UINT64 TotalTexturesSize();
		UINT64 TotalVerticesSize();
		UINT64 TotalIndicesSize();
		UINT64 TotalSize();
		uint32_t TextureCount();
		void CopyToUploadBuffer(ID3D12GraphicsCommandList* cmdList, D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, UINT64 destBufferOffset = 0, UINT64 destOffsetVertex = 0, UINT64 destOffsetIndex = 0);
		void CreateBufferViews(ID3D12Resource* vertexBuffer, ID3D12Resource* indexBuffer);
		void BindDescriptorHeap(ID3D12GraphicsCommandList* cmdList, uint32_t rootParameterIndex);
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