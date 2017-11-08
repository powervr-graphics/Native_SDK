/*!
\brief Contains a function used to get a shader type from a filename.
\file PVRAssets/ShaderDescription.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/AssetIncludes.h"

namespace pvr {
namespace assets {

/// <summary>A function that gets a shader type from a filename.</summary>
/// <param name="shaderName">A filename(with extension) of a shader</param>
/// <returns>Return The ShaderType of this shader</returns>
inline ShaderType getShaderTypeFromFilename(const char* shaderName)
{
	std::string file(shaderName);

	size_t period = file.rfind(".");
	if (period != std::string::npos)
	{
		std::string s = file.substr(period + 1);
		std::transform(s.begin(), s.end(), s.begin(), tolower);
		if (!s.compare("vsh")) { return ShaderType::VertexShader; }
		if (!s.compare("vs")) { return ShaderType::VertexShader; }
		if (!s.compare("fsh")) { return ShaderType::FragmentShader; }
		if (!s.compare("fs")) { return ShaderType::FragmentShader; }
		if (!s.compare("csh")) { return ShaderType::ComputeShader; }
		if (!s.compare("cs")) { return ShaderType::ComputeShader; }
	}
	return ShaderType::UnknownShader;
}
}
}