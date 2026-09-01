#pragma once
// clang-format off
#include "imgdnn/imgdnn.h"
#include <stdexcept>
#include "pvr_openlib.h"
#include <string>

namespace imgdnn {
namespace internal {
	static const char* imgdnnLibName = "libIMGDNN.so";
	namespace imgdnnFuncName {
	enum ImgdnnFuncName
	{
			GetDevices,
			GetDeviceInfo,
			CreateContext,
			ContextDestroy,
			AllocateMemory,
			ImportMemory,
			SubdivideMemory,
			MemoryDestroy,
			MemoryLock,
			MemoryUnlock,
			GetDescriptorSize,
			GetTensorDescriptor,
			CreateNetwork,
			CreateNetworkFromIR,
			NetworkDestroy,
			NetworkInput,
			NetworkFixedInput,
			TensorSetName,
			TensorGetName,
			NetworkFindTensor,
			NetworkSetName,
			NetworkGetName,
			NetworkGetObjectName,
			NetworkFindInputs,
			NetworkFindDefaultOutputs,
			NetworkReshapeOp,
			NetworkTransposeOp,
			NetworkCastOp,
			NetworkBroadcastOp,
			NetworkSubTensor,
			NetworkInterleaveOp,
			NetworkSplitOp,
			NetworkConcatOp,
			NetworkUnaryOp,
			NetworkReLUOp,
			NetworkBinaryOp,
			NetworkConvolution2dOp,
			NetworkConvolution2dOp_v2,
			NetworkGroupedConvolution2dOp,
			NetworkDepthConvolution2dOp,
			NetworkDepthConvolution2dOp_v2,
			NetworkDeconvolution2dOp_v2,
			NetworkDeconvolution2dOp,
			NetworkGroupedDeconvolution2dOp,
			NetworkPooling2dOp,
			NetworkPooling2dOp_v2,
			NetworkPooling2dOp_v3,
			NetworkLrnOp,
			NetworkImageTransformOp,
			NetworkReduceOp,
			NetworkSoftmaxOp,
			NetworkLSTMOp,
			NetworkDepthToSpaceOp,
			NetworkBatchToSpaceNDOp,
			NetworkGatherOp,
			NetworkSpaceToDepthOp,
			NetworkSpaceToBatchNDOp,
			NetworkResizeBilinearOp,
			NetworkPadOp,
			NetworkPadOp_v2,
			NetworkResizeNearestNeighbourOp,
			NetworkROIPoolingOp,
			NetworkROIAlignOp,
			GetInputDescriptor,
			GetOutputDescriptor,
			FillDataLayoutParameters,
			GetInputTensorParameter,
			GetOutputTensorParameter,
			CreateNetworkObject_v2,
			CreateNetworkObject,
			CreateNetworkBinary_v2,
			CreateNetworkBinary,
			NetworkBinaryDestroy,
			LoadNetworkObject,
			NetworkObjectDestroy,
			NetworkObjectGetInputs,
			NetworkObjectGetOutputs,
			CreateBinding,
			BindingDestroy,
			BindingAddInput,
			BindingAddInputSize,
			BindingAddOutput,
			NetworkObjectExecute,
			WaitForEvent,
			EventDestroy,
			SetErrorHandler,
			GetApiVersion,
			GetDriverVersion,
			CreatePerAxisQuantParam,
			DestroyPerAxisQuantParam,
			NUMBER_OF_IMGDNN_FUNCTIONS
	};
}

inline void* getImgdnnFunction(imgdnnFuncName::ImgdnnFuncName funcname)
{
	static void* FunctionTable[imgdnnFuncName::NUMBER_OF_IMGDNN_FUNCTIONS];

	//  Get function pointers. This is done the first time any function is called, then the entire api is initialised.
	if (!FunctionTable[0])
	{
		pvr::lib::LIBTYPE lib = pvr::lib::openlib(imgdnnLibName);
		if (!lib)
		{
			Log_Error("IMGDNN Dynamic Bindings: Failed to open library %s\n", imgdnnLibName);
		}
		else
		{
			Log_Info("IMGDNN Dynamic Bindings: Successfully loaded library %s\n", imgdnnLibName);
		}
		
		FunctionTable[imgdnnFuncName::GetDevices] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnGetDevices"); 
		FunctionTable[imgdnnFuncName::GetDeviceInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnGetDeviceInfo"); 
		FunctionTable[imgdnnFuncName::CreateContext] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCreateContext"); 
		FunctionTable[imgdnnFuncName::ContextDestroy] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnContextDestroy"); 
		FunctionTable[imgdnnFuncName::AllocateMemory] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnAllocateMemory"); 
		FunctionTable[imgdnnFuncName::ImportMemory] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnImportMemory"); 
		FunctionTable[imgdnnFuncName::SubdivideMemory] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnSubdivideMemory"); 
		FunctionTable[imgdnnFuncName::MemoryDestroy] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnMemoryDestroy"); 
		FunctionTable[imgdnnFuncName::MemoryLock] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnMemoryLock"); 
		FunctionTable[imgdnnFuncName::MemoryUnlock] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnMemoryUnlock"); 
		FunctionTable[imgdnnFuncName::GetDescriptorSize] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnGetDescriptorSize"); 
		FunctionTable[imgdnnFuncName::GetTensorDescriptor] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnGetTensorDescriptor"); 
		FunctionTable[imgdnnFuncName::CreateNetwork] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCreateNetwork"); 
		FunctionTable[imgdnnFuncName::CreateNetworkFromIR] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCreateNetworkFromIR"); 
		FunctionTable[imgdnnFuncName::NetworkDestroy] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkDestroy"); 
		FunctionTable[imgdnnFuncName::NetworkInput] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkInput"); 
		FunctionTable[imgdnnFuncName::NetworkFixedInput] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkFixedInput"); 
		FunctionTable[imgdnnFuncName::TensorSetName] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnTensorSetName"); 
		FunctionTable[imgdnnFuncName::TensorGetName] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnTensorGetName"); 
		FunctionTable[imgdnnFuncName::NetworkFindTensor] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkFindTensor"); 
		FunctionTable[imgdnnFuncName::NetworkSetName] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkSetName"); 
		FunctionTable[imgdnnFuncName::NetworkGetName] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkGetName"); 
		FunctionTable[imgdnnFuncName::NetworkGetObjectName] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkGetObjectName"); 
		FunctionTable[imgdnnFuncName::NetworkFindInputs] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkFindInputs"); 
		FunctionTable[imgdnnFuncName::NetworkFindDefaultOutputs] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkFindDefaultOutputs"); 
		FunctionTable[imgdnnFuncName::NetworkReshapeOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkReshapeOp"); 
		FunctionTable[imgdnnFuncName::NetworkTransposeOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkTransposeOp"); 
		FunctionTable[imgdnnFuncName::NetworkCastOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkCastOp"); 
		FunctionTable[imgdnnFuncName::NetworkBroadcastOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkBroadcastOp"); 
		FunctionTable[imgdnnFuncName::NetworkSubTensor] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkSubTensor"); 
		FunctionTable[imgdnnFuncName::NetworkInterleaveOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkInterleaveOp"); 
		FunctionTable[imgdnnFuncName::NetworkSplitOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkSplitOp"); 
		FunctionTable[imgdnnFuncName::NetworkConcatOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkConcatOp"); 
		FunctionTable[imgdnnFuncName::NetworkUnaryOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkUnaryOp"); 
		FunctionTable[imgdnnFuncName::NetworkReLUOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkReLUOp"); 
		FunctionTable[imgdnnFuncName::NetworkBinaryOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkBinaryOp"); 
		FunctionTable[imgdnnFuncName::NetworkConvolution2dOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkConvolution2dOp"); 
		FunctionTable[imgdnnFuncName::NetworkConvolution2dOp_v2] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkConvolution2dOp_v2"); 
		FunctionTable[imgdnnFuncName::NetworkGroupedConvolution2dOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkGroupedConvolution2dOp"); 
		FunctionTable[imgdnnFuncName::NetworkDepthConvolution2dOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkDepthConvolution2dOp"); 
		FunctionTable[imgdnnFuncName::NetworkDepthConvolution2dOp_v2] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkDepthConvolution2dOp_v2"); 
		FunctionTable[imgdnnFuncName::NetworkDeconvolution2dOp_v2] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkDeconvolution2dOp_v2"); 
		FunctionTable[imgdnnFuncName::NetworkDeconvolution2dOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkDeconvolution2dOp"); 
		FunctionTable[imgdnnFuncName::NetworkGroupedDeconvolution2dOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkGroupedDeconvolution2dOp"); 
		FunctionTable[imgdnnFuncName::NetworkPooling2dOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkPooling2dOp"); 
		FunctionTable[imgdnnFuncName::NetworkPooling2dOp_v2] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkPooling2dOp_v2"); 
		FunctionTable[imgdnnFuncName::NetworkPooling2dOp_v3] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkPooling2dOp_v3"); 
		FunctionTable[imgdnnFuncName::NetworkLrnOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkLrnOp"); 
		FunctionTable[imgdnnFuncName::NetworkImageTransformOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkImageTransformOp"); 
		FunctionTable[imgdnnFuncName::NetworkReduceOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkReduceOp"); 
		FunctionTable[imgdnnFuncName::NetworkSoftmaxOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkSoftmaxOp"); 
		FunctionTable[imgdnnFuncName::NetworkLSTMOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkLSTMOp"); 
		FunctionTable[imgdnnFuncName::NetworkDepthToSpaceOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkDepthToSpaceOp"); 
		FunctionTable[imgdnnFuncName::NetworkBatchToSpaceNDOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkBatchToSpaceNDOp"); 
		FunctionTable[imgdnnFuncName::NetworkGatherOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkGatherOp"); 
		FunctionTable[imgdnnFuncName::NetworkSpaceToDepthOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkSpaceToDepthOp"); 
		FunctionTable[imgdnnFuncName::NetworkSpaceToBatchNDOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkSpaceToBatchNDOp"); 
		FunctionTable[imgdnnFuncName::NetworkResizeBilinearOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkResizeBilinearOp"); 
		FunctionTable[imgdnnFuncName::NetworkPadOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkPadOp"); 
		FunctionTable[imgdnnFuncName::NetworkPadOp_v2] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkPadOp_v2"); 
		FunctionTable[imgdnnFuncName::NetworkResizeNearestNeighbourOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkResizeNearestNeighbourOp"); 
		FunctionTable[imgdnnFuncName::NetworkROIPoolingOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkROIPoolingOp"); 
		FunctionTable[imgdnnFuncName::NetworkROIAlignOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkROIAlignOp"); 
		FunctionTable[imgdnnFuncName::GetInputDescriptor] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnGetInputDescriptor"); 
		FunctionTable[imgdnnFuncName::GetOutputDescriptor] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnGetOutputDescriptor"); 
		FunctionTable[imgdnnFuncName::FillDataLayoutParameters] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnFillDataLayoutParameters"); 
		FunctionTable[imgdnnFuncName::GetInputTensorParameter] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnGetInputTensorParameter"); 
		FunctionTable[imgdnnFuncName::GetOutputTensorParameter] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnGetOutputTensorParameter"); 
		FunctionTable[imgdnnFuncName::CreateNetworkObject_v2] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCreateNetworkObject_v2"); 
		FunctionTable[imgdnnFuncName::CreateNetworkObject] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCreateNetworkObject"); 
		FunctionTable[imgdnnFuncName::CreateNetworkBinary_v2] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCreateNetworkBinary_v2"); 
		FunctionTable[imgdnnFuncName::CreateNetworkBinary] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCreateNetworkBinary"); 
		FunctionTable[imgdnnFuncName::NetworkBinaryDestroy] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkBinaryDestroy"); 
		FunctionTable[imgdnnFuncName::LoadNetworkObject] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnLoadNetworkObject"); 
		FunctionTable[imgdnnFuncName::NetworkObjectDestroy] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkObjectDestroy"); 
		FunctionTable[imgdnnFuncName::NetworkObjectGetInputs] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkObjectGetInputs"); 
		FunctionTable[imgdnnFuncName::NetworkObjectGetOutputs] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkObjectGetOutputs"); 
		FunctionTable[imgdnnFuncName::CreateBinding] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCreateBinding"); 
		FunctionTable[imgdnnFuncName::BindingDestroy] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnBindingDestroy"); 
		FunctionTable[imgdnnFuncName::BindingAddInput] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnBindingAddInput"); 
		FunctionTable[imgdnnFuncName::BindingAddInputSize] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnBindingAddInputSize"); 
		FunctionTable[imgdnnFuncName::BindingAddOutput] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnBindingAddOutput"); 
		FunctionTable[imgdnnFuncName::NetworkObjectExecute] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnNetworkObjectExecute"); 
		FunctionTable[imgdnnFuncName::WaitForEvent] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnWaitForEvent"); 
		FunctionTable[imgdnnFuncName::EventDestroy] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnEventDestroy"); 
		FunctionTable[imgdnnFuncName::SetErrorHandler] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnSetErrorHandler"); 
		FunctionTable[imgdnnFuncName::GetApiVersion] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnGetApiVersion"); 
		FunctionTable[imgdnnFuncName::GetDriverVersion] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnGetDriverVersion"); 
		FunctionTable[imgdnnFuncName::CreatePerAxisQuantParam] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnCreatePerAxisQuantParam"); 
		FunctionTable[imgdnnFuncName::DestroyPerAxisQuantParam] = pvr::lib::getLibFunctionChecked<void*>(lib, "imgdnnDestroyPerAxisQuantParam"); 
	}
	return FunctionTable[funcname];
}
}

/**
* Get the list of available devices.
*
* @Input  device_type  The type of requested devices.
* @Input  max_devices  The maximum number of devices that can be added to devices list if not NULL.
* @Output devices      The list of returned IMGDNN devices if not NULL
* @Output num_devices  The number of devices of a particular type that are present. May not be NULL.
* @Return IMGDNN_SUCCESS on success. IMGDNN_INVALID_VALUE or IMGDNN_FAILURE for failure.
*/
inline imgdnn_err_code GetDevices(imgdnn_device_type device_type, unsigned int max_devices, imgdnn_device devices[], unsigned int * num_devices) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_device_type, unsigned int, imgdnn_device[], unsigned int *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::GetDevices);
	return _func_ptr(device_type, max_devices, devices, num_devices);
}

/**
* Get the  specific information of a particular device.
*
* @Input  device                 The device for which information is requested.
* @Input  device_info            The identifier for the device information being requested.
* @Input  device_info_data_size  The size in bytes of memory pointed by device_info_data.
* @Output device_info_data       Pointer to memory location where device info data is stored. May not be NULL.
* @Return IMGDNN_SUCCESS on success. IMGDNN_INVALID_VALUE or IMGDNN_FAILURE for
* failure
*/
inline imgdnn_err_code GetDeviceInfo(imgdnn_device device, imgdnn_device_info device_info, size_t device_info_data_size, void * device_info_data) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_device, imgdnn_device_info, size_t, void *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::GetDeviceInfo);
	return _func_ptr(device, device_info, device_info_data_size, device_info_data);
}

/**
* Create a IMGDNN context from a number of IMGDNN devices.
*
* @Input  num_devices    Number of devices in the device array. Must be > 0
* @Input  devices[]      An array of IMGDNN devices for which DNN context is created.
* @Input  context_flags  Flags to modify how the device context behaves.
* @Output errcode_ret    IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_DEVICE
* @Return The returned IMGDNN context created or NULL on failure.
*/
inline imgdnn_context CreateContext(unsigned int num_devices, const imgdnn_device devices[], const imgdnn_context_flags context_flags, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_context (*PROC_IMGDNN_FUNC_TYPEDEF)(unsigned int, const imgdnn_device[], const imgdnn_context_flags, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::CreateContext);
	return _func_ptr(num_devices, devices, context_flags, errcode_ret);
}

/**
* Destroy a previously created IMGDNN context.
*
* @Input  context  A previously created IMGDNN context
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_CONTEXT
*/
inline imgdnn_err_code ContextDestroy(imgdnn_context context) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_context);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::ContextDestroy);
	return _func_ptr(context);
}

/**
* Allocate Device Memory.
*
* @Input  context      A previously obtained IMGDNN context
* @Input  size         Size of allocation in bytes
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_OUT_OF_MEMORY
* @Return imgdnn_memory object on success and NULL on failure
*/
inline imgdnn_memory AllocateMemory(imgdnn_context context, size_t size, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_memory (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_context, size_t, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::AllocateMemory);
	return _func_ptr(context, size, errcode_ret);
}

/**
* Import Memory from externally allocated memory.
*
* @Input  context           A previously obtained IMGDNN context
* @Input  memory            External memory to be imported to img dnn memory
*                           in case of IMGDNN_IMPORT_MEM_TYPE_FD import, shall point to single 'int'
*                           containing buffer file descriptor
* @Input  size              Size of allocation in bytes
* @Input  import_mem_type   Type of external memory to be imported.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_OUT_OF_MEMORY
* @Return imgdnn_memory object on success and NULL on failure
*/
inline imgdnn_memory ImportMemory(imgdnn_context context, void * memory, size_t size, imgdnn_import_mem_type import_mem_type, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_memory (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_context, void *, size_t, imgdnn_import_mem_type, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::ImportMemory);
	return _func_ptr(context, memory, size, import_mem_type, errcode_ret);
}

/**
* Divide IMG DNN memory into smaller buffers
* Note:
*       - creating overlapping buffers is not supported
*       - only one level allowed (original buffer might be divided into smaller ones,
*         but the smaller buffers cannot be subdivided further)
*       - original buffer may only be freed when all the sub-buffers are freed
*
* @Input  memory            Original memory (obtained via Allocate or Import)
* @Input  offset            Offset of the smaller buffer in memory
* @Input  size              Size of the smaller buffer
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE
* @Return imgdnn_memory object on success and NULL on failure
*/
inline imgdnn_memory SubdivideMemory(imgdnn_memory memory, uint32_t offset, size_t size, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_memory (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_memory, uint32_t, size_t, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::SubdivideMemory);
	return _func_ptr(memory, offset, size, errcode_ret);
}

/**
* Release Previously allocated Device Memory.
*
* @Input  memory  Previously allocated device memory
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE
*/
inline imgdnn_err_code MemoryDestroy(imgdnn_memory memory) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_memory);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::MemoryDestroy);
	return _func_ptr(memory);
}

/**
* Lock the memory for host access. Obtain host accessible pointer to memory.
*
* @Input  memory             Memory object for which host access is sought.
* @Input  lock_access        Lock access type.
* @Output errcode_ret        IMGDNN_SUCCESS, IMGDNN_FAILURE
* @Return Host accessible pointer to memory on success and NULL on failure
*/
inline void * MemoryLock(imgdnn_memory memory, imgdnn_lock_access lock_access, imgdnn_err_code * errcode_ret) {
	typedef void * (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_memory, imgdnn_lock_access, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::MemoryLock);
	return _func_ptr(memory, lock_access, errcode_ret);
}

/**
* Unlock the memory that was previously locked for host access.
*
* @Input  memory    Memory object to unlock.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE
*/
inline imgdnn_err_code MemoryUnlock(imgdnn_memory memory) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_memory);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::MemoryUnlock);
	return _func_ptr(memory);
}

/**
* Utility function to get size of a descriptor
*
* @Input  descriptor   The descriptor to find total size of
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid descriptor.
*                      May be NULL.
* @Return The total size of the data referenced by this descriptor.
*/
inline size_t GetDescriptorSize(const imgdnn_tensor_descriptor * const descriptor, imgdnn_err_code * errcode_ret) {
	typedef size_t (*PROC_IMGDNN_FUNC_TYPEDEF)(const imgdnn_tensor_descriptor * const, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::GetDescriptorSize);
	return _func_ptr(descriptor, errcode_ret);
}

/**
* Obtain the descriptor of a tensor.
*
* If the tensor is of type IMGDNN_TYPE_QPA_* the caller is resposible of destroying the
* imgdnn_per_axis_quant_param structure (desc->quant_param->per_axis) by calling
* imgdnnDestroyPerAxisQuantParam
*
* @Input  tensor  A tensor previously created.
* @Output desc    The descriptor of the given tensor. May not be NULL.
* @Return IMGDNN_SUCCESS or IMGDNN_INVALID_VALUE for invalid tensor.
*/
inline imgdnn_err_code GetTensorDescriptor(imgdnn_tensor tensor, imgdnn_tensor_descriptor * desc) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_tensor, imgdnn_tensor_descriptor *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::GetTensorDescriptor);
	return _func_ptr(tensor, desc);
}

/**
* Create a Network
*
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE.
* @Return The network created.
*/
inline imgdnn_network CreateNetwork(imgdnn_err_code * errcode_ret) {
	typedef imgdnn_network (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::CreateNetwork);
	return _func_ptr(errcode_ret);
}

/**
* Create a Network from IR
*
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE.
* @Return The network created.
*/
inline imgdnn_network CreateNetworkFromIR(const void * arch_data, size_t arch_data_size, const void * params_data, size_t params_data_size, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_network (*PROC_IMGDNN_FUNC_TYPEDEF)(const void *, size_t, const void *, size_t, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::CreateNetworkFromIR);
	return _func_ptr(arch_data, arch_data_size, params_data, params_data_size, errcode_ret);
}

/**
* Destroy a previously created Network
*
* @Input  network      A previously created network.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_VALUE for invalid network.
*/
inline imgdnn_err_code NetworkDestroy(imgdnn_network network) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkDestroy);
	return _func_ptr(network);
}

/**
* Add an input to the network.
*
* @Input  network      Handle to the network.
* @Input  descriptor   The description of the input to add.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid descriptor,
*                      IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY. May be NULL.
* @Return The created input tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkInput(imgdnn_network network, const imgdnn_tensor_descriptor * const descriptor, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, const imgdnn_tensor_descriptor * const, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkInput);
	return _func_ptr(network, descriptor, errcode_ret);
}

/**
* Add a tensor with fixed data to the network.
*
* @Input  network      Handle to the network.
* @Input  descriptor   The description of the fixed tensor to add.
* @Input  fixed_data   The data to use for the tensor. This data is fixed and will
*                      likely be optimised by the implementation. May not be NULL.
*                      Must stay valid until the network object is created.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid descriptor,
*                      IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY.
* @Return The created fixed tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkFixedInput(imgdnn_network network, const imgdnn_tensor_descriptor * const descriptor, const void * const fixed_data, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, const imgdnn_tensor_descriptor * const, const void * const, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkFixedInput);
	return _func_ptr(network, descriptor, fixed_data, errcode_ret);
}

/**
* Sets the name of a tensor.
*
* @Input  tensor  The tensor to set name of. May not be NULL.
* @Input  name  The given name, may not be NULL.
* @Return errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid tensor,
*                      or IMGDNN_OUT_OF_MEMORY.
*/
inline imgdnn_err_code TensorSetName(imgdnn_tensor tensor, const char * name) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_tensor, const char *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::TensorSetName);
	return _func_ptr(tensor, name);
}

/**
* Get the name of a tensor.
*
* The returned pointer will be internal and stay valid until at least the next
* API call on the same thread.
*
* @Input  tensor  The tensor to get the name of. May not be NULL.
* @Output errcode_ret  IMGDNN_SUCCESS or IMGDNN_INVALID_VALUE for invalid tensor.
* @Return The given name, NULL on failure.
*/
inline const char * TensorGetName(imgdnn_tensor tensor, imgdnn_err_code * errcode_ret) {
	typedef const char * (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_tensor, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::TensorGetName);
	return _func_ptr(tensor, errcode_ret);
}

/**
* Find tensor by name.
*
* If multiple tensors in this network have the same name, then only one of them
* will be returned.
*
* @Input  network      Handle to the network.
* @Input  name         The name to look for, may not be NULL.
* @Output errcode_ret  IMGDNN_SUCCESS or IMGDNN_INVALID_VALUE for NULL name.
* @Return The tensor with the given name, or NULL if not found.
*/
inline imgdnn_tensor NetworkFindTensor(imgdnn_network network, const char * name, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, const char *, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkFindTensor);
	return _func_ptr(network, name, errcode_ret);
}

/**
* Set the name of the network.
*
* @Input network  The network to set name of.
* @Input network_name The name for the network
* @Return errcode_ret IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid network/network_name,
*                      or IMGDNN_OUT_OF_MEMORY
*/
inline imgdnn_err_code NetworkSetName(imgdnn_network network, const char * network_name) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, const char *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkSetName);
	return _func_ptr(network, network_name);
}

/**
* Get the name of the network.
*
* @Input network The network to get the name of.
* @Output errcode_ret IMGDNN_SUCCESS, or IMGDNN_INVALID_VALUE for invalid network.
* @Return The given name, NULL on failure
*/
inline const char * NetworkGetName(const imgdnn_network network, imgdnn_err_code * errcode_ret) {
	typedef const char * (*PROC_IMGDNN_FUNC_TYPEDEF)(const imgdnn_network, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkGetName);
	return _func_ptr(network, errcode_ret);
}

/**
*  Get the name of the network_object.
*
* @Input network_object The network object to get name of.
* @Output errcode_ret IMGDNN_SUCCESS, or IMGDNN_INVALID_VALUE for invalid network object.
* @Return The given name on success, NULL on failure
*/
inline const char * NetworkGetObjectName(const imgdnn_network_object network_object, imgdnn_err_code * errcode_ret) {
	typedef const char * (*PROC_IMGDNN_FUNC_TYPEDEF)(const imgdnn_network_object, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkGetObjectName);
	return _func_ptr(network_object, errcode_ret);
}

/**
* Find non-fixed input tensors of the network.
*
* These tensors can then be used as input tensors for the network object.
*
* @Input  network      Handle to the network.
* @Output num_inputs   Number of input tensors found.
* @Output errcode_ret  IMGDNN_SUCCESS or IMGDNN_FAILURE.
* @Return An array containing the non-fixed input tensors of the network.
*/
inline imgdnn_tensor * NetworkFindInputs(imgdnn_network network, unsigned * num_inputs, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor * (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, unsigned *, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkFindInputs);
	return _func_ptr(network, num_inputs, errcode_ret);
}

/**
* Find default output tensors of the network.
*
* Default output tensors are either:
* - Tensors with "output" attribute defined, if there is such a tensor in the network
*   this attribute is an index defining their position in the outputs vector.
* - The tensors which are not inputs of any other tensor, otherwise.
*
* These tensors can then be used as output tensors for the network object.
*
* @Input  network      Handle to the network.
* @Output num_outputs  Number of default output tensors found.
* @Output errcode_ret  IMGDNN_SUCCESS or IMGDNN_FAILURE.
* @Return A vector containing the default output tensors of the network.
*/
inline imgdnn_tensor * NetworkFindDefaultOutputs(imgdnn_network network, unsigned * num_outputs, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor * (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, unsigned *, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkFindDefaultOutputs);
	return _func_ptr(network, num_outputs, errcode_ret);
}

/**
* Add Reshape operation of a tensor to a different layout.
*
* @Input  network      Handle to the network.
* @Input  tensor       The input tensor to reshape. May not be NULL.
* @Input  descriptor   The tensor descriptor of the new tensor. Must be the
*                      same tensor type as the input tensor. May not be NULL.
*                      Can contain 0: Copy one dimension from input tensor descriptor
*                      Can contain -1 only once: Infer single dimension from
*                      the rest of this descriptor and the input tensor descriptor
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid descriptor.
*                      IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY.
* @Return The output reshaped tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkReshapeOp(imgdnn_network network, imgdnn_tensor tensor, const imgdnn_tensor_descriptor * const descriptor, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, const imgdnn_tensor_descriptor * const, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkReshapeOp);
	return _func_ptr(network, tensor, descriptor, errcode_ret);
}

/**
* Add Transpose dimensions operation of a tensor.
*
* e.g.
* tensor=[[1,2,3],[4,5,6]] order=[0,1] => [[1,2,3],[4,5,6]]
* tensor=[[1,2,3],[4,5,6]] order=[1,0] => [[1,4],[2,5],[3,6]]
* tensor=[[1],[1],[1],[1],[1]] order=[1,0] => [[1,1,1,1,1]]
* tensor=[[[1,2,3],[4,5,6]],[[1,2,3],[4,5,6]]] order=[1,2,0] => [[[1,1],[2,2],[3,3]],[[4,4],[5,5],[6,6]]]
*
* @Input  network      Handle to the network.
* @Input  tensor       The input tensor to transpose. May not be NULL.
* @Input  order        The new ordering of tensor dimensions. Must contain every
*                      dimension of the input tensor exactly once. Length is the
*                      same size as the input tensor dimensions.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid tensor or order,
*                      IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY.
* @Return The output transposed tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkTransposeOp(imgdnn_network network, imgdnn_tensor tensor, const int order[], imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, const int[], imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkTransposeOp);
	return _func_ptr(network, tensor, order, errcode_ret);
}

/**
* Add data-type cast operation of a tensor.
*
* @Input  network         Handle to the network.
* @Input  tensor          The input data to cast. May not be NULL.
* @Input  dst_type        imgdnn_type to cast to
* @Input  dst_quant_param quant params if the dst_type is quantized else NULL
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid tensor,
*                      IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY.
* @Return The output type casted tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkCastOp(imgdnn_network network, imgdnn_tensor tensor, imgdnn_type dst_type, const imgdnn_quant_param * const dst_quant_param, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_type, const imgdnn_quant_param * const, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkCastOp);
	return _func_ptr(network, tensor, dst_type, dst_quant_param, errcode_ret);
}

/**
* Add Broadcast operation of a tensor along a new dimension.
*
* e.g.
* tensor=[1], dim=0, size=2 => [[1], [1]]
* tensor=[1], dim=1, size=3 => [[1, 1, 1]]
* tensor=[[1,2],[3,4],[5,6]], dim=0, size=2 => [[[1,2],[3,4],[5,6]], [[1,2],[3,4],[5,6]]]
* tensor=[[1,2],[3,4],[5,6]], dim=2, size=2 => [[[1,1],[2,2]],[[3,3],[4,4]],[[5,5],[6,6]]]
* tensor=1, dim=0, size=5 => [1,1,1,1,1]
*
* @Input  network      Handle to the network.
* @Input  input        The input tensor to broadcast. May not be NULL.
* @Input  dimension    The dimension of the new broadcasted dimension in the
*                      resulting tensor. Must be <= the rank of the input tensor.
* @Input  size         The size of the newly broadcast dimension.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid tensor,
*                      IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY.
* @Return The output broadcasted tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkBroadcastOp(imgdnn_network network, imgdnn_tensor tensor, unsigned int dimension, size_t size, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, unsigned int, size_t, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkBroadcastOp);
	return _func_ptr(network, tensor, dimension, size, errcode_ret);
}

/**
* Obtain subset of a tensor in the network.
*
* start, end and stride must have the same length as number of dimensions in
* input tensor. Start and end are inclusive.
* @Input  network      Handle to the network.
* @Input  tensor       The input tensor. May not be NULL.
* @Input  start        The start of the sub-tensor in each dimension.
* @Input  end          The end of the sub-tensor in each dimension.
* @Input  stride       The stride of the sub-tensor in each dimension.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid tensor,
*                      IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY.
* @Return The output sub-tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkSubTensor(imgdnn_network network, imgdnn_tensor tensor, const size_t start[], const size_t end[], const size_t stride[], imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, const size_t[], const size_t[], const size_t[], imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkSubTensor);
	return _func_ptr(network, tensor, start, end, stride, errcode_ret);
}

/**
* Add Interleave Operation to interleave two tensors along a given dimension.
*
* Slice tensor2 into tensor1 along one dimension. Place initial element of
* tensor2 at start[] index of tensor1, shifting tensor1 values higher by one.
* Skip stride[] elements and place the next element of tensor2, etc. Extra
* elements are appended.
* Dimensions other than interleave dimension must have the same sizes.
* e.g.
* tensor1=[0,1,2,3,4,5,6,7,8], tensor2=[10,11,12,13], dim=0, start=4, stride=2 => [0,1,2,3,10,4,5,11,6,7,12,8,13]
* tensor1=[[0,1,2],[3,4,5]], tensor2=[[6,7],[8,9]], dim=1, start=2, stride=0 => [[0,1,6,7,2],[3,4,8,9,5]]
*
* @Input  network      Handle to the network.
* @Input  tensor1      First tensor. May not be NULL
* @Input  tensor2      Second tensor. Must have same number of dimensions as
*                      tensor1. May not be NULL
* @Input  dimension    The dimension to join along. Must be < the rank of the input tensors.
* @Input  start        The starting element to place tensor2
* @Input  stride       The stride of splicing in tensor2 (tensor1 elements between
*                      tensor2 elements).
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid tensor,
*                      IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY.
* @Return The output interleaved tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkInterleaveOp(imgdnn_network network, imgdnn_tensor tensor1, imgdnn_tensor tensor2, unsigned int dimension, size_t start, size_t stride, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, unsigned int, size_t, size_t, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkInterleaveOp);
	return _func_ptr(network, tensor1, tensor2, dimension, start, stride, errcode_ret);
}

/**
* Add Split Operation to evenly split a tensor along an axis in the network.
*
* @Input  network      Handle to the network.
* @Input  tensor       The input tensor. May not be NULL.
* @Input  dimension    The dimension to slice along. Must be < the rank of
*                      the input tensor.
* @Input  num_slices   The number of slices to be sliced. Must evenly divide
*                      dimension size. Must be > 0.
* @Output out_tensors  The multiple tensors along the requested dimension. Must
*                      have space for num_slices tensors.
* @Return IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid tensor,
*         IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY.
*/
inline imgdnn_err_code NetworkSplitOp(imgdnn_network network, imgdnn_tensor tensor, unsigned int dimension, unsigned int num_slices, imgdnn_tensor out_tensors[]) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, unsigned int, unsigned int, imgdnn_tensor[]);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkSplitOp);
	return _func_ptr(network, tensor, dimension, num_slices, out_tensors);
}

/**
* Add Concat Operation to concat multiple tensors along an axis in the network.
*
* @Input  network      Handle to the network.
* @Input  tensor       The input tensors. Array size of num_concats. May not be NULL.
*                      All tensors must have the same rank.
*                      The size of all the dimensions in the tensor except the dimension
*                      being concatenated must match.
* @Input  dimension    The dimension to concat along.
* @Input  num_concats  The number of concats along the requested dimension. Must be >= 2.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid tensor,
*                      IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY.
* @Return The concated tensor along the requested dimension and size or NULL on failure.
*/
inline imgdnn_tensor NetworkConcatOp(imgdnn_network network, const imgdnn_tensor tensors[], unsigned int dimension, unsigned int num_concats, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, const imgdnn_tensor[], unsigned int, unsigned int, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkConcatOp);
	return _func_ptr(network, tensors, dimension, num_concats, errcode_ret);
}

/**
* Add unary operation on a tensor
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input tensor to operate on
* @Input  operation    The operation to perform on the input tensor.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkUnaryOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_operation_unary operation, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_operation_unary, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkUnaryOp);
	return _func_ptr(network, in_tensor, operation, errcode_ret);
}

/**
* Add ReLU operation on a tensor
*
* output = min(max(input, min_clamp), max_clamp)  if input > 0
* 		  = min(max(input, min_clamp), max_clamp) * negative_slope  if input < 0
*
* @Input  network          Handle to the network.
* @Input  in_tensor        The input tensor to operate on
* @Input  has_min_clamp    Whether there is a minimal clamping value.
* @Input  min_clamp        The minimal clamping value.
* @Input  has_max_clamp    Whether there is a maximal clamping value.
* @Input  max_clamp        The maximal clamping value.
* @Input  negative_slope   Multiplier to apply to negative input values
* @Output errcode_ret      IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                          IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                          IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkReLUOp(imgdnn_network network, imgdnn_tensor in_tensor, bool has_min_clamp, float min_clamp, bool has_max_clamp, float max_clamp, float negative_slope, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, bool, float, bool, float, float, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkReLUOp);
	return _func_ptr(network, in_tensor, has_min_clamp, min_clamp, has_max_clamp, max_clamp, negative_slope, errcode_ret);
}

/**
* Add a binary operation on a set of tensors
* Tensor type must be the same for both arguments.
* The two tensors will be automatically broadcasted for element-wise operations.
* The Tensor dimensions will be matched from highest to lowest.
* A dimension size of 1 (or none) will be broadcasted to the size of the other tensor's dimension.
* e.g. tensor1 size of [1,7,3,1], tensor2 size of [2,1,1,3,4].
* tensor1+tensor2 size = [2,1,7,3,4].
* @Input  network      Handle to the network.
* @Input  in_tensor1   The LHS input tensor to operate on
* @Input  in_tensor2   The RHS input tensor to operate on
* @Input  operation    The operation to perform on the input tensors.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkBinaryOp(imgdnn_network network, imgdnn_tensor in_tensor1, imgdnn_tensor in_tensor2, imgdnn_operation_binary operation, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, imgdnn_operation_binary, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkBinaryOp);
	return _func_ptr(network, in_tensor1, in_tensor2, operation, errcode_ret);
}

/**
* Add a 2D convolution operation on a set of tensors.
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to convolve. 4D: [N, C, H, W]
* @Input  filter       The filter to use. 4D: [Co, Ci, Hf, Wf]
* @Input  stride       The stride in each dimension. 2D: [Hs, Ws]
* @Input  pad          The padding in each dimension. 2D: [Hp, Wp]
* @Input  dilation     The input dilation in each dimension. 2D: [Hd, Wd]
* @Input  with_partial Perform the last calculation in each dimension if it is
*                      not complete, ignoring out of bounds values.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkConvolution2dOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor filter, const unsigned int stride[2], const unsigned int pad[2], const unsigned int dilation[2], bool with_partial, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], bool, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkConvolution2dOp);
	return _func_ptr(network, in_tensor, filter, stride, pad, dilation, with_partial, errcode_ret);
}

/**
* Add a 2D convolution operation on a set of tensors.
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to convolve. 4D: [N, C, H, W]
* @Input  filter       The filter to use. 4D: [Co, Ci, Hf, Wf]
* @Input  stride       The stride in each dimension. 2D: [Hs, Ws]
* @Input  pad_to_begin The padding added before data in each dimension. 2D: [H, W]
* @Input  pad_to_end   The padding added after data in each dimension. 2D: [H, W]
* @Input  dilation     The input dilation in each dimension. 2D: [Hd, Wd]
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkConvolution2dOp_v2(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor filter, const unsigned int stride[2], const unsigned int pad_to_begin[2], const unsigned int pad_to_end[2], const unsigned int dilation[2], imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], const unsigned int[2], imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkConvolution2dOp_v2);
	return _func_ptr(network, in_tensor, filter, stride, pad_to_begin, pad_to_end, dilation, errcode_ret);
}

/**
* Add a grouped 2D convolution operation on a set of tensors.
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to convolve. 4D: [N, C, H, W]
* @Input  filter       The filter to use. 4D: [Co, Ci/groups, Hf, Wf]
* @Input  stride       The stride in each dimension. 2D: [Hs, Ws]
* @Input  pad_to_begin The padding added before data in each dimension. 2D: [H, W]
* @Input  pad_to_end   The padding added after data in each dimension. 2D: [H, W]
* @Input  dilation     The input dilation in each dimension. 2D: [Hd, Wd]
* @Input  groups     The grouping parameter of the convolution. 1 for a normal convolution.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkGroupedConvolution2dOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor filter, const unsigned int stride[2], const unsigned int pad_to_begin[2], const unsigned int pad_to_end[2], const unsigned int dilation[2], unsigned groups, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], const unsigned int[2], unsigned, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkGroupedConvolution2dOp);
	return _func_ptr(network, in_tensor, filter, stride, pad_to_begin, pad_to_end, dilation, groups, errcode_ret);
}

/**
* Add a 2D depthwise convolution operation on a set of tensors.
* out[n, c, h, w] = sum_{hf \in Hf, wf \in Wf} {
* in_tensor[n, c/(Co/Ci), h*Hs+hf*Hd-Hp, w*Ws+wf*Wd-Wp] *
* filter[c%(Co/Ci), c/(Co/Ci), hf, wf]
* }
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to convolve. 4D: [N, C, H, W]
* @Input  filter       The filter to use. 4D: [Co/Ci, Ci, Hf, Wf]
*                      Must be the same type as in_tensor.
*                      Number of channels must be the same as in in_tensor.
*                      Size must be > 0.
* @Input  stride       The stride in each dimension. 2D: [Hs, Ws]
*                      Must be > 0 in both dimensions.
* @Input  pad          The padding in each dimension. 2D: [Hp, Wp]
* @Input  dilation     The input dilation in each dimension. 2D: [Hd, Wd]
*                      Must be > 0 in both dimensions.
* @Input  with_partial Perform the last calculation in each dimension if it is
*                      not complete, ignoring out of bounds values.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkDepthConvolution2dOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor filter, const unsigned int stride[2], const unsigned int pad[2], const unsigned int dilation[2], bool with_partial, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], bool, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkDepthConvolution2dOp);
	return _func_ptr(network, in_tensor, filter, stride, pad, dilation, with_partial, errcode_ret);
}

/**
* Add a 2D depthwise convolution operation on a set of tensors.
* out[n, c, h, w] = sum_{hf \in Hf, wf \in Wf} {
* in_tensor[n, c/(Co/Ci), h*Hs+hf*Hd-Hp, w*Ws+wf*Wd-Wp] *
* filter[c%(Co/Ci), c/(Co/Ci), hf, wf]
* }
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to convolve. 4D: [N, C, H, W]
* @Input  filter       The filter to use. 4D: [Co/Ci, Ci, Hf, Wf]
*                      Must be the same type as in_tensor.
*                      Number of channels must be the same as in in_tensor.
*                      Size must be > 0.
* @Input  stride       The stride in each dimension. 2D: [Hs, Ws]
*                      Must be > 0 in both dimensions.
* @Input  pad_to_begin The padding added before data in each dimension. 2D: [H, W]
* @Input  pad_to_end   The padding added after data in each dimension. 2D: [H, W]
* @Input  dilation     The input dilation in each dimension. 2D: [Hd, Wd]
*                      Must be > 0 in both dimensions.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkDepthConvolution2dOp_v2(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor filter, const unsigned int stride[2], const unsigned int pad_to_begin[2], const unsigned int pad_to_end[2], const unsigned int dilation[2], imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], const unsigned int[2], imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkDepthConvolution2dOp_v2);
	return _func_ptr(network, in_tensor, filter, stride, pad_to_begin, pad_to_end, dilation, errcode_ret);
}

/**
* Add a 2D deconvolution operation.
* out_h = stride * (input_h - 1) + filter_h - pad_to_begin[0] - pad_to_end[0];
* out_w = stride * (input_w - 1) + filter_w - pad_to_begin[1] - pad_to_end[1];
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to deconvolve. 4D: [N, C, H, W]
* @Input  filter       The filter to use. 4D: [Co, Ci, Hf, Wf]
*                      Number of input channels must match number of channels in in_tensor.
*                      Filter width and height must be > 0.
* @Input  stride       The stride in each dimension. 2D: [Hs, Ws]
*                      Must be > 0 in both dimensions.
* @Input  pad_to_begin The padding added before data in each dimension. 2D: [H, W]
* @Input  pad_to_end   The padding added after data in each dimension. 2D: [H, W]
* @Input  dilation     The input dilation in each dimension. 2D: [Hd, Wd]
*                      Must be > 0 in both dimensions.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkDeconvolution2dOp_v2(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor filter, const unsigned int stride[2], const unsigned int pad_to_begin[2], const unsigned int pad_to_end[2], const unsigned int dilation[2], imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], const unsigned int[2], imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkDeconvolution2dOp_v2);
	return _func_ptr(network, in_tensor, filter, stride, pad_to_begin, pad_to_end, dilation, errcode_ret);
}

/**
* Add a 2D deconvolution operation.
* output_size = ((in_size - 1) * stride) - 2*pad + filter_size
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to deconvolve. 4D: [N, C, H, W]
* @Input  filter       The filter to use. 4D: [Co, Ci, Hf, Wf]
*                      Number of input channels must match number of channels in in_tensor.
*                      Filter width and height must be > 0.
* @Input  stride       The stride in each dimension. 2D: [Hs, Ws]
*                      Must be > 0 in both dimensions.
* @Input  pad          The padding in each dimension. 2D: [Hp, Wp]
* @Input  dilation     The input dilation in each dimension. 2D: [Hd, Wd]
*                      Must be > 0 in both dimensions.
* @Input  partial_size The extra padding to add to the right and bottom of the
*                      transposed convolution.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkDeconvolution2dOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor filter, const unsigned int stride[2], const unsigned int pad[2], const unsigned int dilation[2], const unsigned int partial_size[2], imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], const unsigned int[2], imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkDeconvolution2dOp);
	return _func_ptr(network, in_tensor, filter, stride, pad, dilation, partial_size, errcode_ret);
}

/**
* Add a grouped 2D deconvolution operation on a set of tensors.
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to deconvolve. 4D: [N, C, H, W]
* @Input  filter       The filter to use. 4D: [Co/groups, Ci, Hf, Wf]
*                      Number of input channels must match number of channels in in_tensor.
*                      Filter width and height must be > 0.
* @Input  stride       The stride in each dimension. 2D: [Hs, Ws]
*                      Filter width and height must be > 0.
* @Input  pad_to_begin The padding added before data in each dimension. 2D: [H, W]
* @Input  pad_to_end   The padding added after data in each dimension. 2D: [H, W]
* @Input  dilation     The input dilation in each dimension. 2D: [Hd, Wd]
*                      Filter width and height must be > 0.
* @Input  groups       The grouping parameter of the convolution. 1 for a normal convolution.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkGroupedDeconvolution2dOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor filter, const unsigned int stride[2], const unsigned int pad_to_begin[2], const unsigned int pad_to_end[2], const unsigned int dilation[2], unsigned groups, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], const unsigned int[2], unsigned, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkGroupedDeconvolution2dOp);
	return _func_ptr(network, in_tensor, filter, stride, pad_to_begin, pad_to_end, dilation, groups, errcode_ret);
}

/**
* Add 2D pooling operation on a tensor.
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to pool. 4D: [N, C, H, W]
* @Input  size         The size of pooling window for the 2 end dimensions. Must be > 0.
* @Input  stride       The pooling stride in each pooled dimension. Must be > 0.
* @Input  pad          The padding in each pooled dimension.
* @Input  type         The type of pooling to perform.
* @Input  with_partial To do pooling with or without partial data.
*                      If true, all values are used and
*                      output_size = ceil((input_size + 2 * pad_size - pool_size) / stride) + 1.
*                      If false, not all values used and
*                      output_size = floor((input_size + 2 * pad_size - pool_size) / stride) + 1.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkPooling2dOp(imgdnn_network network, imgdnn_tensor in_tensor, const unsigned int size[2], const unsigned int stride[2], const unsigned int pad[2], imgdnn_pooling_type type, bool with_partial, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], imgdnn_pooling_type, bool, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkPooling2dOp);
	return _func_ptr(network, in_tensor, size, stride, pad, type, with_partial, errcode_ret);
}

/**
* Add 2D pooling operation on a tensor.
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to pool. 4D: [N, C, H, W]
* @Input  size         The size of pooling window for the 2 end dimensions. Must be > 0.
* @Input  stride       The pooling stride in each pooled dimension. Must be > 0.
* @Input  pad_to_begin The padding added before data in each dimension. 2D: [H, W]
* @Input  pad_to_end   The padding added after data in each dimension. 2D: [H, W]
* @Input  type         The type of pooling to perform.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkPooling2dOp_v2(imgdnn_network network, imgdnn_tensor in_tensor, const unsigned int size[2], const unsigned int stride[2], const unsigned int pad_to_begin[2], const unsigned int pad_to_end[2], imgdnn_pooling_type type, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], const unsigned int[2], imgdnn_pooling_type, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkPooling2dOp_v2);
	return _func_ptr(network, in_tensor, size, stride, pad_to_begin, pad_to_end, type, errcode_ret);
}

/**
* Add 2D pooling operation on a tensor.
*
* @Input  network            Handle to the network.
* @Input  in_tensor          The input data to pool. 4D: [N, C, H, W]
* @Input  size               The size of pooling window for the 2 end dimensions. Must be > 0.
* @Input  stride             The pooling stride in each pooled dimension. Must be > 0.
* @Input  pad_to_begin       The padding added before data in each dimension. 2D: [H, W]
* @Input  pad_to_end         The padding added after data in each dimension. 2D: [H, W]
* @Input  type               The type of pooling to perform.
* @Input  count_include_pad  If type == IMGDNN_POOLING_AVERAGE
*                            Whether pooling calculation should take padding value into account
*                            Ignored for other types.
* @Output errcode_ret        IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                            IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                            IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkPooling2dOp_v3(imgdnn_network network, imgdnn_tensor in_tensor, const unsigned int size[2], const unsigned int stride[2], const unsigned int pad_to_begin[2], const unsigned int pad_to_end[2], imgdnn_pooling_type type, bool count_include_pad, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, const unsigned int[2], const unsigned int[2], const unsigned int[2], const unsigned int[2], imgdnn_pooling_type, bool, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkPooling2dOp_v3);
	return _func_ptr(network, in_tensor, size, stride, pad_to_begin, pad_to_end, type, count_include_pad, errcode_ret);
}

/**
* Add local response normalisation operation on a tensor.
*
* pre_pad = floor((window_size - 1) / 2)
* post_pad = window_size - pre_pad - 1
*
* - Python array slicing is non-inclusive on the right.
* - Values outside bounds of array are 0.
*
* Across:
* a = alpha / window_size
* sum_squares[n, c, h, w] = sum(input[n, c - pre_pad : c + post_pad + 1, h, w] ^ 2)
* output[n, c, h, w] = input[n, c, h, w] / ((k + a * sum_squares[n, c, h, w]) ^ beta)
*
* Within:
* a = alpha / (window_size ^ 2)
* sum_squares[n, c, h, w] = sum(input[n, c, h - pre_pad : h + post_pad + 1, w - pre_pad : w + post_pad + 1] ^ 2)
* output[n, c, h, w] = input[n, c, h, w] / ((k + a * sum_squares[n, c, h, w]) ^ beta)
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to normalise. 4D: [N, C, H, W]
* @Input  type         Across/within channel
* @Input  window_size  Number of channels to sum over (for cross channel) or
*                      the side length of the square region to sum over (for
*                      within channel). Must be > 0.
* @Input  k            k value
* @Input  alpha        Scaling Parameter
* @Input  beta         Exponent
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkLrnOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_lrn_type type, size_t window_size, float k, float alpha, float beta, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_lrn_type, size_t, float, float, float, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkLrnOp);
	return _func_ptr(network, in_tensor, type, window_size, k, alpha, beta, errcode_ret);
}

/**
* Add projective image transform.
*
* output[n, c, h, w] = filter(input[n, c, (b0*x + b1*y + b2) / k, (a0*x + a1*y + a2) / k]),
* where k = (c0*x + c1*y + 1)
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input data to transform. 4D: [N, C, H, W]
* @Input  transform    The parameters of the transform. 2-dimensions: (N, 8)
*                      transform[n] = (a0, a1, a2, b0, b1, b2, c0, c1)
*                      Must be the same type as in_tensor.
* @Input  type         The filtering type.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkImageTransformOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor transform, imgdnn_image_transform_type type, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, imgdnn_image_transform_type, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkImageTransformOp);
	return _func_ptr(network, in_tensor, transform, type, errcode_ret);
}

/**
* Dimension reduction
*
* @Input  network  Handle to the network.
* @Input  in_tensor  The input data to reduce.
* @Input  type  The type of reduction to perform.
* @Input  axis  List of axis to reduce. The dimension of the input tensor
*               is reduced by 1 for each entry in this list. Each specified axis
*               must be in the range [0, dimensions - 1].
* @Input  num_axis  number of items in the axis parameter.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkReduceOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_reduce_type type, const int axis[], size_t num_axis, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_reduce_type, const int[], size_t, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkReduceOp);
	return _func_ptr(network, in_tensor, type, axis, num_axis, errcode_ret);
}

/**
* Compute Softmax Activation
*
* softmax[i] = exp((input[i] - max(input, axis)) * beta)/sum_{j}{exp((input[j] - max(input, axis)) * beta), axis}
* @Input network		Handle to the network
* @Input in_tensor 	input data
* @Input beta			beta in calculation
* @Input axis  		the axis to sum over when computing softmax
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_VALUE for invalid argument (e.g axis
*                      outside of input tensor dimensions)
*                      IMGDNN_OUT_OF_MEMORY
* @Return The output tensor with same shape as input, or NULL on failure
*/
inline imgdnn_tensor NetworkSoftmaxOp(imgdnn_network network, imgdnn_tensor in_tensor, float beta, unsigned int axis, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, float, unsigned int, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkSoftmaxOp);
	return _func_ptr(network, in_tensor, beta, axis, errcode_ret);
}

/**
* LSTM Cell
*
* @Input  network  Handle to the network.
* @Input  in_tensor        The input data. Size [batch_size, input_size].
* @Input  prev_tensor      The cell output from previous iteration.
*                          Size [batch_size, output_size].
*                          Must be the same type as in_tensor.
* @Input  state_tensor     The cell state. Size [batch_size, num_units].
*                          Must be the same type as in_tensor.
* @Input  weights          Structure of all weight tensors.
* @Input  state_clip       Value to clip the resultant state.
* @Input  projection_clip  Value to clip the output projection.
* @Output errcode_ret      IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                          IMGDNN_INVALID_OPERATION, IMGDNN_OUT_OF_MEMORY or
*                          IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return Structure of output and state tensors. Tensors will be NULL on failure.
*/
inline imgdnn_lstm_output_tensors NetworkLSTMOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor prev_tensor, imgdnn_tensor state_tensor, imgdnn_lstm_weight_tensors * weights, float state_clip, float projection_clip, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_lstm_output_tensors (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, imgdnn_tensor, imgdnn_lstm_weight_tensors *, float, float, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkLSTMOp);
	return _func_ptr(network, in_tensor, prev_tensor, state_tensor, weights, state_clip, projection_clip, errcode_ret);
}

/**
* Rearrange the data from depth into blocks of spatial data
*
* @Input network Handle to the network.
* @Input in_tensor    The input data for depth to space operation. 4D: [N, C, H, W]
*                     Number of channels should be divisible by block size squared.
* @Input block_size   It indicates the input block size and how data is moved.
*                     Must be >= 2.
* @Output errcode_ret IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                     IMGDNN_OUT_OF_MEMORY or IMGDNN_INVALID_VALUE
*                     for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkDepthToSpaceOp(imgdnn_network network, imgdnn_tensor in_tensor, size_t block_size, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, size_t, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkDepthToSpaceOp);
	return _func_ptr(network, in_tensor, block_size, errcode_ret);
}

/**
* Add Batch to Space operation on a tensor
* @Input network      Handle to the network.
* @Input in_tensor    The input data for batch to space operation. 4D: [N, C, H, W]
* @Input block_size   The input block_size for each spatial dimension.
*                     Right now we support two spatial dimensions Height(H) and Width(W).
*                     1D tensor, block_size[2] = [block_size_H, block_size_W].
* @Output errcode_ret IMGDNN_SUCCESS, IMGDNN_FAILURE or IMGDNN_INVALID_VALUE
*                     for invalid tensor or argument
* @Return  The output tensor or NULL on failure
*/
inline imgdnn_tensor NetworkBatchToSpaceNDOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor block_size, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkBatchToSpaceNDOp);
	return _func_ptr(network, in_tensor, block_size, errcode_ret);
}

/**
* Gather slices of the input tensor along axis based on indices.
*
* @Input network       Handle to the network
* @Input in_tensor     The tensor from which to gather values.
*                      Rank must be at least 1.
* @Input indices       The tensor of indices used.
*                      Rank must be at least 1 and of signed or unsigned integer type.
* @Input axis          The axis in in_tensor to gather indices from.
*                      It must be of signed or unsigned integer type.
*                      It must be less than rank of in_tensor.
* @Output errcode_ret  IMGDNN_FAILURE, IMGDNN_SUCCESS or IMGDNN_INVALID_VALUE
*                      for invalid tensor or argument.
* @Return              The ouput tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkGatherOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor indices, unsigned int axis, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, unsigned int, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkGatherOp);
	return _func_ptr(network, in_tensor, indices, axis, errcode_ret);
}

/**
* Add Space to Depth operation on a tensor
*
* @Input  network      Handle to the network.
* @Input  in_tensor    The input tensor to operate on. 4D: [N, C, H, W]
*                      Width and height must be divisible by block_size.
* @Input  block_size   The size of the spatial block.
*                      Must be >= 2.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_OUT_OF_MEMORY or IMGDNN_INVALID_VALUE for
*                      invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkSpaceToDepthOp(imgdnn_network network, imgdnn_tensor in_tensor, size_t block_size, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, size_t, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkSpaceToDepthOp);
	return _func_ptr(network, in_tensor, block_size, errcode_ret);
}

/**
* Add Space to Batch operation on a tensor
*
* @Input network      Handle to the network.
* @Input in_tensor    The input tensor to operate on 4D: [N, C, H, W]
* @Input in_padding   The input padding 2D tensor for each spatial dimension.
*                     Right now we support only two spatial dimension[H, W].
*                     in_padding[2][2] = [padding_start_H, padding_end_H,
*                                         padding_start_W, padding_end_W].
* @Input block_size    The input block_size 1D tensor.
*                      block_size[2] = [block_size_H, block_size_W]
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_VALUE for
*                      invalid tensor or argument.
* @Return               The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkSpaceToBatchNDOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor in_padding, imgdnn_tensor block_size, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, imgdnn_tensor, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkSpaceToBatchNDOp);
	return _func_ptr(network, in_tensor, in_padding, block_size, errcode_ret);
}

/**
* Add Resize Image using Bilinear interpolation.
*
* @Input  network        Handle to the network.
* @Input  in_tensor      The 4D input tensor to resize, in format [N, C, H, W].
* @Input  height         New height of the output tensor.  Must be > 0.
* @Input  width          New width of the output tensor.  Must be > 0.
* @Input  align_corners  If True, the centers of the 4 corner pixels of the
*                        input and output tensors are aligned, preserving
*                        the values at the corner pixels.
* @Output errcode_ret    IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                        IMGDNN_OUT_OF_MEMORY or IMGDNN_INVALID_VALUE
*                        for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkResizeBilinearOp(imgdnn_network network, imgdnn_tensor in_tensor, unsigned int height, unsigned int width, bool align_corners, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, unsigned int, unsigned int, bool, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkResizeBilinearOp);
	return _func_ptr(network, in_tensor, height, width, align_corners, errcode_ret);
}

/**
* Pad tensor
* @Input network		Handle to network
* @Input in_tensor		tensor to be padded
* @Input pad_before	value at idx N correspond to the number of values to add before
* 						tensor contents in dimensions N. The length of this array must be
* 						equal to the input tensor's number of dimensions.
* @Input pad_after		value at idx N correspond to the number of values to add after
* 						tensor contents in dimensions N. The length of this array must be
* 						equal to the input tensor's number of dimensions.
* @Input pad_value		constant pad value.  This float value is converted to the type of
* 						in_tensor when used for padding
* @Output errcode_ret	IMG_SUCCESS,
*                      CL_INVALID_VALUE for invalid tensor or other arguments,
*                      IMGDNN_OUT_OF_MEMORY. May be NULL.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkPadOp(imgdnn_network network, imgdnn_tensor in_tensor, const unsigned int pad_before[], const unsigned int pad_after[], float pad_value, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, const unsigned int[], const unsigned int[], float, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkPadOp);
	return _func_ptr(network, in_tensor, pad_before, pad_after, pad_value, errcode_ret);
}

/**
* Pad tensor
* @Input network     Handle to network
* @Input in_tensor   tensor to be padded
* @Input pad_before  value at idx N correspond to the number of values to add before
*            tensor contents in dimensions N. The length of this array must be
*            equal to the input tensor's number of dimensions.
* @Input pad_after   value at idx N correspond to the number of values to add after
*            tensor contents in dimensions N. The length of this array must be
*            equal to the input tensor's number of dimensions.
* @Input pad_value   constant pad value.  This float value is converted to the type of
*            in_tensor when used for padding
* @Input pad_mode    Padding mode (constant, symmetric, or reflect)
* @Output errcode_ret  IMG_SUCCESS,
*                      CL_INVALID_VALUE for invalid tensor or other arguments,
*                      IMGDNN_OUT_OF_MEMORY. May be NULL.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkPadOp_v2(imgdnn_network network, imgdnn_tensor in_tensor, const unsigned int pad_before[], const unsigned int pad_after[], float pad_value, imgdnn_pad_mode pad_mode, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, const unsigned int[], const unsigned int[], float, imgdnn_pad_mode, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkPadOp_v2);
	return _func_ptr(network, in_tensor, pad_before, pad_after, pad_value, pad_mode, errcode_ret);
}

/**
* Add Resize Image using Nearest Neighbour interpolation.
*
* @Input  network        Handle to the network.
* @Input  in_tensor      The 4D input tensor to resize, in format [N, C, H, W].
* @Input  height         New height of the output tensor.  Must be > 0.
* @Input  width          New width of the output tensor.  Must be > 0.
* @Input  align_corners  If True, the centers of the 4 corner pixels of the
*                        input and output tensors are aligned, preserving
*                        the values at the corner pixels.
* @Output errcode_ret    IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                        IMGDNN_OUT_OF_MEMORY or IMGDNN_INVALID_VALUE
*                        for invalid tensor or argument.
* @Return The output tensor or NULL on failure.
*/
inline imgdnn_tensor NetworkResizeNearestNeighbourOp(imgdnn_network network, imgdnn_tensor in_tensor, unsigned int height, unsigned int width, bool align_corners, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, unsigned int, unsigned int, bool, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkResizeNearestNeighbourOp);
	return _func_ptr(network, in_tensor, height, width, align_corners, errcode_ret);
}

/**
* Add ROI Pooling.
*
* @Input  network		Handle to the network.
* @input  in_tensor		A 4D input feature map, in format [N, C, H, W].
* @input  rois_tensor		A 2D input tensor, in format [num_rois, 4],
*				specifying the locations of Region of Interests in the form {x1, y1, x2, y2}.
*				An ROI is represented by its upper-left coordinate (x1, y1) and
*				lower-right coordinate (x2, y2) in the original image.
*				A spatial scaling factor is applied to map into feature map coordinate.
*				A valid region of interest should satisfy x1 <= x2 and y1 <= y2.
*
* @input  batch_idx		A 1D input tensor specifying batch indices of each box (ROI).
* @input  out_height		A scalar, the height of the output tensor.
* @input  out_width		A scalar, the width of the output tensor.
* @input  scaled_height	A scalar, specifying the ratio from the height of original image
*				to the height of feature map.
* @input  scaled_width		A scalar, specifying the ratio from the width of original image
*				to the width of feature map.
*
* @Output errcode_ret    	IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_OUT_OF_MEMORY
*                        	or IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor in format [num_rois, C, out_height, out_width] or NULL on failure.
*/
inline imgdnn_tensor NetworkROIPoolingOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor roi_tensor, imgdnn_tensor batch_idx_tensor, unsigned int out_height, unsigned int out_width, float scaled_height, float scaled_width, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, imgdnn_tensor, unsigned int, unsigned int, float, float, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkROIPoolingOp);
	return _func_ptr(network, in_tensor, roi_tensor, batch_idx_tensor, out_height, out_width, scaled_height, scaled_width, errcode_ret);
}

