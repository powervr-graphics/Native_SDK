/*!
\brief Contains useful low level utils for shaders (loading, compiling) into low level Api object wrappers.
\file PVRUtils/OGLES/ShaderUtilsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "BindingsGles.h"
namespace pvr {
namespace utils {

/// <summary>Load shader from shader source. Will implicitly load on the current context.</summary>
/// <param name="shaderSource">A stream containing the shader source text data</param>
/// <param name="shaderType">The type (stage) of the shader (vertex, fragment...)</param>
/// <param name="defines">A number of preprocessor definitions that will be passed to the shader</param>
/// <param name="numDefines">The number of defines</param>
/// <param name="outShader">The native Shader object will be saved here</param>
/// <returns>true on success</returns>
bool loadShader(const Stream& shaderSource, ShaderType shaderType, const char* const* defines,
                uint32_t numDefines, GLuint& outShader);


/// <summary>Create a native shader program from an array of native shader handles. Will implicitly load on the current context.</summary>
/// <param name="pShaders">An array of shaders</param>
/// <param name="count">The number shaders in <paramref name="pShaders">pShaders</paramref></param>
/// <param name="attribNames">The list of names of the attributes in the shader, as a c-style array of c-style strings</param>
/// <param name="attribIndices">The list of attribute binding indices, corresponding to <paramref name="attribNames">attribNames</paramref></param>
/// <param name="attribsCount">Number of attributes in <paramref name="attribNames">attribNames</paramref> and <paramref name="attribIndices">attribIndices</paramref>.</param>
/// <param name="outShaderProg">Output, the shader program</param>
/// <param name="infolog">OPTIONAL Output, the infolog of the shader</param>
/// <returns>true on success</returns>
bool createShaderProgram(GLuint pShaders[], uint32_t count, const char** const attribNames, const uint16_t* attribIndices, uint32_t attribsCount,
                         GLuint& outShaderProg, std::string* infolog = NULL);
}
}
