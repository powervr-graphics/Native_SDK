/*!
\brief OpenGL ES Implementation of the AssetStore class.
\file PVREngineUtils/AssetStore.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVREngineUtils/AssetStore.h"
#include "PVRApi/ApiObjects/Sync.h"
#include "PVRAssets/TextureLoad.h"

namespace pvr {
namespace utils {
namespace {
inline bool getPvrFilename(const StringHash& filename, StringHash& outNewName)
{
	size_t period = filename.str().rfind(".");

	if (period == string::npos) //No extension. Add the pvr extension
	{
		outNewName = filename.str() + "." + "pvr";
		return true;
	}

	if (filename.str().substr(period + 1) == "pvr") //Extension is already pvr. Do nothing.
	{
		outNewName = filename;
		return false;
	}
	// Extension exists and is different to pvr. Replace with pvr extension.
	outNewName = filename.str().substr(0, period) + ".pvr";
	return true;
}
}

bool AssetStore::effectOnLoadTexture(const string& textureName, api::TextureView& outTex2d)
{
	return getTextureWithCaching(contextProvider->getGraphicsContext(), textureName, &outTex2d, NULL);
}

bool AssetStore::loadTexture(GraphicsContext& context, const StringHash& filename, TextureFileFormat format,
                             bool forceLoad, api::TextureView* outTexture, TextureHeader* outDescriptor)
{
	Texture tempTexture;
	if (!initialized)
	{
		if (logger) { logger(Log.Error, "AssetStore.loadTexture error for filename %s: Uninitialized AssetStore", filename.c_str()); }
		return false;
	}
	if (format == TextureFileFormat::UNKNOWN)
	{
		if (logger) { logger(Log.Warning, "AssetStore.loadTexture unknown format for filename %s. Will try as PVR texture", filename.c_str()); }
		format = TextureFileFormat::PVR;
	}
	if (!forceLoad)
	{
		std::map<StringHash, TextureData>::iterator found = textureMap.find(filename);
		if (found != textureMap.end())
		{
			logger(Log.Verbose, "AssetStore.loadTexture attempted to load for filename %s : retrieving cached version.",
			       filename.c_str());
			if (outTexture) { (*outTexture) = found->second.texture; }
			if (outDescriptor) { (*outDescriptor) = found->second.textureHeader; }
		}
		return true;
	}

	Stream::ptr_type assetStream = assetProvider->getAssetStream(filename);
	if (!assetStream.get())
	{
		StringHash newFilename;
		if (!getPvrFilename(filename, newFilename))
		{
			if (logger) { logger(Log.Error, "AssetStore.loadTexture: filename %s : File not found", filename.c_str()); }
			return false;
		}
		assetStream = assetProvider->getAssetStream(newFilename);
		if (!assetStream.get())
		{
			if (logger) { logger(Log.Error, "AssetStore.loadTexture: Could not find either filename %s or %s.", filename.c_str(), newFilename.c_str()); }
			return false;
		}
	}

	api::TextureView texView;
	pvr::Result result = assets::textureLoad(assetStream, format, tempTexture);
	if (result == Result::Success)
	{
		texView = contextProvider->getGraphicsContext()->uploadTexture(tempTexture);
	}
	if (result != Result::Success)
	{
		if (logger)
		{
			logger(Log.Error, "AssetStore.loadTexture error for filename %s : Failed to load texture with code %s.",
			       filename.c_str(), Log.getResultCodeString(result));
		}
		return false;
	}
	else
	{
		TextureData tdata;
		tdata.texture = texView;

		//tdata.texture.construct(textureHandle);
		tdata.textureHeader = tempTexture; //Object slicing. This is NOT a reference - we literally only keep the textureheader.
		textureMap[filename] = tdata;
		if (outTexture) { (*outTexture) = tdata.texture; }
		if (outDescriptor) { (*outDescriptor) = tdata.textureHeader; }
		return true;
	}
}


bool AssetStore::generateTextureAtlas(GraphicsContext& context, const StringHash* fileNames,
                                      Rectanglef* outUVs, uint32 numTextures,
                                      api::TextureView* outTexture, TextureHeader* outDescriptor)
{
	TextureHeader header;
	struct SortedImage
	{
		uint32          id;
		pvr::api::TextureView tex;
		pvr::uint16       width;
		pvr::uint16       height;
		pvr::uint16       srcX;
		pvr::uint16       srcY;
		bool            hasAlpha;
	};
	std::vector<SortedImage> sortedImage(numTextures);
	struct SortCompare
	{
		bool operator()(const SortedImage& a, const SortedImage& b)
		{
			pvr::uint32 aSize = a.width * a.height;
			pvr::uint32 bSize = b.width * b.height;
			return (aSize > bSize);
		}
	};

	struct Area
	{
		pvr::int32    x;
		pvr::int32    y;
		pvr::int32    w;
		pvr::int32    h;
		pvr::int32    size;
		bool      isFilled;

		Area*     right;
		Area*     left;

	private:
		void setSize(pvr::int32 width, pvr::int32 height) { w = width;  h = height; size = width * height;  }
	public:
		Area(pvr::int32 width, pvr::int32 height) : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(width, height); }
		Area() : x(0), y(0), isFilled(false), right(NULL), left(NULL) { setSize(0, 0); }

		Area* insert(pvr::int32 width, pvr::int32 height)
		{
			// If this area has branches below it (i.e. is not a leaf) then traverse those.
			// Check the left branch first.
			if (left)
			{
				Area* tempPtr = NULL;
				tempPtr = left->insert(width, height);
				if (tempPtr != NULL) { return tempPtr; }
			}
			// Now check right
			if (right) { return right->insert(width, height); }
			// Already filled!
			if (isFilled) { return NULL; }

			// Too small
			if (size < width * height || w < width || h < height) { return NULL; }

			// Just right!
			if (size == width * height && w == width && h == height)
			{
				isFilled = true;
				return this;
			}
			// Too big. Split up.
			if (size > width * height && w >= width && h >= height)
			{
				// Initializes the children, and sets the left child's coordinates as these don't change.
				left = new Area;
				right = new Area;
				left->x = x;
				left->y = y;

				// --- Splits the current area depending on the size and position of the placed texture.
				// Splits vertically if larger free distance across the texture.
				if ((w - width) > (h - height))
				{
					left->w = width;
					left->h = h;

					right->x = x + width;
					right->y = y;
					right->w = w - width;
					right->h = h;
				}
				// Splits horizontally if larger or equal free distance downwards.
				else
				{
					left->w = w;
					left->h = height;

					right->x = x;
					right->y = y + height;
					right->w = w;
					right->h = h - height;
				}

				//Initializes the child members' size attributes.
				left->size = left->h  * left->w;
				right->size = right->h * right->w;

				//Inserts the texture into the left child member.
				return left->insert(width, height);
			}
			//Catch all error return.
			return NULL;
		}

		bool deleteArea()
		{
			if (left != NULL)
			{
				if (left->left != NULL)
				{
					if (!left->deleteArea())  { return false; }
					if (!right->deleteArea()) { return false; }
				}
			}
			if (right != NULL)
			{
				if (right->left != NULL)
				{
					if (!left->deleteArea())  { return false; }
					if (!right->deleteArea()) { return false; }
				}
			}
			delete right;
			right = NULL;
			delete left;
			left = NULL;
			return true;
		}
	};

	// load the textures
	for (uint32 i = 0; i < numTextures; ++i)
	{
		if (!getTextureWithCaching(context, fileNames[i], &sortedImage[i].tex, &header))
		{
			return false;
		}

		sortedImage[i].id = i;
		sortedImage[i].width = (uint16)header.getWidth();
		sortedImage[i].height = (uint16)header.getHeight();
		const pvr::uint8* pixelString = header.getPixelFormat().getPixelTypeChar();
		if (header.getPixelFormat().getPixelTypeId() == (uint64)pvr::CompressedPixelFormat::PVRTCI_2bpp_RGBA ||
		    header.getPixelFormat().getPixelTypeId() == (uint64)pvr::CompressedPixelFormat::PVRTCI_4bpp_RGBA ||
		    pixelString[0] == 'a' || pixelString[1] == 'a' || pixelString[2] == 'a' || pixelString[3] == 'a')
		{
			sortedImage[i].hasAlpha = true;
		}
		else
		{
			sortedImage[i].hasAlpha = false;
		}
	}
	//// sort the sprites
	std::sort(sortedImage.begin(), sortedImage.end(), SortCompare());
	// find the best width and height
	pvr::int32 width = 0, height = 0, area = 0;
	pvr::uint32 preferredDim[] = {8, 16, 32, 64, 128, 256, 512, 1024};
	const uint32 atlasPixelBorder = 1;
	const uint32 totalBorder = atlasPixelBorder * 2;
	uint32 i = 0;
	// calculate the total area
	for (; i < sortedImage.size(); ++i)
	{
		area += (sortedImage[i].width + totalBorder) * (sortedImage[i].height + totalBorder);
	}
	i = 0;
	while (((int32)preferredDim[i] * (int32)preferredDim[i]) < area && i < sizeof(preferredDim) / sizeof(preferredDim[0]))
	{
		++i;
	}
	if (i >= sizeof(preferredDim) / sizeof(preferredDim[0]))
	{
		Log("Cannot find a best size for the texture atlas");
		return false;
	}
	width = height = preferredDim[i];
	float32 oneOverWidth = 1.f / width;
	float32 oneOverHeight = 1.f / height;
	api::CommandBuffer cmdBlit = context->createCommandBufferOnDefaultPool();


	Area* head = new Area(width, height);
	Area* pRtrn = NULL;
	types::Offset3D dstOffset[2];


	// create the out texture store
	ImageStorageFormat outFmt(PixelFormat::RGBA_32323232, 1, types::ColorSpace::lRGB, VariableType::Float);
	api::TextureStore outTexStore = context->createTexture();

	outTexStore->allocate2D(sortedImage[0].tex->getResource()->getFormat(), width, height, types::ImageUsageFlags::TransferDest | types::ImageUsageFlags::Sampled,
	                        types::ImageLayout::TransferDstOptimal);

	cmdBlit->beginRecording();
	api::TextureView view =context->createTextureView(outTexStore);

	// NO NEED FOR ES
	if(context->getApiType() > Api::OpenGLESMaxVersion)
	{
		cmdBlit->clearColorImage(view, glm::vec4(0.0f));
	}
	for (uint32 i = 0; i < numTextures; ++i)
	{
		const SortedImage& image = sortedImage[i];
		pRtrn = head->insert((pvr::int32)sortedImage[i].width + totalBorder, (pvr::int32)sortedImage[i].height + totalBorder);
		if (!pRtrn)
		{
			pvr::Log("ERROR: Not enough room in texture atlas!\n");
			head->deleteArea();
			delete head;
			return false;
		}
		dstOffset[0].offsetX = (uint16)(pRtrn->x + atlasPixelBorder);
		dstOffset[0].offsetY = (uint16)(pRtrn->y + atlasPixelBorder);
		dstOffset[0].offsetZ = 0;

		dstOffset[1].offsetX = (uint16)(dstOffset[0].offsetX + sortedImage[i].width);
		dstOffset[1].offsetY = (uint16)(dstOffset[0].offsetY + sortedImage[i].height);
		dstOffset[1].offsetZ = 1;


		outUVs[image.id].x = dstOffset[0].offsetX * oneOverWidth;
		outUVs[image.id].y = dstOffset[0].offsetY * oneOverHeight;
		outUVs[image.id].width = sortedImage[i].width * oneOverWidth;
		outUVs[image.id].height = sortedImage[i].height * oneOverHeight;

		types::ImageBlitRange blit(types::Offset3D(0, 0, 0),
		    types::Offset3D(image.width, image.height, 1), dstOffset[0], dstOffset[1]);

		cmdBlit->blitImage(sortedImage[i].tex->getResource(), outTexStore,
		    types::ImageLayout::TransferSrcOptimal, types::ImageLayout::TransferDstOptimal,
		    &blit, 1, types::SamplerFilter::Nearest);
	}
	if (outDescriptor)
	{
		outDescriptor->setWidth(width); outDescriptor->setHeight(height);
		outDescriptor->setChannelType(outFmt.dataType);
		outDescriptor->setColorSpace(outFmt.colorSpace);
		outDescriptor->setDepth(1);
		outDescriptor->setPixelFormat(outFmt.format);
	}
	(*outTexture) =
	  context->createTextureView(outTexStore, types::SwizzleChannels(types::Swizzle::Identity,
	                             types::Swizzle::Identity, types::Swizzle::Identity, types::Swizzle::Identity));

	api::MemoryBarrierSet barrier;
	barrier.addBarrier(api::ImageAreaBarrier(types::AccessFlags::TransferWrite,
	                   types::AccessFlags::ShaderRead, outTexStore,
	                   types::ImageSubresourceRange(types::ImageLayersSize(1, 1), types::ImageSubresource()),
	                   types::ImageLayout::TransferDstOptimal,
	                   types::ImageLayout::ShaderReadOnlyOptimal));
	cmdBlit->pipelineBarrier(types::PipelineStageFlags::TopOfPipeline, types::PipelineStageFlags::TopOfPipeline, barrier);

	cmdBlit->endRecording();
	cmdBlit->submit(api::Semaphore(), api::Semaphore());
	context->waitIdle();
	head->deleteArea();
	delete head;
	return true;
}

bool AssetStore::loadModel(const char* filename, assets::ModelHandle& outModel, bool forceLoad)
{
	if (!initialized)
	{
		if (logger) { logger(Log.Error, "AssetStore.loadModel error for filename %s : Uninitialized AssetStore", filename); }
		return false;
	}

	if (!forceLoad)
	{
		std::map<StringHash, assets::ModelHandle>::iterator found = modelMap.find(filename);
		if (found != modelMap.end())
		{
			outModel = found->second;
			return true;
		}
	}

	Stream::ptr_type assetStream = assetProvider->getAssetStream(filename);
	if (!assetStream.get())
	{
		if (logger) { logger(Log.Error, "AssetStore.loadModel  error for filename %s : File not found", filename); }
		return false;
	}

	assets::PODReader reader(assetStream);
	assets::ModelHandle handle = assets::Model::createWithReader(reader);

	if (handle.isNull())
	{
		if (logger) { logger(Log.Error, "AssetStore.loadModel error : Failed to load model %s"); }
		return false;
	}
	else
	{
		outModel = handle;
		return true;
	}
}
}
}
//!\endcond
