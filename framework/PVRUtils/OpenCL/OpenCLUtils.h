/*!
\brief Contains utility functions and helpers for working with OpenCL
\file PVRUtils/OpenCL/OpenCLUtils.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
//!\cond NO_DOXYGEN
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 110
//!\endcond
#include "DynamicOCL.h"
#include "PVRCore/texture/PixelFormat.h"
#include "PVRCore/Errors.h"
#include "PVRCore/texture/Texture.h"

namespace clutils {
/// <summary>Get a string representation of an OpenCL error code.</summary>
/// <param name="error">The OpenCL error code</param>
/// <param name="error">The OpenCL error code</param>
/// <returns>The string representation of the OpenCL error code.</returns>
inline const char* getOpenCLError(cl_int error)
{
	switch (error)
	{
	case CL_SUCCESS: return "CL_SUCCESS";
	case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
	case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE";
	case CL_COMPILER_NOT_AVAILABLE: return "CL_COMPILER_NOT_AVAILABLE";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
	case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
	case CL_PROFILING_INFO_NOT_AVAILABLE: return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case CL_MEM_COPY_OVERLAP: return "CL_MEM_COPY_OVERLAP";
	case CL_IMAGE_FORMAT_MISMATCH: return "CL_IMAGE_FORMAT_MISMATCH";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
	case CL_MAP_FAILURE: return "CL_MAP_FAILURE";
#ifdef CL_VERSION_1_1
	case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
#endif
#ifdef CL_VERSION_1_2
	case CL_COMPILE_PROGRAM_FAILURE: return "CL_COMPILE_PROGRAM_FAILURE";
	case CL_LINKER_NOT_AVAILABLE: return "CL_LINKER_NOT_AVAILABLE";
	case CL_LINK_PROGRAM_FAILURE: return "CL_LINK_PROGRAM_FAILURE";
	case CL_DEVICE_PARTITION_FAILED: return "CL_DEVICE_PARTITION_FAILED";
	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
#endif
	case CL_INVALID_VALUE: return "CL_INVALID_VALUE";
	case CL_INVALID_DEVICE_TYPE: return "CL_INVALID_DEVICE_TYPE";
	case CL_INVALID_PLATFORM: return "CL_INVALID_PLATFORM";
	case CL_INVALID_DEVICE: return "CL_INVALID_DEVICE";
	case CL_INVALID_CONTEXT: return "CL_INVALID_CONTEXT";
	case CL_INVALID_QUEUE_PROPERTIES: return "CL_INVALID_QUEUE_PROPERTIES";
	case CL_INVALID_COMMAND_QUEUE: return "CL_INVALID_COMMAND_QUEUE";
	case CL_INVALID_HOST_PTR: return "CL_INVALID_HOST_PTR";
	case CL_INVALID_MEM_OBJECT: return "CL_INVALID_MEM_OBJECT";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case CL_INVALID_IMAGE_SIZE: return "CL_INVALID_IMAGE_SIZE";
	case CL_INVALID_SAMPLER: return "CL_INVALID_SAMPLER";
	case CL_INVALID_BINARY: return "CL_INVALID_BINARY";
	case CL_INVALID_BUILD_OPTIONS: return "CL_INVALID_BUILD_OPTIONS";
	case CL_INVALID_PROGRAM: return "CL_INVALID_PROGRAM";
	case CL_INVALID_PROGRAM_EXECUTABLE: return "CL_INVALID_PROGRAM_EXECUTABLE";
	case CL_INVALID_KERNEL_NAME: return "CL_INVALID_KERNEL_NAME";
	case CL_INVALID_KERNEL_DEFINITION: return "CL_INVALID_KERNEL_DEFINITION";
	case CL_INVALID_KERNEL: return "CL_INVALID_KERNEL";
	case CL_INVALID_ARG_INDEX: return "CL_INVALID_ARG_INDEX";
	case CL_INVALID_ARG_VALUE: return "CL_INVALID_ARG_VALUE";
	case CL_INVALID_ARG_SIZE: return "CL_INVALID_ARG_SIZE";
	case CL_INVALID_KERNEL_ARGS: return "CL_INVALID_KERNEL_ARGS";
	case CL_INVALID_WORK_DIMENSION: return "CL_INVALID_WORK_DIMENSION";
	case CL_INVALID_WORK_GROUP_SIZE: return "CL_INVALID_WORK_GROUP_SIZE";
	case CL_INVALID_WORK_ITEM_SIZE: return "CL_INVALID_WORK_ITEM_SIZE";
	case CL_INVALID_GLOBAL_OFFSET: return "CL_INVALID_GLOBAL_OFFSET";
	case CL_INVALID_EVENT_WAIT_LIST: return "CL_INVALID_EVENT_WAIT_LIST";
	case CL_INVALID_EVENT: return "CL_INVALID_EVENT";
	case CL_INVALID_OPERATION: return "CL_INVALID_OPERATION";
	case CL_INVALID_GL_OBJECT: return "CL_INVALID_GL_OBJECT";
	case CL_INVALID_BUFFER_SIZE: return "CL_INVALID_BUFFER_SIZE";
	case CL_INVALID_MIP_LEVEL: return "CL_INVALID_MIP_LEVEL";
	case CL_INVALID_GLOBAL_WORK_SIZE: return "CL_INVALID_GLOBAL_WORK_SIZE";
#ifdef CL_VERSION_1_1
	case CL_INVALID_PROPERTY: return "CL_INVALID_PROPERTY";
#endif
#ifdef CL_VERSION_1_2
	case CL_INVALID_IMAGE_DESCRIPTOR: return "CL_INVALID_IMAGE_DESCRIPTOR";
	case CL_INVALID_COMPILER_OPTIONS: return "CL_INVALID_COMPILER_OPTIONS";
	case CL_INVALID_LINKER_OPTIONS: return "CL_INVALID_LINKER_OPTIONS";
	case CL_INVALID_DEVICE_PARTITION_COUNT: return "CL_INVALID_DEVICE_PARTITION_COUNT";
#endif
#ifdef CL_VERSION_2_0
	case CL_INVALID_PIPE_SIZE: return "CL_INVALID_PIPE_SIZE";
	case CL_INVALID_DEVICE_QUEUE: return "CL_INVALID_DEVICE_QUEUE";
#endif
#ifdef CL_VERSION_2_2
	case CL_INVALID_SPEC_ID: return "CL_INVALID_SPEC_ID";
	case CL_MAX_SIZE_RESTRICTION_EXCEEDED: return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
#endif
	default: return "UNKNOWN_OPENCL_ERROR_CODE";
	}
}

/// <summary>A simple std::runtime_error wrapper for throwing exceptions when receiving OpenCL
/// errors.</summary>
class OpenCLError : public pvr::PvrError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="errorCode">The OpenCL error code to stringify.</param>
	OpenCLError(cl_int errorCode) : PvrError(std::string("OpenCL Error [") + getOpenCLError(errorCode) + "]") {}
	/// <summary>Constructor.</summary>
	/// <param name="errorCode">The OpenCL error code to stringify.</param>
	/// <param name="message">A message to log alongside the OpenCL error.</param>
	OpenCLError(cl_int errorCode, const std::string& message) : PvrError(std::string("OpenCL Error [") + getOpenCLError(errorCode) + "] - " + message) {}
};

/// <summary>Determines whether the given OpenCL extension is supported.</summary>
/// <param name="platform">The OpenCL platform.</param>
/// <param name="extension">The OpenCL extension to check support for.</param>
/// <returns>True if the given OpenCL extension is supported.</returns>
inline bool isExtensionSupported(cl_platform_id platform, const char* extension)
{
	// Extension names should not have spaces.
	const char* where = strchr(extension, ' ');

	if (where || *extension == '\0') { return 0; }

	size_t size;
	cl::GetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, 0, nullptr, &size);
	std::vector<char> extensions(size + 1);
	cl::GetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, size + 1, extensions.data(), nullptr);

	const char* start;
	const char* terminator;

	start = extensions.data();
	for (;;)
	{
		where = strstr(start, extension);
		if (!where) { break; }
		terminator = where + strlen(extension);
		if (where == start || *(where - 1) == ' ')
		{
			if (*terminator == ' ' || *terminator == '\0') { return true; }
		}
		start = terminator;
	}
	return false;
}

class OpenCLFormatConversionError : public std::runtime_error
{
public:
	OpenCLFormatConversionError(const pvr::PixelFormat& fmt, const std::string& message)
		: std::runtime_error("clutils::convertToOpenCLFormat - PixelFormat '" + to_string(fmt) + " could not be matched to an OpenCL Image Format - " + message)
	{}
	OpenCLFormatConversionError(const pvr::PixelFormat& fmt, const pvr::VariableType type, const pvr::ColorSpace colorspace, const std::string& message)
		: std::runtime_error("clutils::convertToOpenCLFormat - PixelFormat '" + to_string(fmt) + "' with channel datatype '" + to_string(type) + "'and colorspace '" +
			  to_string(colorspace) + "' could not be matched to an OpenCL Image Format - " + message)
	{}
	OpenCLFormatConversionError(const pvr::ImageDataFormat& fmt, const std::string& message) : OpenCLFormatConversionError(fmt.format, fmt.dataType, fmt.colorSpace, message) {}
};

inline cl_channel_type convertChannelContent(int bits0, int bits1, int bits2, int bits3, pvr::VariableType type)
{
	if (bits0 == 8)
	{
		if ((bits1 != 0 && bits1 != 8) || (bits2 != 0 && bits2 != 8) || (bits3 != 0 && bits3 != 8)) { return 0; }
		if (type == pvr::VariableType::UnsignedByteNorm) { return CL_UNORM_INT8; }
		else if (type == pvr::VariableType::SignedByteNorm) { return CL_UNORM_INT8; }
		else if (type == pvr::VariableType::UnsignedByte) { return CL_UNSIGNED_INT8; }
		else if (type == pvr::VariableType::SignedByte) { return CL_SIGNED_INT8; }
		return 0;
	}
	if (bits0 == 16)
	{
		if ((bits1 != 0 && bits1 != 16) || (bits2 != 0 && bits2 != 16) || (bits3 != 0 && bits3 != 16)) { return 0; }
		switch (type)
		{
		case pvr::VariableType::SignedFloat: return CL_HALF_FLOAT;
		case pvr::VariableType::UnsignedFloat: return CL_HALF_FLOAT;
		case pvr::VariableType::UnsignedShortNorm: return CL_UNORM_INT16;
		case pvr::VariableType::SignedShortNorm: return CL_UNORM_INT8;
		case pvr::VariableType::UnsignedShort: return CL_UNSIGNED_INT8;
		case pvr::VariableType::SignedShort: return CL_SIGNED_INT8;
		default: return 0;
		}
	}
	if (bits0 == 24)
	{
		if (bits1 != 8 || bits2 != 0 || bits3 != 0) { return 0; }
		return CL_UNORM_INT24;
	}
	if (bits0 == 32)
	{
		if ((bits1 != 0 && bits1 != 32 && bits1 != 8) || (bits2 != 0 && bits2 != 32) || (bits3 != 0 && bits3 != 32)) { return 0; }
		switch (type)
		{
		case pvr::VariableType::SignedFloat: return CL_FLOAT;
		case pvr::VariableType::UnsignedFloat: return CL_FLOAT;
		case pvr::VariableType::SignedInteger: return CL_SIGNED_INT32;
		case pvr::VariableType::UnsignedInteger: return CL_UNSIGNED_INT32;
		default: return 0;
		}
	}
	if (bits0 == 5 && bits1 == 5 && bits2 == 5 && bits3 == 0) { return CL_UNORM_SHORT_555; }
	if (bits0 == 5 && bits1 == 5 && bits2 == 6 && bits3 == 0) { return CL_UNORM_SHORT_565; }
	if (bits1 == 10 && bits2 == 10 && (bits0 == 10 || bits3 == 10)) { return CL_UNORM_INT_101010; }
	if (bits1 == 5 && bits2 == 6 && bits3 == 0) { return CL_UNORM_SHORT_565; }
	return 0;

} // namespace clutils

inline cl_channel_order convertChannelOrder(char ch0, char ch1, char ch2, char ch3)
{
	//"hot"path
	if (ch0 == 'r' && ch1 == 'g' && ch2 == 'b')
	{
		if (ch3 == 'a') { return CL_RGBA; }
		if (ch3 == 0) { return CL_RGB; }

		return 0;
	}
	else
	{
		switch (ch0)
		{
		case 'l':
		case 'i':
			if (ch1 == 0) { return CL_LUMINANCE; }
			if (ch1 == 'a' && ch2 == 0) { return CL_RA; }
			return 0;
		case 'a':
			if (ch1 == 0) { return CL_A; }
			if (ch1 == 'r' && ch2 == 'g' && ch3 == 'b') { return CL_ARGB; }
			return 0;
		case 'd':
			if (ch1 == 0) { return CL_DEPTH; }
			if (ch1 == 's' && ch2 == 0) { return CL_DEPTH_STENCIL; }
			return 0;

		case 'b':
			if (ch1 == 'g' && ch2 == 'r' && ch3 == 'a') { return CL_BGRA; }
			return 0;
		case 'r':
			switch (ch1)
			{
			case 0: return CL_R;
			case 'a':
				if (ch2 == 0) { return CL_RA; }
				return 0;
			case 'g':
				switch (ch2)
				{
				case 0: return CL_RG;
				// case 'b': ALREADY HANDLED
				//	switch (ch3)
				//	{
				//	case 0: return CL_RGB;
				//	case 'a': return CL_RGBA;
				//	}
				default: return 0;
				}
			default: return 0;
			}
		default: return 0;
		}
	}
} // namespace clutils

inline cl_image_format convertToOpenCLFormat(const pvr::PixelFormat& format, pvr::VariableType variableType, pvr::ColorSpace colorSpace)
{
	cl_image_format retval;

	retval.image_channel_order = convertChannelOrder(format.getChannelContent(0), format.getChannelContent(1), format.getChannelContent(2), format.getChannelContent(3));
	retval.image_channel_data_type = convertChannelContent(format.getChannelBits(0), format.getChannelBits(1), format.getChannelBits(2), format.getChannelBits(3), variableType);

	if (retval.image_channel_order == 0 || retval.image_channel_data_type == 0)
	{
		throw OpenCLFormatConversionError(format, variableType, colorSpace, "Could not match to an OpenCL format");
	}
	return retval;
}

/// <summary>Verify the extensions required specified in vectorRequiredExtensions are present in vectorOpenCLExtensions.</summary>
/// <param name="vectorOpenCLExtensions">Vector of char with all OpenCL extension names.</param>
/// <param name="vectorRequiredExtensions">Vector with the required extensions.</param>
/// <returns>True if the required extensions are supported, false otherwise.</returns>
bool verifySupportedExtensions(std::vector<char> vectorOpenCLExtensions, const std::vector<std::string>& vectorRequiredExtensions)
{
	for (size_t i = 0; i < vectorRequiredExtensions.size(); ++i)
	{
		if (strstr(vectorOpenCLExtensions.data(), vectorRequiredExtensions[i].c_str()) == nullptr) { return false; }
	}

	return true;
}

/// <summary>Creates an OpenCL context.</summary>
/// <param name="vectorPlatformID">Vector with the platform IDs to rearrange.</param>
/// <param name="vectorLessPrioritaryVendor">Vector with vendors which are less prioritary when selecting one platform for the context.</param>
/// <param name="vectorRearrangedPlatformID">Vector with the rearra nged platform IDs, where prioritary vendors are present in the first part of the vector, and non prioritary vendors are present in the last part of the vector.</param>
void rearrangePlatformIDForLessPrioritayVendor(
	const std::vector<cl_platform_id>& vectorPlatformID, const std::vector<std::string>& vectorLessPrioritaryVendor, std::vector<cl_platform_id>& vectorRearrangedPlatformID)
{
	for (std::vector<cl_platform_id>::const_iterator it = vectorPlatformID.begin(); it != vectorPlatformID.end(); ++it)
	{
		size_t platformVendorLength = 0;
		cl_int errcode = cl::GetPlatformInfo(*it, CL_PLATFORM_VENDOR, 0, NULL, &platformVendorLength);

		if (errcode != 0) { Log(LogLevel::Error, "[clutils::createOpenCLContext] GetPlatformInfo had error %d.", getOpenCLError(errcode)); }

		std::vector<char> platformVendor(platformVendorLength);
		errcode = cl::GetPlatformInfo(*it, CL_PLATFORM_VENDOR, platformVendorLength, platformVendor.data(), NULL);

		if (errcode != 0) { Log(LogLevel::Error, "[clutils::createOpenCLContext] GetPlatformInfo had error %d.", getOpenCLError(errcode)); }

		std::string vendorLowerCase = std::string(platformVendor.data());
		std::transform(vendorLowerCase.begin(), vendorLowerCase.end(), vendorLowerCase.begin(), ::tolower);
		bool isLessPrioritaryVendor = false;
		for (size_t j = 0; (j < vectorLessPrioritaryVendor.size()) && !isLessPrioritaryVendor; ++j)
		{
			std::string lessPrioritaryVendorLowerCase = std::string(vectorLessPrioritaryVendor[j].data());
			std::transform(lessPrioritaryVendorLowerCase.begin(), lessPrioritaryVendorLowerCase.end(), lessPrioritaryVendorLowerCase.begin(), ::tolower);
			if (vendorLowerCase.find(lessPrioritaryVendorLowerCase.c_str()) != std::string::npos)
			{
				vectorRearrangedPlatformID.insert(vectorRearrangedPlatformID.end(), *it);
				isLessPrioritaryVendor = true;
			}
		}

		if (!isLessPrioritaryVendor) { vectorRearrangedPlatformID.insert(vectorRearrangedPlatformID.begin(), *it); }
	}
}

/// <summary>Creates an OpenCL context.</summary>
/// <param name="outPlatform">The OpenCL platform to be created.</param>
/// <param name="outDevice">The OpenCL device to be created.</param>
/// <param name="outContext">The OpenCL context to be created.</param>
/// <param name="outQueue">The OpenCL queue to be created.</param>
/// <param name="queue_properties">The OpenCL queue properties</param>
/// <param name="device_type">The OpenCL device type.</param>
/// <param name="platformName">The OpenCL platform name to use.</param>
/// <param name="err">The OpenCL error.</param>
/// <param name="vectorRequiredExtensions">Extensions required to be supported.</param>
/// <param name="forceDeviceUUID">If != nullptr, force a device UUID from each platform ID to build the OpenCL context.</param>
inline void createOpenCLContext(cl_platform_id& outPlatform, cl_device_id& outDevice, cl_context& outContext, cl_command_queue& outQueue,
	cl_command_queue_properties queue_properties = 0, cl_device_type device_type = CL_DEVICE_TYPE_ALL, const char* const platformName = NULL, cl_int* err = 0,
	const std::vector<std::string>& vectorRequiredExtensions = std::vector<std::string>(), cl_uchar* forceDeviceUUID = nullptr)
{
	bool contextCreated = false;
	cl_int errstr = 0;
	cl_int& errcode = err ? *err : errstr;

	/*
	 *  Query the available OpenCL platforms.
	 */
	cl_uint numPlatforms = 0;
	errcode = cl::GetPlatformIDs(0, nullptr, &numPlatforms);

	if (numPlatforms == 0) { throw OpenCLError(errcode, "[clutils::createOpenCLContext] clGetPlatformIDs : No OpenCL capable platform found (clutils::createOpenCLContext)"); }

	std::vector<cl_platform_id> vectorPlatforms(numPlatforms);
	errcode = cl::GetPlatformIDs(numPlatforms, vectorPlatforms.data(), &numPlatforms);

	if (errcode != CL_SUCCESS) { throw OpenCLError(errcode, "[clutils::createOpenCLContext] clGetPlatformIDs : Failed to query platform IDs"); }

	/*
	 *  Iterate over all of the available platforms until one is found that matches the
	 * requirements.
	 */
	for (cl_uint i = 0; i < vectorPlatforms.size(); i++)
	{
		/*
		 *  Check whether the platform matches the requested one.
		 */
		size_t platformNameLength = 0;
		errcode = cl::GetPlatformInfo(vectorPlatforms[i], CL_PLATFORM_NAME, 0, NULL, &platformNameLength);
		std::vector<char> platName(platformNameLength);
		errcode = cl::GetPlatformInfo(vectorPlatforms[i], CL_PLATFORM_NAME, platformNameLength, platName.data(), NULL);
		if (errcode != CL_SUCCESS)
		{
			Log(LogLevel::Error, "[clutils::createOpenCLContext] clGetPlatformInfo had error %d, attempting next platform.", getOpenCLError(errcode));
			continue;
		}
		if (platformName)
		{
			bool bPlatformNameMatches = false;
			if (strncmp(platName.data(), platformName, platformNameLength) == 0) { bPlatformNameMatches = true; }

			// Continue with the next platform if the current one is unsuitable.
			if (!bPlatformNameMatches) { continue; }
		}

		/*
		 *  Query for the first available device that matches the requirements.
		 */
		cl_uint numDevices = 0;
		errcode = cl::GetDeviceIDs(vectorPlatforms[i], device_type, 0, NULL, &numDevices);
		std::vector<cl_device_id> vectorDevices;
		if (errcode != CL_SUCCESS && errcode != CL_DEVICE_NOT_FOUND)
		{
			throw OpenCLError(errcode, "[clutils::createOpenCLContext] clGetDeviceIds: Failed to query OpenCL devices");
		}
		if (errcode == CL_SUCCESS)
		{
			vectorDevices.resize(numDevices);

			errcode = cl::GetDeviceIDs(vectorPlatforms[i], device_type, numDevices, vectorDevices.data(), NULL);
			if (errcode != CL_SUCCESS && errcode != CL_DEVICE_NOT_FOUND)
			{
				throw OpenCLError(errcode, "[clutils::createOpenCLContext] clGetDeviceIds: Failed to query OpenCL devices");
			}
		}

		if (vectorDevices.empty())
		{
			Log(LogLevel::Information, "c.", getOpenCLError(errcode));
			// This platform does not have a suitable device, continue with the next platform.
			continue;
		}

		cl_device_id finalDeviceID = nullptr;

		for (auto device = vectorDevices.begin(); device != vectorDevices.end(); ++device)
		{
			size_t platformExtensionsStringLength, deviceExtensionsStringLength;
			cl::GetPlatformInfo(vectorPlatforms[i], CL_PLATFORM_EXTENSIONS, 0, NULL, &platformExtensionsStringLength);
			std::vector<char> platform_extensions(platformExtensionsStringLength);
			cl::GetPlatformInfo(vectorPlatforms[i], CL_PLATFORM_EXTENSIONS, platformExtensionsStringLength, platform_extensions.data(), NULL);

			if (!verifySupportedExtensions(platform_extensions, vectorRequiredExtensions))
			{
				Log(LogLevel::Warning, "Extensions required not supported by current device %s", platName.data());
				continue;
			}

			cl::GetDeviceInfo(*device, CL_DEVICE_EXTENSIONS, 0, NULL, &deviceExtensionsStringLength);
			std::vector<char> device_extensions(deviceExtensionsStringLength);
			cl::GetDeviceInfo(*device, CL_DEVICE_EXTENSIONS, deviceExtensionsStringLength, device_extensions.data(), NULL);

			if (!verifySupportedExtensions(device_extensions, vectorRequiredExtensions))
			{
				Log(LogLevel::Warning, "Extensions required not supported by current device %s", platName.data());
				continue;
			}

			if (forceDeviceUUID != nullptr)
			{
				cl_uchar deviceUUID[CL_UUID_SIZE_KHR];
				cl::GetDeviceInfo(*device, CL_DEVICE_UUID_KHR, sizeof(deviceUUID), &deviceUUID, nullptr);
				bool matchingUUID = true;
				for (uint32_t j = 0; j < CL_UUID_SIZE_KHR; ++j)
				{
					if (deviceUUID[j] != forceDeviceUUID[j])
					{
						matchingUUID = false;
						break;
					}
				}

				if (!matchingUUID) { continue; }
			}

			finalDeviceID = *device;
			break;
		}

		if (finalDeviceID == nullptr) { continue; }

		cl_context_properties contextProperties[] = {
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)vectorPlatforms[i],
			0,
			0,
			0,
			0,
			0,
			0,
			0,
		};

		cl_context context = cl::CreateContext(contextProperties, 1, &finalDeviceID, nullptr, nullptr, &errcode);
		if (errcode != CL_SUCCESS) { throw OpenCLError(errcode, "[cl::createOpenCLContext]: Failed to create context"); }

		cl_command_queue q;
		{
			typedef cl_command_queue(CL_API_CALL * PFNclCreateCommandQueue)(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int * errcode_ret);
			static PFNclCreateCommandQueue _clCreateCommandQueue = (PFNclCreateCommandQueue)cl::internals::getClFunction(cl::CLFunctions::CreateCommandQueue);
			q = _clCreateCommandQueue(context, finalDeviceID, queue_properties, &errcode);
		}

		cl_command_queue queue(q);
		if (errcode != CL_SUCCESS || q == NULL) { throw OpenCLError(errcode, "[cl::createOpenCLContext]: Failed to create command queue"); }

		Log(LogLevel::Information, "[cl::createOpenCLContext]: Created context on platform %s.", platName.data());
		outPlatform = vectorPlatforms[i];
		outContext = context;
		outDevice = finalDeviceID;
		outQueue = queue;
		contextCreated = true;
		break;
	}

	if (!contextCreated)
	{
		throw OpenCLError(CL_SUCCESS,
			"[cl::createOpenCLContext]: No errors occured, but could not find any suitable OpenCL "
			"context(platform, context, device and queue)");
	}
}

