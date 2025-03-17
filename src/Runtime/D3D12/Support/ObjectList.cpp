#include <Support/ObjectList.h>

UINT64 ObjectList::TotalTexturesSize()
{
	UINT64 size = 0;
	for (Material& material : m_materials)
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
		size += object.m_mesh.VerticesSize();
	}
	return size;
}

UINT64 ObjectList::TotalIndicesSize()
{
	UINT64 size = 0;
	for (SceneObject& object : m_list)
	{
		size += object.m_mesh.IndicesSize();
	}
	return size;
}

UINT64 ObjectList::TotalSize()
{
	UINT64 size = 0;
	for(SceneObject& object : m_list)
	{
		size += object.m_mesh.Size();
	}
	for (Material& material : m_materials)
	{
		size += material.TextureSize();
	}
	return size;
}

uint32_t ObjectList::TextureCount()
{
	uint32_t count = 0;
	for (Material& material : m_materials)
	{
		count += material.GetTextures().m_count;
	}
	return count;
}

void ObjectList::CopyToUploadBuffer(ID3D12GraphicsCommandList* cmdList, D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, UINT64 destOffsetTexture, UINT64 destOffsetVertex, UINT64 destOffsetIndex)
{
	// Bindless Textures Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC bindlessDescriptorHeapDescText{};
	bindlessDescriptorHeapDescText.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	bindlessDescriptorHeapDescText.NumDescriptors = TextureCount();
	bindlessDescriptorHeapDescText.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	bindlessDescriptorHeapDescText.NodeMask = 0;

	DXContext::Get().GetDevice()->CreateDescriptorHeap(&bindlessDescriptorHeapDescText, IID_PPV_ARGS(&m_srvHeap));
	std::string temp = m_list.front().m_name + "_DescriptorHeap";
	std::wstring wideHeapName = std::wstring(temp.begin(), temp.end());

	m_srvHeap.Get()->SetName(wideHeapName.c_str());

	// Textures copy
	uint32_t srvIndex = 0;
	for (Material& material : m_materials)
	{
		material.GetTextures().Init(defaultHeapProperties, m_srvHeap, srvIndex);

		if (destOffsetTexture + material.TextureSize() > uploadBuffer->GetDesc().Width)
		{
			//Not enough space for next material in the upload buffer
			DXContext::Get().ExecuteCommandList();	// Fence synchronization
			cmdList = DXContext::Get().InitCommandList();
			destOffsetTexture = 0;
		}
		material.GetTextures().CopyToUploadBuffer(uploadBuffer, destOffsetTexture, cmdList);
		srvIndex += material.GetTextures().m_count;
		destOffsetTexture += material.TextureSize();
	}

	// Waiting for the buffer to get back before writing meshes
	DXContext::Get().ExecuteCommandList();
	cmdList = DXContext::Get().InitCommandList();
	destOffsetTexture = 0;

	// Meshes copy 
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
		if (object.m_mesh.m_nSubmeshes > 0)
		{
			UINT64 submeshVertexOffset = 0;
			UINT64 submeshIndexOffset = 0;
			for (unsigned int i = 0; i < object.m_mesh.m_nSubmeshes; i++)
			{
				memcpy(&uploadBufferAdress
					[destOffsetVertex + objectVertexOffset + submeshVertexOffset],
					object.m_mesh.GetSubmesh(i).GetVertices().data(), 
					object.m_mesh.GetSubmesh(i).VerticesSize());
				object.m_mesh.GetSubmesh(i).m_vertexBufferOffset = destOffsetVertex + objectVertexOffset + submeshVertexOffset;

				memcpy(&uploadBufferAdress
					[destOffsetIndex + objectIndexOffset + submeshIndexOffset],
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
				&uploadBufferAdress[destOffsetVertex + objectVertexOffset],
				object.m_mesh.GetVertices().data(),
				object.m_mesh.VerticesSize());
			object.m_mesh.m_vertexBufferOffset = destOffsetVertex + objectVertexOffset;

			memcpy(&uploadBufferAdress
				[destOffsetIndex + objectIndexOffset],
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

void ObjectList::BindDescriptorHeap(ID3D12GraphicsCommandList* cmdList, uint32_t rootParameterIndex)
{
	cmdList->SetDescriptorHeaps(1, &m_srvHeap);
	cmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
}

void ObjectList::ShadowPassDraw(ID3D12GraphicsCommandList* cmdList, Camera& camera)
{
	for (SceneObject& object : m_list)
	{
		object.m_mesh.Draw(cmdList, camera, object.m_transform);
	}
}

void ObjectList::Draw(ID3D12GraphicsCommandList* cmdList, Camera& camera)
{
	for (SceneObject& object : m_list)
	{
		object.m_mesh.Draw(cmdList, camera, object.m_transform);
	}
}