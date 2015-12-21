/*!*********************************************************************************************************************
\file         PVRPlatformGlue\ExtensionLoader.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the declaration of extension-loading functions.
***********************************************************************************************************************/
#pragma once

#if defined(TARGET_OS_IPHONE)
#define PVR_APIENTRY GL_APIENTRY
#else
#define PVR_APIENTRY KHRONOS_APIENTRY
#endif

namespace pvr {
namespace native {
/*!*********************************************************************************************************************
\brief         Returns an extension's function pointer as a void pointer. Will need to be casted to the correct type.
\Description  The actual PVRPlatformGlue implementation used will provide the specifics of this function. Prefer the templated
              getExtensionProcAddress as it handles the typecasting for you.
***********************************************************************************************************************/
void* glueGetProcAddress(const char* functionName);


/*!*********************************************************************************************************************
\brief         Returns an extension's function pointer.
, const char* secondaryFunctionName = 0)
\tparam T The type of the function pointer expected (e.g. PFNGLDISPATCHCOMPUTE)
\param  functionName The name of the function to find (e.g. glDispatchCompute)
\param  secondaryFunctionName Alternative name for the function to find (e.g. glFunctionNameEXT vs glFunctionNameIMG)
\Description  This functions wraps the glueGetProcAddress in order to automatically handle type casting an alternative names.
              An alternative name might be another name for an extension (for example ARB and EXT).
***********************************************************************************************************************/
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