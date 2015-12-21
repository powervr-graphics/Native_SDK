/*!*********************************************************************************************************************
\file         PVRAssets/Geometry.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains definition of the Geometry.h function.
***********************************************************************************************************************/
#include "PVRAssets/Geometry.h"

namespace pvr {

inline static void setVertex(glm::vec3& vertex, pvr::float32 x, pvr::float32 y, pvr::float32 z)
{
    vertex.x = x, vertex.y = y, vertex.z = z;
}

inline static void setUV(glm::vec2& uv, pvr::float32 u, pvr::float32 v)
{
    uv.x = u, uv.y = v;
}

void tool::createSkyBox(pvr::float32 scale, bool adjustUV, pvr::uint32 textureSize, std::vector<glm::vec3>& outVertices, std::vector<glm::vec2>& outUVs)
{
	pvr::float32 unit = 1.f, a0 = 1.f, a1 = 1.f;
	outVertices.resize(24);
	outUVs.resize(24);

	if (adjustUV)
	{
		const pvr::float32 oneOverTexSize = 1.f / textureSize;
		a0 = 4.f * oneOverTexSize;
		a1 = a0 - unit;
	}

	unit *= scale;
	// Front
	setVertex(outVertices[0], -unit, +unit, -unit);
	setVertex(outVertices[1], +unit, +unit, -unit);
	setVertex(outVertices[2], -unit, -unit, -unit);
	setVertex(outVertices[3], +unit, -unit, -unit);
	setUV(outUVs[0], a0, a1);
	setUV(outUVs[1], a1, a1);
	setUV(outUVs[2], a0, a0);
	setUV(outUVs[3], a1, a0);

	// Right
	setVertex(outVertices[4], +unit, +unit, -unit);
	setVertex(outVertices[5], +unit, +unit, +unit);
	setVertex(outVertices[6], +unit, -unit, -unit);
	setVertex(outVertices[7], +unit, -unit, +unit);
	setUV(outUVs[4], a0, a1);
	setUV(outUVs[5], a1, a1);
	setUV(outUVs[6], a0, a0);
	setUV(outUVs[7], a1, a0);

	// Back
	setVertex(outVertices[8], +unit, +unit, +unit);
	setVertex(outVertices[9], -unit, +unit, +unit);
	setVertex(outVertices[10], +unit, -unit, +unit);
	setVertex(outVertices[11], -unit, -unit, +unit);
	setUV(outUVs[8], a0, a1);
	setUV(outUVs[9], a1, a1);
	setUV(outUVs[10], a0, a0);
	setUV(outUVs[11], a1, a0);

	// Left
	setVertex(outVertices[12], -unit, +unit, +unit);
	setVertex(outVertices[13], -unit, +unit, -unit);
	setVertex(outVertices[14], -unit, -unit, +unit);
	setVertex(outVertices[15], -unit, -unit, -unit);
	setUV(outUVs[ 12], a0, a1);
	setUV(outUVs[13], a1, a1);
	setUV(outUVs[14], a0, a0);
	setUV(outUVs[15], a1, a0);

	// Top
	setVertex(outVertices[16], -unit, +unit, +unit);
	setVertex(outVertices[17], +unit, +unit, +unit);
	setVertex(outVertices[18], -unit, +unit, -unit);
	setVertex(outVertices[19], +unit, +unit, -unit);
	setUV(outUVs[16], a0, a1);
	setUV(outUVs[17], a1, a1);
	setUV(outUVs[18], a0, a0);
	setUV(outUVs[19], a1, a0);

	// Bottom
	setVertex(outVertices[20], -unit, -unit, -unit);
	setVertex(outVertices[21], +unit, -unit, -unit);
	setVertex(outVertices[22], -unit, -unit, +unit);
	setVertex(outVertices[23], +unit, -unit, +unit);
	setUV(outUVs[20], a0, a1);
	setUV(outUVs[21], a1, a1);
	setUV(outUVs[22], a0, a0);
	setUV(outUVs[23], a1, a0);
}
}