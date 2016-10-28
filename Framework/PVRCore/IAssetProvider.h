/*!*********************************************************************************************************************
\file         PVRCore\IAssetProvider.h
\author       PowerVR by Imagination, Developer Technology Team.
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains the IAssetProvider interface, which decouples the Shell file retrieval functions from the Shell class.
***********************************************************************************************************************/
#pragma once

#include "PVRCore/Defines.h"

namespace pvr {
class IGraphicsContext;
class Stream;
/*!*********************************************************************************************************************
\brief        The IAssetProvider interface marks a class that provides the getAssetStream and getGraphicsContext methods.
***********************************************************************************************************************/
class IAssetProvider
{
public:
	/*!*********************************************************************************************************************
	\brief    Create a Stream from the provided filename. The actual source of the stream is abstracted (filesystem, resources
			etc.).
	\param    filename The name of the asset. May contain a path or not. Platform-specific paths and built-in resources should
	          be searched.
	\param    logErrorOnNotFound OPTIONAL. Set this to false to avoid logging an error when the file is not found.
	\return   A pointer to a Stream. NULL if the asset is not found.
	\description  If the file is not found, this function will return a NULL function pointer, and log an error. In cases where
	          file not found is to be expected (for example, testing for different files due to convention), set logErrorOnNotFound
			  to false to avoid cluttering the Log.
	***********************************************************************************************************************/
	virtual std::auto_ptr<Stream> getAssetStream(const string& filename, bool logErrorOnNotFound = true) = 0;

};
}