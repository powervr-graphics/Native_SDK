/*!
\brief Contains the AssetStore, a convenience class that can be used to load assets from the filesystem and upload them
into API objects, while ensuring no duplicate loading happens.
\file PVREngineUtils/AssetStore.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/EffectApi.h"
#include "PVRAssets/Shader.h"
#include "PVRAssets/Model.h"
#include "PVRAssets/FileIO/PODReader.h"
#include "PVRAssets/FileIO/PFXParser.h"
#include "PVRCore/Stream.h"
#include "PVRCore/Texture.h"
namespace pvr {
namespace utils {

/// <summary>Manages scene assets. Use this class to easily load assets without needing to worry about duplicates.
/// This class keeps references to assets, so remember to release them if they are no longer required.</summary>
class AssetStore : public pvr::utils::AssetLoadingDelegate
{
public:
	typedef int32 AssetId;

	static const AssetId NoAsset = -1;

private:
	struct TextureData { TextureHeader textureHeader; api::TextureView texture; };

	IAssetProvider* assetProvider;
	OSManager* contextProvider;

	std::map<StringHash, TextureData> textureMap;
	std::map<StringHash, assets::ModelHandle> modelMap;
	std::vector<legacyPfx::EffectApi> effects;
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
	/// <summary>Implementation of the AssetLoadingDelegate. Allows this class to be passed to the EffectApi constructor as the
	/// effectDelegate to automate loading and uploading of textures.</summary>
	/// <param name="textureName">A texture filename to load from a file.</param>
	/// <param name="outTex2d">A handle where the loaded and uploaded texture will be saved.</param>
	bool effectOnLoadTexture(const string& textureName, api::TextureView& outTex2d);

	/// <summary>Constructor.</summary>
	/// <param name="logger">A printf-style function pointer which will be used by the class to record errors that
	/// happen during any operation.</param>
	AssetStore(ErrorLogger logger = &Logger::static_output) : assetProvider(NULL), logger(logger),
		initialized(false)
	{ }

	/// <summary>Initialize with the application class (the Shell, hence the Application, is-a IPlatformProvider</summary>
	/// <param name="theShell">The IAssetProvider that this AssetStore will use to load the requested assets from
	/// disk. pvr::Shell implements the AssetStore interface, so normally the instance of the Application Class will
	/// be passed here.</param>
	void init(pvr::IPlatformProvider& theShell)
	{
		assetProvider = &theShell;
		contextProvider = &theShell;
		initialized = true;
	}

	/// <summary>Initialize with separate Context and Asset providers. Prefer the other overload if you want to just pass the
	/// application.</summary>
	/// <param name="assetProvider">The IAssetProvider that this AssetStore will use to load the requested assets from
	/// disk. pvr::Shell implements the AssetStore interface, so normally the instance of the Application Class will
	/// be passed here.</param>
	/// <param name="contextProvider">The ContextProvider that this AssetStore will use to get a graphics context when
	/// required.</param>
	void init(IAssetProvider& assetProvider, OSManager& contextProvider)
	{
		this->assetProvider = &assetProvider;
		this->contextProvider = &contextProvider;
		initialized = true;
	}

	/// <summary>Load a texture from the asset store, cache it and return a Texture object and/or Descriptor. If the
	/// texture is already loaded, return the cached information without loading from disc.</summary>
	/// <param name="context">The context on which the Texture object will be created.</param>
	/// <param name="filename">texture file name</param>
	/// <param name="outTexture">Optional (set NULL to ignore) : A api::Texture2D object into which the texture object is
	/// returned</param>
	/// <param name="outDescriptor">Optional (set NULL to ignore) : A TextureHeader object into which the texture metadata
	/// is returned</param>
	/// <returns>Returns true if successful, false if any error occurs.</returns>
	/// <remarks>This function will look for a previously loaded texture with the specified filename. Texture format is
	/// inferred from the filename. If the texture is found in the cache, it will be returned from there, otherwise it
	/// will be loaded from the platform specific asset store. Errors are logged in the AssetManager logger.
	/// </remarks>
	bool getTextureWithCaching(GraphicsContext& context, const StringHash& filename,
	                           api::TextureView* outTexture, TextureHeader* outDescriptor)
	{
		return getTextureWithCaching(context, filename, getTextureFormatFromFilename(filename.c_str()),
		                             (api::TextureView*)outTexture, outDescriptor);
	}

	/// <summary>Load a texture from the asset store, cache it and return a Texture object and/or Descriptor. If the
	/// texture is already loaded, return the cached information without loading from disc.</summary>
	/// <param name="context">The context on which the Texture object will be created.</param>
	/// <param name="filename">texture file name</param>
	/// <param name="format">A TextureFileFormat symbolising the file format.</param>
	/// <param name="outTexture">Optional (set NULL to ignore) : A api::Texture2D object into which the texture object is
	/// returned</param>
	/// <param name="outDescriptor">Optional (set NULL to ignore) : A TextureHeader object into which the texture metadata
	/// is returned</param>
	/// <returns>Returns true if successful, false if any error occurs.</returns>
	/// <remarks>This function will look for a previously loaded texture with the specified filename. Texture format is
	/// explicit. If the texture is found in the cache, it will be returned from there, otherwise it will be loaded
	/// from the platform specific asset store (android asset, windows resource, filesystem etc.). Errors are logged
	/// in the AssetManager logger.</remarks>
	bool getTextureWithCaching(GraphicsContext& context, const StringHash& filename, TextureFileFormat format,
	                           api::TextureView* outTexture, TextureHeader* outDescriptor)
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

	/// <summary>Load a texture from the asset store and cache it and for later use (through getTextureWithCaching). If
	/// the texture already exists, it is re-loaded from disk anyway (see loadTexture, where, if a texture is found,
	/// it is returned instead of reloaded)</summary>
	/// <param name="context">The context on which the Texture object will be created.</param>
	/// <param name="filename">texture file name</param>
	/// <param name="format">A TextureFileFormat symbolising the file format.</param>
	/// <returns>Returns true if successful, false if any error occurs.</returns>
	/// <remarks>This function will look for a previously loaded texture with the specified filename. Texture format is
	/// explicit. If the texture is found in the cache, it will be returned from there, otherwise it will be loaded
	/// from the platform specific asset store (android asset, windows resource, filesystem etc.). Errors are logged
	/// in the AssetManager logger.</remarks>
	bool forceLoadTexture(GraphicsContext& context, const StringHash& filename, TextureFileFormat format)
	{
		return loadTexture(context, filename, format, true, NULL, NULL);
	}

	/// <summary>Load a texture from the asset store and cache it and for later use (through getTextureWithCaching). If
	/// the texture already exists, it is re-loaded from disk anyway (see loadTexture, where, if a texture is found,
	/// it is returned instead of reloaded). Texture format is inferred from the filename.</summary>
	/// <param name="context">The context on which the Texture object will be created.</param>
	/// <param name="filename">texture file name</param>
	/// <returns>Returns true if successful, false if any error occurs.</returns>
	/// <remarks>This function will look for a previously loaded texture with the specified filename. Texture format is
	/// explicit. If the texture is found in the cache, it will be returned from there, otherwise it will be loaded
	/// from the platform specific asset store (android asset, windows resource, filesystem etc.). Errors are logged
	/// in the AssetManager logger.</remarks>
	bool forceLoadTexture(GraphicsContext& context, const StringHash& filename)
	{
		return forceLoadTexture(context, filename, getTextureFormatFromFilename(filename.c_str()));
	}

	bool generateTextureAtlas(GraphicsContext& context, const StringHash* fileNames, Rectanglef* outUVs, uint32 numTextures,
	                          api::TextureView* outTexture, TextureHeader* outDescriptor);

private:
	bool loadTexture(GraphicsContext& context, const StringHash& filename,
	                 TextureFileFormat format, bool forceLoad = false,
	                 api::TextureView* outTexture = NULL, TextureHeader* outDescriptor = NULL);

public:

	/// <summary>Load model from file.</summary>
	/// <param name="filename">Model file name</param>
	/// <param name="outModel">A reference to a ModelHandle object. The model will be loaded there.</param>
	/// <param name="force">(Default false) If true, will force loading the asset from the file, even if it is already
	/// cached by the AssetStore.</param>
	/// <returns>Returns true if successful, false if any error occurs.</returns>
	bool loadModel(const char* filename, assets::ModelHandle& outModel, bool force = false);

	/// <summary>Load pfx.</summary>
	/// <param name="filename">The filename of the pfx.</param>
	/// <param name="outPfx">The PFX effect will be loaded into this Effect object.</param>
	/// <param name="force">(Default false) If true, will force loading the asset from the file, even if it is already
	/// cached by the AssetStore.</param>
	/// <returns>return true if load success, else error</returns>
	bool loadPfx(const char* filename, legacyPfx::EffectApi& outPfx, bool force = false);

	/// <summary>Release all assets held by this AssetManager. Best practice is to always call this function in
	/// ReleaseView, as any resources held by the AssetManager will be invalid anyway. Calling this function or
	/// similar (usually releaseAll) is necessary so that the resources may be released. Otherwise, since the
	/// AssetStore is holding references to its objects, these objects are kept from being destroyed even after the
	/// user stops using them.</summary>
	void releaseAll()
	{
		textureMap.clear();
		modelMap.clear();

		compact(effects, unusedEffects);
		effects.clear();
		effectMap.clear();
	}

	/// <summary>Release any references to a specified Texture object that this AssetStore may be holding.</summary>
	/// <param name="textureName">The filename of a texture</param>
	/// <remarks>Calling this function or similar (usually releaseAll) is necessary so that the resources may be released.
	/// Otherwise, since the AssetStore is holding references to its objects, these objects are kept from being
	/// destroyed even after the user stops using them.</remarks>
	void releaseTexture(const StringHash& textureName)
	{
		textureMap.erase(textureName);
	}

	/// <summary>Release any references to a specified Texture object that this AssetStore may be holding.</summary>
	/// <param name="texture">A texture</param>
	/// <remarks>Calling this function or similar (usually releaseAll) is necessary so that the resources may be released.
	/// Otherwise, since the AssetStore is holding references to its objects, these objects are kept from being
	/// destroyed even after the user stops using them.</remarks>
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

	/// <summary>Release any references to a specified Model object that this AssetStore may be holding.</summary>
	/// <param name="model">A handle to the model.</param>
	/// <remarks>Calling this function or similar (usually releaseAll) is necessary so that the resources may be released.
	/// Otherwise, since the AssetStore is holding references to its objects, these objects are kept from being
	/// destroyed even after the user stops using them.</remarks>
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

	/// <summary>Release any references to a specified Model object that this AssetStore may be holding.</summary>
	/// <param name="modelName">The filename of the model to release.</param>
	/// <remarks>Calling this function or similar (usually releaseAll) is necessary so that the resources may be released.
	/// Otherwise, since the AssetStore is holding references to its objects, these objects are kept from being
	/// destroyed even after the user stops using them.</remarks>
	void releaseModel(const StringHash& modelName)
	{
		modelMap.erase(modelName);
	}
};


typedef RefCountedResource<legacyPfx::impl::EffectApi_> EffectApi;

inline bool AssetStore::loadPfx(const char* /*fileName*/, utils::EffectApi& /*outPfx*/, bool /*forceLoad*/)
{
	assertion(0 ,  "UNSUPPORTED REQUEST");
	return false;
}

}
}