#pragma once

#include <D3D/Texture.h>
#include <glm/glm.hpp>

struct MaterialData
{
	glm::vec3 baseColor;
	uint32_t diffuseID;

	glm::vec3 emissiveColor;
	uint32_t normalID;

	uint32_t ormID;
	uint32_t emissiveID;

	float opacity;
	float roughness;
	float metalness;
	float padding1;
	float padding2;
	float padding3;
};

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
		void SendShaderRootParams(ID3D12GraphicsCommandList* cmdList, UINT bufferSlot);
		MaterialData& GetData();

		uint32_t m_ID = 0;

		uint32_t m_diffuseTextureID		= 0;
		uint32_t m_normalTextureID		= 0;
		uint32_t m_ormTextureID			= 0;
		uint32_t m_emissiveTextureID	= 0;

		std::string m_name				= "";
		glm::vec3 m_baseColor			= glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 m_emissiveColor		= glm::vec3(0.0f, 0.0f, 0.0f);

		float m_opacity					= 1.0f;
		float m_metallicFactor			= 0.0f;
		float m_roughnessFactor			= 1.0f;

	private:
		Texture m_Textures;
		bool m_dataGenerated = false;
		MaterialData m_data;
};
