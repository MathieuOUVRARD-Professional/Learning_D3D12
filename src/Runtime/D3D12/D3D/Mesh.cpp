#include <D3D/Mesh.h>

uint32_t Mesh::Size()
{
	uint32_t size = 0;

	if (m_nSubmeshes > 0)
	{
		for (unsigned int i = 0; i < m_nSubmeshes; i++)
		{
			size += m_submeshes[i].Size();
		}		
	}
	else
	{
		size = VerticesSize() + IndicesSize();
		
	}
	return size;
}

uint32_t Mesh::VerticesSize()
{
	uint32_t size = 0;

	if (m_nSubmeshes > 0)
	{
		for (unsigned int i = 0; i < m_nSubmeshes; i++)
		{
			size += m_submeshes[i].VerticesSize();
		}
	}
	else
	{
		size = uint32_t(sizeof(Vertex) * m_vertices.size());
	}
	return size;
}

uint32_t Mesh::IndicesSize()
{
	uint32_t size = 0;

	if (m_nSubmeshes > 0)
	{
		for (unsigned int i = 0; i < m_nSubmeshes; i++)
		{
			size += m_submeshes[i].IndicesSize();
		}
	}
	else
	{
		size = uint32_t(sizeof(uint32_t) * m_indices.size());
	}
	return size;
}

Mesh& Mesh::GetSubmesh(int index)
{
	return m_submeshes[index];
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList, Camera& camera, glm::mat4& transform)
{
	if (m_nSubmeshes > 0)
	{
		for (unsigned int i = 0; i < m_nSubmeshes; i++)
		{
			m_submeshes[i].Draw(cmdList, camera, transform);
		}
	}
	else
	{
		// === IA === //
		cmdList->IASetVertexBuffers(0, 1, &m_vbv);
		cmdList->IASetIndexBuffer(&m_ibv);
		// === ROOT === //
		camera.UpdateMatrix(cmdList, 0, transform);
		cmdList->SetGraphicsRoot32BitConstants(1, 4, &DebugColorsVector[m_materialID], 0);

		cmdList->DrawIndexedInstanced(m_nIndex, 1, 0, 0, 0);
	}
}