/**
* Add ROI Align.
*
* @Input  network		Handle to the network.
* @input  in_tensor		A 4D input feature map, in format [N, C, H, W].
* @input  rois_tensor		A 2D input tensor, in format [num_rois, 4],
*				specifying the locations of Region of Interests in the form {x1, y1, x2, y2}.
*				An ROI is represented by its upper-left coordinate (x1, y1) and
*				lower-right coordinate (x2, y2) in the original image.
*				A spatial scaling factor is applied to map into feature map coordinate.
*				A valid region of interest should satisfy x1 <= x2 and y1 <= y2.
*
* @input  batch_idx		A 1D input tensor specifying batch indices of each box (ROI).
* @input  out_height		A scalar, the height of the output tensor.
* @input  out_width		A scalar, the width of the output tensor.
* @input  scaled_height	A scalar, specifying the ratio from the height of original image
*				to the height of feature map.
* @input  scaled_widtht	A scalar, specifying the ratio from the width of original image
*				to the width of feature map.
* @input  num_samples_height	A scalar, the number of sampling points in height dimension used to compute the output.
*				Set to 0 for adaptive value of ceil(roi_height/out_height).
* @input  num_samples_width	A scalar, the number of sampling points in width dimension used to compute the output.
*				Set to 0 for adaptive value of ceil(roi_width/out_width).
*
* @Output errcode_ret    	IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_OUT_OF_MEMORY
*				or IMGDNN_INVALID_VALUE for invalid tensor or argument.
* @Return The output tensor in format [num_rois, C, out_height, out_width] or NULL on failure.
*/
inline imgdnn_tensor NetworkROIAlignOp(imgdnn_network network, imgdnn_tensor in_tensor, imgdnn_tensor roi_tensor, imgdnn_tensor batch_idx_tensor, unsigned int out_height, unsigned int out_width, float scaled_height, float scaled_width, unsigned int num_samples_height, unsigned int num_samples_width, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network, imgdnn_tensor, imgdnn_tensor, imgdnn_tensor, unsigned int, unsigned int, float, float, unsigned int, unsigned int, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkROIAlignOp);
	return _func_ptr(network, in_tensor, roi_tensor, batch_idx_tensor, out_height, out_width, scaled_height, scaled_width, num_samples_height, num_samples_width, errcode_ret);
}

