/*!
\brief Contains the declaration of extension-loading functions.
\file PVRNativeApi/ExtensionLoader.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#if defined(TARGET_OS_IPHONE)
#define PVR_APIENTRY GL_APIENTRY
#else
#define PVR_APIENTRY KHRONOS_APIENTRY
#endif

namespace pvr {
namespace native {
/// <summary>Returns an extension's function pointer as a void pointer. Will need to be casted to the correct type.
/// </summary>
/// <remarks>The actual PVRNativeApi implementation used will provide the specifics of this function. Prefer
/// the templated getExtensionProcAddress as it handles the typecasting for you.</remarks>
void* glueGetProcAddress(const char* functionName);


/// <summary>Returns an extension's function pointer. , const char* secondaryFunctionName = 0)</summary>
/// <param name="functionName">The name of the function to find (e.g. glDispatchCompute)</param>
/// <param name="secondaryFunctionName">Alternative name for the function to find (e.g. glFunctionNameEXT vs
/// glFunctionNameIMG)</param>
/// <typeparam name="T">The type of the function pointer expected (e.g. PFNGLDISPATCHCOMPUTE)</typeparam>
/// <remarks>This functions wraps the glueGetProcAddress in order to automatically handle type casting an
/// alternative names. An alternative name might be another name for an extension (for example ARB and EXT).
/// </remarks>
template <typename T>
inline T getExtensionProcAddress(const char* functionName, const char* secondaryFunctionName = 0)
{
	T func = reinterpret_cast<T>(glueGetProcAddress(functionName));

	if (!func)
	{
		if (secondaryFunctionName)
		{
			func = reinterpret_cast<T>(glueGetProcAddress(secondaryFunctionName));
		}
	}
	return func;
}

}
}