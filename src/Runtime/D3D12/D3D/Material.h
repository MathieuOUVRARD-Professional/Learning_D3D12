#pragma once

#include <D3D/Texture.h>
#include <glm/glm.hpp>

struct MaterialData
{
	uint32_t diffuseID;
	uint32_t normalID;
	uint32_t ormID;
	uint32_t emissiveID;

	glm::vec3 baseColor;
	float padding1;

	glm::vec3 emissiveColor;
	float padding2;

	float opacity;
	float roughness;
	float metalness;
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

		ComPointer<ID3D12Resource> m_dataResource = nullptr;

	private:
		Texture m_Textures;
		bool m_dataGenerated = false;
		MaterialData m_data;
};
