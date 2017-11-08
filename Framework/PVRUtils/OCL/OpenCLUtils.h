#pragma once
#define DYNAMICCL_NO_NAMESPACE
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#include "CL/cl2.hpp"

namespace cl {
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
	case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case CL_COMPILE_PROGRAM_FAILURE: return "CL_COMPILE_PROGRAM_FAILURE";
	case CL_LINKER_NOT_AVAILABLE: return "CL_LINKER_NOT_AVAILABLE";
	case CL_LINK_PROGRAM_FAILURE: return "CL_LINK_PROGRAM_FAILURE";
	case CL_DEVICE_PARTITION_FAILED: return "CL_DEVICE_PARTITION_FAILED";
	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
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
	case CL_INVALID_PROPERTY: return "CL_INVALID_PROPERTY";
	case CL_INVALID_IMAGE_DESCRIPTOR: return "CL_INVALID_IMAGE_DESCRIPTOR";
	case CL_INVALID_COMPILER_OPTIONS: return "CL_INVALID_COMPILER_OPTIONS";
	case CL_INVALID_LINKER_OPTIONS: return "CL_INVALID_LINKER_OPTIONS";
	case CL_INVALID_DEVICE_PARTITION_COUNT: return "CL_INVALID_DEVICE_PARTITION_COUNT";
#ifdef CL_INVALID_PIPE_SIZE
	case CL_INVALID_PIPE_SIZE: return "CL_INVALID_PIPE_SIZE";
#endif
#ifdef CL_INVALID_DEVICE_QUEUE
	case CL_INVALID_DEVICE_QUEUE: return "CL_INVALID_DEVICE_QUEUE";
#endif
#ifdef CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR
	case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
#endif
#ifdef CL_PLATFORM_NOT_FOUND_KHR
	case CL_PLATFORM_NOT_FOUND_KHR: return "CL_PLATFORM_NOT_FOUND_KHR";
#endif
#ifdef CL_INVALID_D3D10_DEVICE_KHR
	case CL_INVALID_D3D10_DEVICE_KHR: return "CL_INVALID_D3D10_DEVICE_KHR";
	case CL_INVALID_D3D10_RESOURCE_KHR: return "CL_INVALID_D3D10_RESOURCE_KHR";
	case CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	case CL_INVALID_D3D11_DEVICE_KHR: return "CL_INVALID_D3D11_DEVICE_KHR";
	case CL_INVALID_D3D11_RESOURCE_KHR: return "CL_INVALID_D3D11_RESOURCE_KHR";
	case CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR: return "CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR";
	case CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR: return "CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR";
	case CL_INVALID_D3D9_DEVICE_NV: return "CL_INVALID_D3D9_DEVICE_NV";
	case CL_INVALID_D3D9_RESOURCE_NV: return "CL_INVALID_D3D9_RESOURCE_NV";
	case CL_D3D9_RESOURCE_ALREADY_ACQUIRED_NV: return "CL_D3D9_RESOURCE_ALREADY_ACQUIRED_NV";
	case CL_D3D9_RESOURCE_NOT_ACQUIRED_NV: return "CL_D3D9_RESOURCE_NOT_ACQUIRED_NV";
#endif
#ifdef CL_EGL_RESOURCE_NOT_ACQUIRED_KHR
	case CL_EGL_RESOURCE_NOT_ACQUIRED_KHR: return "CL_EGL_RESOURCE_NOT_ACQUIRED_KHR";
	case CL_INVALID_EGL_OBJECT_KHR: return "CL_INVALID_EGL_OBJECT_KHR";
#endif
#ifdef CL_INVALID_ACCELERATOR_INTEL
	case CL_INVALID_ACCELERATOR_INTEL: return "CL_INVALID_ACCELERATOR_INTEL";
	case CL_INVALID_ACCELERATOR_TYPE_INTEL: return "CL_INVALID_ACCELERATOR_TYPE_INTEL";
	case CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL: return "CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL";
	case CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL: return "CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL";
	case CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL: return "CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL";
	case CL_INVALID_VA_API_MEDIA_SURFACE_INTEL: return "CL_INVALID_VA_API_MEDIA_SURFACE_INTEL";
	case CL_VA_API_MEDIA_SURFACE_ALREADY_ACQUIRED_INTEL: return "CL_VA_API_MEDIA_SURFACE_ALREADY_ACQUIRED_INTEL";
	case CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL: return "CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL";
#endif
	case -9999: return "NVIDIA_INVALID_BUFFER_ACCESS";
	default: return "UNKNOWN_OPENCL_ERROR_CODE";
	}
}

