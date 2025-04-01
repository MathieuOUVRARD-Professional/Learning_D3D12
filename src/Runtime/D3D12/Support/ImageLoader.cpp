#include "ImageLoader.h"

// GUID to DXGI format lookup table (can be extended)
 const std::vector<ImageLoader::GUID_to_DXGI> ImageLoader::s_lookupTable =
 {
	 { GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM},
	 { GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM},
	 {GUID_WICPixelFormat24bppBGR, DXGI_FORMAT_B8G8R8A8_UNORM},
	 {GUID_WICPixelFormat24bppRGB, DXGI_FORMAT_R8G8B8A8_UNORM}
 };

bool ImageLoader::LoadImageFromDisk(const std::filesystem::path& imagePath, ImageData& imageData, bool useMips)
{
	if (useMips)
	{
		std::string path = imagePath.string();

		// If imagePath extension != .dds use generated Mips
		if (path.substr(path.size() - 4, path.size()) != ".dds")
		{
			// Changing image extension "PathToTexture.png" --> "PathToTexture.dds"
			std::string ddsPath = path.substr(0, path.size() - (4));
			ddsPath += ".dds";

			// Adding .dds folder to image path "PathToTexture.dds" --> "PathTo/DSS/Texture.dds"
			uint32_t offset = (uint32_t)(ddsPath.find_last_of('/', ddsPath.size()) + 1);
			ddsPath.insert(offset, "DDS/");

			DirectX::ScratchImage image;

			// Check if .dds file exist
			struct stat buffer;
			if (stat(ddsPath.c_str(), &buffer) == 0)
			{
				spdlog::info("DDS already generated: " + path.substr(offset, path.size() - offset));

				// Load previously generaterated Mips
				DirectX::LoadFromDDSFile(std::wstring(ddsPath.begin(), ddsPath.end()).c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);				
 
				// Copy metadata
				imageData.bitPerPixel = (uint32_t) DirectX::BitsPerPixel(image.GetMetadata().format);
				imageData.height = (uint32_t)image.GetMetadata().height;
				imageData.width = (uint32_t)image.GetMetadata().width;
				imageData.mipsLevels = (uint32_t)image.GetMetadata().mipLevels;
				imageData.giPixelFormat = image.GetMetadata().format;

				// Copy content for each Mip level
				for (unsigned int i = 0; i < imageData.mipsLevels; i++)
				{					
					imageData.content.emplace_back();
					imageData.content[i] = std::vector<char>(image.GetImage(i, 0, 0)->slicePitch);
					memcpy
					(
						imageData.content[i].data(), 
						image.GetImage(i, 0, 0)->pixels, 
						image.GetImage(i, 0, 0)->slicePitch
					);
				}
				return true;
			}
			else
			{
				spdlog::info("Generating DDS: " + path.substr(offset, path.size() - offset));

				// Load Image
				HRESULT hr = DirectX::LoadFromWICFile(std::wstring(path.begin(), path.end()).c_str(), DirectX::WIC_FLAGS::WIC_FLAGS_NONE, nullptr, image);
				if (FAILED(hr)) 
				{
					spdlog::error("Generate Mips: Failed to load texture: " + imagePath.string());
					return false;
				}

				// Generate Mips
				DirectX::ScratchImage mipChain;
				hr = DirectX::GenerateMipMaps(*image.GetImages(), DirectX::TEX_FILTER_BOX, 0, mipChain);
				if (FAILED(hr)) {
					spdlog::error("Generate Mips: Failed to generate mipmaps!");
					return false;
				}

				// Save .dds file
				hr = DirectX::SaveToDDSFile(mipChain.GetImages(), mipChain.GetMetadata().mipLevels, mipChain.GetMetadata(), DirectX::DDS_FLAGS_NONE, std::wstring(ddsPath.begin(), ddsPath.end()).c_str());
				if (FAILED(hr)) {
					spdlog::error("Failed to save DDS!");
					return false;
				}

				imageData.bitPerPixel = (uint32_t)DirectX::BitsPerPixel(mipChain.GetMetadata().format);
				imageData.height = (uint32_t)mipChain.GetMetadata().height;
				imageData.width = (uint32_t)mipChain.GetMetadata().width;
				imageData.mipsLevels = (uint32_t)mipChain.GetMetadata().mipLevels;
				imageData.giPixelFormat = mipChain.GetMetadata().format;

				for (unsigned int i = 0; i < imageData.mipsLevels; i++)
				{
					imageData.content[i] = std::vector<char>(mipChain.GetImage(i, 0, 0)->slicePitch);
					memcpy
					(
						imageData.content[i].data(),
						mipChain.GetImage(i, 0, 0)->pixels,
						mipChain.GetImage(i, 0, 0)->slicePitch
					);
				}

				return true;
			}
		}		
	}
	else
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

		ComPointer<IWICFormatConverter> wicConverter;
		if (imageData.wicPixelFormat != GUID_WICPixelFormat32bppRGBA)
		{
			__ImageLoader_CAR(
				wicFactory->CreateFormatConverter(&wicConverter)
			);
			__ImageLoader_CAR(
				wicConverter->Initialize
				(
					wicFrameDecoder.Get(),	// Conversion to R8G8B8
					GUID_WICPixelFormat32bppRGBA,
					WICBitmapDitherTypeNone,
					nullptr,
					0.0,
					WICBitmapPaletteTypeCustom
				)
			);
			imageData.wicPixelFormat = GUID_WICPixelFormat32bppRGBA;
		}
		else
		{
			wicConverter = nullptr;
			__ImageLoader_CAR(
				wicFactory->CreateFormatConverter(&wicConverter)
			);
			__ImageLoader_CAR(
				wicConverter->Initialize
				(
					wicFrameDecoder.Get(), // No conversion, just adapt it for uniformity
					GUID_WICPixelFormat32bppRGBA,
					WICBitmapDitherTypeNone,
					nullptr,
					0.0,
					WICBitmapPaletteTypeCustom
				)
			);
		}

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

		imageData.content.emplace_back();
		imageData.content[0].resize(size);

		WICRect copyRect;
		copyRect.X = copyRect.Y = 0;
		copyRect.Width = imageData.width;
		copyRect.Height = imageData.height;

		__ImageLoader_CAR(
			wicConverter->CopyPixels(&copyRect, stride, size, (BYTE*)imageData.content[0].data())
		);

		return true;
	}
}
