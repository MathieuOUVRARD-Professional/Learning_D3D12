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

void ObjectList::CopyToUploadBuffer(ID3D12Resource* uploadBuffer, uint32_t destOffset)
{
	char* uploadBufferAdress;
	D3D12_RANGE uploadRange;
	uploadRange.Begin = destOffset;
	uploadRange.End = destOffset + TotalSize();
	uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAdress);

	uint32_t objectVertexOffset = 0;
	uint32_t objectIndexOffset = 0;
	for (SceneObject object : m_list)
	{
		if (object.m_mesh.m_nSubmeshes > 0)
		{
			uint32_t meshVertexOffset = 0;
			uint32_t meshIndexOffset = 0;
			for (int i = 0; i < object.m_mesh.m_nSubmeshes; i++)
			{
				memcpy(
					&uploadBufferAdress[objectVertexOffset + meshVertexOffset],
					object.m_mesh.GetSubmesh(i).GetVertices().data(), 
					object.m_mesh.GetSubmesh(i).VerticesSize());

				memcpy(&uploadBufferAdress[objectIndexOffset + meshIndexOffset + TotalVerticesSize()],
					object.m_mesh.GetSubmesh(i).GetIndices().data(),
					object.m_mesh.GetSubmesh(i).IndicesSize());

				meshVertexOffset += object.m_mesh.GetSubmesh(i).VerticesSize();
				meshIndexOffset += object.m_mesh.GetSubmesh(i).IndicesSize();
			}
			objectVertexOffset += meshVertexOffset;
			objectIndexOffset += meshIndexOffset;
		}
		else
		{
			memcpy(
				&uploadBufferAdress[objectVertexOffset],
				object.m_mesh.GetVertices().data(),
				object.m_mesh.VerticesSize());

			memcpy(&uploadBufferAdress[objectIndexOffset + TotalVerticesSize()],
				object.m_mesh.GetIndices().data(),
				object.m_mesh.IndicesSize());			

			objectVertexOffset += object.m_mesh.VerticesSize();
			objectIndexOffset += object.m_mesh.IndicesSize();
		}
	}	
}
