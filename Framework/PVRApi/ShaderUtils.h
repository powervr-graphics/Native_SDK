/*!*********************************************************************************************************************
\file         PVRApi\ShaderUtils.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		Contains useful low level utils for shaders (loading, compiling) into low level Api object wrappers.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/IGraphicsContext.h"
#include "PVRApi/ApiObjects/Shader.h"
#include "PVRAssets/Shader.h"

namespace pvr {
namespace native {
struct HShader_;
typedef RefCountedResource<HShader_> HShader;
struct HShaderProgram_;
typedef RefCountedResource<HShaderProgram_> HShaderProgram;
}

namespace utils {

/*!*********************************************************************************************************************
\brief Load a shader from binary.
\param shaderData A stream containing the shader binary data
\param shaderType The type (stage) of the shader (vertex, fragment...)
\param binaryFormat The binary format of the shader
\param[out] outShader The native Shader object will be saved here
\param contextCapabilities Optional A pointer to the API capabilities can be passed here
\return true on success
***********************************************************************************************************************/
bool loadShader(Stream& shaderData, ShaderType::Enum shaderType, assets::ShaderBinaryFormat::Enum binaryFormat, native::HShader& outShader, const ApiCapabilities* contextCapabilities = 0);


/*!*********************************************************************************************************************
\brief Load shader from shader source.
\param shaderSource A stream containing the shader source text data
\param shaderType The type (stage) of the shader (vertex, fragment...)
\param defines A number of preprocessor definitions that will be passed to the shader
\param numDefines The number of defines
\param[out] outShader The native Shader object will be saved here
\param contextCapabilities Optional A pointer to the API capabilities can be passed here
\return true on success
***********************************************************************************************************************/
bool loadShader(const Stream& shaderSource, ShaderType::Enum shaderType, const char* const* defines, uint32 numDefines, native::HShader& outShader, const ApiCapabilities* contextCapabilities = 0);


/*!*********************************************************************************************************************
\brief Create a native shader program from an array of native shader handles.
\param pShaders array of shaders
\param count number shaders in the array
\param attribs array of attributes
\param attribIndex array of attributeIndices
\param attribCount Number of attributes in the attributes array
\param outShaderProg Output, the shader program
\param infolog OPTIONAL Output, the infolog of the shader
\param contextCapabilities OPTIONAL can be used to pass specific context capabilities
\return true on success
***********************************************************************************************************************/
bool createShaderProgram(native::HShader_ pShaders[], uint32 count, const char** const attribs, pvr::uint16* attribIndex,uint32 attribCount, native::HShaderProgram& outShaderProg,
                                 string* infolog, const ApiCapabilities* contextCapabilities = 0);
}
}