#include "Material.h"

void Material::SendShaderParams(ID3D12GraphicsCommandList* cmdList, UINT bufferSlot)
{
	MaterialData materialData =
	{
		m_index,
		m_Textures.m_count,	

		m_baseColor,
		m_emissiveColor,

		m_opacity, 
		m_metallicFactor, 
		m_roughnessFactor
	};

	cmdList->SetGraphicsRoot32BitConstants(bufferSlot, 12, &materialData, 0);
}
