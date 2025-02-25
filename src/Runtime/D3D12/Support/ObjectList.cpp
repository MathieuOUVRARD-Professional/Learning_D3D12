#include <Support/ObjectList.h>

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
		size += object.m_mesh.GetSize();
	}
	return size;
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

void ObjectList::CopyToUploadBuffer(ID3D12Resource* uploadBuffer, uint32_t destOffsetVertex, uint32_t destOffsetIndex, uint32_t destBufferOffset)
{
	char* uploadBufferAdress;
	D3D12_RANGE uploadRange;
	uploadRange.Begin = destBufferOffset;
	uploadRange.End = destBufferOffset + destOffsetVertex + destOffsetIndex + TotalSize();
	uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAdress);

	destOffsetIndex = destOffsetVertex + TotalVerticesSize() + destOffsetIndex;

	uint32_t objectVertexOffset = 0;
	uint32_t objectIndexOffset = 0;
	for (SceneObject& object : m_list)
	{
		if (object.m_mesh.m_nSubmeshes > 0)
		{
			uint32_t meshVertexOffset = 0;
			uint32_t meshIndexOffset = 0;
			for (unsigned int i = 0; i < object.m_mesh.m_nSubmeshes; i++)
			{
				memcpy(&uploadBufferAdress
					[destBufferOffset + destOffsetVertex + objectVertexOffset + meshVertexOffset],
					object.m_mesh.GetSubmesh(i).GetVertices().data(), 
					object.m_mesh.GetSubmesh(i).VerticesSize());
				object.m_mesh.GetSubmesh(i).m_vertexBufferOffset = destOffsetVertex + objectVertexOffset + meshVertexOffset;

				memcpy(&uploadBufferAdress
					[destBufferOffset + destOffsetIndex + objectIndexOffset + meshIndexOffset],
					object.m_mesh.GetSubmesh(i).GetIndices().data(),
					object.m_mesh.GetSubmesh(i).IndicesSize());
				object.m_mesh.GetSubmesh(i).m_indexBufferOffset = (destOffsetIndex - destOffsetVertex - TotalVerticesSize()) + objectIndexOffset + meshIndexOffset;

				meshVertexOffset += object.m_mesh.GetSubmesh(i).VerticesSize();
				meshIndexOffset += object.m_mesh.GetSubmesh(i).IndicesSize();
			}
			objectVertexOffset += meshVertexOffset;
			objectIndexOffset += meshIndexOffset;
		}
		else
		{
			memcpy(
				&uploadBufferAdress[destBufferOffset + destOffsetVertex + objectVertexOffset],
				object.m_mesh.GetVertices().data(),
				object.m_mesh.VerticesSize());
			object.m_mesh.m_vertexBufferOffset = destOffsetVertex + objectVertexOffset;

			memcpy(&uploadBufferAdress
				[destBufferOffset + destOffsetIndex + objectIndexOffset],
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
