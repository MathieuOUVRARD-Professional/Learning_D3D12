#pragma once

#include <vector>
#include <D3D/Vertex.h>
#include <d3d12.h>

class Mesh
{
public:
	unsigned int m_materialID = -1;
	unsigned int m_nSubmeshes = 0;
	unsigned int m_vertexBufferOffset = 0;
	unsigned int m_indexBufferOffset = 0;

	uint32_t GetSize();
	uint32_t VerticesSize();
	uint32_t IndicesSize();
	Mesh& GetSubmesh(int index);
	void Draw();

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

	inline D3D12_VERTEX_BUFFER_VIEW* GetVBV()
	{
		return &m_vbv;
	}
	inline void SetVBV(D3D12_VERTEX_BUFFER_VIEW& vbv)
	{
		m_vbv = vbv;
	}

	inline D3D12_INDEX_BUFFER_VIEW* GetIBV()
	{
		return &m_ibv;
	}
	inline void SetIBV(D3D12_INDEX_BUFFER_VIEW& ibv)
	{
		m_ibv = ibv;
	}

	inline void AddSubmesh(Mesh& submesh)
	{
		m_submeshes.emplace_back(submesh);
		m_nSubmeshes++;
	}
	inline void ClearVectors()
	{
		m_vertices.clear();
		m_indices.clear();
	}

private:
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	std::vector<Mesh> m_submeshes;

	D3D12_VERTEX_BUFFER_VIEW m_vbv{};
	D3D12_INDEX_BUFFER_VIEW m_ibv{};
};