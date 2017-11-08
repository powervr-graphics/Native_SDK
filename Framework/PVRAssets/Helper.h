/*!
\brief Internal helper classes
\file PVRAssets/Helper.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRAssets/Model/Mesh.h"
#include "PVRAssets/Model.h"
namespace pvr {
namespace assets {
namespace helper {
/// <summary>Read vertex data into float buffer.</summary>
/// <param name="data">Data to read from</param>
/// <param name="type">Data type of the vertex to read</param>
/// <param name="count">Number of vertices to read</param>
/// <param name="out">Array of vertex read</param>
void VertexRead(const uint8_t* data, const DataType type, uint32_t count, float* out);

/// <summary>Read vertex index data into uin32 buffer.</summary>
/// <param name="data">Data to read from</param>
/// <param name="type">Index type to read</param>
/// <param name="out">of index data read</param>
void VertexIndexRead(const uint8_t* data, const IndexType type, uint32_t* const out);

/// <summary>loads a model from the provided file and fill out the model.</summary>
/// <param name="assetProvider">The asset provider to use for opening the asset stream</param>
/// <param name="filename">The filename to read the model from</param>
/// <param name="outModel">The model to fill</param>
/// <returns>True if the loading was successful, false if not</param>
bool loadModel(IAssetProvider& assetProvider, const char* filename, assets::ModelHandle& outModel);
}// namespace helper
}// namespace assets
}// namespace pvr