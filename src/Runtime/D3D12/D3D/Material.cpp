#include "Material.h"

void Material::SendShaderParams(ID3D12GraphicsCommandList* cmdList, UINT bufferSlot)
{
	MaterialData materialData =
	{
		m_diffuseTextureID,
		m_normalTextureID,
		m_ormTextureID,

		m_baseColor,
		m_emissiveColor,

		m_opacity, 
		m_metallicFactor, 
		m_roughnessFactor
	};

	cmdList->SetGraphicsRoot32BitConstants(bufferSlot, 12, &materialData, 0);
}
