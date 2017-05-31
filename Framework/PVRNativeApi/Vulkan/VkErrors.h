<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRNativeApi\Vulkan\VkErrors.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contain functions for logging vulkan errors.
***********************************************************************************************************************/
=======
/*!
\brief Contain functions for logging vulkan errors.
\file PVRNativeApi/Vulkan/VkErrors.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
>>>>>>> 1776432f... 4.3
#pragma once
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRCore/StringFunctions.h"
#include "PVRCore/Log.h"
namespace pvr {
<<<<<<< HEAD

/*!******************************************************************************************************************************
\brief Convert Vulkan error code to string
\param errorCode Vulkan error
\return Error string
********************************************************************************************************************************/
=======
namespace nativeVk {
/// <summary>Convert Vulkan error code to string</summary>
/// <param name="errorCode">Vulkan error</param>
/// <returns>Error string</returns>
>>>>>>> 1776432f... 4.3
inline char const* vkErrorToStr(VkResult errorCode)
{
	switch (errorCode)
	{
	case VK_SUCCESS             : return "VK_SUCCESS";
	case VK_NOT_READY           : return "VK_NOT_READY";
	case VK_TIMEOUT             : return "VK_TIMEOUT" ;
	case VK_EVENT_SET           : return "VK_EVENT_SET" ;
	case VK_EVENT_RESET           : return "VK_EVENT_RESET" ;
	case VK_INCOMPLETE            : return "VK_INCOMPLETE" ;
	case VK_ERROR_OUT_OF_HOST_MEMORY    : return "VK_ERROR_OUT_OF_HOST_MEMORY" ;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY    : return "VK_ERROR_OUT_OF_DEVICE_MEMORY" ;
	case VK_ERROR_INITIALIZATION_FAILED   : return "VK_ERROR_INITIALIZATION_FAILED" ;
	case VK_ERROR_DEVICE_LOST       : return "VK_ERROR_DEVICE_LOST" ;
	case VK_ERROR_MEMORY_MAP_FAILED     : return "VK_ERROR_MEMORY_MAP_FAILED" ;
	case VK_ERROR_LAYER_NOT_PRESENT     : return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT   : return "VK_ERROR_EXTENSION_NOT_PRESENT" ;
	case VK_ERROR_FEATURE_NOT_PRESENT   : return "VK_ERROR_FEATURE_NOT_PRESENT" ;
	case VK_ERROR_INCOMPATIBLE_DRIVER   : return "VK_ERROR_INCOMPATIBLE_DRIVER" ;
	case VK_ERROR_TOO_MANY_OBJECTS      : return "VK_ERROR_TOO_MANY_OBJECTS" ;
	case VK_ERROR_FORMAT_NOT_SUPPORTED    : return "VK_ERROR_FORMAT_NOT_SUPPORTED" ;
	case VK_ERROR_SURFACE_LOST_KHR      : return "VK_ERROR_SURFACE_LOST_KHR" ;
	case VK_SUBOPTIMAL_KHR          : return "VK_SUBOPTIMAL_KHR" ;
	case VK_ERROR_OUT_OF_DATE_KHR     : return "VK_ERROR_OUT_OF_DATE_KHR" ;
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR  : return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" ;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR  : return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" ;

	// suppress the warning
	case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
<<<<<<< HEAD
    case VK_RESULT_RANGE_SIZE: return "VK_RESULT_RANGE_SIZE";
    case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
=======
	case VK_RESULT_RANGE_SIZE: return "VK_RESULT_RANGE_SIZE";
	case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
>>>>>>> 1776432f... 4.3
	default: return ("-- ? UNKNOWN ERROR ?--");
		break;
	}
}

/// <summary>Convert Vulkan error code to string</summary>
/// <param name="result">Vulkan error</param>
/// <param name="msg">Message to log if the error code is not success</param>
/// <returns>Error string</returns>
inline void vkThrowIfFailed(VkResult result, const char* msg)
{
	if (result < VK_SUCCESS)
	{
		std::string s = pvr::strings::createFormatted("Vulkan call (%s) failed.\nVulkan error was %d[%s]", msg, result, vkErrorToStr(result));
		Log(Log.Error, s.c_str());
		assertion(false, s.c_str());
	}
}

/// <summary>Check if the code is success, else log the error</summary>
/// <param name="result">Vulkan error</param>
/// <param name="msg">Message to log if the error code is not success</param>
/// <returns>True if no error</returns>
inline bool vkIsSuccessful(VkResult result, const char* msg)
{
	if (result != VK_SUCCESS)
	{
		Log(Log.Error, "Failed: %s. Vulkan has raised an error: %s", msg, vkErrorToStr(result));
		return false;
	}
	return true;
}
}
}
//!\endcond