/// <summary>Loads a kernel program.</summary>
/// <param name="ctx">The OpenCL context.</param>
/// <param name="device">The OpenCL device</param>
/// <param name="kernelSource">The kernel source code</param>
/// <param name="compilerOptions">A set of compiler option to use</param>
/// <param name="defines">A list of defineCount defines to use in the shader</param>
/// <param name="defineCount">The number of defines to use in the shader</param>
/// <returns>The created kernel program</returns>
cl_program loadKernelProgram(
	const cl_context& ctx, const cl_device_id& device, const pvr::Stream& kernelSource, const char* compilerOptions = 0, const char* const* defines = 0, uint32_t defineCount = 0)
{
	cl_int errcode;

	// Append define's here if there are any
	std::string pszKernelString;

	std::string shaderSrc;
	kernelSource.readIntoString(shaderSrc);

	std::string sourceDataStr;
	// insert the defines
	for (uint32_t i = 0; i < defineCount; ++i)
	{
		sourceDataStr.append(defines[i]);
		sourceDataStr.append("\n");
	}
	sourceDataStr.append("\n");
	sourceDataStr.append(shaderSrc);

	const char* tmp = sourceDataStr.c_str();
	const char** source_string_list_tmp = &tmp;

	cl_program program = cl::CreateProgramWithSource(ctx, 1, source_string_list_tmp, NULL, &errcode);

	if (program == NULL || errcode != CL_SUCCESS) { throw OpenCLError(errcode, "[cl::loadKernel]:[cl::CreateProgramWithSource] Failed to create OpenCL program "); }

	errcode = cl::BuildProgram(program, 1, &device, compilerOptions, nullptr, nullptr);
	if (errcode != CL_SUCCESS)
	{
		cl_int build_errcode = errcode;
		size_t size_of_log;
		errcode = cl::GetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size_of_log);
		std::vector<char> errlog(size_of_log);
		errcode = cl::GetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, size_of_log, errlog.data(), NULL);
		if (errcode != CL_SUCCESS) { throw OpenCLError(errcode, "[cl::loadKernel]: Failed to build program. Failed to get program build log."); }
		throw OpenCLError(build_errcode, "[cl::loadKernel]: Failed to build program. Build log:\n" + std::string(errlog.data()));
	}
	return program;
}

inline void throwOnFailure(cl_int err)
{
	if (err != CL_SUCCESS) { throw OpenCLError(err); }
}

inline void throwOnFailure(cl_int err, const std::string& str)
{
	if (err != CL_SUCCESS) { throw OpenCLError(err, str); }
}

} // namespace clutils