/**
* Utility function to obtain the descriptor of an input.
*
* If the tensor is of type IMGDNN_TYPE_QPA_* the caller is resposible of destroying the
* imgdnn_per_axis_quant_param structure (ret.quant_param->per_axis) by calling
* imgdnnDestroyPerAxisQuantParam
*
* @Input  input        An input previously created.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid input.
* @Return The descriptor of the given input.
*/
inline imgdnn_tensor_descriptor GetInputDescriptor(imgdnn_input input, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor_descriptor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_input, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::GetInputDescriptor);
	return _func_ptr(input, errcode_ret);
}

/**
* Utility function to obtain the descriptor of an output.
*
* If the tensor is of type IMGDNN_TYPE_QPA_* the caller is resposible of destroying the
* imgdnn_per_axis_quant_param structure (ret.quant_param->per_axis) by calling
* imgdnnDestroyPerAxisQuantParam
*
* @Input  output       An output previously created.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid output.
* @Return The descriptor of the given output.
*/
inline imgdnn_tensor_descriptor GetOutputDescriptor(imgdnn_output output, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_tensor_descriptor (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_output, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::GetOutputDescriptor);
	return _func_ptr(output, errcode_ret);
}

/**
* Fill the data layout parameters structure based on the specified pre-defined layout.
*
* @Input  data_layout   Pointer to the data layout structure to be filled.
* @Input  predef_layout Pre-defined data layout to be set for this tensor.
* @Input  tensor_desc   Tensor descriptor to check that the predef layout is compatible with the tensor.
* @Return IMGDNN_SUCCESS or IMGDNN_INVALID_VALUE for invalid tensor or invalid pre-defined data layout.
*/
inline imgdnn_err_code FillDataLayoutParameters(imgdnn_data_layout_param * data_layout, imgdnn_predefined_data_layout predef_layout, const imgdnn_tensor_descriptor * const desc) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_data_layout_param *, imgdnn_predefined_data_layout, const imgdnn_tensor_descriptor * const);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::FillDataLayoutParameters);
	return _func_ptr(data_layout, predef_layout, desc);
}

