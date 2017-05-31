/*!
\brief Includes all API object types.
\file         PVRApi/ApiObjects.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRApi/ApiObjects/Buffer.h"
#include "PVRApi/ApiObjects/Sampler.h"
#include "PVRApi/ApiObjects/GraphicsPipeline.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRApi/ApiObjects/CommandPool.h"
#include "PVRApi/ApiObjects/RenderPass.h"
<<<<<<< HEAD
#include "PVRApi/ApiIncludes.h"

=======
#include "PVRApi/ApiObjects/Fbo.h"
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/Sync.h"
#include "PVRApi/ApiObjects/VertexRayPipeline.h"
#include "PVRApi/ApiObjects/SceneTraversalPipeline.h"
#include "PVRApi/ApiObjects/SceneHierarchy.h"
#include "PVRApi/ApiObjects/IndirectRayPipeline.h"
>>>>>>> 1776432f... 4.3

namespace pvr {
namespace utils {
/// <summary>Interface for a class that can provide Asset Loading and Uploading. Used by PFX API parser and similar.
/// </summary>
class AssetLoadingDelegate
{
public:
	/// <summary>This function must implement Texture loading from a filename. Will be called to load the texture.
	/// </summary>
	/// <param name="textureName">The filename of the texture that must be loaded into outTex2d</param>
	/// <param name="outTex2d">The implementation of this function must put the created texture into this variable
	/// </param>
	/// <returns>error if texture could not be loaded</returns>
	virtual bool effectOnLoadTexture(const string& textureName, api::TextureView& outTex2d) = 0;

	/// <summary>dtor</summary>
	virtual ~AssetLoadingDelegate() { }
};
}
}
