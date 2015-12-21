/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Includes all API object types.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/ForwardDecApiObjects.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRApi/ApiObjects/CommandBuffer.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"

namespace pvr {
namespace api {
/*!*****************************************************************************************************************
\brief        Interface for a class that can provide Asset Loading and Uploading. Used by PFX API parser and similar.
*******************************************************************************************************************/
class AssetLoadingDelegate
{
public:
	/*!****************************************************************************************************************
	\brief This function must implement Texture loading from a filename. Will be called to load the texture.
	\param[in] textureName The filename of the texture that must be loaded into outTex2d
	\param[out] outTex2d The implementation of this function must put the created texture into this variable
	\return error if texture could not be loaded
	*******************************************************************************************************************/
	virtual bool effectOnLoadTexture(const string& textureName, api::TextureView& outTex2d) = 0;

	/*!****************************************************************************************************************
	\brief dtor
	*******************************************************************************************************************/
	virtual ~AssetLoadingDelegate() { }
};
}
}

