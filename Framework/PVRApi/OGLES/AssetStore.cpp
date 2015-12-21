/*!*********************************************************************************************************************
\file         PVRApi\OGLES\AssetStore.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES Implementation of the AssetStore class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/AssetStore.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/TextureUtils.h"
#include "PVRApi/OGLES/ConvertToApiTypes.h"


namespace pvr {
namespace api {
namespace {
inline bool getPvrFilename(const StringHash& filename, StringHash& outNewName)
{
	size_t period = filename.getString().rfind(".");

	if (period == string::npos) //No extension. Add the pvr extension
	{
		outNewName = filename.getString() + "." + "pvr";
		return true;
	}

	if (filename.getString().substr(period + 1) == "pvr") //Extension is already pvr. Do nothing.
	{
		outNewName = filename;
		return false;
	}
	// Extension exists and is different to pvr. Replace with pvr extension.
	outNewName = filename.getString().substr(0, period) + ".pvr";
	return true;
}
}

bool AssetStore::effectOnLoadTexture(const string& textureName, api::TextureView& outTex2d)
{
	return getTextureWithCaching(assetProvider->getGraphicsContext(), textureName, &outTex2d, NULL);
}

bool AssetStore::loadTexture(GraphicsContext& context, const StringHash& filename, assets::TextureFileFormat::Enum format,
                                     bool forceLoad, api::TextureView* outTexture, assets::TextureHeader* outDescriptor)
{
	assets::Texture tempTexture;
	if (!initialised)
	{
		if (logger) { logger(Log.Error, "AssetStore.loadTexture error for filename %s: Uninitialised AssetStore", filename.c_str()); }
		return false;
	}
	if (format == assets::TextureFileFormat::UNKNOWN)
	{
		if (logger) { logger(Log.Warning, "AssetStore.loadTexture unknown format for filename %s. Will try as PVR texture", filename.c_str()); }
		format = assets::TextureFileFormat::PVR;
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

	
	native::HTexture_ textureHandle;
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

	pvr::Result::Enum result = assets::textureLoad(assetStream, format, tempTexture);
	if (result == Result::Success)
	{
		result = pvr::utils::textureUpload(assetProvider->getGraphicsContext(), tempTexture, textureHandle);
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
		api::TextureView tex;
		tex.construct(context, textureHandle);
		tdata.texture = tex;


		//tdata.texture.construct(textureHandle);
		tdata.textureHeader = tempTexture; //Object slicing. This is NOT a reference - we literally only keep the textureheader.
		textureMap[filename] = tdata;
		if (outTexture) { (*outTexture) = tdata.texture; }
		if (outDescriptor) { (*outDescriptor) = tdata.textureHeader; }
		return true;
	}
}


bool AssetStore::loadModel(const char* filename, assets::ModelHandle& outModel, bool forceLoad)
{
	if (!initialised)
	{
		if (logger) { logger(Log.Error, "AssetStore.loadModel error for filename %s : Uninitialised AssetStore", filename); }
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
