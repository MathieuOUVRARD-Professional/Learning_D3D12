#pragma once

#include <vector>
#include <D3D/Vertex.h>

class Mesh
{
public:
	unsigned int m_materialID;
	unsigned int m_nSubmeshes = 0;

	uint32_t GetSize();
	uint32_t VerticesSize();
	uint32_t IndicesSize();
	Mesh& GetSubmesh(int index);

	inline std::vector<Vertex>& GetVertices()
	{
		return m_vertices;
	}
	inline void SetVertices(std::vector<Vertex>& vertices)
	{
		m_vertices = vertices;
	}

	inline std::vector<uint32_t>& GetIndices()
	{
		return m_indices;
	}
	inline void SetIndices(std::vector<uint32_t>& indices)
	{
		m_indices = indices;
	}
	inline void AddSubmesh(Mesh& submesh)
	{
		m_submeshes.emplace_back(submesh);
		m_nSubmeshes++;
	}

private:
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	std::vector<Mesh> m_submeshes;
};