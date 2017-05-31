/*!
\brief Contains a representation of a collection of Shader files that represent the same Shader for different versions
of an API. A useful helper for automatically selecting shader versions based on API level.
\file PVRAssets/Shader.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/AssetIncludes.h"
#include "PVRCore/Interfaces/IAssetProvider.h"
#include "PVRCore/StringFunctions.h"
<<<<<<< HEAD
#include "PVRCore/IGraphicsContext.h"
=======
#include "PVRCore/Interfaces/IGraphicsContext.h"
>>>>>>> 1776432f... 4.3
namespace pvr {
namespace assets {

/// <summary>The ShaderFile class wraps the number of shader files of a specific shader for different apis.
/// </summary>
class ShaderFile
{
	const std::string empty_string;
	//Use this with std::lower bound to get the first item that is Not Less than
	struct ApiComparatorNotLessThan
	{
		Api api;
		ApiComparatorNotLessThan(Api api) : api(api) {}
<<<<<<< HEAD
		bool operator()(const std::pair<pvr::Api, std::string>& lhs) {	return lhs.first >= api; }
=======
		bool operator()(const std::pair<pvr::Api, std::string>& lhs) {  return lhs.first >= api; }
>>>>>>> 1776432f... 4.3
	};
	//Use this with std::lower bound and reverse iterators to get the first item that is Not Greater than.
	//Useful to get the "best match" file
	struct ApiComparatorNotGreaterThan
	{
		Api api;
		ApiComparatorNotGreaterThan(Api api) : api(api) {}
<<<<<<< HEAD
		bool operator()(const std::pair<pvr::Api, std::string>& lhs) {	return lhs.first <= api; }
	};
	std::vector<std::pair<pvr::Api, std::string>/**/> m_filenames;
	IAssetProvider* m_assetProvider;
=======
		bool operator()(const std::pair<pvr::Api, std::string>& lhs) {  return lhs.first <= api; }
	};
	std::vector<std::pair<pvr::Api, std::string>/**/> _filenames;
	IAssetProvider* _assetProvider;
>>>>>>> 1776432f... 4.3
public:
	ShaderFile() : empty_string("") {}
	ShaderFile(const std::string& filename, IAssetProvider& assetProvider) : empty_string("")
	{
		populateValidVersions(filename, assetProvider);
	}


<<<<<<< HEAD
	/*!*******************************************************************************************************************************
	\brief	Get a stream object of this shader file for specific api
	\return Return stream object of this shader, else NULL object if no valid shader is found for given api
	\param	api Specific api requested for this shader file
	**********************************************************************************************************************************/
=======
	/// <summary>Get a stream object of this shader file for specific api</summary>
	/// <param name="api">Specific api requested for this shader file</param>
	/// <returns>Return stream object of this shader, else NULL object if no valid shader is found for given api
	/// </returns>
>>>>>>> 1776432f... 4.3
	Stream::ptr_type getStreamForSpecificApi(Api api)const
	{
		auto it = std::find_if(_filenames.begin(), _filenames.end(), ApiComparatorNotLessThan(api));
		if (it != _filenames.end() && it->first != api) //Only want perfect match
		{
			return Stream::ptr_type();
		}
		return _assetProvider->getAssetStream(it->second);
	}

<<<<<<< HEAD
	/*!*******************************************************************************************************************************
	\brief	Get the file name of this shader for specific api
	\return	Return a string of a file name for given api, else empty string if the shader is not supported for the given api
	\param	api Specific api requested for this shader file
	**********************************************************************************************************************************/
=======
	/// <summary>Get the file name of this shader for specific api</summary>
	/// <param name="api">Specific api requested for this shader file</param>
	/// <returns>Return a string of a file name for given api, else empty string if the shader is not supported for the
	/// given api</returns>
>>>>>>> 1776432f... 4.3
	const std::string& getFilenameForSpecificApi(Api api)const
	{
		auto it = std::find_if(_filenames.begin(), _filenames.end(), ApiComparatorNotLessThan(api));
		if (it != _filenames.end() && it->first != api) { return empty_string;  }
		return it->second;
	}

<<<<<<< HEAD
	/*!*******************************************************************************************************************************
	\brief	Get a best stream for the given api.
	\description The return stream may not be the exact one for the given api, but the one still supported by the api.
	\return	Return stream object of this shader, else NULL object if no valid shader is found for given api
	\param	api Specific api requested for this shader file
	**********************************************************************************************************************************/
=======
	/// <summary>Get a best stream for the given api.</summary>
	/// <param name="api">Specific api requested for this shader file</param>
	/// <returns>Return stream object of this shader, else NULL object if no valid shader is found for given api
	/// </returns>
	/// <remarks>The return stream may not be the exact one for the given api, but the one still supported by the api.
	/// </remarks>
