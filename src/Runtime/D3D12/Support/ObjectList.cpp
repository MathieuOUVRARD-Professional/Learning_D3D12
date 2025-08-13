#include <Support/ObjectList.h>

ObjectList::ObjectList()
{
}

UINT64 ObjectList::TotalTexturesSize()
{
	UINT64 size = 0;
	for (Material& material : *m_materials)
	{
		size += material.TextureSize();
	}
	return size;
}

UINT64 ObjectList::TotalVerticesSize()
{
	UINT64 size = 0;
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh)
		{
			size += object.m_mesh->VerticesSize();
		}
	}
	return size;
}

UINT64 ObjectList::TotalIndicesSize()
{
	UINT64 size = 0;
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh)
		{
			size += object.m_mesh->IndicesSize();
		}		
	}
	return size;
}

UINT64 ObjectList::TotalMatDataSize()
{
	return sizeof(MaterialData) * MaterialsCount();
}

UINT64 ObjectList::TotalMeshes()
{
	UINT64 nMeshes = 0;
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh)
		{
			if (object.m_mesh->m_nSubmeshes > 0)
			{
				nMeshes += object.m_mesh->m_nSubmeshes;
			}
			else
			{
				nMeshes++;
			}
		}
	}
	return nMeshes;
}

UINT64 ObjectList::TotalSize()
{
	UINT64 size = 0;
	for(SceneObject& object : m_list)
	{
		if (object.m_mesh)
		{
			size += object.m_mesh->Size();
		}
	}
	for (Material& material : *m_materials)
	{
		size += material.TextureSize();
	}
	return size;
}

uint32_t ObjectList::TextureCount()
{
	uint32_t count = 0;
	for (Material& material : *m_materials)
	{
		count += material.GetTextures().m_count;
	}
	return count;
}

void ObjectList::BindDescriptorHeaps(ID3D12GraphicsCommandList* cmdList, uint32_t rootParameterIndex)
{
	ID3D12DescriptorHeap* myHeap = m_bindlessHeapAllocator->GetHeap();
	ID3D12DescriptorHeap* const* heapPtr = &myHeap;

	cmdList->SetDescriptorHeaps(1, heapPtr);
	cmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_bindlessHeapAllocator->GetGPUHandle(0));
}

void ObjectList::ShadowPassDraw(ID3D12GraphicsCommandList* cmdList, Light& light)
{
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh)
		{
			object.m_mesh->ShadowPassDraw(cmdList, light.m_viewProjMatrix, object.m_transform.m_matrix);
		}
	}
}

void ObjectList::Draw(ID3D12GraphicsCommandList* cmdList)
{
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh)
		{
			object.m_mesh->Draw(cmdList, object.m_transform.m_matrix);
		}
	}
}

void ObjectList::CopyTextures(ID3D12GraphicsCommandList* cmdList, D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, UINT64 destBufferOffset)
{
	for (Material& material : *m_materials)
	{
		if (&material.GetTextures() != nullptr)
		{
			material.GetTextures().Init(defaultHeapProperties, m_bindlessHeapAllocator);

			//Check if enough space for next material in the upload buffer
			if (destBufferOffset + material.TextureSize() > (1024 * 1024 * 512))
			{
				//Not enought space, sending current upload buffer and syncing
				DXContext::Get().ExecuteCommandList();	// Fence synchronization
				cmdList = DXContext::Get().InitCommandList();
				destBufferOffset = 0;
			}

			destBufferOffset += material.GetTextures().CopyToGPU(uploadBuffer, destBufferOffset, cmdList);;
		}		
	}

	// Waiting for the buffer to get back before writing meshes
	DXContext::Get().ExecuteCommandList();
	cmdList = DXContext::Get().InitCommandList();
}

