/*!
\brief Apple EGL does not use the extension mechanism hence this implementation is empty.
\file PVRNativeApi/EAGL/EaglGetProcAddress.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
 //!\cond NO_DOXYGEN
#include <stdio.h>


namespace pvr {
namespace native {
// Apple EGL does not use the extension mechanism hence this functions always returns null.
	void* glueGetProcAddress(const char* functionName)
	{
		return NULL;
	}
}
}
//!\endcond