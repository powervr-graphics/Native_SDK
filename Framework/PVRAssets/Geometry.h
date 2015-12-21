/*!*********************************************************************************************************************
\file         PVRAssets/Geometry.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
namespace tool {
	/*!********************************************************************************************************************
	\brief    Create a Skybox vertices and UVs for a specified texture size
    \param[in] scale scale the vertices
    \param[in] adjustUV
    \param[in] textureSize size of the texture
    \param[out] outVertices array of generated vertices
    \param[out] outUVs  array of generated UVs
	**********************************************************************************************************************/
    void createSkyBox(pvr::float32 scale, bool adjustUV, pvr::uint32 textureSize, std::vector<glm::vec3>& outVertices, std::vector<glm::vec2>& outUVs);
}
}