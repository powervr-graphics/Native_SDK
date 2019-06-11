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

namespace clutils {
/// <summary>Get a string representation of an OpenCL error code.</summary>
/// <param name="error">The OpenCL error code</param>
/// <param name="error">The OpenCL error code</param>
/// <returns>The string representation of the OpenCL error code.</returns>
inline const char* getOpenCLError(cl_int error)
{
	switch (error)
	{
	case CL_SUCCESS:
		return "CL_SUCCESS";
	case CL_DEVICE_NOT_FOUND:
		return "CL_DEVICE_NOT_FOUND";
	case CL_DEVICE_NOT_AVAILABLE:
		return "CL_DEVICE_NOT_AVAILABLE";
	case CL_COMPILER_NOT_AVAILABLE:
		return "CL_COMPILER_NOT_AVAILABLE";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:
		return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case CL_OUT_OF_RESOURCES:
		return "CL_OUT_OF_RESOURCES";
	case CL_OUT_OF_HOST_MEMORY:
		return "CL_OUT_OF_HOST_MEMORY";
	case CL_PROFILING_INFO_NOT_AVAILABLE:
		return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case CL_MEM_COPY_OVERLAP:
		return "CL_MEM_COPY_OVERLAP";
	case CL_IMAGE_FORMAT_MISMATCH:
		return "CL_IMAGE_FORMAT_MISMATCH";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:
		return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case CL_BUILD_PROGRAM_FAILURE:
		return "CL_BUILD_PROGRAM_FAILURE";
	case CL_MAP_FAILURE:
		return "CL_MAP_FAILURE";
	case CL_MISALIGNED_SUB_BUFFER_OFFSET:
		return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
		return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case CL_COMPILE_PROGRAM_FAILURE:
		return "CL_COMPILE_PROGRAM_FAILURE";
	case CL_LINKER_NOT_AVAILABLE:
		return "CL_LINKER_NOT_AVAILABLE";
	case CL_LINK_PROGRAM_FAILURE:
		return "CL_LINK_PROGRAM_FAILURE";
	case CL_DEVICE_PARTITION_FAILED:
		return "CL_DEVICE_PARTITION_FAILED";
	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
		return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
	case CL_INVALID_VALUE:
		return "CL_INVALID_VALUE";
	case CL_INVALID_DEVICE_TYPE:
		return "CL_INVALID_DEVICE_TYPE";
	case CL_INVALID_PLATFORM:
		return "CL_INVALID_PLATFORM";
	case CL_INVALID_DEVICE:
		return "CL_INVALID_DEVICE";
	case CL_INVALID_CONTEXT:
		return "CL_INVALID_CONTEXT";
	case CL_INVALID_QUEUE_PROPERTIES:
		return "CL_INVALID_QUEUE_PROPERTIES";
	case CL_INVALID_COMMAND_QUEUE:
		return "CL_INVALID_COMMAND_QUEUE";
	case CL_INVALID_HOST_PTR:
		return "CL_INVALID_HOST_PTR";
	case CL_INVALID_MEM_OBJECT:
		return "CL_INVALID_MEM_OBJECT";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
		return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case CL_INVALID_IMAGE_SIZE:
		return "CL_INVALID_IMAGE_SIZE";
	case CL_INVALID_SAMPLER:
		return "CL_INVALID_SAMPLER";
	case CL_INVALID_BINARY:
		return "CL_INVALID_BINARY";
	case CL_INVALID_BUILD_OPTIONS:
		return "CL_INVALID_BUILD_OPTIONS";
	case CL_INVALID_PROGRAM:
		return "CL_INVALID_PROGRAM";
	case CL_INVALID_PROGRAM_EXECUTABLE:
		return "CL_INVALID_PROGRAM_EXECUTABLE";
	case CL_INVALID_KERNEL_NAME:
		return "CL_INVALID_KERNEL_NAME";
	case CL_INVALID_KERNEL_DEFINITION:
		return "CL_INVALID_KERNEL_DEFINITION";
	case CL_INVALID_KERNEL:
		return "CL_INVALID_KERNEL";
	case CL_INVALID_ARG_INDEX:
		return "CL_INVALID_ARG_INDEX";
	case CL_INVALID_ARG_VALUE:
		return "CL_INVALID_ARG_VALUE";
	case CL_INVALID_ARG_SIZE:
		return "CL_INVALID_ARG_SIZE";
	case CL_INVALID_KERNEL_ARGS:
		return "CL_INVALID_KERNEL_ARGS";
	case CL_INVALID_WORK_DIMENSION:
		return "CL_INVALID_WORK_DIMENSION";
	case CL_INVALID_WORK_GROUP_SIZE:
		return "CL_INVALID_WORK_GROUP_SIZE";
	case CL_INVALID_WORK_ITEM_SIZE:
		return "CL_INVALID_WORK_ITEM_SIZE";
	case CL_INVALID_GLOBAL_OFFSET:
		return "CL_INVALID_GLOBAL_OFFSET";
	case CL_INVALID_EVENT_WAIT_LIST:
		return "CL_INVALID_EVENT_WAIT_LIST";
	case CL_INVALID_EVENT:
		return "CL_INVALID_EVENT";
	case CL_INVALID_OPERATION:
		return "CL_INVALID_OPERATION";
	case CL_INVALID_GL_OBJECT:
		return "CL_INVALID_GL_OBJECT";
	case CL_INVALID_BUFFER_SIZE:
		return "CL_INVALID_BUFFER_SIZE";
	case CL_INVALID_MIP_LEVEL:
		return "CL_INVALID_MIP_LEVEL";
	case CL_INVALID_GLOBAL_WORK_SIZE:
		return "CL_INVALID_GLOBAL_WORK_SIZE";
	case CL_INVALID_PROPERTY:
		return "CL_INVALID_PROPERTY";
	case CL_INVALID_IMAGE_DESCRIPTOR:
		return "CL_INVALID_IMAGE_DESCRIPTOR";
	case CL_INVALID_COMPILER_OPTIONS:
		return "CL_INVALID_COMPILER_OPTIONS";
	case CL_INVALID_LINKER_OPTIONS:
		return "CL_INVALID_LINKER_OPTIONS";
	case CL_INVALID_DEVICE_PARTITION_COUNT:
		return "CL_INVALID_DEVICE_PARTITION_COUNT";
#ifdef CL_INVALID_PIPE_SIZE
	case CL_INVALID_PIPE_SIZE:
		return "CL_INVALID_PIPE_SIZE";
#endif
#ifdef CL_INVALID_DEVICE_QUEUE
	case CL_INVALID_DEVICE_QUEUE:
		return "CL_INVALID_DEVICE_QUEUE";
#endif
#ifdef CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR
	case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
		return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
#endif
#ifdef CL_PLATFORM_NOT_FOUND_KHR
	case CL_PLATFORM_NOT_FOUND_KHR:
		return "CL_PLATFORM_NOT_FOUND_KHR";
#endif
#ifdef CL_INVALID_D3D10_DEVICE_KHR
	case CL_INVALID_D3D10_DEVICE_KHR:
		return "CL_INVALID_D3D10_DEVICE_KHR";
	case CL_INVALID_D3D10_RESOURCE_KHR:
		return "CL_INVALID_D3D10_RESOURCE_KHR";
	case CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR:
		return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR:
		return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	case CL_INVALID_D3D11_DEVICE_KHR:
		return "CL_INVALID_D3D11_DEVICE_KHR";
	case CL_INVALID_D3D11_RESOURCE_KHR:
		return "CL_INVALID_D3D11_RESOURCE_KHR";
	case CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR:
		return "CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR";
	case CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR:
		return "CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR";
	case CL_INVALID_D3D9_DEVICE_NV:
		return "CL_INVALID_D3D9_DEVICE_NV";
	case CL_INVALID_D3D9_RESOURCE_NV:
		return "CL_INVALID_D3D9_RESOURCE_NV";
	case CL_D3D9_RESOURCE_ALREADY_ACQUIRED_NV:
		return "CL_D3D9_RESOURCE_ALREADY_ACQUIRED_NV";
	case CL_D3D9_RESOURCE_NOT_ACQUIRED_NV:
		return "CL_D3D9_RESOURCE_NOT_ACQUIRED_NV";
#endif
#ifdef CL_EGL_RESOURCE_NOT_ACQUIRED_KHR
	case CL_EGL_RESOURCE_NOT_ACQUIRED_KHR:
		return "CL_EGL_RESOURCE_NOT_ACQUIRED_KHR";
	case CL_INVALID_EGL_OBJECT_KHR:
		return "CL_INVALID_EGL_OBJECT_KHR";
#endif
#ifdef CL_INVALID_ACCELERATOR_INTEL
	case CL_INVALID_ACCELERATOR_INTEL:
		return "CL_INVALID_ACCELERATOR_INTEL";
	case CL_INVALID_ACCELERATOR_TYPE_INTEL:
		return "CL_INVALID_ACCELERATOR_TYPE_INTEL";
	case CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL:
		return "CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL";
	case CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL:
		return "CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL";
	case CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL:
		return "CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL";
	case CL_INVALID_VA_API_MEDIA_SURFACE_INTEL:
		return "CL_INVALID_VA_API_MEDIA_SURFACE_INTEL";
	case CL_VA_API_MEDIA_SURFACE_ALREADY_ACQUIRED_INTEL:
		return "CL_VA_API_MEDIA_SURFACE_ALREADY_ACQUIRED_INTEL";
	case CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL:
		return "CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL";
#endif
	case -9999:
		return "NVIDIA_INVALID_BUFFER_ACCESS";
	default:
		return "UNKNOWN_OPENCL_ERROR_CODE";
	}
}

/// <summary>A simple std::runtime_error wrapper for throwing exceptions when receiving OpenCL errors.</summary>
class OpenCLError : public std::runtime_error
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="errorCode">The OpenCL error code to stringify.</param>
	OpenCLError(cl_int errorCode) : runtime_error(std::string("OpenCL Error [") + getOpenCLError(errorCode) + "]") {}
	/// <summary>Constructor.</summary>
	/// <param name="errorCode">The OpenCL error code to stringify.</param>
	/// <param name="message">A message to log alongside the OpenCL error.</param>
	OpenCLError(cl_int errorCode, const std::string& message) : runtime_error(std::string("OpenCL Error [") + getOpenCLError(errorCode) + "] - " + message) {}
};