/**
* Get the requested parameter from the specified input
*
* @Input  input     The input from which to retrieve the parameter.
* @Input  parameter The requested parameter.
* @Output out       Pointer to the memory to fill with parameter value.
*/
inline imgdnn_err_code GetInputTensorParameter(imgdnn_input input, imgdnn_data_layout_param_type type, void * out) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_input, imgdnn_data_layout_param_type, void *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::GetInputTensorParameter);
	return _func_ptr(input, type, out);
}

/**
* Get the requested parameter from the specified output
*
* @Input  output     The output from which to retrieve the parameter.
* @Input  parameter The requested parameter.
* @Output out       Pointer to the memory to fill with parameter value.
*/
inline imgdnn_err_code GetOutputTensorParameter(imgdnn_output output, imgdnn_data_layout_param_type type, void * out) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_output, imgdnn_data_layout_param_type, void *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::GetOutputTensorParameter);
	return _func_ptr(output, type, out);
}

/**
* Produces a network object used for execution of network.
*
* The difference between using multiple outputs here, or multiple calls with
* single outputs is the network optimisation. A tensor that is not an output
* may be elided completely.
*
* @Input  device         The IMGDNN device for which to create this network.
* @Input  context        The IMGDNN context in which to create this network output.
* @Input  network        Network object for which object is being created.
* @Input  num_inputs     The number of elements in the input tensor array.
* @Input  inputs         Array of tensors to use as network inputs.
* @Input  inputs_layout  Array of data layout parameters for the specified input tensors.
* @Input  num_outputs    The number of elements in the output tensor array.
* @Input  outputs        Array of tensors to create network outputs from.
* @Input  outputs_layout Array of data layout parameters for the specified output tensors.
* @Input  flags          Flags specifying the options to create the network object.
* @Input  options        String of options to pass to the graph compiler,
*                        device dependent.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument or
*                      IMGDNN_INVALID_DEVICE for invalid device.
* @Return The output network object node for execution or NULL on failure.
*/
inline imgdnn_network_object CreateNetworkObject_v2(const imgdnn_device device, const imgdnn_context context, const imgdnn_network network, unsigned int num_inputs, const imgdnn_tensor inputs[], const imgdnn_data_layout_param inputs_layout[], unsigned int num_outputs, const imgdnn_tensor outputs[], const imgdnn_data_layout_param outputs_layout[], const imgdnn_network_object_flags flags, const char * options, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_network_object (*PROC_IMGDNN_FUNC_TYPEDEF)(const imgdnn_device, const imgdnn_context, const imgdnn_network, unsigned int, const imgdnn_tensor[], const imgdnn_data_layout_param[], unsigned int, const imgdnn_tensor[], const imgdnn_data_layout_param[], const imgdnn_network_object_flags, const char *, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::CreateNetworkObject_v2);
	return _func_ptr(device, context, network, num_inputs, inputs, inputs_layout, num_outputs, outputs, outputs_layout, flags, options, errcode_ret);
}

