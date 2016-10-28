/*!*********************************************************************************************************************
\file         PVRApi\AssetStore.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the AssetStore, a convenience class that can be used to load assets from the filesystem and upload them
              into API objects, while ensuring no duplicate loading happens.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/EffectApi.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRAssets/Model.h"
#include "PVRAssets/FileIO/PODReader.h"
#include "PVRCore/Stream.h"
#include "PVRAssets/Texture/Texture.h"
namespace pvr {
namespace api {

/*!****************************************************************************************************************
\brief Manages scene assets. Use this class to easily load assets without needing to worry about duplicates. This
       class keeps references to assets, so remember to release them if they are no longer required.
*******************************************************************************************************************/
class AssetStore : public pvr::api::AssetLoadingDelegate
{
public:
	typedef int32 AssetId;

	static const AssetId NoAsset = -1;

private:
	struct TextureData { assets::TextureHeader textureHeader; api::TextureView texture; };

	IAssetProvider* assetProvider;
	OSManager* contextProvider;

	std::map<StringHash, TextureData> textureMap;
	std::map<StringHash, assets::ModelHandle> modelMap;
	std::vector<api::EffectApi> effects;
	std::map<StringHash, AssetId> effectMap;
	std::set<AssetId> unusedEffects;

	ErrorLogger logger;
	bool initialized;

	template<typename T_, typename iterable>
	static void compact(std::vector<T_> items, iterable container)
	{
		for (typename iterable::iterator it = container.begin(); it != container.end(); ++it)
		{
			items[*it] = items.back(); items.pop_back();
		}
	}

class FunctorTextureDataToHandle { public: api::TextureView operator()(TextureData td) { return td.texture; } };

public:
	/*!****************************************************************************************************************
	\brief Implementation of the AssetLoadingDelegate. Allows this class to be passed to the EffectApi constructor
	       as the effectDelegate to automate loading and uploading of textures.
	\param	textureName A texture filename to load from a file.
	\param outTex2d A handle where the loaded and uploaded texture will be saved.
	*******************************************************************************************************************/
	bool effectOnLoadTexture(const string& textureName, api::TextureView& outTex2d);

	/*!****************************************************************************************************************
	\brief Constructor.
	\param	logger	A printf-style function pointer which will be used by the class to record errors that happen
	during any operation.
	*******************************************************************************************************************/
	AssetStore(ErrorLogger logger = &Logger::static_output) : assetProvider(NULL), logger(logger),
		initialized(false)
	{ }

	/*!****************************************************************************************************************
	\brief Initialize with the application class (the Shell, hence the Application, is-a IPlatformProvider
	\param theShell The IAssetProvider that this AssetStore will use to load the requested assets from disk. pvr::Shell
	       implements the AssetStore interface, so normally the instance of the Application Class will be passed here.
	*******************************************************************************************************************/
	void init(pvr::IPlatformProvider& theShell)
	{
		assetProvider = &theShell;
		contextProvider = &theShell;
		initialized = true;
	}

	/*!****************************************************************************************************************
	\brief Initialize with separate Context and Asset providers. Prefer the other overload if you want to just pass the application.
	\param assetProvider The IAssetProvider that this AssetStore will use to load the requested assets from disk. pvr::Shell
	implements the AssetStore interface, so normally the instance of the Application Class will be passed here.
	\param contextProvider The ContextProvider that this AssetStore will use to get a graphics context when required.
	*******************************************************************************************************************/
	void init(IAssetProvider& assetProvider, OSManager& contextProvider)
	{
		this->assetProvider = &assetProvider;
		this->contextProvider = &contextProvider;
		initialized = true;
	}

	/*!****************************************************************************************************************
	\brief Load a texture from the asset store, cache it and return a Texture object and/or Descriptor. If the texture
	is already loaded, return the cached information without loading from disc.
	\param[in] context The context on which the Texture object will be created.
	\param[in] filename texture file name
	\param[out] outTexture Optional (set NULL to ignore) : A api::Texture2D object into which the texture object
	is returned
	\param[out] outDescriptor Optional (set NULL to ignore) : A TextureHeader object into which the texture metadata
	is returned
	\return Returns true if successful, false if any error occurs.
	\description This function will look for a previously loaded texture with the specified filename. Texture format is
	inferred from the filename. If the texture is found in the cache, it will be returned from there, otherwise it will
	be loaded from the platform specific asset store. Errors are logged in the AssetManager logger.
	*******************************************************************************************************************/
	bool getTextureWithCaching(GraphicsContext& context, const StringHash& filename,
	                           api::TextureView* outTexture, assets::TextureHeader* outDescriptor)
	{
		return getTextureWithCaching(context, filename, assets::getTextureFormatFromFilename(filename.c_str()),
		                             (api::TextureView*)outTexture, outDescriptor);
	}

