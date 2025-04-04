#include "Material.h"

void Material::SendShaderRootParams(ID3D12GraphicsCommandList* cmdList, UINT bufferSlot)
{
	MaterialData materialData
	{
		m_baseColor,
		m_diffuseTextureID,

		m_emissiveColor,
		m_normalTextureID,

		m_ormTextureID,
		m_emissiveTextureID,
		m_opacity,
		m_roughnessFactor,
		m_metallicFactor,
	};

	cmdList->SetGraphicsRoot32BitConstants(bufferSlot, 16, &materialData, 0);
}

MaterialData& Material::GetData()
{
	if (!m_dataGenerated)
	{
		MaterialData materialData
		{
			m_baseColor,
			m_diffuseTextureID,

			m_emissiveColor,
			m_normalTextureID,

			m_ormTextureID,
			m_emissiveTextureID,

			m_opacity,
			m_roughnessFactor,
			m_metallicFactor,
		};

		m_data = materialData;
		m_dataGenerated = true;
	}	

	return m_data;
}