/**
* Produces a network object used for execution of network.
*
* The difference between using multiple outputs here, or multiple calls with
* single outputs is the network optimisation. A tensor that is not an output
* may be elided completely.
*
* @Input  device         The IMGDNN device for which to create this network.
* @Input  context        The IMGDNN context in which to create this network output.
* @Input  network        Network object for which object is being created.
* @Input  num_inputs     The number of elements in the input tensor array.
* @Input  inputs         Array of tensors to use as network inputs.
* @Input  num_outputs    The number of elements in the output tensor array.
* @Input  outputs        Array of tensors to create network outputs from.
* @Input  flags          Flags specifying the options to create the network object.
* @Input  options        String of options to pass to the graph compiler,
*                        device dependent.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument or
*                      IMGDNN_INVALID_DEVICE for invalid device.
* @Return The output network object node for execution or NULL on failure.
*/
inline imgdnn_network_object CreateNetworkObject(const imgdnn_device device, const imgdnn_context context, const imgdnn_network network, unsigned int num_inputs, const imgdnn_tensor inputs[], unsigned int num_outputs, const imgdnn_tensor outputs[], const imgdnn_network_object_flags flags, const char * options, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_network_object (*PROC_IMGDNN_FUNC_TYPEDEF)(const imgdnn_device, const imgdnn_context, const imgdnn_network, unsigned int, const imgdnn_tensor[], unsigned int, const imgdnn_tensor[], const imgdnn_network_object_flags, const char *, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::CreateNetworkObject);
	return _func_ptr(device, context, network, num_inputs, inputs, num_outputs, outputs, flags, options, errcode_ret);
}

