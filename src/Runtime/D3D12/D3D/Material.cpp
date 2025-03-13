#include "Material.h"

void Material::SendShaderParams(ID3D12GraphicsCommandList* cmdList, UINT bufferSlot)
{
	MaterialData materialData
	{
		m_diffuseTextureID,
		m_normalTextureID,
		m_ormTextureID,
		m_emissiveTextureID,

		m_baseColor,
		0.0f,

		m_emissiveColor,
		0.0f,

		m_opacity,
		m_roughnessFactor,
		m_metallicFactor,		
		0.0f
	};

	/*materialData.diffuseID = m_diffuseTextureID;
	materialData.normalID = m_normalTextureID;
	materialData.ormID = m_ormTextureID;
	materialData.emissiveID = m_emissiveTextureID;

	materialData.baseColor = m_baseColor;
	materialData.emissiveColor = m_emissiveColor;

	materialData.opacity = m_opacity;
	materialData.roughness = m_roughnessFactor;
	materialData.metalness = m_metallicFactor;

	materialData.padding1 = 0.0f;
	materialData.padding2 = 0.0f;
	materialData.padding3 = 0.0f;*/

	cmdList->SetGraphicsRoot32BitConstants(bufferSlot, 16, &materialData, 0);
}