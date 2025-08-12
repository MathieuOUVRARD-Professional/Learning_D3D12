#pragma once

#include <D3D/DescriptorHeapAllocator.h>
#include <D3D/Light.h>
#include <D3D/Material.h>
#include <D3D/SceneObject.h>

#include <d3d12.h>

#include <list>
#include <vector>

class ObjectList
{
	public:
		std::string m_name = "UnnamedList";

		ComPointer<ID3D12Resource> m_materialDatas = nullptr;
		ComPointer<ID3D12Resource> m_modelData = nullptr;

		bool m_hasDefaultTexture = false;
		bool m_hasDefaultNormalTexture = false;

		UINT64 TotalTexturesSize();
		UINT64 TotalVerticesSize();
		UINT64 TotalIndicesSize();
		UINT64 TotalMatDataSize();
		UINT64 TotalMeshes();
		UINT64 TotalSize();
		uint32_t TextureCount();

		inline void SetHeapAllocator(DescriptorHeapAllocator& allocator)
		{
			m_bindlessHeapAllocator = &allocator;
		}

		void CopyToUploadBuffer(ID3D12GraphicsCommandList* cmdList, D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, UINT64 destBufferOffset = 0, UINT64 destOffsetVertex = 0, UINT64 destOffsetIndex = 0);
		void CreateBufferViews(ID3D12Resource* vertexBuffer, ID3D12Resource* indexBuffer);
		void BindDescriptorHeaps(ID3D12GraphicsCommandList* cmdList, uint32_t rootParameterIndex);
		void ShadowPassDraw(ID3D12GraphicsCommandList* cmdList, Light& light);
		void Draw(ID3D12GraphicsCommandList* cmdList);
		

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
			return *m_materials;
		}
		inline void SetMaterials(std::unique_ptr<std::vector<Material>> & materials)
		{
			m_materials = std::move(materials);
		}
		inline uint32_t MaterialsCount()
		{
			return (uint32_t)m_materials->size();
		}

	private:
		std::list<SceneObject> m_list;
		std::unique_ptr<std::vector<Material>> m_materials;

		DescriptorHeapAllocator* m_bindlessHeapAllocator = nullptr;

		void CopyTextures(ID3D12GraphicsCommandList* cmdList, D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, UINT64 destBufferOffset = 0);
		void CopyMeshes(ID3D12Resource* uploadBuffer, UINT64 destOffsetVertex = 0, UINT64 destOffsetIndex = 0);
		void CopyMaterialsData();
		void CopyModelsData();

		struct ModelData
		{
			glm::mat4 modelMatrix;
			UINT materialID;
			float padding0 = 0;
			float padding1 = 0;
			float padding2 = 0;
		};
};