/**
* Produces a network binary used for storing to a file or loading for
* execution.
*
* The call to imgdnnNetworkBinaryDestroy() is needed to deallocate the memory
*
* @Input  device         The IMGDNN device for which to create this network.
* @Input  context        The IMGDNN context in which to create this network output.
* @Input  network        Network object for which object is being created.
* @Input  num_inputs     The number of elements in the input tensor array.
* @Input  inputs         Array of tensors to use as network inputs.
* @Input  inputs_layout  Array of data layout parameters for the specified input tensors.
* @Input  num_outputs    The number of elements in the output tensor array.
* @Input  outputs        Array of tensors to create network outputs from.
* @Input  outputs_layout Array of data layout parameters for the specified output tensors.
* @Input  flags          Flags specifying the options to create the network object.
* @Input  options        String of options to pass to the graph compiler,
*                        device dependent.
* @Output errcode_ret    IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_OUT_OF_MEMORY or
*                        IMGDNN_INVALID_VALUE for invalid tensor or argument or
*                        IMGDNN_INVALID_DEVICE for invalid device.
* @Return The output network binary. Returns {0, NULL} on failure.
*/
inline imgdnn_network_binary CreateNetworkBinary_v2(const imgdnn_device device, const imgdnn_context context, const imgdnn_network network, unsigned int num_inputs, const imgdnn_tensor inputs[], const imgdnn_data_layout_param inputs_layout[], unsigned int num_outputs, const imgdnn_tensor outputs[], const imgdnn_data_layout_param outputs_layout[], const imgdnn_network_object_flags flags, const char * options, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_network_binary (*PROC_IMGDNN_FUNC_TYPEDEF)(const imgdnn_device, const imgdnn_context, const imgdnn_network, unsigned int, const imgdnn_tensor[], const imgdnn_data_layout_param[], unsigned int, const imgdnn_tensor[], const imgdnn_data_layout_param[], const imgdnn_network_object_flags, const char *, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::CreateNetworkBinary_v2);
	return _func_ptr(device, context, network, num_inputs, inputs, inputs_layout, num_outputs, outputs, outputs_layout, flags, options, errcode_ret);
}

