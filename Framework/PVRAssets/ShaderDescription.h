/*!*********************************************************************************************************************
\file         PVRAssets/ShaderDescription.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains a function used to get a shader type from a filename.
***********************************************************************************************************************/
#pragma once
#include "PVRAssets/AssetIncludes.h"

namespace pvr {
namespace assets {

/*!*********************************************************************************************************************
\brief     A function that gets a shader type from a filename.
\param     shaderName A filename(with extension) of a shader
\return    Return The ShaderType of this shader
***********************************************************************************************************************/
inline ShaderType getShaderTypeFromFilename(const char* shaderName)
{
	std::string file(shaderName);

	size_t period = file.rfind(".");
	if (period != string::npos)
	{
		string s = file.substr(period + 1);
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