void ObjectList::CopyMeshes(ID3D12Resource* uploadBuffer, UINT64 destOffsetVertex, UINT64 destOffsetIndex)
{
	char* uploadBufferAdress;
	D3D12_RANGE uploadRange;
	uploadRange.Begin = 0;
	uploadRange.End = TotalVerticesSize() + TotalIndicesSize();
	uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAdress);

	destOffsetIndex = destOffsetVertex + TotalVerticesSize() + destOffsetIndex;

	UINT64 objectVertexOffset = 0;
	UINT64 objectIndexOffset = 0;
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh)
		{
			if (object.m_mesh->m_nSubmeshes > 0)
			{
				UINT64 submeshVertexOffset = 0;
				UINT64 submeshIndexOffset = 0;
				for (unsigned int i = 0; i < object.m_mesh->m_nSubmeshes; i++)
				{
					memcpy(&uploadBufferAdress
						[destOffsetVertex + objectVertexOffset + submeshVertexOffset],
						object.m_mesh->GetSubmesh(i).GetVertices().data(),
						object.m_mesh->GetSubmesh(i).VerticesSize());
					object.m_mesh->GetSubmesh(i).m_vertexBufferOffset = destOffsetVertex + objectVertexOffset + submeshVertexOffset;

					memcpy(&uploadBufferAdress
						[destOffsetIndex + objectIndexOffset + submeshIndexOffset],
						object.m_mesh->GetSubmesh(i).GetIndices().data(),
						object.m_mesh->GetSubmesh(i).IndicesSize());
					object.m_mesh->GetSubmesh(i).m_indexBufferOffset = (destOffsetIndex - destOffsetVertex - TotalVerticesSize()) + objectIndexOffset + submeshIndexOffset;

					submeshVertexOffset += object.m_mesh->GetSubmesh(i).VerticesSize();
					submeshIndexOffset += object.m_mesh->GetSubmesh(i).IndicesSize();
				}
				objectVertexOffset += submeshVertexOffset;
				objectIndexOffset += submeshIndexOffset;
			}
			else
			{
				memcpy(&uploadBufferAdress
					[destOffsetVertex + objectVertexOffset],
					object.m_mesh->GetVertices().data(),
					object.m_mesh->VerticesSize());
				object.m_mesh->m_vertexBufferOffset = destOffsetVertex + objectVertexOffset;

				memcpy(&uploadBufferAdress
					[destOffsetIndex + objectIndexOffset],
					object.m_mesh->GetIndices().data(),
					object.m_mesh->IndicesSize());
				object.m_mesh->m_indexBufferOffset = (destOffsetIndex - destOffsetVertex - TotalVerticesSize()) + objectIndexOffset;

				objectVertexOffset += object.m_mesh->VerticesSize();
				objectIndexOffset += object.m_mesh->IndicesSize();
			}
		}
		
	}
	uploadBuffer->Unmap(0, &uploadRange);
}


void ObjectList::CopyMaterialsData()
{
	std::vector<MaterialData> materials;
	for (uint32_t i = 0; i < MaterialsCount(); i++)
	{
		MaterialData currentMaterialData  = (*m_materials)[i].GetData();

		materials.emplace_back(currentMaterialData);

		(*m_materials)[i].m_ID = i;
	}

	m_materialDataBuffer = ConstantBuffer(static_cast<UINT>(sizeof(MaterialData) * MaterialsCount()), "MaterialsDatas");

	m_materialDataBuffer.CreateResource();
	m_materialDataBuffer.CopyData(materials.data());
	m_materialDataBuffer.CreateCBV(m_bindlessHeapAllocator);

	m_materialDatas = m_materialDataBuffer.GetData();
}

void ObjectList::CopyModelsData()
{
	std::vector<ModelData> models;
	uint32_t modelID = 0;

	for (SceneObject& object : m_list)
	{
		if (object.m_mesh)
		{
			ModelData meshData;
			meshData.modelMatrix = object.m_transform.m_matrix;
			if (object.m_mesh->m_nSubmeshes > 0)
			{
				for (unsigned int i = 0; i < object.m_mesh->m_nSubmeshes; i++)
				{
					meshData.materialID = object.m_mesh->GetSubmesh(i).GetMaterial().m_ID;
					models.emplace_back(meshData);

					object.m_mesh->GetSubmesh(i).m_ID = modelID;
					modelID++;
				}
			}
			else
			{
				meshData.materialID = object.m_mesh->GetMaterial().m_ID;
				models.emplace_back(meshData);

				object.m_mesh->m_ID = modelID;
				modelID++;
			}
		}
	}

	m_modelsDataBuffer = ConstantBuffer(sizeof(ModelData) * TotalMeshes(), "ModelsDatas");

	m_modelsDataBuffer.CreateResource();
	m_modelsDataBuffer.CopyData(models.data());
	m_modelsDataBuffer.CreateCBV(m_bindlessHeapAllocator);

	m_modelsData = m_modelsDataBuffer.GetData();
}