/**
* Produces a network binary used for storing to a file or loading for
* execution.
*
* The call to imgdnnNetworkBinaryDestroy() is needed to deallocate the memory
*
* @Input  device       The IMGDNN device for which to create this network.
* @Input  context      The IMGDNN context in which to create this network output.
* @Input  network      Network object for which object is being created.
* @Input  num_inputs   The number of elements in the input tensor array.
* @Input  inputs       Array of tensors to use as network inputs.
* @Input  num_outputs  The number of elements in the output tensor array.
* @Input  outputs      Array of tensors to create network outputs from.
* @Input  flags        Flags specifying the options to create the network object.
* @Input  options      String of options to pass to the graph compiler,
*                      device dependent.
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_OUT_OF_MEMORY or
*                      IMGDNN_INVALID_VALUE for invalid tensor or argument or
*                      IMGDNN_INVALID_DEVICE for invalid device.
* @Return The output network binary. Returns {0, NULL} on failure.
*/
inline imgdnn_network_binary CreateNetworkBinary(const imgdnn_device device, const imgdnn_context context, const imgdnn_network network, unsigned int num_inputs, const imgdnn_tensor inputs[], unsigned int num_outputs, const imgdnn_tensor outputs[], const imgdnn_network_object_flags flags, const char * options, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_network_binary (*PROC_IMGDNN_FUNC_TYPEDEF)(const imgdnn_device, const imgdnn_context, const imgdnn_network, unsigned int, const imgdnn_tensor[], unsigned int, const imgdnn_tensor[], const imgdnn_network_object_flags, const char *, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::CreateNetworkBinary);
	return _func_ptr(device, context, network, num_inputs, inputs, num_outputs, outputs, flags, options, errcode_ret);
}

/**
* Destroy compiled network binary.
*
* @Input  binary  The network binary object to destroy.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE or IMGDNN_INVALID_VALUE
*/
inline imgdnn_err_code NetworkBinaryDestroy(imgdnn_network_binary * binary) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network_binary *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkBinaryDestroy);
	return _func_ptr(binary);
}

/**
* Produces a network object used for execution using binary object.
*
* @Input  device        The IMGDNN device for which to create this network.
* @Input  context       The IMGDNN context in which to create this network output.
* @Output size          Size of the object data in bytes.
* @Output object_data   The object data from binary file.
* @Output errcode_ret   IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_OUT_OF_MEMORY or
*                       IMGDNN_INVALID_VALUE for invalid tensor or argument or
*                       IMGDNN_INVALID_DEVICE for invalid device.
* @Return The output network object for execution or NULL on failure.
*/
inline imgdnn_network_object LoadNetworkObject(imgdnn_device device, imgdnn_context context, size_t size, const void * object_data, imgdnn_err_code * errcode_ret) {
	typedef imgdnn_network_object (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_device, imgdnn_context, size_t, const void *, imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::LoadNetworkObject);
	return _func_ptr(device, context, size, object_data, errcode_ret);
}

/**
* Destroy a Network Object previously created.
*
* @Input  network_object  A network object created previously.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE,
*         IMGDNN_INVALID_VALUE for invalid network object
*/
inline imgdnn_err_code NetworkObjectDestroy(imgdnn_network_object network_object) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_network_object);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkObjectDestroy);
	return _func_ptr(network_object);
}

/**
* Get inputs to a network.
*
* @Input  network_object  The network from which inputs requested.
* @Input  max_inputs      The maximum number of elements in the inputs array.
*                         Must be same as the total number of network inputs.
* @Output inputs[]        Array of inputs to network created if not NULL.
* @Output num_inputs      The number of inputs present in the network.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE or
*         IMGDNN_INVALID_VALUE for invalid network or arguments
*/
inline imgdnn_err_code NetworkObjectGetInputs(const imgdnn_network_object network_object, unsigned int max_inputs, imgdnn_input inputs[], unsigned int * num_inputs) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(const imgdnn_network_object, unsigned int, imgdnn_input[], unsigned int *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkObjectGetInputs);
	return _func_ptr(network_object, max_inputs, inputs, num_inputs);
}

/**
* Get outputs from a network.
*
* @Input  network_object  The network from which outputs requested.
* @Input  max_outputs     The maximum number of elements in the outputs array.
* @Output outputs[]       Array of outputs from network created if not NULL.
* @Output num_outputs     The number of outputs present in the network.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE or
*         IMGDNN_INVALID_VALUE for invalid network or arguments
*/
inline imgdnn_err_code NetworkObjectGetOutputs(const imgdnn_network_object network_object, unsigned int max_outputs, imgdnn_output outputs[], unsigned int * num_outputs) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(const imgdnn_network_object, unsigned int, imgdnn_output[], unsigned int *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkObjectGetOutputs);
	return _func_ptr(network_object, max_outputs, outputs, num_outputs);
}

/**
* Create a binding descriptor.
* Effectively a map of memory to input or output objects.
*
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE or
*                      IMGDNN_OUT_OF_MEMORY.
* @Return A new binding descriptor or NULL on failure.
*/
inline imgdnn_binding CreateBinding(imgdnn_err_code * errcode_ret) {
	typedef imgdnn_binding (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_err_code *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::CreateBinding);
	return _func_ptr(errcode_ret);
}

/**
* Destroy a binding descriptor.
*
* @Input  binding      The binding descriptor to destroy
* @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
*                      IMGDNN_INVALID_VALUE for invalid descriptor.
*/
inline imgdnn_err_code BindingDestroy(imgdnn_binding binding) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_binding);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::BindingDestroy);
	return _func_ptr(binding);
}

/**
* Add an input binding to a descriptor.
* If a binding already exists for this input in the descriptor, then it is
* updated.
*
* @Input  descriptor  The binding descriptor to add to.
* @Input  input       The input object to bind memory to.
* @Input  memory      The memory to bind to a tensor.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE,
*         IMGDNN_INVALID_VALUE for invalid input or memory.
*/
inline imgdnn_err_code BindingAddInput(imgdnn_binding binding, imgdnn_input input, imgdnn_memory memory) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_binding, imgdnn_input, imgdnn_memory);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::BindingAddInput);
	return _func_ptr(binding, input, memory);
}

/**
* Add an input size binding to a descriptor.
* If a binding already exists for this input size in the descriptor, then it is
* updated.
*
* @Input  descriptor  The binding descriptor to add to.
* @Input  input       The input object to bind size of.
* @Input  dimension   The dimension number of the size to set. Must have been
*                     set to 0 in the original descriptor.
* @Input  size        The dimension size to set.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE,
*         IMGDNN_INVALID_VALUE for invalid input or dimension.
*/
inline imgdnn_err_code BindingAddInputSize(imgdnn_binding binding, imgdnn_input input, unsigned int dimension, size_t size) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_binding, imgdnn_input, unsigned int, size_t);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::BindingAddInputSize);
	return _func_ptr(binding, input, dimension, size);
}

/**
* Add a output binding to a descriptor.
* If a binding already exists for this output in the descriptor, then it is
* updated.
*
* @Input  descriptor  The binding descriptor to add to.
* @Input  output      The output object to bind memory to.
* @Input  memory      The memory to bind to a tensor.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE,
*         IMGDNN_INVALID_VALUE for invalid output or memory.
*/
inline imgdnn_err_code BindingAddOutput(imgdnn_binding descriptor, imgdnn_output output, imgdnn_memory memory) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_binding, imgdnn_output, imgdnn_memory);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::BindingAddOutput);
	return _func_ptr(descriptor, output, memory);
}

/**
* Execute the network object with the given bindings.
*
* When the network object is executed, the event object follows the object's execution
* and when complete the given output here contains the output tensor.
*
* A binding descriptor is allowed to have bindings for inputs and ouputs that do
* not exist in the network object.
*
* After this, the user still will need to wait for the event to finish.
*
* @Input  network_object          The network object to execute.
* @Input  bindings                The bindings to make to the input and output objects in the network.
* @Input  blocking_execute        To indicate whether the execute call is blocking or non-blocking.
* @Input  num_events_in_wait_list The number of events in wait list that need to be completed before
*                                 executing this network object.
* @Input  event_wait_list         List of events to wait before executing this network object.
*                                 These events must have been created from within the same imgdnn_context.
*                                 (i.e. Executing a network object that was compiled for the same context).
* @Output event                   Returns an event object that identifies this execute command.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_CONTEXT for an invalid context,
*         IMGDNN_OUT_OF_MEMORY or IMGDNN_INVALID_VALUE for invalid argument
*/
inline imgdnn_err_code NetworkObjectExecute(const imgdnn_network_object network_object, const imgdnn_binding bindings, bool blocking_execute, unsigned int num_events_in_wait_list, const imgdnn_event event_wait_list[], imgdnn_event * event) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(const imgdnn_network_object, const imgdnn_binding, bool, unsigned int, const imgdnn_event[], imgdnn_event *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::NetworkObjectExecute);
	return _func_ptr(network_object, bindings, blocking_execute, num_events_in_wait_list, event_wait_list, event);
}

/**
* Wait for a img dnn event.
*
* @Input  event  The event object on which to wait for.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE
*/
inline imgdnn_err_code WaitForEvent(imgdnn_event event) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_event);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::WaitForEvent);
	return _func_ptr(event);
}

/**
* Destroy a img dnn event object.
*
* @Input  event  The event object to release.
* @Return IMGDNN_SUCCESS, IMGDNN_FAILURE
*/
inline imgdnn_err_code EventDestroy(imgdnn_event event) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_event);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::EventDestroy);
	return _func_ptr(event);
}

/**
* Callback function typedef for IMGDNN error reporting functions. The callback function
* should be made thread-safe by the user.
*
* @Input   flags               E.g. Information/warning/error/verbose/etc.
* @Input   tensor_names        Array of tensor names where the error occurred.
* @Input   num_tensor_names    Number of tensor names in tensor_names array
* @Input   error_code          Error code for the error which caused the callback to be called.
* @Input   error_message       Error message for the error which caused the callback to be called.
*//**
* Set the error handling function. This fuction is called by the driver when
* error occurs in the driver and will provide some information about the
* natute of the error. See definition of PFN_imgdnn_debug_report_callack for
* more detail.
*
* @Input   err_callback      The callback function to use.
* @Return  IMGDNN_SUCCESS if the error handling function has been set successfully, else returns IMGDNN_FAILURE
*/
inline imgdnn_err_code SetErrorHandler(PFN_imgdnn_debug_report_callback err_callback) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(PFN_imgdnn_debug_report_callback);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::SetErrorHandler);
	return _func_ptr(err_callback);
}

