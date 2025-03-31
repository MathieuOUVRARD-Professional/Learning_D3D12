#pragma once

#include<Support/WinInclude.h>
#include<Support/ComPointer.h>

#include <Util/LoggingProvider.h>

#include<vector>
#include<filesystem>
#include<algorithm>
#include<DirectXTex.h>

#define __ImageLoader_CAR(expr) do { if(FAILED(expr)) { return false; } } while(false)

class ImageLoader
{
	public:
		struct ImageData
		{
			std::vector<char> content;
			uint32_t width;
			uint32_t height;
			uint32_t bitPerPixel;
			uint32_t chanelCount;

			GUID		wicPixelFormat;
			DXGI_FORMAT giPixelFormat;
		};

		static bool LoadImageFromDisk(const std::filesystem::path& imagePath, ImageData& data, bool useMips);
	private:
		struct GUID_to_DXGI
		{
			GUID wic;
			DXGI_FORMAT gi;
		};

		static const std::vector<GUID_to_DXGI> s_lookupTable;

	private:
		ImageLoader() = default;
		ImageLoader(const ImageLoader&) = default;
		ImageLoader& operator=(const ImageLoader&) = default;
};