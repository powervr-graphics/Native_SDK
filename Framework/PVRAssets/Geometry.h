/*!
\brief Geometry helpers, such as skybox generations
\file PVRAssets/Geometry.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
namespace tool {
	/// <summary>Create a Skybox vertices and UVs for a specified texture size</summary>
	/// <param name="scale">scale the vertices</param>
	/// <param name="adjustUV"></param>
	/// <param name="textureSize">size of the texture</param>
	/// <param name="outVertices">array of generated vertices</param>
	/// <param name="outUVs">array of generated UVs</param>
    void createSkyBox(float scale, bool adjustUV, uint32_t textureSize, std::vector<glm::vec3>& outVertices, std::vector<glm::vec2>& outUVs);
}
}