void ObjectList::CopyToUploadBuffer(ID3D12GraphicsCommandList* cmdList, D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, UINT64 destOffsetTexture, UINT64 destOffsetVertex, UINT64 destOffsetIndex)
{
	if (m_bindlessHeapAllocator == nullptr)
	{
		D3EZ_EXCEPTION_W(std::format("No SRV Heap allocator for {} list!", m_name));
	}

	// Bindless MaterialData Descriptor Heap
	/*D3D12_DESCRIPTOR_HEAP_DESC bindlessMaterialDataHeapDesc = {};
	bindlessMaterialDataHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	bindlessMaterialDataHeapDesc.NumDescriptors = MaterialsCount();  
	bindlessMaterialDataHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	DXContext::Get().GetDevice()->CreateDescriptorHeap(&bindlessMaterialDataHeapDesc, IID_PPV_ARGS(&m_cbvHeap));
	std::string cbvHeapName = m_name + "_Data_CBV";
	m_cbvHeap.Get()->SetName(std::wstring(cbvHeapName.begin(), cbvHeapName.end()).c_str());*/

	// Textures copy
	spdlog::info("=====     TEXTURES     ======\n");
	CopyTextures(cmdList, defaultHeapProperties, uploadBuffer, destOffsetTexture);
	spdlog::info("=====       DONE       ======\n");

	// Meshes copy 
	spdlog::info("=====      MESHES      ======\n");
	CopyMeshes(uploadBuffer, destOffsetVertex, destOffsetIndex);
	spdlog::info("=====       DONE       ======\n");

	// Material datas copy
	spdlog::info("=====     MATERIALS     =====\n");
	CopyMaterialsData();
	spdlog::info("=====       DONE       ======\n");

	// Models datas copy
	spdlog::info("=====   MODELS-DATAS   ======\n");
	CopyModelsData();
	spdlog::info("=====       DONE       ======\n");
}

void ObjectList::CreateBufferViews(ID3D12Resource* vertexBuffer, ID3D12Resource* indexBuffer)
{
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh)
		{
			if (object.m_mesh->m_nSubmeshes > 0)
			{
				for (unsigned int i = 0; i < object.m_mesh->m_nSubmeshes; i++)
				{
					// === Vertex buffer view === //
					D3D12_VERTEX_BUFFER_VIEW vbv{};
					vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress() + object.m_mesh->GetSubmesh(i).m_vertexBufferOffset;
					vbv.SizeInBytes = object.m_mesh->GetSubmesh(i).VerticesSize();
					vbv.StrideInBytes = sizeof(Vertex);

					object.m_mesh->GetSubmesh(i).SetVBV(vbv);

					// === Index buffer view === //	
					D3D12_INDEX_BUFFER_VIEW ibv{};
					ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress() + object.m_mesh->GetSubmesh(i).m_indexBufferOffset;
					ibv.SizeInBytes = object.m_mesh->GetSubmesh(i).IndicesSize();
					ibv.Format = DXGI_FORMAT_R32_UINT;

					object.m_mesh->GetSubmesh(i).SetIBV(ibv);

					// Cleaning
					object.m_mesh->GetSubmesh(i).ClearVectors();
				}
			}
			else
			{
				// === Vertex buffer view === //
				D3D12_VERTEX_BUFFER_VIEW vbv{};
				vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress() + object.m_mesh->m_vertexBufferOffset;
				vbv.SizeInBytes = object.m_mesh->VerticesSize();
				vbv.StrideInBytes = sizeof(Vertex);

				object.m_mesh->SetVBV(vbv);

				// === Index buffer view === //
				D3D12_INDEX_BUFFER_VIEW ibv{};
				ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress() + object.m_mesh->m_indexBufferOffset;
				ibv.SizeInBytes = object.m_mesh->IndicesSize();
				ibv.Format = DXGI_FORMAT_R32_UINT;

				object.m_mesh->SetIBV(ibv);

				// Cleaning
				object.m_mesh->ClearVectors();
			}
		}
	}
}
