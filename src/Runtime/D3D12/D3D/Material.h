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

	float padding0;
	float padding1;
	float padding2;
};

// Material posses a non copiable member with m_textures being a unique_ptr<Texture>.
// As unique_ptr are uniques they're not copyable and though move semantics must be use.

// SO MATERIALS CANNOT BE COPIED BUT ONLY MOVED AND PASS BY REFERENCE

// Using unique_ptr through the whole life of the resource make so that it's never copied.
// But it's ownership can be passed so it never gets deleted as long as it's referenced 

class Material
{
	public:

		// Delete copy constructor & assignment
		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;

		// Move constructor & assignement
		Material(Material&& other) noexcept = default;
		Material& operator=(Material&& other) noexcept = default;

		Material();

		inline UINT64 TextureSize()
		{
			return m_textures->GetTotalTextureSize();			
		}
		inline Texture& GetTextures()
		{
			return *m_textures;
		}
		inline void SetTextures(std::unique_ptr<Texture> & textures)
		{
			m_textures = std::move(textures);
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
		std::unique_ptr<Texture> m_textures = nullptr;
		bool m_dataGenerated = false;
		MaterialData m_data;
};