/**
* Return the imgdnn API version.
*
* @Input  api_version    Pointer to the cstring that will hold the API version.
* @Input  param_size     When api_version is not NULL this parameter is the size of the input string and it is used for
*                        size checking. If api_version is NULL param_size is used to return the size of the version.
* @Return IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE
*/
inline imgdnn_err_code GetApiVersion(char * api_version, size_t * param_size) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(char *, size_t *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::GetApiVersion);
	return _func_ptr(api_version, param_size);
}

/**
* Return the imgdnn version.
*
* @Input  driver_version  Pointer to the cstring with the imgdnn version.
* @Input  param_size      When driver_version is not NULL this parameter is the size of the input string and it is used for
*                         size checking. If driver_version is NULL param_size is used to return the size of the version.
* @Return IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE,
*         IMGDNN_OUT_OF_MEMORY.
*/
inline imgdnn_err_code GetDriverVersion(char * driver_version, size_t * param_size) {
	typedef imgdnn_err_code (*PROC_IMGDNN_FUNC_TYPEDEF)(char *, size_t *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::GetDriverVersion);
	return _func_ptr(driver_version, param_size);
}

/**
* Allocates a new imgdnn_per_axis_quant_param structure and its internal arrays.
* After being used the memory needs to be freed with imgdnnDestroyPerAxisQuantParam function.
*
* @Input  axis        axis along which the quantization parameters are applied
* @Input  count       number of scales/zero_points
* @Input  scales      array of integers representing the scales of the quantization parameters
*                     along the specified axis. If NULL the internal array is allocated and filled
*                     with 1.0f
* @Input  zero_points array of integers representing the zero_points of the quantization parameters
*                     along the specified axis. If NULL the internal array is allocated and filled
*                     with 0
*
* @Return pointer to the new and filled imgdnn_per_axis_quant_param structure or NULL in case of
*         failure
*/
inline imgdnn_per_axis_quant_param * CreatePerAxisQuantParam(unsigned axis, unsigned count, const float * scales, const int * zero_points) {
	typedef imgdnn_per_axis_quant_param * (*PROC_IMGDNN_FUNC_TYPEDEF)(unsigned, unsigned, const float *, const int *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::CreatePerAxisQuantParam);
	return _func_ptr(axis, count, scales, zero_points);
}

/**
* Frees the memory used by the imgdnn_per_axis_quant_param structure and its internal arrays.
*
* @Input  pa_param parameters structure to destroy
*/
inline void DestroyPerAxisQuantParam(imgdnn_per_axis_quant_param * pa_param) {
	typedef void (*PROC_IMGDNN_FUNC_TYPEDEF)(imgdnn_per_axis_quant_param *);
	static PROC_IMGDNN_FUNC_TYPEDEF _func_ptr = (PROC_IMGDNN_FUNC_TYPEDEF)imgdnn::internal::getImgdnnFunction(imgdnn::internal::imgdnnFuncName::DestroyPerAxisQuantParam);
	return _func_ptr(pa_param);
}

} //namespace imgdnn

inline std::string to_string(imgdnn_err_code enumvalue)
{
	static const std::string imgdnn_success("IMGDNN_SUCCESS");
	static const std::string imgdnn_failure("IMGDNN_FAILURE");
	static const std::string imgdnn_invalid_device("IMGDNN_INVALID_DEVICE");
	static const std::string imgdnn_invalid_context("IMGDNN_INVALID_CONTEXT");
	static const std::string imgdnn_invalid_value("IMGDNN_INVALID_VALUE");
	static const std::string imgdnn_invalid_operation("IMGDNN_INVALID_OPERATION");
	static const std::string imgdnn_out_of_memory("IMGDNN_OUT_OF_MEMORY");
	static const std::string imgdnn_unsupported("IMGDNN_UNSUPPORTED");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_SUCCESS: return imgdnn_success;
		case IMGDNN_FAILURE: return imgdnn_failure;
		case IMGDNN_INVALID_DEVICE: return imgdnn_invalid_device;
		case IMGDNN_INVALID_CONTEXT: return imgdnn_invalid_context;
		case IMGDNN_INVALID_VALUE: return imgdnn_invalid_value;
		case IMGDNN_INVALID_OPERATION: return imgdnn_invalid_operation;
		case IMGDNN_OUT_OF_MEMORY: return imgdnn_out_of_memory;
		case IMGDNN_UNSUPPORTED: return imgdnn_unsupported;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_report_flags enumvalue)
{
	static const std::string imgdnn_report_verbose("IMGDNN_REPORT_VERBOSE");
	static const std::string imgdnn_report_info("IMGDNN_REPORT_INFO");
	static const std::string imgdnn_report_warning("IMGDNN_REPORT_WARNING");
	static const std::string imgdnn_report_error("IMGDNN_REPORT_ERROR");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_REPORT_VERBOSE: return imgdnn_report_verbose;
		case IMGDNN_REPORT_INFO: return imgdnn_report_info;
		case IMGDNN_REPORT_WARNING: return imgdnn_report_warning;
		case IMGDNN_REPORT_ERROR: return imgdnn_report_error;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_dimensions_order enumvalue)
{
	static const std::string imgdnn_unknown("IMGDNN_UNKNOWN");
	static const std::string imgdnn_nchw("IMGDNN_NCHW");
	static const std::string imgdnn_nhwc("IMGDNN_NHWC");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_UNKNOWN: return imgdnn_unknown;
		case IMGDNN_NCHW: return imgdnn_nchw;
		case IMGDNN_NHWC: return imgdnn_nhwc;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_device_type enumvalue)
{
	static const std::string imgdnn_device_type_cpu("IMGDNN_DEVICE_TYPE_CPU");
	static const std::string imgdnn_device_type_gpu("IMGDNN_DEVICE_TYPE_GPU");
	static const std::string imgdnn_device_type_accelerator("IMGDNN_DEVICE_TYPE_ACCELERATOR");
	static const std::string imgdnn_device_type_all("IMGDNN_DEVICE_TYPE_ALL");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_DEVICE_TYPE_CPU: return imgdnn_device_type_cpu;
		case IMGDNN_DEVICE_TYPE_GPU: return imgdnn_device_type_gpu;
		case IMGDNN_DEVICE_TYPE_ACCELERATOR: return imgdnn_device_type_accelerator;
		case IMGDNN_DEVICE_TYPE_ALL: return imgdnn_device_type_all;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_device_info enumvalue)
{
	static const std::string imgdnn_device_type("IMGDNN_DEVICE_TYPE");
	static const std::string imgdnn_device_id("IMGDNN_DEVICE_ID");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_DEVICE_TYPE: return imgdnn_device_type;
		case IMGDNN_DEVICE_ID: return imgdnn_device_id;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_lock_access enumvalue)
{
	static const std::string imgdnn_lock_access_read_only("IMGDNN_LOCK_ACCESS_READ_ONLY");
	static const std::string imgdnn_lock_access_write_only("IMGDNN_LOCK_ACCESS_WRITE_ONLY");
	static const std::string imgdnn_lock_access_read_write("IMGDNN_LOCK_ACCESS_READ_WRITE");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_LOCK_ACCESS_READ_ONLY: return imgdnn_lock_access_read_only;
		case IMGDNN_LOCK_ACCESS_WRITE_ONLY: return imgdnn_lock_access_write_only;
		case IMGDNN_LOCK_ACCESS_READ_WRITE: return imgdnn_lock_access_read_write;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_import_mem_type enumvalue)
{
	static const std::string imgdnn_import_mem_type_cpu("IMGDNN_IMPORT_MEM_TYPE_CPU");
	static const std::string imgdnn_import_mem_type_opencl("IMGDNN_IMPORT_MEM_TYPE_OPENCL");
	static const std::string imgdnn_import_mem_type_fd("IMGDNN_IMPORT_MEM_TYPE_FD");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_IMPORT_MEM_TYPE_CPU: return imgdnn_import_mem_type_cpu;
		case IMGDNN_IMPORT_MEM_TYPE_OPENCL: return imgdnn_import_mem_type_opencl;
		case IMGDNN_IMPORT_MEM_TYPE_FD: return imgdnn_import_mem_type_fd;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_pooling_type enumvalue)
{
	static const std::string imgdnn_pooling_max("IMGDNN_POOLING_MAX");
	static const std::string imgdnn_pooling_average("IMGDNN_POOLING_AVERAGE");
	static const std::string imgdnn_pooling_min("IMGDNN_POOLING_MIN");
	static const std::string imgdnn_pooling_l2("IMGDNN_POOLING_L2");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_POOLING_MAX: return imgdnn_pooling_max;
		case IMGDNN_POOLING_AVERAGE: return imgdnn_pooling_average;
		case IMGDNN_POOLING_MIN: return imgdnn_pooling_min;
		case IMGDNN_POOLING_L2: return imgdnn_pooling_l2;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_lrn_type enumvalue)
{
	static const std::string imgdnn_lrn_across("IMGDNN_LRN_ACROSS");
	static const std::string imgdnn_lrn_within("IMGDNN_LRN_WITHIN");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_LRN_ACROSS: return imgdnn_lrn_across;
		case IMGDNN_LRN_WITHIN: return imgdnn_lrn_within;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_image_transform_type enumvalue)
{
	static const std::string imgdnn_image_transform_nearest("IMGDNN_IMAGE_TRANSFORM_NEAREST");
	static const std::string imgdnn_image_transform_bilinear("IMGDNN_IMAGE_TRANSFORM_BILINEAR");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_IMAGE_TRANSFORM_NEAREST: return imgdnn_image_transform_nearest;
		case IMGDNN_IMAGE_TRANSFORM_BILINEAR: return imgdnn_image_transform_bilinear;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_pad_mode enumvalue)
{
	static const std::string imgdnn_pad_constant("IMGDNN_PAD_CONSTANT");
	static const std::string imgdnn_pad_symmetric("IMGDNN_PAD_SYMMETRIC");
	static const std::string imgdnn_pad_reflect("IMGDNN_PAD_REFLECT");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_PAD_CONSTANT: return imgdnn_pad_constant;
		case IMGDNN_PAD_SYMMETRIC: return imgdnn_pad_symmetric;
		case IMGDNN_PAD_REFLECT: return imgdnn_pad_reflect;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_predefined_data_layout enumvalue)
{
	static const std::string rgba("RGBA");
	static const std::string planar_rgb("PLANAR_RGB");
	static const std::string planar_bgr("PLANAR_BGR");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case RGBA: return rgba;
		case PLANAR_RGB: return planar_rgb;
		case PLANAR_BGR: return planar_bgr;
		default: return unknown_value;
	}
}
inline std::string to_string(imgdnn_data_layout_param_type enumvalue)
{
	static const std::string imgdnn_data_layout_interleave("IMGDNN_DATA_LAYOUT_INTERLEAVE");
	static const std::string imgdnn_data_layout_strides("IMGDNN_DATA_LAYOUT_STRIDES");
	static const std::string imgdnn_data_layout_order("IMGDNN_DATA_LAYOUT_ORDER");
	static const std::string imgdnn_data_layout_byte_size("IMGDNN_DATA_LAYOUT_BYTE_SIZE");
	static const std::string unknown_value("UNKNOWN");
	
	switch (enumvalue)
	{
		case IMGDNN_DATA_LAYOUT_INTERLEAVE: return imgdnn_data_layout_interleave;
		case IMGDNN_DATA_LAYOUT_STRIDES: return imgdnn_data_layout_strides;
		case IMGDNN_DATA_LAYOUT_ORDER: return imgdnn_data_layout_order;
		case IMGDNN_DATA_LAYOUT_BYTE_SIZE: return imgdnn_data_layout_byte_size;
		default: return unknown_value;
	}
}

#define throwError(message) do {\
	std::string s(std::string(__FILE__) + ":" + std::to_string((int)__LINE__) + " : Function returned with error " + std::string(message));\
	throw std::runtime_error(s.c_str());\
} while (false)

#define throwOnImgdnnError(err) do {\
	if (err != IMGDNN_SUCCESS)\
	{\
		throwError("code " + to_string(err));\
	}\
} while (false)
