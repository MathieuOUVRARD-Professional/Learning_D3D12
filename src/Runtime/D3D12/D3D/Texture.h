#pragma once

#include <directx/d3dx12.h>

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <Support/ImageLoader.h>

#include <Util/EzException.h>

#include <D3D/DescriptorHeapAllocator.h>
#include <D3D/DXContext.h>

#include <string>

class Texture
{
	public:
		void Init(D3D12_HEAP_PROPERTIES* defaultHeapProperties, DescriptorHeapAllocator* srvHeapAllocator = nullptr);
		UINT64 CopyToUploadBuffer(ID3D12Resource* uploadBuffer, UINT64 uploadBufferOffset, ID3D12GraphicsCommandList* cmdList);
		void AddCommands(ID3D12GraphicsCommandList*& cmdList, uint32_t rootParameterIndex);

		inline UINT64 GetTextureSize(int textureIndex)
		{
			// Only Mip 0
			if (m_textureDatas[textureIndex].mipsLevels == 1)
			{
				return m_textureSizes[textureIndex];
			}
			// Multiple Mip levels 
			else
			{
				UINT result = 0;
				UINT mipLevels = m_textureDatas[textureIndex].mipsLevels;

				for (int i = 0; i < mipLevels; i++)
				{
					result += m_textureSizes[textureIndex] / pow(4, mipLevels);
				}
				return result;
			}
		};
		inline UINT64 GetTotalTextureSize()
		{
			UINT64 totalSize = 0;
			for (unsigned int i = 0; i < m_count; i++)
			{
				totalSize += GetTextureSize(i);
			}
			return totalSize;
		}
		inline char* GetTextureData(int textureIndex)
		{
			// Only Mip 0
			if (m_textureDatas[textureIndex].mipsLevels == 1)
			{
				return m_textureDatas[textureIndex].content[0].data();
			}
			// Multiple Mip levels 
			else
			{
				std::vector<char> result;
				UINT mipCount = m_textureDatas[textureIndex].mipsLevels;

				for (int i = 0; i < mipCount; i++)
				{
					result.emplace_back(m_textureDatas[textureIndex].content[i].data());
				}
				return result.data();
			}
		};
		
		Texture(std::vector<std::string>& paths, std::vector<std::string>& names, bool useMips);
		Texture();

		uint32_t m_count = 0;

	private:
		std::vector<std::string> m_names;
		std::vector<ComPointer<ID3D12Resource>> m_textures;
		ComPointer<ID3D12DescriptorHeap> m_srvHeap = nullptr;

		std::vector<ImageLoader::ImageData> m_textureDatas;
		std::vector<UINT64> m_textureSizes;
		std::vector<uint32_t> m_textureStrides;
};