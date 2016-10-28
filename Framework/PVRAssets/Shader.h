/*!*********************************************************************************************************************
\file         PVRAssets/Shader.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains a representation of a collection of Shader files that represent the same Shader for different versions
              of an API. A useful helper for automatically selecting shader versions based on API level.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRAssets/Asset.h"
#include "PVRCore/IAssetProvider.h"
#include "PVRCore/StringFunctions.h"
#include "PVRCore/IGraphicsContext.h"
namespace pvr {
namespace assets {

/*!*******************************************************************************************************************************
\brief	The ShaderFile class wraps the number of shader files of a specific shader for different apis.
**********************************************************************************************************************************/
class ShaderFile
{
	const std::string empty_string;
	//Use this with std::lower bound to get the first item that is Not Less than
	struct ApiComparatorNotLessThan
	{
		Api api;
		ApiComparatorNotLessThan(Api api) : api(api) {}
		bool operator()(const std::pair<pvr::Api, std::string>& lhs) {	return lhs.first >= api; }
	};
	//Use this with std::lower bound and reverse iterators to get the first item that is Not Greater than.
	//Useful to get the "best match" file
	struct ApiComparatorNotGreaterThan
	{
		Api api;
		ApiComparatorNotGreaterThan(Api api) : api(api) {}
		bool operator()(const std::pair<pvr::Api, std::string>& lhs) {	return lhs.first <= api; }
	};
	std::vector<std::pair<pvr::Api, std::string>/**/> m_filenames;
	IAssetProvider* m_assetProvider;
public:
	ShaderFile() : empty_string("") {}
	ShaderFile(const std::string& filename, IAssetProvider& assetProvider) : empty_string("")
	{
		populateValidVersions(filename, assetProvider);
	}


	/*!*******************************************************************************************************************************
	\brief	Get a stream object of this shader file for specific api
	\return Return stream object of this shader, else NULL object if no valid shader is found for given api
	\param	api Specific api requested for this shader file
	**********************************************************************************************************************************/
	Stream::ptr_type getStreamForSpecificApi(Api api)const
	{
		auto it = std::find_if(m_filenames.begin(), m_filenames.end(), ApiComparatorNotLessThan(api));
		if (it != m_filenames.end() && it->first != api) //Only want perfect match
		{
			return Stream::ptr_type();
		}
		return m_assetProvider->getAssetStream(it->second);
	}

	/*!*******************************************************************************************************************************
	\brief	Get the file name of this shader for specific api
	\return	Return a string of a file name for given api, else empty string if the shader is not supported for the given api
	\param	api Specific api requested for this shader file
	**********************************************************************************************************************************/
	const std::string& getFilenameForSpecificApi(Api api)const
	{
		auto it = std::find_if(m_filenames.begin(), m_filenames.end(), ApiComparatorNotLessThan(api));
		if (it != m_filenames.end() && it->first != api) {	return empty_string;	}
		return it->second;
	}

	/*!*******************************************************************************************************************************
	\brief	Get a best stream for the given api.
	\description The return stream may not be the exact one for the given api, but the one still supported by the api.
	\return	Return stream object of this shader, else NULL object if no valid shader is found for given api
	\param	api Specific api requested for this shader file
	**********************************************************************************************************************************/
	Stream::ptr_type getBestStreamForApi(Api api = Api::OpenGLESMaxVersion)const
	{
		auto it = std::find_if(m_filenames.rbegin(), m_filenames.rend(), ApiComparatorNotGreaterThan(api));
		if (it != m_filenames.rend()) //Will find the first filename that is not more than supported
		{
			return m_assetProvider->getAssetStream(it->second);
		}
		return Stream::ptr_type();
	}

	/*!*******************************************************************************************************************************
	\brief	Get a best stream for the given context
	\description The return stream may not be the exact one for the given context, but the one still supported by the context.
	\return	Return stream object of this shader, else NULL object if no valid shader is found for given context
	\param	context Specific context requested for this shader file
	**********************************************************************************************************************************/
	Stream::ptr_type getBestStreamForContext(const GraphicsContext& context)const
	{
		auto it = std::find_if(m_filenames.rbegin(), m_filenames.rend(), ApiComparatorNotGreaterThan(context->getApiType()));
		if (it != m_filenames.rend()) //Will find the first filename that is not more than supported
		{
			return m_assetProvider->getAssetStream(it->second);
		}
		return Stream::ptr_type();
	}

