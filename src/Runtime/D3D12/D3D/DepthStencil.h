
#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

#include <D3D/DXContext.h>

class ZBuffer
{
	public:
		void Init(D3D12_HEAP_PROPERTIES* defaultHeapProperties, ID3D12Resource* uploadBuffer, ID3D12GraphicsCommandList* cmdList);
		void AddCommands(ID3D12GraphicsCommandList* cmdList, UINT rootParameterIndex);

		inline uint32_t GetTextureSize()
		{
			return m_textureSize;
		};
		inline ImageLoader::ImageData GetTextureData()
		{
			return m_textureData;
		};

		Texture(std::string path, LPCWSTR name);

	private:
		LPCWSTR m_name;
		ComPointer<ID3D12Resource> m_texture;
		ComPointer<ID3D12DescriptorHeap> m_srvHeap;

		uint32_t m_textureSize;
		uint32_t m_textureStride;
};