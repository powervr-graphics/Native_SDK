/*!
\brief Utilities to facilitate loading native libraries in a platform-agnostic way.
\file PVRCore/Base/NativeLibrary.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <string>
#include "PVRCore/Log.h"

namespace pvr {
/// <summary>Contains functionality used to interface with the underlying native platform</summary>
namespace native {
/// <summary>A struct representing a native library. Has utilities to facilitate platform-agnostic
/// loading/unloading.</summary>
struct NativeLibrary
{
public:
	/// <summary>Check if the library was loaded properly.</summary>
	/// <returns>True if the library did NOT load properly.</returns>
	bool LoadFailed();
	Logger::Severity errorSeverity; //!< Set to true to avoid printing errors

	/// <summary>Load a library with the specified filename.</summary>
	/// <param name="libraryPath">The path to find the library (name or Path+name).</param>
	NativeLibrary(const std::string& libraryPath, Logger::Severity errorSeverity = Logger::Critical);
	virtual ~NativeLibrary();

	/// <summary>Get a function pointer from the library.</summary>
	/// <param name="functionName">The name of the function to retrieve the pointer to.</param>
	/// <returns>The function pointer as a void pointer. Null if failed. Cast to the proper type.</returns>
	void* getFunction(const char* functionName);

	/// <summary>Get a function pointer from the library.</summary>
	/// <param name="functionName">The name of the function to retrieve the pointer to</param>
	/// <typeparam name="PtrType_">The type of the function pointer</typeparam>
	/// <returns>The function pointer. Null if failed.</returns>
	template<typename PtrType_> PtrType_ getFunction(const char* functionName)
	{
		return reinterpret_cast<PtrType_>(getFunction(functionName));
	}

	/// <summary>Release this library.</summary>
	void CloseLib();
protected:
	void*		_hostLib;
	bool		_bError;
};
}
}