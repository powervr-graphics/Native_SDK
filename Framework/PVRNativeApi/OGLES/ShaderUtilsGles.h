/*!
\brief Contains useful low level utils for shaders (loading, compiling) into low level Api object wrappers.
\file PVRNativeApi/OGLES/ShaderUtilsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Interfaces/IGraphicsContext.h"

namespace pvr {
namespace nativeGles {

/// <summary>Load a shader from binary. Will implicitly load on the current context.</summary>
/// <param name="shaderData">A stream containing the shader binary data</param>
/// <param name="shaderType">The type (stage) of the shader (vertex, fragment...)</param>
/// <param name="binaryFormat">The binary format of the shader</param>
/// <param name="outShader">The native Shader object will be saved here</param>
/// <param name="contextCapabilities">Optional A pointer to the API capabilities can be passed here</param>
/// <returns>true on success</returns>
bool loadShader(Stream& shaderData, types::ShaderType shaderType, types::ShaderBinaryFormat binaryFormat,
                native::HShader_& outShader, const ApiCapabilities* contextCapabilities = 0);


/// <summary>Load shader from shader source. Will implicitly load on the current context.</summary>
/// <param name="shaderSource">A stream containing the shader source text data</param>
/// <param name="shaderType">The type (stage) of the shader (vertex, fragment...)</param>
/// <param name="defines">A number of preprocessor definitions that will be passed to the shader</param>
/// <param name="numDefines">The number of defines</param>
/// <param name="outShader">The native Shader object will be saved here</param>
/// <param name="contextCapabilities">Optional A pointer to the API capabilities can be passed here</param>
/// <returns>true on success</returns>
bool loadShader(const Stream& shaderSource, types::ShaderType shaderType, const char* const* defines,
                uint32 numDefines, native::HShader_& outShader, const ApiCapabilities* contextCapabilities = 0);


/// <summary>Create a native shader program from an array of native shader handles. Will implicitly load on the current context.</summary>
/// <param name="pShaders">An array of shaders</param>
/// <param name="count">The number shaders in <paramref>pShaders</paramref></param>
/// <param name="attribNames">The list of names of the attributes in the shader, as a c-style array of c-style strings</param>
/// <param name="attribIndexes">The list of attribute binding indexes, corresponding to <paramref>attribNames</paramref></param>
/// <param name="attribsCount">Number of attributes in <paramref>attribNames</paramref> and <paramref> attribIndexes</paramref>.</param>
/// <param name="outShaderProg">Output, the shader program</param>
/// <param name="infolog">OPTIONAL Output, the infolog of the shader</param>
/// <param name="contextCapabilities">OPTIONAL can be used to pass specific context capabilities</param>
/// <returns>true on success</returns>
bool createShaderProgram(native::HShader_ pShaders[], uint32 count, const char** const attribNames, pvr::uint16* attribIndexes, uint32 attribsCount,
                         native::HPipeline_& outShaderProg, string* infolog, const ApiCapabilities* contextCapabilities = 0);
}
}