inline bool isExtensionSupported(const cl::Platform& platform, const char* extension)
{
	// Extension names should not have spaces.
	const char* where = strchr(extension, ' ');

	if (where || *extension == '\0')
	{
		return 0;
	}

	std::string extensions;
	platform.getInfo(CL_PLATFORM_EXTENSIONS, &extensions);

	const char* start;
	const char* terminator;

	start = extensions.c_str();
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

inline bool createOpenCLContext(
  cl::Platform& outPlatform, cl::Device& outDevice, cl::Context& outContext, cl::CommandQueue& outQueue,
  cl_command_queue_properties queue_properties = 0, cl_device_type device_type = CL_DEVICE_TYPE_ALL,
  const char* const platformName = NULL, cl_int* err = 0)
{
	bool contextCreated = false;
	cl_int errstr;
	cl_int& errcode = err ? *err : errstr;

	/*
	*  Query the available OpenCL platforms.
	*/
	std::vector<cl::Platform> platforms;
	errcode = cl::Platform::get(&platforms);
	if (errcode != CL_SUCCESS)
	{
		Log(LogLevel::Error, "createOpenCLContext : Failed to query platform IDs with code %s.", getOpenCLError(errcode));
		return false;
	}

	if (platforms.size() == 0)
	{
		Log(LogLevel::Error, "createOpenCLContext : No OpenCL capable platform found with code %s.", getOpenCLError(errcode));
		return false;
	}

	/*
	*  Iterate over all of the available platforms until one is found that matches the requirements.
	*/
	for (cl_uint i = 0; i < platforms.size(); i++)
	{
		/*
		*  Check whether the platform matches the requested one.
		*/
		std::string platName;
		size_t platformNameLength = 0;
		errcode = platforms[i].getInfo(CL_PLATFORM_NAME, &platName);
		if (errcode != CL_SUCCESS)
		{
			Log(LogLevel::Error, "createOpenCLContext: Error: Failed to query platform name with code %s.", getOpenCLError(errcode));
			continue ;
		}
		if (platformName)
		{
			bool bPlatformNameMatches = false;
			if (strncmp(platName.c_str(), platformName, platformNameLength) == 0)
			{ bPlatformNameMatches = true; }

			// Continue with the next platform if the current one is unsuitable.
			if (!bPlatformNameMatches)
			{ continue; }
		}

		/*
		*  Query for the first available device that matches the requirements.
		*/
		std::vector<cl::Device> devices;
		errcode = platforms[i].getDevices(device_type, &devices);
		if (errcode != CL_SUCCESS && errcode != CL_DEVICE_NOT_FOUND)
		{
			Log(LogLevel::Error, "createOpenCLContext: Failed to query OpenCL devices with code %s.", getOpenCLError(errcode));
			return false;
		}

		if (devices.size() == 0)
		{
			// This platform does not have a suitable device, continue with the next platform.
			continue;
		}

		std::string platform_extensions;
		platforms[i].getInfo(CL_PLATFORM_EXTENSIONS, &platform_extensions);
		std::string device_extensions;
		devices[0].getInfo(CL_DEVICE_EXTENSIONS, &device_extensions);

		cl_platform_id platId = platforms[i].Wrapper<cl_platform_id>::get();
		cl_context_properties contextProperties[] =
		{
			CL_CONTEXT_PLATFORM, (cl_context_properties)platId,
			0, 0,
			0, 0,
			0, 0,
			0,
		};

		cl::Context context = cl::Context(devices, contextProperties, nullptr, nullptr, &errcode);
		if (errcode != CL_SUCCESS)
		{
			Log(LogLevel::Error, "createOpenCLContext: Failed to create context with code %s.", getOpenCLError(errcode));
			return false;
		}

#ifdef __APPLE__
		cl_queue_properties_APPLE props[] = { 0 };

		cl_command_queue q =
		  clCreateCommandQueueWithPropertiesAPPLE(context.get(), devices[0].get(), props, &errcode);
#else
		cl_command_queue q;
		{
			cl_command_queue_properties props = { NULL };
			q = clCreateCommandQueue(context.get(), devices[0].get(), props, &errcode);
		}
#endif
		cl::CommandQueue queue(q);
		if (errcode != CL_SUCCESS || q == NULL)
		{
			Log(LogLevel::Error, "createOpenCLContext: Failed to create command queue with code %s.", getOpenCLError(errcode));
			return false;
		}

		Log(LogLevel::Information, "createOpenCLContext: Created context on platform %s.", platName.c_str());
		outPlatform = platforms[i];
		outContext = context;
		outDevice = devices[0];
		outQueue = queue;
		contextCreated = true;
		break;
	}

	if (!contextCreated)
	{
		Log(LogLevel::Error, "createOpenCLContext: No suitable OpenCL context(platform, context, device and queue) found");
		return false;
	}

	return true;
}

cl::Program loadKernel(const cl::Context& ctx, const cl::Device& device, pvr::Stream& kernelSource, const char* compilerOptions = 0, const char* const* defines = 0, uint32_t defineCount = 0)
{
	cl_int errcode;

	// Append define's here if there are any
	std::string pszKernelString;

	if (!kernelSource.isopen() && !kernelSource.open())
	{
		Log(LogLevel::Error, "loadShader: Could not open the shaderSource stream.");
		return cl::Program();
	}
	std::string shaderSrc;
	if (!kernelSource.readIntoString(shaderSrc))
	{
		Log(LogLevel::Error, "Error: Failed to load the OpenCL shader");
		return cl::Program();
	};

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

	cl::Program program(ctx, sourceDataStr, false, &errcode);
	if (program.get() == NULL || errcode != CL_SUCCESS)
	{
		Log(LogLevel::Error, "Error: Failed to create OpenCL program with code %s", getOpenCLError(errcode));
		return cl::Program();
	}
	errcode = program.build();
	if (errcode != CL_SUCCESS)
	{
		std::string errlog;
		errcode = program.getBuildInfo<std::string>(device, CL_PROGRAM_BUILD_LOG, &errlog);
		if (errcode != CL_SUCCESS)
		{
			Log(LogLevel::Error, "Error: Failed to get build log with code %s", getOpenCLError(errcode));
			return cl::Program();
		}
		Log(LogLevel::Error, errlog.c_str());
		return cl::Program();
	}
	return program;
}
}
