
#include <directx/d3dx12.h>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/ImageLoader.h>

#include <Util/EzException.h>

#include <D3D/DXContext.h>

#include <string>
class Texture
{
	public:
		void Init(D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, ID3D12GraphicsCommandList* cmdList);
		void AddCommands(ID3D12GraphicsCommandList* cmdList, UINT rootParameterIndex);

		inline uint32_t GetTextureSize(int textureIndex)
		{
			return m_textureSizes[textureIndex];
		};
		inline uint32_t GetTotalTextureSize()
		{
			uint32_t totalSize = 0;
			for (unsigned int i = 0; i < m_count; i++)
			{
				totalSize += GetTextureSize(i);
			}
			return totalSize;
		}
		inline char* GetTextureData(int textureIndex)
		{
			return m_textureDatas[textureIndex].content.data();
		};
		
		Texture(int count, std::string* path, LPCWSTR* name);

		unsigned int m_count = 0;

	private:
		std::vector<LPCWSTR> m_names;
		std::vector<ComPointer<ID3D12Resource>> m_textures;
		ComPointer<ID3D12DescriptorHeap> m_srvHeap;

		std::vector<ImageLoader::ImageData> m_textureDatas;
		std::vector<uint32_t> m_textureSizes;
		std::vector<uint32_t> m_textureStrides;
};