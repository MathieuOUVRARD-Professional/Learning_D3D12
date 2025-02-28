#include <Support/ObjectList.h>

uint32_t ObjectList::TotalTexturesSize()
{
	uint32_t size = 0;
	for (Material material : m_materials)
	{
		size += material.TextureSize();
	}
	return size;
}

uint32_t ObjectList::TotalVerticesSize()
{
	uint32_t size = 0;
	for (SceneObject object : m_list)
	{
		size += object.m_mesh.VerticesSize();
	}
	return size;
}

uint32_t ObjectList::TotalIndicesSize()
{
	uint32_t size = 0;
	for (SceneObject object : m_list)
	{
		size += object.m_mesh.IndicesSize();
	}
	return size;
}

uint32_t ObjectList::TotalSize()
{
	uint32_t size = 0;
	for(SceneObject object : m_list)
	{
		size += object.m_mesh.Size();
	}
	for (Material material : m_materials)
	{
		size += material.TextureSize();
	}
	return size;
}

uint32_t ObjectList::TextureCount()
{
	uint32_t count = 0;
	for (Material material : m_materials)
	{
		count += material.GetTextures().m_count;
	}
	return count;
}

void ObjectList::Draw(ID3D12GraphicsCommandList* cmdList, Camera& camera)
{
	for (SceneObject& object : m_list)
	{	
		if (object.m_name == "Sponza")
		{
			continue;
		}
		object.m_mesh.Draw(cmdList, camera, object.m_transform);
	}
}

void ObjectList::CopyToUploadBuffer(ID3D12GraphicsCommandList* cmdList, D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, uint32_t destOffsetVertex, uint32_t destOffsetIndex, uint32_t destOffsetTexture)
{
	// Bindless Textures Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC bindlessDescriptorHeapDescText{};
	bindlessDescriptorHeapDescText.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	bindlessDescriptorHeapDescText.NumDescriptors = TextureCount();
	bindlessDescriptorHeapDescText.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	bindlessDescriptorHeapDescText.NodeMask = 0;

	DXContext::Get().GetDevice()->CreateDescriptorHeap(&bindlessDescriptorHeapDescText, IID_PPV_ARGS(&m_srvHeap));
	std::string temp = m_list.front().m_name + " DescriptorHeap";
	std::wstring wideHeapName = std::wstring(temp.begin(), temp.end());

	m_srvHeap.Get()->SetName(wideHeapName.c_str());

	// Textures copy
	uint32_t srvOffset = 0;
	for (Material material : m_materials)
	{
		material.GetTextures().Init(defaultHeapProperties, uploadBuffer, destOffsetTexture, cmdList, m_srvHeap, srvOffset);
		material.GetTextures().CopyToUploadBuffer(uploadBuffer, destOffsetTexture);
		srvOffset += material.GetTextures().m_count;
		destOffsetTexture += material.GetTextures().GetTotalTextureSize();
	}

	// Meshes copy 
	char* uploadBufferAdress;
	D3D12_RANGE uploadRange;
	uploadRange.Begin = destOffsetTexture;
	uploadRange.End = destOffsetTexture + destOffsetVertex + destOffsetIndex + TotalSize();
	uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAdress);

	destOffsetIndex = destOffsetVertex + TotalVerticesSize() + destOffsetIndex;

	uint32_t objectVertexOffset = 0;
	uint32_t objectIndexOffset = 0;
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh.m_nSubmeshes > 0)
		{
			uint32_t submeshVertexOffset = 0;
			uint32_t submeshIndexOffset = 0;
			for (unsigned int i = 0; i < object.m_mesh.m_nSubmeshes; i++)
			{
				memcpy(&uploadBufferAdress
					[destOffsetTexture + destOffsetVertex + objectVertexOffset + submeshVertexOffset],
					object.m_mesh.GetSubmesh(i).GetVertices().data(), 
					object.m_mesh.GetSubmesh(i).VerticesSize());
				object.m_mesh.GetSubmesh(i).m_vertexBufferOffset = destOffsetVertex + objectVertexOffset + submeshVertexOffset;

				memcpy(&uploadBufferAdress
					[destOffsetTexture + destOffsetIndex + objectIndexOffset + submeshIndexOffset],
					object.m_mesh.GetSubmesh(i).GetIndices().data(),
					object.m_mesh.GetSubmesh(i).IndicesSize());
				object.m_mesh.GetSubmesh(i).m_indexBufferOffset = (destOffsetIndex - destOffsetVertex - TotalVerticesSize()) + objectIndexOffset + submeshIndexOffset;

				submeshVertexOffset += object.m_mesh.GetSubmesh(i).VerticesSize();
				submeshIndexOffset += object.m_mesh.GetSubmesh(i).IndicesSize();
			}
			objectVertexOffset += submeshVertexOffset;
			objectIndexOffset += submeshIndexOffset;
		}
		else
		{
			memcpy(
				&uploadBufferAdress[destOffsetTexture + destOffsetVertex + objectVertexOffset],
				object.m_mesh.GetVertices().data(),
				object.m_mesh.VerticesSize());
			object.m_mesh.m_vertexBufferOffset = destOffsetVertex + objectVertexOffset;

			memcpy(&uploadBufferAdress
				[destOffsetTexture + destOffsetIndex + objectIndexOffset],
				object.m_mesh.GetIndices().data(),
				object.m_mesh.IndicesSize());
			object.m_mesh.m_indexBufferOffset = (destOffsetIndex - destOffsetVertex - TotalVerticesSize()) + objectIndexOffset;

			objectVertexOffset += object.m_mesh.VerticesSize();
			objectIndexOffset += object.m_mesh.IndicesSize();
		}
	}
	uploadBuffer->Unmap(0, &uploadRange);
}

