#include <D3D/Mesh.h>

uint32_t Mesh::GetSize()
{
	uint32_t size = 0;

	if (m_nSubmeshes > 0)
	{
		for (unsigned int i = 0; i < m_nSubmeshes; i++)
		{
			size += m_submeshes[i].GetSize();
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

void Mesh::Draw()
{

}
