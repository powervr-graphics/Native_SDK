/*!*********************************************************************************************************************
\file         PVRCore\NativeLibrary.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Utilities to facilitate loading native libraries in a platform-agnostic way.
***********************************************************************************************************************/
#pragma once
#include <string>

namespace pvr {
/*!*********************************************************************************************************************
\brief    Contains functionality used to interface with the underlying native platform
***********************************************************************************************************************/
namespace native {
/*!*********************************************************************************************************************
\brief    A struct representing a native library. Has utilities to facilitate platform-agnostic loading/unloading.
***********************************************************************************************************************/
struct NativeLibrary
{
public:
	/*!*********************************************************************************************************************
	\brief   Check if the library was loaded properly.
	\return   True if the library did NOT load properly.
	***********************************************************************************************************************/
	bool LoadFailed();
	bool m_disableErrorPrint; //!< Set to true to avoid printing errors

	/*!*********************************************************************************************************************
	\brief   Load a library with the specified filename.
	\param   libraryPath The path to find the library (name or Path+name).
	***********************************************************************************************************************/
	NativeLibrary(const std::string& libraryPath);
	virtual ~NativeLibrary();

	/*!*********************************************************************************************************************
	\brief   Get a function pointer from the library.
	\param   functionName  The name of the function to retrieve the pointer to.
	\return  The function pointer as a void pointer. Null if failed. Cast to the proper type.
	***********************************************************************************************************************/
	void* getFunction(const char* functionName);

	/*!*********************************************************************************************************************
	\brief   Get a function pointer from the library.
	\tparam  PtrType_ The type of the function pointer
	\param   functionName  The name of the function to retrieve the pointer to
	\return  The function pointer. Null if failed.
	***********************************************************************************************************************/
	template<typename PtrType_> PtrType_ getFunction(const char* functionName)
	{
		return reinterpret_cast<PtrType_>(getFunction(functionName));
	}

	/*!*********************************************************************************************************************
	\brief   Release this library.
	***********************************************************************************************************************/
	void CloseLib();
protected:
	void*		m_hHostLib;
	bool		m_bError;
};
}
}