>>>>>>> 1776432f... 4.3
	Stream::ptr_type getBestStreamForApi(Api api = Api::OpenGLESMaxVersion)const
	{
		auto it = std::find_if(_filenames.rbegin(), _filenames.rend(), ApiComparatorNotGreaterThan(api));
		if (it != _filenames.rend()) //Will find the first filename that is not more than supported
		{
			return _assetProvider->getAssetStream(it->second);
		}
		return Stream::ptr_type();
	}

	/// <summary>Get a best stream for the given context</summary>
	/// <param name="context">Specific context requested for this shader file</param>
	/// <returns>Return stream object of this shader, else NULL object if no valid shader is found for given context
	/// </returns>
	/// <remarks>The return stream may not be the exact one for the given context, but the one still supported by the
	/// context.</remarks>
	Stream::ptr_type getBestStreamForContext(const GraphicsContext& context)const
	{
		auto it = std::find_if(_filenames.rbegin(), _filenames.rend(), ApiComparatorNotGreaterThan(context->getApiType()));
		if (it != _filenames.rend()) //Will find the first filename that is not more than supported
		{
			return _assetProvider->getAssetStream(it->second);
		}
		return Stream::ptr_type();
	}

<<<<<<< HEAD
	/*!*******************************************************************************************************************************
	\brief	Get the best shader file name for the given api
	\description The return file name may not be the exact one for the given api, but the one still supported by the api.
	\return	Return a string of a file name for given api, else empty string if the shader is not supported for the given api
	\param	api Specific api requested for this shader file
	**********************************************************************************************************************************/
=======
	/// <summary>Get the best shader file name for the given api</summary>
	/// <param name="api">Specific api requested for this shader file</param>
	/// <returns>Return a string of a file name for given api, else empty string if the shader is not supported for the
	/// given api</returns>
	/// <remarks>The return file name may not be the exact one for the given api, but the one still supported by the
	/// api.</remarks>
>>>>>>> 1776432f... 4.3
	const std::string& getBestFilenameForApi(Api api)const
	{
		auto it = std::find_if(_filenames.rbegin(), _filenames.rend(), ApiComparatorNotGreaterThan(api));
		if (it != _filenames.rend()) {  return it->second;  }
		return empty_string;
	}

<<<<<<< HEAD
	/*!*******************************************************************************************************************************
	\brief	Get list of api versions supported
	\return	Return a list of supported version
	**********************************************************************************************************************************/
	std::vector<pvr::Api> getApiVersionsSupported() const
	{
		std::vector<pvr::Api> retval;
		for (auto it = m_filenames.begin(); it != m_filenames.end(); ++it)
=======
	/// <summary>Get list of api versions supported</summary>
	/// <returns>Return a list of supported version</returns>
	std::vector<pvr::Api> getApiVersionsSupported() const
	{
		std::vector<pvr::Api> retval;
		for (auto it = _filenames.begin(); it != _filenames.end(); ++it)
>>>>>>> 1776432f... 4.3
		{
			retval.push_back(it->first);
		}
		return retval;
	}

<<<<<<< HEAD
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
=======
	/// <summary>Get list of all supported api files for this shader</summary>
	/// <returns>Return list of all api shader files</returns>
	std::vector<std::pair<pvr::Api, std::string>/**/>& getAllFiles() { return _filenames; }

	/// <summary>Set the shader file for specific api</summary>
	/// <param name="filename">shader file name</param>
	/// <param name="api">specific api</param>
>>>>>>> 1776432f... 4.3
	void setFileForApi(const std::string& filename, Api api)
	{
		auto it = std::find_if(_filenames.begin(), _filenames.end(), ApiComparatorNotLessThan(api));
		if (it != _filenames.end() && it->first == api)
		{
			it->second = filename;
		}
		else
		{
			_filenames.insert(it, std::make_pair(api, filename));
		}
	}

	/// <summary>Set the asset provider for this shader file which takes care of loading this shader</summary>
	/// <param name="assetProvider">shader loader</param>
	void setAssetProvider(IAssetProvider* assetProvider) {  _assetProvider = assetProvider; }

	/// <summary>Populate a list of valid shader version for a given file</summary>
	/// <param name="filename">shader file name to populate</param>
	/// <param name="assetProvider">asset loader</param>
	/// <returns>Return number of version populated.</returns>
	int populateValidVersions(const std::string& filename, IAssetProvider& assetProvider)
	{
		_filenames.clear();
		_assetProvider = &assetProvider;

		//Case 1: The filename already refers to a specific API. In this case, as soon as it
		//is found, stop. We won't bother with others.
		std::string name, extension;
		strings::getFileNameAndExtension(filename, name, extension);
		for (int i = 1; i < (int)Api::Count; ++i)
		{
			if (strings::endsWith(name, apiCode(Api(i))))
			{
				Stream::ptr_type str = _assetProvider->getAssetStream(filename, false);
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
<<<<<<< HEAD
			Stream::ptr_type str = m_assetProvider->getAssetStream(file1, false);
=======
			Stream::ptr_type str = _assetProvider->getAssetStream(file1, false);
>>>>>>> 1776432f... 4.3
			if (str.get())
			{
				setFileForApi(file1, Api(i));
				++count;
				continue;
			}
			file1 = name + apiCode(Api(i)) + (extension.size() ? "." + extension : "");
<<<<<<< HEAD
			str = m_assetProvider->getAssetStream(file1, false);
=======
			str = _assetProvider->getAssetStream(file1, false);
>>>>>>> 1776432f... 4.3
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
			Stream::ptr_type str = _assetProvider->getAssetStream(filename, false);
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
