#include <D3D/Mesh.h>

uint32_t Mesh::GetSize()
{
	uint32_t size = 0;

	if (m_nSubmeshes > 0)
	{
		for (int i = 0; i < m_nSubmeshes; i++)
		{
			size += m_submeshes[i].GetSize();
		}		
	}
	else
	{
		size = sizeof(m_vertices) + sizeof(m_indices);
	}
	return size;
}

uint32_t Mesh::VerticesSize()
{
	uint32_t size = 0;

	if (m_nSubmeshes > 0)
	{
		for (int i = 0; i < m_nSubmeshes; i++)
		{
			size += m_submeshes[i].VerticesSize();
		}
	}
	else
	{
		size = sizeof(m_vertices);
	}
	return size;
}

uint32_t Mesh::IndicesSize()
{
	uint32_t size = 0;

	if (m_nSubmeshes > 0)
	{
		for (int i = 0; i < m_nSubmeshes; i++)
		{
			size += m_submeshes[i].IndicesSize();
		}
	}
	else
	{
		size = sizeof(m_indices);
	}
	return size;
}

Mesh& Mesh::GetSubmesh(int index)
{
	return m_submeshes[index];
}