	/*!****************************************************************************************************************
	\brief Load a texture from the asset store, cache it and return a Texture object and/or Descriptor. If the texture
	is already loaded, return the cached information without loading from disc.
	\param[in] context The context on which the Texture object will be created.
	\param[in] filename texture file name
	\param[in] format A assets::TextureFileFormat symbolising the file format.
	\param[out] outTexture Optional (set NULL to ignore) : A api::Texture2D object into which the texture object
	is returned
	\param[out] outDescriptor Optional (set NULL to ignore) : A TextureHeader object into which the texture metadata
	is returned
	\return Returns true if successful, false if any error occurs.
	\description This function will look for a previously loaded texture with the specified filename. Texture format is
	explicit. If the texture is found in the cache, it will be returned from there, otherwise it will be loaded from the
	platform specific asset store (android asset, windows resource, filesystem etc.). Errors are logged in the
	AssetManager logger.
	*******************************************************************************************************************/
	bool getTextureWithCaching(GraphicsContext& context, const StringHash& filename, assets::TextureFileFormat format,
	                           api::TextureView* outTexture, assets::TextureHeader* outDescriptor)
	{
		std::map<StringHash, TextureData>::iterator found = textureMap.find(filename);
		if (found != textureMap.end())
		{
			if (outTexture)
			{
				*outTexture = found->second.texture;
			}
			if (outDescriptor)
			{
				*outDescriptor = found->second.textureHeader;
			}
			return true;
		}
		else
		{
			return loadTexture(context, filename, format, true, outTexture, outDescriptor);
		}
	}

	/*!****************************************************************************************************************
	\brief Load a texture from the asset store and cache it and for later use (through getTextureWithCaching). If the
	texture already exists, it is re-loaded from disk anyway (see loadTexture, where, if a texture is found, it is
	 returned instead of reloaded)
	\param[in] context The context on which the Texture object will be created.
	\param[in] filename texture file name
	\param[in] format A assets::TextureFileFormat symbolising the file format.
	\return Returns true if successful, false if any error occurs.
	\description This function will look for a previously loaded texture with the specified filename. Texture format is
	explicit. If the texture is found in the cache, it will be returned from there, otherwise it will be loaded from the
	platform specific asset store (android asset, windows resource, filesystem etc.). Errors are logged in the
	AssetManager logger.
	*******************************************************************************************************************/
	bool forceLoadTexture(GraphicsContext& context, const StringHash& filename, assets::TextureFileFormat format)
	{
		return loadTexture(context, filename, format, true, NULL, NULL);
	}

	/*!****************************************************************************************************************
	\brief Load a texture from the asset store and cache it and for later use (through getTextureWithCaching). If the
	texture already exists, it is re-loaded from disk anyway (see loadTexture, where, if a texture is found, it is
	 returned instead of reloaded). Texture format is inferred from the filename.
	\param[in] context The context on which the Texture object will be created.
	\param[in] filename texture file name
	\return Returns true if successful, false if any error occurs.
	\description This function will look for a previously loaded texture with the specified filename. Texture format is
	explicit. If the texture is found in the cache, it will be returned from there, otherwise it will be loaded from the
	platform specific asset store (android asset, windows resource, filesystem etc.). Errors are logged in the
	AssetManager logger.
	*******************************************************************************************************************/
	bool forceLoadTexture(GraphicsContext& context, const StringHash& filename)
	{
		return forceLoadTexture(context, filename, assets::getTextureFormatFromFilename(filename.c_str()));
	}

	bool generateTextureAtlas(GraphicsContext& context, const StringHash* fileNames, Rectanglef* outUVs, uint32 numTextures,
	                          api::TextureView* outTexture, assets::TextureHeader* outDescriptor);

private:
	bool loadTexture(GraphicsContext& context, const StringHash& filename,
	                 assets::TextureFileFormat format, bool forceLoad = false,
	                 api::TextureView* outTexture = NULL, assets::TextureHeader* outDescriptor = NULL);

public:

