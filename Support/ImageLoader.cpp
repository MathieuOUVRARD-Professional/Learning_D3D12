#include "ImageLoader.h"

// GUID to DXGI format lookup table (can be extended)
 const std::vector<ImageLoader::GUID_to_DXGI> ImageLoader::s_lookupTable =
 {
	 { GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM},
	 { GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM}
 };

bool ImageLoader::LoadImageFromDisk(const std::filesystem::path& imagePath, ImageData& imageData)
{
	// Factory
	ComPointer<IWICImagingFactory> wicFactory;
	__ImageLoader_CAR(
		CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory))
		);

	// Load the image
	ComPointer<IWICStream> wicFileStream;
	__ImageLoader_CAR(
		wicFactory->CreateStream(&wicFileStream)
	);
	__ImageLoader_CAR(
		wicFileStream->InitializeFromFilename(imagePath.wstring().c_str(), GENERIC_READ)
	);

	ComPointer<IWICBitmapDecoder> wicDecoder;
	__ImageLoader_CAR(
		wicFactory->CreateDecoderFromStream(wicFileStream, nullptr, WICDecodeMetadataCacheOnDemand, &wicDecoder)
	);

	ComPointer<IWICBitmapFrameDecode> wicFrameDecoder;
	__ImageLoader_CAR(
		wicDecoder->GetFrame(0, &wicFrameDecoder)
	);

	// Trivial metadata
	__ImageLoader_CAR(
		wicFrameDecoder->GetSize(&imageData.width, &imageData.height)
	);
	__ImageLoader_CAR(
		wicFrameDecoder->GetPixelFormat(&imageData.wicPixelFormat)
	);

	// Metadata of pixel format
	ComPointer<IWICComponentInfo> wicComponentInfo;
	__ImageLoader_CAR(
		wicFactory->CreateComponentInfo(imageData.wicPixelFormat, &wicComponentInfo)
	);
	ComPointer<IWICPixelFormatInfo> wicPixelFormatInfo;
	__ImageLoader_CAR(
		wicComponentInfo->QueryInterface(&wicPixelFormatInfo)
	);
	__ImageLoader_CAR(
		wicPixelFormatInfo->GetBitsPerPixel(&imageData.bitPerPixel)
	);
	__ImageLoader_CAR(
		wicPixelFormatInfo->GetChannelCount(&imageData.chanelCount)
	);
	
	// DXGI Pixel format
	auto findIt = std::find_if(s_lookupTable.begin(), s_lookupTable.end(),
		[&](const GUID_to_DXGI& entry) 
		{
			return memcmp(&entry.wic, &imageData.wicPixelFormat, sizeof(GUID)) == 0;
		}
	);
	if (findIt == s_lookupTable.end())
	{
		return false;
	}
	imageData.giPixelFormat = findIt->gi;

	// Image loading
	// Hardcoded values reasons: we devide by 8 to have the number of bytes but in the case where bitPerPixel = 10 result would be 1 and we would loss data
	// So we add 7 to be sure to have all the necessary data. (rounding up) 
	uint32_t stride = ((imageData.bitPerPixel + 7) / 8) * imageData.width;
	uint32_t size = stride * imageData.height;
	imageData.content.resize(size);

	WICRect copyRect;
	copyRect.X = copyRect.Y = 0;
	copyRect.Width = imageData.width;
	copyRect.Height = imageData.height;

	__ImageLoader_CAR(
		wicFrameDecoder->CopyPixels(&copyRect,stride, size, (BYTE*)imageData.content.data())
	);

	return true;
}
