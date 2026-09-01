#pragma once
// clang-format off
#include "imgdnn/cl.h"
#include "DynamicIMGDNN.h"

namespace imgdnn {
namespace internal {
	namespace imgdnnFuncName {
	enum ImgdnnCLFuncName
	{
			CLCreateContext,
			CLImportEvent,
			CLExportEvent,
			NUMBER_OF_IMGDNN_CL_FUNCTIONS
	};
}

inline void* getImgdnnCLFunction(imgdnnFuncName::ImgdnnCLFuncName funcname)
{
	static void* FunctionTable[imgdnnFuncName::NUMBER_OF_IMGDNN_CL_FUNCTIONS];

	//  Get function pointers. This is done the first time any function is called, then the entire api is initialised.
	if (!FunctionTable[0])
	{
		pvr::lib::LIBTYPE lib = pvr::lib::openlib(imgdnnLibName);
		if (!lib)
		{
			Log_Error("IMGDNN_CL Dynamic Bindings: Failed to open library %s\n", imgdnnLibName);
		}
		else
		{
			Log_Info("IMGDNN_CL Dynamic Bindings: Successfully loaded library %s\n", imgdnnLibName);
		}
		
		FunctionTable[imgdnnFuncName::CLCreateContext] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCLCreateContext"); 
		FunctionTable[imgdnnFuncName::CLImportEvent] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCLImportEvent"); 
		FunctionTable[imgdnnFuncName::CLExportEvent] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCLExportEvent"); 
	}
	return FunctionTable[funcname];
}
}





/*************************************************************************//*!
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        MIT
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
@file
@brief          OpenCL interop extensions to IMGDNN
*//**************************************************************************//**
* Create a IMGDNN context from an OpenCL context.
*
* An IMGDNN context can be created that uses the given OpenCL context. IMGDNN
* requires all the devices to be known, to be able to set up memory and events
* to use the context correctly. Although the devices could be queried through
* the OpenCL context, for safety and simplicity we require them as inputs and
* pass back the equivalent IMGDNN devices in the same respective order.
*
* @Input  context  An OpenCL context to use for this IMGDNN context.
* @Input  num_devices  Number of devices in this OpenCL context. As queried by
*                      clGetContextInfo.
* @Input  devices  A num_devices sized array of OpenCL devices of this OpenCL
*                  context. Must be all the devices as queried by
*                  clGetContextInfo, though not necessarily in the same order.
* @Input  context_flags  Flags to modify how the context behaves.
* @Output out_devices  A num_devices sized output array of IMGDNN devices.
*                      These are the IMGDNN device equivalents to the OpenCL
*                      devices in the same respective order.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_VALUE for
*                      an incorrect number or set of devices, or
*                      IMGDNN_OUT_OF_MEMORY.
* @Return The returned IMGDNN context created or NULL on failure.
*/
inline imgdnn_context CLCreateContext(cl_context context, unsigned int num_devices, const cl_device_id devices[], const imgdnn_context_flags context_flags, imgdnn_device * out_devices, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_context (*PROC_IMGDNN_FUNC_TYPEDEF)(cl_context, unsigned int, const cl_device_id[], const imgdnn_context_flags, imgdnn_device *, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnCLFunction(imgdnn::internal::imgdnnFuncName::CLCreateContext);
	return _func_ptr(context, num_devices, devices, context_flags, out_devices, errcode_ret);
}

/**
* Create an IMGDNN event from an OpenCL event.
*
* The event must be destroyed as normal.
*
* @Input  context  An IMGDNN context to import into. Must be created from the
*                  same cl_context as the cl_event originates from.
* @Input  event  The OpenCL event to import.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_VALUE for
*                      mismatching contexts or invalid objects, or
*                      IMGDNN_OUT_OF_MEMORY.
* @Return The returned IMGDNN event or NULL on failure.
*/
inline imgdnn_event CLImportEvent(imgdnn_context context, cl_event event, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_event (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_context, cl_event, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnCLFunction(imgdnn::internal::imgdnnFuncName::CLImportEvent);
	return _func_ptr(context, event, errcode_ret);
}

/**
* Create an OpenCL event from an IMGDNN event.
*
* The returned cl_event can only be used in the cl_context associated with the
* imgdnn_context of the imgdnn_event. The cl_event must be released as normal.
*
* @Input  event  The IMGDNN event to export.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_VALUE for
*                      an invalid event, or IMGDNN_OUT_OF_MEMORY.
* @Return The returned OpenCL event or NULL on failure.
*/
inline cl_event CLExportEvent(imgdnn_event event, imgdnn_err_code * errcode_ret) {
	typedef cl_event (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_event, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnCLFunction(imgdnn::internal::imgdnnFuncName::CLExportEvent);
	return _func_ptr(event, errcode_ret);
}

} //namespace imgdnn