	/*!****************************************************************************************************************
	\brief Load model from file.
	\param[in] filename Model file name
	\param[out]	outModel A reference to a ModelHandle object. The model will be loaded there.
	\param[in] force (Default false) If true, will force loading the asset from the file, even if it is already
	           cached by the AssetStore.
	\return Returns true if successful, false if any error occurs.
	*******************************************************************************************************************/
	bool loadModel(const char* filename, assets::ModelHandle& outModel, bool force = false);

	/*!****************************************************************************************************************
	\brief Load pfx.
	\param[in] filename The filename of the pfx.
	\param[out] outPfx The PFX effect will be loaded into this Effect object.
	\param[in] force (Default false) If true, will force loading the asset from the file, even if it is already
	           cached by the AssetStore.
	\return return true if load success, else error
	*******************************************************************************************************************/
	bool loadPfx(const char* filename, EffectApi& outPfx, bool force = false);

	/*!****************************************************************************************************************
	\brief  Release all assets held by this AssetManager. Best practice is to always call this function in ReleaseView,
			as any resources held by the AssetManager will be invalid anyway.
			Calling this function or similar (usually releaseAll) is necessary so that the resources may be released.
			Otherwise, since the AssetStore is holding references to its objects, these objects are kept from being
			destroyed even after the user stops using them.
	*******************************************************************************************************************/
	void releaseAll()
	{
		textureMap.clear();
		modelMap.clear();

		compact(effects, unusedEffects);
		effects.clear();
		effectMap.clear();
	}

	/*!****************************************************************************************************************
	\brief Release any references to a specified Texture object that this AssetStore may be holding.
	\param[in] textureName The filename of a texture
	\description Calling this function or similar (usually releaseAll) is necessary so that the resources may be released.
	         Otherwise, since the AssetStore is holding references to its objects, these objects are kept from being
			 destroyed even after the user stops using them.
	*******************************************************************************************************************/
	void releaseTexture(const StringHash& textureName)
	{
		textureMap.erase(textureName);
	}

	/*!****************************************************************************************************************
	\brief Release any references to a specified Texture object that this AssetStore may be holding.
	\param[in] texture A texture
	\description Calling this function or similar (usually releaseAll) is necessary so that the resources may be released.
	         Otherwise, since the AssetStore is holding references to its objects, these objects are kept from being
			 destroyed even after the user stops using them.
	*******************************************************************************************************************/
	void findAndReleaseTexture(api::TextureView texture)
	{
		for (std::map<StringHash, TextureData>::iterator it = textureMap.begin(); it != textureMap.end(); ++it)
		{
			if (it->second.texture.get() == texture.get())
			{
				textureMap.erase(it);
				break;
			}
		}
	}

	/*!****************************************************************************************************************
	\brief Release any references to a specified Model object that this AssetStore may be holding.
	\param[in] model A handle to the model.
	\description Calling this function or similar (usually releaseAll) is necessary so that the resources may be released.
	         Otherwise, since the AssetStore is holding references to its objects, these objects are kept from being
			 destroyed even after the user stops using them.
	*******************************************************************************************************************/
	void findAndReleaseModel(assets::ModelHandle model)
	{
		for (std::map<StringHash, assets::ModelHandle>::iterator it = modelMap.begin(); it != modelMap.end();
		     ++it)
		{
			if (it->second.get() == model.get())
			{
				modelMap.erase(it);
				break;
			}
		}
	}

	/*!****************************************************************************************************************
	\brief Release any references to a specified Model object that this AssetStore may be holding.
	\param[in] modelName The filename of the model to release.
	\description Calling this function or similar (usually releaseAll) is necessary so that the resources may be released.
	         Otherwise, since the AssetStore is holding references to its objects, these objects are kept from being
			 destroyed even after the user stops using them.
	*******************************************************************************************************************/
	void releaseModel(const StringHash& modelName)
	{
		modelMap.erase(modelName);
	}
};


inline bool AssetStore::loadPfx(const char* /*fileName*/, EffectApi& /*outPfx*/, bool /*forceLoad*/)
{
	assertion(0 ,  "UNSUPPORTED REQUEST");
	return false;
}

}
}