/// <summary>Determines whether the given OpenCL extension is supported.</summary>
/// <param name="platform">The OpenCL platform.</param>
/// <param name="extension">The OpenCL extension to check support for.</param>
/// <returns>True if the given OpenCL extension is supported.</returns>
inline bool isExtensionSupported(cl_platform_id platform, const char* extension)
{
	// Extension names should not have spaces.
	const char* where = strchr(extension, ' ');

	if (where || *extension == '\0')
	{
		return 0;
	}

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
		if (!where)
		{
			break;
		}
		terminator = where + strlen(extension);
		if (where == start || *(where - 1) == ' ')
		{
			if (*terminator == ' ' || *terminator == '\0')
			{
				return true;
			}
		}
		start = terminator;
	}
	return false;
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
inline void createOpenCLContext(cl_platform_id& outPlatform, cl_device_id& outDevice, cl_context& outContext, cl_command_queue& outQueue,
	cl_command_queue_properties queue_properties = 0, cl_device_type device_type = CL_DEVICE_TYPE_ALL, const char* const platformName = NULL, cl_int* err = 0)
{
	bool contextCreated = false;
	cl_int errstr;
	cl_int& errcode = err ? *err : errstr;

	/*
	 *  Query the available OpenCL platforms.
	 */
	cl_uint numPlatforms;
	errcode = cl::GetPlatformIDs(0, nullptr, &numPlatforms);
	
	if (numPlatforms == 0)
	{
		throw OpenCLError(errcode, "cl::createOpenCLContext : No OpenCL capable platform found");
	}

	std::vector<cl_platform_id> platforms(numPlatforms);
	errcode = cl::GetPlatformIDs(numPlatforms, platforms.data(), NULL);

	if (errcode != CL_SUCCESS)
	{
		throw OpenCLError(errcode, "cl::createOpenCLContext : Failed to query platform IDs");
	}

	/*
	 *  Iterate over all of the available platforms until one is found that matches the requirements.
	 */
	for (cl_uint i = 0; i < platforms.size(); i++)
	{
		/*
		 *  Check whether the platform matches the requested one.
		 */
		size_t platformNameLength = 0;
		errcode = cl::GetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL, &platformNameLength);
		std::vector<char> platName(platformNameLength);
		errcode = cl::GetPlatformInfo(platforms[i], CL_PLATFORM_NAME, platformNameLength, platName.data(), NULL);
		if (errcode != CL_SUCCESS)
		{
			Log(LogLevel::Error, "c.", getOpenCLError(errcode));
			continue;
		}
		if (platformName)
		{
			bool bPlatformNameMatches = false;
			if (strncmp(platName.data(), platformName, platformNameLength) == 0)
			{
				bPlatformNameMatches = true;
			}

			// Continue with the next platform if the current one is unsuitable.
			if (!bPlatformNameMatches)
			{
				continue;
			}
		}

		/*
		 *  Query for the first available device that matches the requirements.
		 */
		cl_uint numDevices;
		errcode = cl::GetDeviceIDs(platforms[i], device_type, 0, NULL, &numDevices);
		std::vector<cl_device_id> devices(numDevices);
		errcode = cl::GetDeviceIDs(platforms[i], device_type, numDevices, devices.data(), NULL);
		if (errcode != CL_SUCCESS && errcode != CL_DEVICE_NOT_FOUND)
		{
			throw OpenCLError(errcode, "[cl::createOpenCLContext]: Failed to query OpenCL devices");
		}

		if (devices.size() == 0)
		{
			// This platform does not have a suitable device, continue with the next platform.
			continue;
		}

		size_t platformExtensionsStringLength, deviceExtensionsStringLength;
		cl::GetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, 0, NULL, &platformExtensionsStringLength);
		std::vector<char> platform_extensions(platformExtensionsStringLength);
		cl::GetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, platformExtensionsStringLength, platform_extensions.data(), NULL);

		cl::GetDeviceInfo(devices[0], CL_DEVICE_EXTENSIONS, 0, NULL, &deviceExtensionsStringLength);
		std::vector<char> device_extensions(deviceExtensionsStringLength);
		cl::GetDeviceInfo(devices[0], CL_DEVICE_EXTENSIONS, deviceExtensionsStringLength, device_extensions.data(), NULL);

		cl_context_properties contextProperties[] = {
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)platforms[i],
			0,
			0,
			0,
			0,
			0,
			0,
			0,
		};

		cl_context context = cl::CreateContext(contextProperties, static_cast<cl_uint>(devices.size()), devices.data(), nullptr, nullptr, &errcode);
		if (errcode != CL_SUCCESS)
		{
			throw OpenCLError(errcode, "[cl::createOpenCLContext]: Failed to create context");
		}

		cl_command_queue q;
		{
			cl_command_queue_properties props = { 0 };

			typedef cl_command_queue(CL_API_CALL * PFNclCreateCommandQueue)(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int * errcode_ret);
			static PFNclCreateCommandQueue _clCreateCommandQueue = (PFNclCreateCommandQueue)cl::internals::getClFunction(cl::CLFunctions::CreateCommandQueue);
			q = _clCreateCommandQueue(context, devices[0], props, &errcode);
		}

		cl_command_queue queue(q);
		if (errcode != CL_SUCCESS || q == NULL)
		{
			throw OpenCLError(errcode, "[cl::createOpenCLContext]: Failed to create command queue");
		}

		Log(LogLevel::Information, "[cl::createOpenCLContext]: Created context on platform %s.", platName.data());
		outPlatform = platforms[i];
		outContext = context;
		outDevice = devices[0];
		outQueue = queue;
		contextCreated = true;
		break;
	}

	if (!contextCreated)
	{
		throw OpenCLError(CL_SUCCESS, "[cl::createOpenCLContext]: No errors occured, but could not find any suitable OpenCL context(platform, context, device and queue)");
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
cl_program loadKernel(
	const cl_context& ctx, const cl_device_id& device, pvr::Stream& kernelSource, const char* compilerOptions = 0, const char* const* defines = 0, uint32_t defineCount = 0)
{
	cl_int errcode;

	// Append define's here if there are any
	std::string pszKernelString;

	kernelSource.open();
	std::string shaderSrc;
	kernelSource.readIntoString(shaderSrc);

	std::string sourceDataStr;
	// insert the defines
	for (uint32_t i = 0; i < defineCount; ++i)
	{
		sourceDataStr.append("#define ");
		sourceDataStr.append(defines[i]);
		sourceDataStr.append("\n");
	}
	sourceDataStr.append("\n");
	sourceDataStr.append(shaderSrc);

	const char* tmp = sourceDataStr.c_str();
	const char** source_string_list_tmp = &tmp;

	cl_program program = cl::CreateProgramWithSource(ctx, 1, source_string_list_tmp, NULL, &errcode);
	if (program == NULL || errcode != CL_SUCCESS)
	{
		throw OpenCLError(errcode, "[cl::loadKernel]:[cl::CreateProgramWithSource] Failed to create OpenCL program ");
	}
	errcode = cl::BuildProgram(program, 1, &device, compilerOptions, nullptr, nullptr);
	if (errcode != CL_SUCCESS)
	{
		cl_int build_errcode = errcode;
		size_t size_of_log;
		errcode = cl::GetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size_of_log);
		std::vector<char> errlog(size_of_log);
		errcode = cl::GetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, size_of_log, &errlog, NULL);
		if (errcode != CL_SUCCESS)
		{
			throw OpenCLError(errcode, "[cl::loadKernel]: Failed to build program. Failed to get program build log.");
		}
		throw OpenCLError(build_errcode, "[cl::loadKernel]: Failed to build program. Build log:\n" + std::string(errlog.data()));
	}
	return program;
}
} // namespace clutils
