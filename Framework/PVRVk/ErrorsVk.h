/*!
\brief Functionality for logging vulkan errors, such as converting Vulkan errors to strings.
\file PVRVk/ErrorsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#pragma once

#include "PVRVk/BindingsVk.h"
#include "PVRVk/Log.h"
namespace pvrvk {
namespace impl {
/// <summary>Convert Vulkan error code to std::string</summary>
/// <param name="errorCode">Vulkan error</param>
/// <returns>Error std::string</returns>
inline char const* vkErrorToStr(VkResult errorCode)
{
	switch (errorCode)
	{
	case VkResult::e_SUCCESS                             : return "VkResult::e_SUCCESS";
	case VkResult::e_NOT_READY                           : return "VkResult::e_NOT_READY";
	case VkResult::e_TIMEOUT                             : return "VkResult::e_TIMEOUT" ;
	case VkResult::e_EVENT_SET                           : return "VkResult::e_EVENT_SET" ;
	case VkResult::e_EVENT_RESET                         : return "VkResult::e_EVENT_RESET" ;
	case VkResult::e_INCOMPLETE                          : return "VkResult::e_INCOMPLETE" ;
	case VkResult::e_ERROR_OUT_OF_HOST_MEMORY            : return "VkResult::e_ERROR_OUT_OF_HOST_MEMORY" ;
	case VkResult::e_ERROR_OUT_OF_DEVICE_MEMORY          : return "VkResult::e_ERROR_OUT_OF_DEVICE_MEMORY" ;
	case VkResult::e_ERROR_INITIALIZATION_FAILED         : return "VkResult::e_ERROR_INITIALIZATION_FAILED" ;
	case VkResult::e_ERROR_DEVICE_LOST                   : return "VkResult::e_ERROR_DEVICE_LOST" ;
	case VkResult::e_ERROR_MEMORY_MAP_FAILED             : return "VkResult::e_ERROR_MEMORY_MAP_FAILED" ;
	case VkResult::e_ERROR_LAYER_NOT_PRESENT             : return "VkResult::e_ERROR_LAYER_NOT_PRESENT";
	case VkResult::e_ERROR_EXTENSION_NOT_PRESENT         : return "VkResult::e_ERROR_EXTENSION_NOT_PRESENT" ;
	case VkResult::e_ERROR_FEATURE_NOT_PRESENT           : return "VkResult::e_ERROR_FEATURE_NOT_PRESENT" ;
	case VkResult::e_ERROR_INCOMPATIBLE_DRIVER           : return "VkResult::e_ERROR_INCOMPATIBLE_DRIVER" ;
	case VkResult::e_ERROR_TOO_MANY_OBJECTS              : return "VkResult::e_ERROR_TOO_MANY_OBJECTS" ;
	case VkResult::e_ERROR_FORMAT_NOT_SUPPORTED          : return "VkResult::e_ERROR_FORMAT_NOT_SUPPORTED" ;
	case VkResult::e_ERROR_SURFACE_LOST_KHR              : return "VkResult::e_ERROR_SURFACE_LOST_KHR" ;
	case VkResult::e_SUBOPTIMAL_KHR                      : return "VkResult::e_SUBOPTIMAL_KHR" ;
	case VkResult::e_ERROR_OUT_OF_DATE_KHR               : return "VkResult::e_ERROR_OUT_OF_DATE_KHR" ;
	case VkResult::e_ERROR_INCOMPATIBLE_DISPLAY_KHR      : return "VkResult::e_ERROR_INCOMPATIBLE_DISPLAY_KHR" ;
	case VkResult::e_ERROR_NATIVE_WINDOW_IN_USE_KHR      : return "VkResult::e_ERROR_NATIVE_WINDOW_IN_USE_KHR" ;

	// suppress the warning
	case VkResult::e_ERROR_VALIDATION_FAILED_EXT: return "VkResult::e_ERROR_VALIDATION_FAILED_EXT";
	case VkResult::e_RANGE_SIZE: return "VkResult::e_RANGE_SIZE";
	case VkResult::e_MAX_ENUM: return "VkResult::e_MAX_ENUM";
	default: return ("-- ? UNKNOWN ERROR ?--");
		break;
	}
}

/// <summary>Convert Vulkan error code to std::string</summary>
/// <param name="result">Vulkan error</param>
/// <param name="msg">Message to log if the error code is not success</param>
/// <returns>Error std::string</returns>
inline void vkThrowIfFailed(VkResult result, const char* msg)
{
	if (result < VkResult::e_SUCCESS)
	{

		Log(LogLevel::Error, "Vulkan call (%s) failed.\nVulkan error was %d[%s]", msg, result, vkErrorToStr(result));
		assertion(false, "Vulkan call failed");
	}
}

/// <summary>Check if the code is success, else log the error</summary>
/// <param name="result">Vulkan error</param>
/// <param name="msg">Message to log if the error code is not success</param>
/// <returns>True if no error</returns>
inline bool vkIsSuccessful(VkResult result, const char* msg)
{
	if (result != VkResult::e_SUCCESS)
	{
		Log(LogLevel::Error, "Failed: %s. Vulkan has raised an error: %s", msg, vkErrorToStr(result));
		return false;
	}
	return true;
}
}
}
//!\endcond
