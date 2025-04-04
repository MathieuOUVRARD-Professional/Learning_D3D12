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

void Mesh::ShadowPassDraw(ID3D12GraphicsCommandList* cmdList, glm::mat4& viewProjectionMatrix, glm::mat4& modelTransform)
{
	if (m_nSubmeshes > 0)
	{
		for (unsigned int i = 0; i < m_nSubmeshes; i++)
		{
			m_submeshes[i].ShadowPassDraw(cmdList, viewProjectionMatrix, modelTransform);
		}
	}
	else
	{		
		struct Matrices
		{
			glm::mat4 viewProj = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);
		};

		Matrices matrices;
		matrices.viewProj = viewProjectionMatrix;
		matrices.model = modelTransform;

		// === IA === //
		cmdList->IASetVertexBuffers(0, 1, &m_vbv);
		cmdList->IASetIndexBuffer(&m_ibv);
		// === ROOT === //
		cmdList->SetGraphicsRoot32BitConstants(0, 32, &matrices, 0);

		cmdList->DrawIndexedInstanced(m_nIndex, 1, 0, 0, 0);
	}
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList, glm::mat4& modelTransform)
{
	if (m_nSubmeshes > 0)
	{
		for (unsigned int i = 0; i < m_nSubmeshes; i++)
		{
			m_submeshes[i].Draw(cmdList, modelTransform);
		}
	}
	else
	{
		// === IA === //
		cmdList->IASetVertexBuffers(0, 1, &m_vbv);
		cmdList->IASetIndexBuffer(&m_ibv);
		// === ROOT === //f
		cmdList->SetGraphicsRoot32BitConstants(1, 4, &m_ID, 0);

		cmdList->DrawIndexedInstanced(m_nIndex, 1, 0, 0, 0);
	}
}
