#pragma once

#include <D3D/Vertex.h>
#include <Support/Camera.h>
#include <Util/DebugColors.h>

#include <d3d12.h>

#include <vector>
#include "Material.h"

class Mesh
{
public:
	uint32_t m_ID = 0;

	uint32_t m_nSubmeshes = 0;
	std::vector<Mesh> m_submeshes;

	UINT64 m_vertexBufferOffset = 0;
	UINT64 m_indexBufferOffset = 0;

	uint32_t Size();
	uint32_t VerticesSize();
	uint32_t IndicesSize();
	Mesh& GetSubmesh(int index);
	void ShadowPassDraw(ID3D12GraphicsCommandList* cmdList, glm::mat4& viewProjectionMatrix, glm::mat4& modelTransform);
	void Draw(ID3D12GraphicsCommandList* cmdList, glm::mat4& modelTransform);

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
		m_nIndex = (uint32_t)m_indices.size();
	}

	inline void SetMaterial(Material& material)
	{
		m_material = &material;
	}
	inline Material& GetMaterial()
	{
		return *m_material;
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

	Material* m_material;

	uint32_t m_nIndex = 0;

	D3D12_VERTEX_BUFFER_VIEW m_vbv{};
	D3D12_INDEX_BUFFER_VIEW m_ibv{};
};
