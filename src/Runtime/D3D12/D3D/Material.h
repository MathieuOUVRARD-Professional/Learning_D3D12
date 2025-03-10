#pragma once

#include <D3D/Texture.h>
#include <glm/glm.hpp>

class Material
{
	public:
		inline UINT64 TextureSize()
		{
			return m_Textures.GetTotalTextureSize();
		}
		inline Texture& GetTextures()
		{
			return m_Textures;
		}
		inline void SetTextures(Texture& textures)
		{
			m_Textures = textures;
		}
		void SendShaderParams(ID3D12GraphicsCommandList* cmdList, UINT bufferSlot);

		uint32_t m_diffuseTextureID;
		uint32_t m_normalTextureID;
		uint32_t m_ormTextureID;

		std::string m_name;
		glm::vec3 m_baseColor = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 m_emissiveColor = glm::vec3(0.0f, 0.0f, 0.0f);
		float m_opacity = 1.0f;
		float m_metallicFactor = 1.0f;
		float m_roughnessFactor = 1.0f;

	private:
		Texture m_Textures;
};

struct MaterialData
{
	uint32_t diffuseID;
	uint32_t normalID;
	uint32_t ormID;

	glm::vec3 baseColor;
	glm::vec3 emmisiveColor;

	float opacity;
	float roughness;
	float metalness;
};