void ObjectList::CreateBufferViews(ID3D12Resource* vertexBuffer, ID3D12Resource* indexBuffer)
{
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh.m_nSubmeshes > 0)
		{
			for (unsigned int i = 0; i < object.m_mesh.m_nSubmeshes; i++)
			{
				// === Vertex buffer view === //
				D3D12_VERTEX_BUFFER_VIEW vbv{};
				vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress() + object.m_mesh.GetSubmesh(i).m_vertexBufferOffset;
				vbv.SizeInBytes = object.m_mesh.GetSubmesh(i).VerticesSize();
				vbv.StrideInBytes = sizeof(Vertex);			

				object.m_mesh.GetSubmesh(i).SetVBV(vbv);

				// === Index buffer view === //	
				D3D12_INDEX_BUFFER_VIEW ibv{};
				ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress() + object.m_mesh.GetSubmesh(i).m_indexBufferOffset;
				ibv.SizeInBytes = object.m_mesh.GetSubmesh(i).IndicesSize();
				ibv.Format = DXGI_FORMAT_R32_UINT;

				object.m_mesh.GetSubmesh(i).SetIBV(ibv);
				
				// Cleaning
				object.m_mesh.GetSubmesh(i).ClearVectors();
			}
		}
		else 
		{
			// === Vertex buffer view === //
			D3D12_VERTEX_BUFFER_VIEW vbv{};
			vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress() + object.m_mesh.m_vertexBufferOffset;
			vbv.SizeInBytes = object.m_mesh.VerticesSize();
			vbv.StrideInBytes = sizeof(Vertex);

			object.m_mesh.SetVBV(vbv);

			// === Index buffer view === //
			D3D12_INDEX_BUFFER_VIEW ibv{};
			ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress() + object.m_mesh.m_indexBufferOffset;
			ibv.SizeInBytes = object.m_mesh.IndicesSize();
			ibv.Format = DXGI_FORMAT_R32_UINT;

			object.m_mesh.SetIBV(ibv);

			// Cleaning
			object.m_mesh.ClearVectors();
		}
	}
}
