#include "Material.h"

void Material::SendShaderParams(ID3D12GraphicsCommandList* cmdList, UINT bufferSlot)
{
	MaterialData materialData;

	materialData.diffuseID = m_diffuseTextureID;
	materialData.normalID = m_normalTextureID;
	materialData.ormID = m_ormTextureID;

	materialData.baseColor = m_baseColor;
	materialData.emissiveColor = m_emissiveColor;

	materialData.opacity = m_opacity;
	materialData.roughness = m_roughnessFactor;
	materialData.metalness = m_metallicFactor;

	cmdList->SetGraphicsRoot32BitConstants(bufferSlot, 12, &materialData, 0);
}