	/*!*******************************************************************************************************************************
	\brief	Get the best shader file name for the given api
	\description The return file name may not be the exact one for the given api, but the one still supported by the api.
	\return	Return a string of a file name for given api, else empty string if the shader is not supported for the given api
	\param	api Specific api requested for this shader file
	**********************************************************************************************************************************/
	const std::string& getBestFilenameForApi(Api api)const
	{
		auto it = std::find_if(m_filenames.rbegin(), m_filenames.rend(), ApiComparatorNotGreaterThan(api));
		if (it != m_filenames.rend()) {	return it->second;	}
		return empty_string;
	}

	/*!*******************************************************************************************************************************
	\brief	Get list of api versions supported
	\return	Return a list of supported version
	**********************************************************************************************************************************/
	std::vector<pvr::Api> getApiVersionsSupported() const
	{
		std::vector<pvr::Api> retval;
		for (auto it = m_filenames.begin(); it != m_filenames.end(); ++it)
		{
			retval.push_back(it->first);
		}
		return retval;
	}

	/*!*******************************************************************************************************************************
	\brief	Get list of all supported api files for this shader
	\return	Return list of all api shader files
	**********************************************************************************************************************************/
	std::vector<std::pair<pvr::Api, std::string>/**/>& getAllFiles() { return m_filenames; }

	/*!*******************************************************************************************************************************
	\brief	Set the shader file for specific api
	\param	filename shader file name
	\param	api specific api
	**********************************************************************************************************************************/
	void setFileForApi(const std::string& filename, Api api)
	{
		auto it = std::find_if(m_filenames.begin(), m_filenames.end(), ApiComparatorNotLessThan(api));
		if (it != m_filenames.end() && it->first == api)
		{
			it->second = filename;
		}
		else
		{
			m_filenames.insert(it, std::make_pair(api, filename));
		}
	}

	/*!*******************************************************************************************************************************
	\brief	Set the asset provider for this shader file which takes care of loading this shader
	\param	assetProvider shader loader
	**********************************************************************************************************************************/
	void setAssetProvider(IAssetProvider* assetProvider) {	m_assetProvider = assetProvider;	}

	/*!*******************************************************************************************************************************
	\brief	Populate a list of valid shader version for a given file
	\return	Return number of version populated.
	\param	filename shader file name to populate
	\param	assetProvider asset loader
	**********************************************************************************************************************************/
	int populateValidVersions(const std::string& filename, IAssetProvider& assetProvider)
	{
		m_filenames.clear();
		m_assetProvider = &assetProvider;

		//Case 1: The filename already refers to a specific API. In this case, as soon as it
		//is found, stop. We won't bother with others.
		std::string name, extension;
		strings::getFileNameAndExtension(filename, name, extension);
		for (int i = 1; i < (int)Api::Count; ++i)
		{
			if (strings::endsWith(name, apiCode(Api(i))))
			{
				Stream::ptr_type str = m_assetProvider->getAssetStream(filename, false);
				if (str.get())
				{
					setFileForApi(filename, Api(i));
					return 1; //This file has a suffix, so it only has 1 api
				}
				return 0; //Filename has a suffix but was not found... Let's not go over the top here, assume this is failure.
			}
		}
		int count = 0;

		//Being here means that the filename did not have an API suffix. Which is the main case: Load everything you find.
		//So now do the main work: Test all possible Apis...
		for (int i = 1; i < (int)Api::Count; ++i)
		{
			string file1 = name + "_" + apiCode(Api(i)) + (extension.size() ? "." + extension : "");
			if (i == (int)Api::Vulkan) { file1 += ".spv"; }
			Stream::ptr_type str = m_assetProvider->getAssetStream(file1, false);
			if (str.get())
			{
				setFileForApi(file1, Api(i));
				++count;
				continue;
			}
			file1 = name + apiCode(Api(i)) + (extension.size() ? "." + extension : "");
			str = m_assetProvider->getAssetStream(file1, false);
			if (str.get())
			{
				setFileForApi(file1, Api(i));
				++count;
			}
		}

		//Lastly: If we still cannot find a file, there is a last case: Legacy (the user is using a file without extension).
		//In this case, we will load it as minimum version (ES2) and get it over with.
		if (count == 0)
		{
			Stream::ptr_type str = m_assetProvider->getAssetStream(filename, false);
			if (str.get())
			{
				setFileForApi(filename, Api::OpenGLES2);
				return 1; //This file has a suffix, so it only has 1 api
			}
		}
		if (count == 0)
		{
			Log(Log.Error, "ShaderFile::populateValidVersions: No valid files found for filename [%s]", filename.c_str());
		}
		return count;
	}
};

}
}
