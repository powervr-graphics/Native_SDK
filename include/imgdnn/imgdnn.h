/*************************************************************************/ /*!
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
@brief          IMG DNN - the Imagination Neural Network programming interface
*/ /**************************************************************************/
#ifndef _IMG_DNN_H
#define _IMG_DNN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup IMGDNN IMGDNN API
 *
 * @{
 * IMGDNN API - The IMGDNN API.
 *
 * The Imagination Neural Network interface is used for controlling the Imagination
 * Neural Network GPU and hardware acceleration solutions.
 */

#if !defined(IMGDNN_MAJOR_VERSION)
#define IMGDNN_MAJOR_VERSION 1
#endif

#if !defined(IMGDNN_MINOR_VERSION)
#define IMGDNN_MINOR_VERSION 9
#endif

typedef struct _imgdnn_device_t *imgdnn_device;
typedef struct _imgdnn_context_t *imgdnn_context;
typedef struct _imgdnn_memory_t *imgdnn_memory;
typedef struct _imgdnn_tensor_t *imgdnn_tensor;
typedef struct _imgdnn_input_t *imgdnn_input;
typedef struct _imgdnn_output_t *imgdnn_output;
typedef struct _imgdnn_network_t *imgdnn_network;
typedef struct _imgdnn_event_t *imgdnn_event;
typedef struct _imgdnn_binding_t *imgdnn_binding;
typedef struct _imgdnn_network_object_t *imgdnn_network_object;
typedef struct _imgdnn_device_capabilities_t* imgdnn_device_capabilities;

typedef enum _imgdnn_err_code_
{
  IMGDNN_SUCCESS = 0,
  IMGDNN_FAILURE,
  IMGDNN_INVALID_DEVICE,
  IMGDNN_INVALID_CONTEXT,
  IMGDNN_INVALID_VALUE,
  IMGDNN_INVALID_OPERATION,
  IMGDNN_OUT_OF_MEMORY,
  IMGDNN_UNSUPPORTED
} imgdnn_err_code;

typedef enum _imgdnn_report_flags_
{
	IMGDNN_REPORT_VERBOSE,
	IMGDNN_REPORT_INFO,
	IMGDNN_REPORT_WARNING,
	IMGDNN_REPORT_ERROR
} imgdnn_report_flags;

typedef enum _imgdnn_type_t
{
  IMGDNN_TYPE_I8,
  IMGDNN_TYPE_U8,
  IMGDNN_TYPE_I16,
  IMGDNN_TYPE_U16,
  IMGDNN_TYPE_I32,
  IMGDNN_TYPE_U32,
  IMGDNN_TYPE_F16,
  IMGDNN_TYPE_F32,
  IMGDNN_TYPE_Q_I8,
  IMGDNN_TYPE_Q_U8,
#if IMGDNN_MINOR_VERSION >= 5
	IMGDNN_TYPE_QPA_I8,
	IMGDNN_TYPE_QPA_U8,
#endif
  IMGDNN_TYPE_MAX
} imgdnn_type;

#if IMGDNN_MINOR_VERSION >= 4
typedef enum _imgdnn_dimensions_order_t
{
  IMGDNN_UNKNOWN,
  IMGDNN_NCHW,
  IMGDNN_NHWC
} imgdnn_dimensions_order;
#endif

#if IMGDNN_MINOR_VERSION >= 5
/**
 * A structure to hold per axis quantization parameters 'scale' and 'zero_point'.
 *       real_value = scale * (quantized_value - zero_point)
 * 'scales'      is an array of per axis scales. This is only used for tensors of
 *               IMGDNN_TYPE_QPA_*
 * 'zero_points' is an array of per axis scales. This is only used for tensors of
 *               IMGDNN_TYPE_QPA_*
 * 'axis'        the axis to which the scales and zero_points apply.
 * 'count'       number of scales and zero_points. Should match the dimension of the tensor in the
 *               specified axis
 */
typedef struct _imgdnn_per_axis_quant_param_t
{
	float *scales;
	int *zero_points;
	unsigned axis;
	unsigned count;
} imgdnn_per_axis_quant_param;
#endif

/**
 * A structure to hold quantization parameters 'scale' and 'zero_point'.
 *       real_value = scale * (quantized_value - zero_point)
 * 'zero_point'     is the quantized value that corresponds to the real value 0
 * 'scale'          is the difference of real values corresponding to consecutive quantized values,
 *                  i.e. the quantization step
 * 'per_axis'       pointer to structure containing per axis quantization parameters. Only used
 *                  by IMGDNN_TYPE_QA* type tensors. Should be created and destroyed using the
 *                  appropriate functions.
 *                  \sa imgdnn_per_axis_quant_param,imgdnnCreatePerAxisQuantParam,imgdnnDestroyPerAxisQuantParam
 */
typedef struct _imgdnn_quant_param_t
{
	union {
		struct {
			float scale;
			int zero_point;
		};
#if IMGDNN_MINOR_VERSION >= 5
		imgdnn_per_axis_quant_param *per_axis;
#endif
	};
} imgdnn_quant_param;

#define IMGDNN_DESCRIPTOR_MAX_DIM (6u)
typedef struct _imgdnn_tensor_descriptor_t
{
  unsigned int dimensions;
  imgdnn_type type;
  size_t size[IMGDNN_DESCRIPTOR_MAX_DIM];
  imgdnn_quant_param quant_param;
} imgdnn_tensor_descriptor;

/* General setup and teardown *************************************************/
typedef enum _imgdnn_device_type_t
{
  IMGDNN_DEVICE_TYPE_CPU,
  IMGDNN_DEVICE_TYPE_GPU,
  IMGDNN_DEVICE_TYPE_ACCELERATOR,
  IMGDNN_DEVICE_TYPE_ALL
} imgdnn_device_type;

typedef enum _imgdnn_device_info_t
{
  IMGDNN_DEVICE_TYPE,
  IMGDNN_DEVICE_ID
} imgdnn_device_info;

#define IMGDNN_CTX_FLAGS_NONE (0)
typedef unsigned imgdnn_context_flags;

/**
 * Get the list of available devices.
 *
 * @Input  device_type  The type of requested devices.
 * @Input  max_devices  The maximum number of devices that can be added to devices list if not NULL.
 * @Output devices      The list of returned IMGDNN devices if not NULL
 * @Output num_devices  The number of devices of a particular type that are present. May not be NULL.
 * @Return IMGDNN_SUCCESS on success. IMGDNN_INVALID_VALUE or IMGDNN_FAILURE for failure.
 */
imgdnn_err_code imgdnnGetDevices(imgdnn_device_type device_type,
                                 unsigned int max_devices,
                                 imgdnn_device devices[],
                                 unsigned int *num_devices);

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
imgdnn_err_code imgdnnGetDeviceInfo(imgdnn_device device,
                                    imgdnn_device_info device_info,
                                    size_t device_info_data_size,
                                    void *device_info_data);

/**
 * Create a IMGDNN context from a number of IMGDNN devices.
 *
 * @Input  num_devices    Number of devices in the device array. Must be > 0
 * @Input  devices[]      An array of IMGDNN devices for which DNN context is created.
 * @Input  context_flags  Flags to modify how the device context behaves.
 * @Output errcode_ret    IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_DEVICE
 * @Return The returned IMGDNN context created or NULL on failure.
 */
imgdnn_context imgdnnCreateContext(unsigned int num_devices,
                                   const imgdnn_device devices[],
                                   const imgdnn_context_flags context_flags,
                                   imgdnn_err_code *errcode_ret);

/**
 * Destroy a previously created IMGDNN context.
 *
 * @Input  context  A previously created IMGDNN context
 * @Return IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_CONTEXT
 */
imgdnn_err_code imgdnnContextDestroy(imgdnn_context context);

/* Memory **********************************************************/
typedef enum _imgdnn_lock_access_t
{
  IMGDNN_LOCK_ACCESS_READ_ONLY,
  IMGDNN_LOCK_ACCESS_WRITE_ONLY,
  IMGDNN_LOCK_ACCESS_READ_WRITE
} imgdnn_lock_access;

typedef enum _imgdnn_import_mem_type_t
{
  IMGDNN_IMPORT_MEM_TYPE_CPU,
  /* Only valid to use with a context created with the IMGDNN CL extension.
   * Imports a cl_mem to use as an imgdnn_memory object. */
  IMGDNN_IMPORT_MEM_TYPE_OPENCL,
  /* Used for importing buffer file descriptors to be used as imgdnn_memory objects */
  IMGDNN_IMPORT_MEM_TYPE_FD
} imgdnn_import_mem_type;

/**
 * Allocate Device Memory.
 *
 * @Input  context      A previously obtained IMGDNN context
 * @Input  size         Size of allocation in bytes
 * @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_OUT_OF_MEMORY
 * @Return imgdnn_memory object on success and NULL on failure
 */
imgdnn_memory imgdnnAllocateMemory(imgdnn_context context,
                                   size_t size,
                                   imgdnn_err_code *errcode_ret);

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
imgdnn_memory imgdnnImportMemory(imgdnn_context context,
                                 void* memory,
                                 size_t size,
                                 imgdnn_import_mem_type import_mem_type,
                                 imgdnn_err_code *errcode_ret);

#if IMGDNN_MINOR_VERSION >= 5
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
imgdnn_memory imgdnnSubdivideMemory(imgdnn_memory memory,
                                 uint32_t offset,
                                 size_t size,
                                 imgdnn_err_code *errcode_ret);
#endif

/**
 * Release Previously allocated Device Memory.
 *
 * @Input  memory  Previously allocated device memory
 * @Return IMGDNN_SUCCESS, IMGDNN_FAILURE
 */
imgdnn_err_code imgdnnMemoryDestroy(imgdnn_memory memory);

/**
 * Lock the memory for host access. Obtain host accessible pointer to memory.
 *
 * @Input  memory             Memory object for which host access is sought.
 * @Input  lock_access        Lock access type.
 * @Output errcode_ret        IMGDNN_SUCCESS, IMGDNN_FAILURE
 * @Return Host accessible pointer to memory on success and NULL on failure
 */
void* imgdnnMemoryLock(imgdnn_memory memory,
                       imgdnn_lock_access lock_access,
                       imgdnn_err_code *errcode_ret);

/**
 * Unlock the memory that was previously locked for host access.
 *
 * @Input  memory    Memory object to unlock.
 * @Return IMGDNN_SUCCESS, IMGDNN_FAILURE
 */
imgdnn_err_code imgdnnMemoryUnlock(imgdnn_memory memory);

/* Network **********************************************************/

/* A tensor is an opaque memory region. It may correspond to actual memory, or
 * it may be elided in whole network optimisation and hence not actually exist
 * when executing. In either case its use is to connect nodes together
 * logically.
 *
 * A tensor layout is described by a tensor descriptor which fully defines the
 * layout. This allows the user to access data in a memory object without necessarily
 * needing to pass through an opaque tensor. This is useful for custom kernels
 * in a network.
 *
 * The layout of all tensors can be considered to be linearly ordered in row
 * major order and alignment of 16 bytes.
 */

typedef enum _imgdnn_operation_unary_t
{
  IMGDNN_OPERATION_SIGN,
  IMGDNN_OPERATION_SIGMOID,
  IMGDNN_OPERATION_TANH,
  IMGDNN_OPERATION_RELU, /* max(input_value, 0) */
  IMGDNN_OPERATION_NEGATE,
  IMGDNN_OPERATION_ABS,
  IMGDNN_OPERATION_NOT,
  IMGDNN_OPERATION_LOG,
  IMGDNN_OPERATION_SQRT,
  IMGDNN_OPERATION_EXP,
  IMGDNN_OPERATION_FLOOR,
  IMGDNN_OPERATION_CEIL,
#if IMGDNN_MINOR_VERSION >= 6
  IMGDNN_OPERATION_SIN,
  IMGDNN_OPERATION_RSQRT
#endif
} imgdnn_operation_unary;

typedef enum _imgdnn_operation_binary_t
{
  IMGDNN_OPERATION_ADD,
  IMGDNN_OPERATION_SUB,
  IMGDNN_OPERATION_MUL,
  IMGDNN_OPERATION_DIV,
  IMGDNN_OPERATION_AND,
  IMGDNN_OPERATION_OR,
  IMGDNN_OPERATION_XOR,
  IMGDNN_OPERATION_MAX,
  IMGDNN_OPERATION_MIN,
  IMGDNN_OPERATION_MATMUL,
  IMGDNN_OPERATION_ADD_SAT,
  IMGDNN_OPERATION_SUB_SAT,
  IMGDNN_OPERATION_MUL_SAT,
#if IMGDNN_MINOR_VERSION >= 4
  IMGDNN_OPERATION_PRELU,
#endif
} imgdnn_operation_binary;

typedef enum _imgdnn_pooling_type_t
{
  IMGDNN_POOLING_MAX,
  IMGDNN_POOLING_AVERAGE,
  IMGDNN_POOLING_MIN,
  IMGDNN_POOLING_L2
} imgdnn_pooling_type;

typedef enum _imgdnn_lrn_type_t
{
  IMGDNN_LRN_ACROSS,
  IMGDNN_LRN_WITHIN
} imgdnn_lrn_type;

typedef enum _imgdnn_image_transform_type_t
{
  IMGDNN_IMAGE_TRANSFORM_NEAREST,
  IMGDNN_IMAGE_TRANSFORM_BILINEAR
} imgdnn_image_transform_type;

typedef enum _imgdnn_reduce_type_t
{
  IMGDNN_REDUCE_SUM,
  IMGDNN_REDUCE_MEAN,
  IMGDNN_REDUCE_MAX,
#if IMGDNN_MINOR_VERSION >= 3
  IMGDNN_REDUCE_ARGMAX,
#endif
#if IMGDNN_MINOR_VERSION >=6
  IMGDNN_REDUCE_MIN
#endif
} imgdnn_reduce_type;

/**
 * Utility function to get size of a descriptor
 *
 * @Input  descriptor   The descriptor to find total size of
 * @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid descriptor.
 *                      May be NULL.
 * @Return The total size of the data referenced by this descriptor.
 */
size_t imgdnnGetDescriptorSize(const imgdnn_tensor_descriptor *const descriptor,
                               imgdnn_err_code *errcode_ret);

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
imgdnn_err_code imgdnnGetTensorDescriptor(imgdnn_tensor tensor,
                                          imgdnn_tensor_descriptor *desc);

/**
 * Create a Network
 *
 * @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE.
 * @Return The network created.
 */
imgdnn_network imgdnnCreateNetwork(imgdnn_err_code *errcode_ret);

/**
 * Create a Network from IR
 *
 * @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE.
 * @Return The network created.
 */
imgdnn_network imgdnnCreateNetworkFromIR(const void *arch_data,
                                         size_t arch_data_size,
                                         const void *params_data,
                                         size_t params_data_size,
                                         imgdnn_err_code *errcode_ret);

/**
 * Destroy a previously created Network
 *
 * @Input  network      A previously created network.
 * @Return IMGDNN_SUCCESS, IMGDNN_FAILURE, IMGDNN_INVALID_VALUE for invalid network.
 */
imgdnn_err_code imgdnnNetworkDestroy(imgdnn_network network);

/**
 * Add an input to the network.
 *
 * @Input  network      Handle to the network.
 * @Input  descriptor   The description of the input to add.
 * @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid descriptor,
 *                      IMGDNN_FAILURE or IMGDNN_OUT_OF_MEMORY. May be NULL.
 * @Return The created input tensor or NULL on failure.
 */
imgdnn_tensor imgdnnNetworkInput(imgdnn_network network,
                                 const imgdnn_tensor_descriptor *const descriptor,
                                 imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkFixedInput(imgdnn_network network,
                                      const imgdnn_tensor_descriptor *const descriptor,
                                      const void *const fixed_data,
                                      imgdnn_err_code *errcode_ret);

/**
 * Sets the name of a tensor.
 *
 * @Input  tensor  The tensor to set name of. May not be NULL.
 * @Input  name  The given name, may not be NULL.
 * @Return errcode_ret  IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid tensor,
 *                      or IMGDNN_OUT_OF_MEMORY.
 */
imgdnn_err_code imgdnnTensorSetName(imgdnn_tensor tensor, const char *name);

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
const char *imgdnnTensorGetName(imgdnn_tensor tensor, imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkFindTensor(imgdnn_network network,
                                      const char *name,
                                      imgdnn_err_code *errcode_ret);


#if IMGDNN_MINOR_VERSION >= 6

/**
 * Set the name of the network.
 *
 * @Input network  The network to set name of.
 * @Input network_name The name for the network
 * @Return errcode_ret IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE for invalid network/network_name,
 *                      or IMGDNN_OUT_OF_MEMORY
 */
imgdnn_err_code imgdnnNetworkSetName(imgdnn_network network, const char *network_name);

/**
 * Get the name of the network.
 *
 * @Input network The network to get the name of.
 * @Output errcode_ret IMGDNN_SUCCESS, or IMGDNN_INVALID_VALUE for invalid network.
 * @Return The given name, NULL on failure
 */
const char *imgdnnNetworkGetName(const imgdnn_network network, imgdnn_err_code *errcode_ret);

/**
 *  Get the name of the network_object.
 *
 * @Input network_object The network object to get name of.
 * @Output errcode_ret IMGDNN_SUCCESS, or IMGDNN_INVALID_VALUE for invalid network object.
 * @Return The given name on success, NULL on failure
 */
const char *imgdnnNetworkGetObjectName(const imgdnn_network_object network_object,
                                        imgdnn_err_code *errcode_ret);

#endif

#if IMGDNN_MINOR_VERSION >= 2
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
imgdnn_tensor* imgdnnNetworkFindInputs(imgdnn_network network,
                                       unsigned *num_inputs,
                                       imgdnn_err_code *errcode_ret);

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
imgdnn_tensor* imgdnnNetworkFindDefaultOutputs(imgdnn_network network,
                                               unsigned *num_outputs,
                                               imgdnn_err_code *errcode_ret);
#endif

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
imgdnn_tensor imgdnnNetworkReshapeOp(imgdnn_network network,
                                     imgdnn_tensor tensor,
                                     const imgdnn_tensor_descriptor *const descriptor,
                                     imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkTransposeOp(imgdnn_network network,
                                       imgdnn_tensor tensor,
                                       const int order[],
                                       imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkCastOp(imgdnn_network network,
                                  imgdnn_tensor tensor,
                                  imgdnn_type dst_type,
                                  const imgdnn_quant_param *const dst_quant_param,
                                  imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkBroadcastOp(imgdnn_network network,
                                       imgdnn_tensor tensor,
                                       unsigned int dimension,
                                       size_t size,
                                       imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkSubTensor(imgdnn_network network,
                                     imgdnn_tensor tensor,
                                     const size_t start[],
                                     const size_t end[],
                                     const size_t stride[],
                                     imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkInterleaveOp(imgdnn_network network,
                                        imgdnn_tensor tensor1,
                                        imgdnn_tensor tensor2,
                                        unsigned int dimension,
                                        size_t start,
                                        size_t stride,
                                        imgdnn_err_code *errcode_ret);

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
imgdnn_err_code imgdnnNetworkSplitOp(imgdnn_network network,
                                     imgdnn_tensor tensor,
                                     unsigned int dimension,
                                     unsigned int num_slices,
                                     imgdnn_tensor out_tensors[]);

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
imgdnn_tensor imgdnnNetworkConcatOp(imgdnn_network network,
                                    const imgdnn_tensor tensors[],
                                    unsigned int dimension,
                                    unsigned int num_concats,
                                    imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkUnaryOp(imgdnn_network network,
                                   imgdnn_tensor in_tensor,
                                   imgdnn_operation_unary operation,
                                   imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkReLUOp(imgdnn_network network,
                                  imgdnn_tensor in_tensor,
                                  bool has_min_clamp,
                                  float min_clamp,
                                  bool has_max_clamp,
                                  float max_clamp,
                                  float negative_slope,
                                  imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkBinaryOp(imgdnn_network network,
                                    imgdnn_tensor in_tensor1,
                                    imgdnn_tensor in_tensor2,
                                    imgdnn_operation_binary operation,
                                    imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkConvolution2dOp(imgdnn_network network,
                                           imgdnn_tensor in_tensor,
                                           imgdnn_tensor filter,
                                           const unsigned int stride[2],
                                           const unsigned int pad[2],
                                           const unsigned int dilation[2],
                                           bool with_partial,
                                           imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkConvolution2dOp_v2(imgdnn_network network,
                                           imgdnn_tensor in_tensor,
                                           imgdnn_tensor filter,
                                           const unsigned int stride[2],
                                           const unsigned int pad_to_begin[2],
                                           const unsigned int pad_to_end[2],
                                           const unsigned int dilation[2],
                                           imgdnn_err_code *errcode_ret);

#if IMGDNN_MINOR_VERSION >= 2
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
imgdnn_tensor imgdnnNetworkGroupedConvolution2dOp(imgdnn_network network,
                                           imgdnn_tensor in_tensor,
                                           imgdnn_tensor filter,
                                           const unsigned int stride[2],
                                           const unsigned int pad_to_begin[2],
                                           const unsigned int pad_to_end[2],
                                           const unsigned int dilation[2],
                                           unsigned groups,
                                           imgdnn_err_code *errcode_ret);
#endif

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
imgdnn_tensor imgdnnNetworkDepthConvolution2dOp(imgdnn_network network,
                                                imgdnn_tensor in_tensor,
                                                imgdnn_tensor filter,
                                                const unsigned int stride[2],
                                                const unsigned int pad[2],
                                                const unsigned int dilation[2],
                                                bool with_partial,
                                                imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkDepthConvolution2dOp_v2(imgdnn_network network,
                                                imgdnn_tensor in_tensor,
                                                imgdnn_tensor filter,
                                                const unsigned int stride[2],
                                                const unsigned int pad_to_begin[2],
                                                const unsigned int pad_to_end[2],
                                                const unsigned int dilation[2],
                                                imgdnn_err_code *errcode_ret);

#if IMGDNN_MINOR_VERSION >= 2

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
imgdnn_tensor imgdnnNetworkDeconvolution2dOp_v2(imgdnn_network network,
        imgdnn_tensor in_tensor,
        imgdnn_tensor filter,
        const unsigned int stride[2],
        const unsigned int pad_to_begin[2],
        const unsigned int pad_to_end[2],
        const unsigned int dilation[2],
        imgdnn_err_code *errcode_ret);

#endif

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
imgdnn_tensor imgdnnNetworkDeconvolution2dOp(imgdnn_network network,
                                             imgdnn_tensor in_tensor,
                                             imgdnn_tensor filter,
                                             const unsigned int stride[2],
                                             const unsigned int pad[2],
                                             const unsigned int dilation[2],
                                             const unsigned int partial_size[2],
                                             imgdnn_err_code *errcode_ret);

#if IMGDNN_MINOR_VERSION >= 2
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
imgdnn_tensor imgdnnNetworkGroupedDeconvolution2dOp(imgdnn_network network,
                                                    imgdnn_tensor in_tensor,
                                                    imgdnn_tensor filter,
                                                    const unsigned int stride[2],
                                                    const unsigned int pad_to_begin[2],
                                                    const unsigned int pad_to_end[2],
                                                    const unsigned int dilation[2],
                                                    unsigned groups,
                                                    imgdnn_err_code *errcode_ret);
#endif

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
imgdnn_tensor imgdnnNetworkPooling2dOp(imgdnn_network network,
                                       imgdnn_tensor in_tensor,
                                       const unsigned int size[2],
                                       const unsigned int stride[2],
                                       const unsigned int pad[2],
                                       imgdnn_pooling_type type,
                                       bool with_partial,
                                       imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkPooling2dOp_v2(imgdnn_network network,
                                       imgdnn_tensor in_tensor,
                                       const unsigned int size[2],
                                       const unsigned int stride[2],
                                       const unsigned int pad_to_begin[2],
                                       const unsigned int pad_to_end[2],
                                       imgdnn_pooling_type type,
                                       imgdnn_err_code *errcode_ret);

#if IMGDNN_MINOR_VERSION >= 5
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
imgdnn_tensor imgdnnNetworkPooling2dOp_v3(imgdnn_network network,
                                       imgdnn_tensor in_tensor,
                                       const unsigned int size[2],
                                       const unsigned int stride[2],
                                       const unsigned int pad_to_begin[2],
                                       const unsigned int pad_to_end[2],
                                       imgdnn_pooling_type type,
                                       bool count_include_pad,
                                       imgdnn_err_code *errcode_ret);
#endif

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
imgdnn_tensor imgdnnNetworkLrnOp(imgdnn_network network,
                                 imgdnn_tensor in_tensor,
                                 imgdnn_lrn_type type,
                                 size_t window_size,
                                 float k,
                                 float alpha,
                                 float beta,
                                 imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkImageTransformOp(imgdnn_network network,
                                            imgdnn_tensor in_tensor,
                                            imgdnn_tensor transform,
                                            imgdnn_image_transform_type type,
                                            imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkReduceOp(imgdnn_network network,
                                    imgdnn_tensor in_tensor,
                                    imgdnn_reduce_type type,
                                    const int axis[],
                                    size_t num_axis,
                                    imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkSoftmaxOp(imgdnn_network network,
                                    imgdnn_tensor in_tensor,
                                    float beta,
									unsigned int axis,
									imgdnn_err_code *errcode_ret);

typedef struct _imgdnn_lstm_weight_tensors_t
{
	/* Weights for the input data
	 * Size [input_size, num_units]
	 */
	imgdnn_tensor in_to_remember; // Set to NULL for coupled remember/forget
	imgdnn_tensor in_to_forget; // Must be non-NULL
	imgdnn_tensor in_to_state; // Must be non-NULL
	imgdnn_tensor in_to_output; // Must be non-NULL

	/* Weights for previous
	 * Size [output_size, num_units]
	 */
	imgdnn_tensor prev_to_remember; // Set to NULL for coupled remember/forget
	imgdnn_tensor prev_to_forget; // Must be non-NULL
	imgdnn_tensor prev_to_state; // Must be non-NULL
	imgdnn_tensor prev_to_output; // Must be non-NULL

	/* Weights for peephole connections, set all to NULL for no-peephole.
	 * Size [num_units]
	 */
	imgdnn_tensor state_to_remember; // Set to NULL for coupled remember/forget or no-peephole
	imgdnn_tensor state_to_forget; // Set to NULL for no-peephole
	imgdnn_tensor state_to_output; // Set to NULL for no-peephole

	/* Bias for each stage
	 * Size [num_units]
	 */
	imgdnn_tensor remember_bias; // Set to NULL for coupled remember/forget, otherwise optional
	imgdnn_tensor forget_bias; // Optional
	imgdnn_tensor state_bias; // Optional
	imgdnn_tensor output_bias; // Optional

	/* Output projection weights
	 * Size [num_units, output_size]
	 */
	imgdnn_tensor projection; // Set to NULL for no-projection
	/* Output projection bias
	 * Size [output_size]
	 */
	imgdnn_tensor projection_bias; // Set to NULL for no-projection, otherwise optional
} imgdnn_lstm_weight_tensors;

typedef struct _imgdnn_lstm_output_tensors_t
{
	imgdnn_tensor output;
	imgdnn_tensor state;
} imgdnn_lstm_output_tensors;

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
imgdnn_lstm_output_tensors imgdnnNetworkLSTMOp(imgdnn_network network,
		imgdnn_tensor in_tensor,
		imgdnn_tensor prev_tensor,
		imgdnn_tensor state_tensor,
		imgdnn_lstm_weight_tensors *weights,
		float state_clip,
		float projection_clip,
		imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkDepthToSpaceOp(imgdnn_network network,
		imgdnn_tensor in_tensor,
		size_t block_size,
		imgdnn_err_code *errcode_ret);

#if IMGDNN_MINOR_VERSION >= 4
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
imgdnn_tensor imgdnnNetworkBatchToSpaceNDOp(imgdnn_network network,
		imgdnn_tensor in_tensor,
		imgdnn_tensor block_size,
		imgdnn_err_code *errcode_ret);
#endif

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
imgdnn_tensor imgdnnNetworkGatherOp(imgdnn_network network,
                                  imgdnn_tensor in_tensor,
                                  imgdnn_tensor indices,
                                  unsigned int axis,
                                  imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkSpaceToDepthOp(imgdnn_network network,
                                          imgdnn_tensor in_tensor,
                                          size_t block_size,
                                          imgdnn_err_code *errcode_ret);
#if IMGDNN_MINOR_VERSION >= 4
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
imgdnn_tensor imgdnnNetworkSpaceToBatchNDOp(imgdnn_network network,
                                            imgdnn_tensor in_tensor,
                                            imgdnn_tensor in_padding,
                                            imgdnn_tensor block_size,
                                            imgdnn_err_code *errcode_ret);
#endif

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
imgdnn_tensor imgdnnNetworkResizeBilinearOp(imgdnn_network network,
					    imgdnn_tensor in_tensor,
					    unsigned int height,
					    unsigned int width,
					    bool align_corners,
					    imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkPadOp(imgdnn_network network,
						imgdnn_tensor in_tensor,
						const unsigned int pad_before[],
						const unsigned int pad_after[],
						float pad_value,
						imgdnn_err_code *errcode_ret);

#if IMGDNN_MINOR_VERSION >= 2

typedef enum _imgdnn_pad_mode_t
{
  IMGDNN_PAD_CONSTANT,
  IMGDNN_PAD_SYMMETRIC,
  IMGDNN_PAD_REFLECT
} imgdnn_pad_mode;


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
imgdnn_tensor imgdnnNetworkPadOp_v2(imgdnn_network network,
                                    imgdnn_tensor in_tensor,
                                    const unsigned int pad_before[],
                                    const unsigned int pad_after[],
                                    float pad_value,
                                    imgdnn_pad_mode pad_mode,
                                    imgdnn_err_code *errcode_ret);
#endif

#if IMGDNN_MINOR_VERSION >= 3

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
imgdnn_tensor imgdnnNetworkResizeNearestNeighbourOp(imgdnn_network network,
					    imgdnn_tensor in_tensor,
					    unsigned int height,
					    unsigned int width,
					    bool align_corners,
					    imgdnn_err_code *errcode_ret);

#endif

#if IMGDNN_MINOR_VERSION >= 4

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
imgdnn_tensor imgdnnNetworkROIPoolingOp(imgdnn_network network,
					imgdnn_tensor in_tensor,
					imgdnn_tensor roi_tensor,
					imgdnn_tensor batch_idx_tensor,
					unsigned int out_height,
					unsigned int out_width,
					float scaled_height,
					float scaled_width,
					imgdnn_err_code *errcode_ret);

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
imgdnn_tensor imgdnnNetworkROIAlignOp(imgdnn_network network,
				      imgdnn_tensor in_tensor,
				      imgdnn_tensor roi_tensor,
				      imgdnn_tensor batch_idx_tensor,
				      unsigned int out_height,
				      unsigned int out_width,
				      float scaled_height,
				      float scaled_width,
				      unsigned int num_samples_height,
				      unsigned int num_samples_width,
				      imgdnn_err_code *errcode_ret);

#endif

/* Network Object **********************************************************/
#define IMGDNN_NETWORK_OBJ_FLAG_NONE (0U)
#define IMGDNN_NETWORK_OBJ_FLAG_BLOCKING_OPTIMISATION (1U << 0)
typedef unsigned imgdnn_network_object_flags;

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
imgdnn_tensor_descriptor imgdnnGetInputDescriptor(imgdnn_input input,
                                                  imgdnn_err_code *errcode_ret);

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
imgdnn_tensor_descriptor imgdnnGetOutputDescriptor(imgdnn_output output,
                                                   imgdnn_err_code *errcode_ret);

#if IMGDNN_MINOR_VERSION >= 4
/**
 * A structure to hold data layout parameters.
 * 'data_stride' array of per dimension strides, in bytes. Should have the same number of dimensions as the tensor. Can be 0 (contiguous data) or -1 (device can decide the best option)
 * 'interleave' amount of data (elements) from 2nd dimension interleaved at innermost dimension (only makes sense for 4D tensors).
 * 'order' array of chars specifying the data order. This is optional and currently only 4D orders "NCHW" and "NHWC" are supported. Set to "\0" when not used
 */
typedef struct _imgdnn_data_layout_param_t {
  long long int data_stride[IMGDNN_DESCRIPTOR_MAX_DIM];
  uint32_t interleave;
  imgdnn_dimensions_order order;
} imgdnn_data_layout_param;

/**
 * Enum to list commonly defined data layouts.
 */
typedef enum _imgdnn_predefined_data_layout_t {
  RGBA,
  PLANAR_RGB,
  PLANAR_BGR
} imgdnn_predefined_data_layout;

/*
 * Enum to list the different types of I/O data layout parameters.
 */
typedef enum _imgdnn_data_layout_param_type_t {
  IMGDNN_DATA_LAYOUT_INTERLEAVE, // int representing the data interleaving
  IMGDNN_DATA_LAYOUT_STRIDES, // long long int[IMGDNN_DESCRIPTOR_MAX_DIM] representing strides in each dimension in bytes
  IMGDNN_DATA_LAYOUT_ORDER, // imgdnn_dimensions_order. Currently only valid for 4D tensors
  IMGDNN_DATA_LAYOUT_BYTE_SIZE // uint32 representing the size of the expected buffer in bytes
} imgdnn_data_layout_param_type;

/**
 * Fill the data layout parameters structure based on the specified pre-defined layout.
 *
 * @Input  data_layout   Pointer to the data layout structure to be filled.
 * @Input  predef_layout Pre-defined data layout to be set for this tensor.
 * @Input  tensor_desc   Tensor descriptor to check that the predef layout is compatible with the tensor.
 * @Return IMGDNN_SUCCESS or IMGDNN_INVALID_VALUE for invalid tensor or invalid pre-defined data layout.
 */
imgdnn_err_code imgdnnFillDataLayoutParameters(imgdnn_data_layout_param *data_layout,
                                               imgdnn_predefined_data_layout predef_layout,
                                               const imgdnn_tensor_descriptor* const desc);

/**
 * Get the requested parameter from the specified input
 *
 * @Input  input     The input from which to retrieve the parameter.
 * @Input  parameter The requested parameter.
 * @Output out       Pointer to the memory to fill with parameter value.
 */
imgdnn_err_code imgdnnGetInputTensorParameter(imgdnn_input input, imgdnn_data_layout_param_type type, void *out);

/**
 * Get the requested parameter from the specified output
 *
 * @Input  output     The output from which to retrieve the parameter.
 * @Input  parameter The requested parameter.
 * @Output out       Pointer to the memory to fill with parameter value.
 */
imgdnn_err_code imgdnnGetOutputTensorParameter(imgdnn_output output, imgdnn_data_layout_param_type type, void *out);

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
imgdnn_network_object imgdnnCreateNetworkObject_v2(const imgdnn_device device,
                                                   const imgdnn_context context,
                                                   const imgdnn_network network,
                                                   unsigned int num_inputs,
                                                   const imgdnn_tensor inputs[],
                                                   const imgdnn_data_layout_param inputs_layout[],
                                                   unsigned int num_outputs,
                                                   const imgdnn_tensor outputs[],
                                                   const imgdnn_data_layout_param outputs_layout[],
                                                   const imgdnn_network_object_flags flags,
                                                   const char *options,
                                                   imgdnn_err_code *errcode_ret);
#endif

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
imgdnn_network_object imgdnnCreateNetworkObject(const imgdnn_device device,
                                                const imgdnn_context context,
                                                const imgdnn_network network,
                                                unsigned int num_inputs,
                                                const imgdnn_tensor inputs[],
                                                unsigned int num_outputs,
                                                const imgdnn_tensor outputs[],
                                                const imgdnn_network_object_flags flags,
                                                const char *options,
                                                imgdnn_err_code *errcode_ret);


/**
 * Stores serialised compiled binary object
 */
typedef struct _imgdnn_network_binary_t {
    size_t size;
    void *data;
} imgdnn_network_binary;

#if IMGDNN_MINOR_VERSION >= 4
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
imgdnn_network_binary imgdnnCreateNetworkBinary_v2(const imgdnn_device device,
                                                   const imgdnn_context context,
                                                   const imgdnn_network network,
                                                   unsigned int num_inputs,
                                                   const imgdnn_tensor inputs[],
                                                   const imgdnn_data_layout_param inputs_layout[],
                                                   unsigned int num_outputs,
                                                   const imgdnn_tensor outputs[],
                                                   const imgdnn_data_layout_param outputs_layout[],
                                                   const imgdnn_network_object_flags flags,
                                                   const char *options,
                                                   imgdnn_err_code *errcode_ret);
#endif

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
imgdnn_network_binary imgdnnCreateNetworkBinary(const imgdnn_device device,
                                                const imgdnn_context context,
                                                const imgdnn_network network,
                                                unsigned int num_inputs,
                                                const imgdnn_tensor inputs[],
                                                unsigned int num_outputs,
                                                const imgdnn_tensor outputs[],
                                                const imgdnn_network_object_flags flags,
                                                const char *options,
                                                imgdnn_err_code *errcode_ret);


/**
 * Destroy compiled network binary.
 *
 * @Input  binary  The network binary object to destroy.
 * @Return IMGDNN_SUCCESS, IMGDNN_FAILURE or IMGDNN_INVALID_VALUE
 */
imgdnn_err_code imgdnnNetworkBinaryDestroy(imgdnn_network_binary *binary);

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
imgdnn_network_object imgdnnLoadNetworkObject(imgdnn_device device,
                                              imgdnn_context context,
                                              size_t size,
                                              const void* object_data,
                                              imgdnn_err_code *errcode_ret);

/**
 * Destroy a Network Object previously created.
 *
 * @Input  network_object  A network object created previously.
 * @Return IMGDNN_SUCCESS, IMGDNN_FAILURE,
 *         IMGDNN_INVALID_VALUE for invalid network object
 */
imgdnn_err_code imgdnnNetworkObjectDestroy(imgdnn_network_object network_object);

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
imgdnn_err_code imgdnnNetworkObjectGetInputs(const imgdnn_network_object network_object,
                                             unsigned int max_inputs,
                                             imgdnn_input inputs[],
                                             unsigned int *num_inputs);

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
imgdnn_err_code imgdnnNetworkObjectGetOutputs(const imgdnn_network_object network_object,
                                              unsigned int max_outputs,
                                              imgdnn_output outputs[],
                                              unsigned int *num_outputs);

/* Network Execution ************************************************************/

/**
 * Create a binding descriptor.
 * Effectively a map of memory to input or output objects.
 *
 * @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE or
 *                      IMGDNN_OUT_OF_MEMORY.
 * @Return A new binding descriptor or NULL on failure.
 */
imgdnn_binding imgdnnCreateBinding(imgdnn_err_code *errcode_ret);

/**
 * Destroy a binding descriptor.
 *
 * @Input  binding      The binding descriptor to destroy
 * @Output errcode_ret  IMGDNN_SUCCESS, IMGDNN_FAILURE,
 *                      IMGDNN_INVALID_VALUE for invalid descriptor.
 */
imgdnn_err_code imgdnnBindingDestroy(imgdnn_binding binding);

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
imgdnn_err_code imgdnnBindingAddInput(imgdnn_binding binding,
                                      imgdnn_input input,
                                      imgdnn_memory memory);

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
imgdnn_err_code imgdnnBindingAddInputSize(imgdnn_binding binding,
                                          imgdnn_input input,
                                          unsigned int dimension,
                                          size_t size);

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
imgdnn_err_code imgdnnBindingAddOutput(imgdnn_binding descriptor,
                                       imgdnn_output output,
                                       imgdnn_memory memory);

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
imgdnn_err_code imgdnnNetworkObjectExecute(const imgdnn_network_object network_object,
                                           const imgdnn_binding bindings,
                                           bool blocking_execute,
                                           unsigned int num_events_in_wait_list,
                                           const imgdnn_event event_wait_list[],
                                           imgdnn_event *event);

/**
 * Wait for a img dnn event.
 *
 * @Input  event  The event object on which to wait for.
 * @Return IMGDNN_SUCCESS, IMGDNN_FAILURE
 */
imgdnn_err_code imgdnnWaitForEvent(imgdnn_event event);

/**
 * Destroy a img dnn event object.
 *
 * @Input  event  The event object to release.
 * @Return IMGDNN_SUCCESS, IMGDNN_FAILURE
 */
imgdnn_err_code imgdnnEventDestroy(imgdnn_event event);

#if IMGDNN_MINOR_VERSION >= 8
/**
 * Enum to list supported sync types.
 */
typedef enum _imgdnn_external_sync_type_t
{
  IMGDNN_SYNC_FD  // external sync based on file descriptor, data type expected is: int.
} imgdnn_external_sync_type;

/**
 * Get external sync object of selected type from IMGDNN event.
 *
 * Caller takes ownership of the returned sync. For example, in case of IMGDNN_SYNC_FD - for closing
 * the file descriptor, when sync is not needed anymore.
 *
 * The source IMGDNN event retains original sync, caller gets independent (lifetime wise) copy
 * of the sync object (in case of IMGDNN_SYNC_FD via dup() call).
 *
 * @Input  event          The event object to get sync object from.
 * @Input  type           The type of expected sync object.
 * @Output external_sync  Requested external sync.
 *
 * @Return IMGDNN_SUCCESS, IMGDNN_FAILURE
 */
imgdnn_err_code imgdnnEventGetExternalSync(const imgdnn_event event,
                                           imgdnn_external_sync_type type,
                                           void* external_sync);

/**
 * Create IMGDNN event based on external sync of specific type.
 *
 * IMGDNN does not take ownership of provided external sync (however it should remain valid for
 * the duration of this call).
 *
 * Created IMGDNN event holds a copy of provided sync object (in case of IMGDNN_SYNC_FD
 * via dup() call), in order to have independent lifetime of the sync. Duplicated sync is closed
 * during mandatory call to imgdnnEventDestroy().
 *
 * @Input  type           The type of provided external sync object.
 * @Input  external_sync  Provided external sync.
 * @Output event          The IMGDNN event object based on provided sync.
 *
 * @Return IMGDNN_SUCCESS, IMGDNN_FAILURE
 */
imgdnn_err_code imgdnnCreateEventFromExternalSync(imgdnn_external_sync_type type,
                                                  void* external_sync,
                                                  imgdnn_event* event);
#endif

/**
 * Callback function typedef for IMGDNN error reporting functions. The callback function
 * should be made thread-safe by the user.
 *
 * @Input   flags               E.g. Information/warning/error/verbose/etc.
 * @Input   tensor_names        Array of tensor names where the error occurred.
 * @Input   num_tensor_names    Number of tensor names in tensor_names array
 * @Input   error_code          Error code for the error which caused the callback to be called.
 * @Input   error_message       Error message for the error which caused the callback to be called.
 */
typedef void (*PFN_imgdnn_debug_report_callback)(imgdnn_report_flags        flags,
                                                 const char**                     tensor_names,
                                                 int                       num_tensor_names,
                                                 imgdnn_err_code            error_code,
                                                 const char*                error_message);

/**
 * Set the error handling function. This fuction is called by the driver when
 * error occurs in the driver and will provide some information about the
 * natute of the error. See definition of PFN_imgdnn_debug_report_callack for
 * more detail.
 *
 * @Input   err_callback      The callback function to use.
 * @Return  IMGDNN_SUCCESS if the error handling function has been set successfully, else returns IMGDNN_FAILURE
 */
imgdnn_err_code imgdnnSetErrorHandler(PFN_imgdnn_debug_report_callback err_callback);

/**
 * Return the imgdnn API version.
 *
 * @Input  api_version    Pointer to the cstring that will hold the API version.
 * @Input  param_size     When api_version is not NULL this parameter is the size of the input string and it is used for
 *                        size checking. If api_version is NULL param_size is used to return the size of the version.
 * @Return IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE
 */
imgdnn_err_code  imgdnnGetApiVersion(char *api_version, size_t *param_size);

/**
 * Return the imgdnn version.
 *
 * @Input  driver_version  Pointer to the cstring with the imgdnn version.
 * @Input  param_size      When driver_version is not NULL this parameter is the size of the input string and it is used for
 *                         size checking. If driver_version is NULL param_size is used to return the size of the version.
 * @Return IMGDNN_SUCCESS, IMGDNN_INVALID_VALUE,
 *         IMGDNN_OUT_OF_MEMORY.
 */
imgdnn_err_code  imgdnnGetDriverVersion(char *driver_version, size_t *param_size);

#if IMGDNN_MINOR_VERSION >= 5
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
imgdnn_per_axis_quant_param *imgdnnCreatePerAxisQuantParam(unsigned axis,
                                                           unsigned count,
                                                           const float *scales,
                                                           const int *zero_points);

/**
 * Frees the memory used by the imgdnn_per_axis_quant_param structure and its internal arrays.
 *
 * @Input  pa_param parameters structure to destroy
 */
void imgdnnDestroyPerAxisQuantParam(imgdnn_per_axis_quant_param *pa_param);
#endif


/***** Get capabilities API ***********************************************************************/

/* Level of support for operations */
typedef enum _imgdnn_support_level_
{
  IMGDNN_UNSUPPORTED_OPERATION,
  IMGDNN_SUPPORTED_BY_HW,
  IMGDNN_SUPPORTED_BY_SW
} imgdnn_support_level;

/**
 * Structure to contain support levels for different operations and devices.
 * We do not replicate the list of devices, as the user will have created it prior to calling
 * one of the functions.
 * support_levels[d][j] indicates how operation j (in operation_names) is supported on device d
 */
struct _imgdnn_device_capabilities_t
{
  unsigned num_operations;
  unsigned num_devices;
  char** operation_names;
  imgdnn_support_level** support_levels;
};

#if IMGDNN_MINOR_VERSION >= 7
/**
 * Return support levels of the network's operations for the given devices.
 * If the network has been created from IR, the operation names will be the ones of the IR.
 * If it has not, the operation names will be created internally by IMGDNN.
 *
 * @Input network        Handle to the network
 * @Input num_devices    Number of devices to check capabilities for
 * @Input device_types   Array containing device types to check
 * @Input device_options Array containing device options (nullptr, HW config filepath, ...)
 * @Output errcode_ret   IMGDNN_SUCCESS or error code
 * @Return               Capabilities structure
 */
imgdnn_device_capabilities imgdnnGetDeviceCapabilities(imgdnn_network network,
                                                       unsigned num_devices,
                                                       const imgdnn_device_type* device_types,
                                                       const void** device_options,
                                                       imgdnn_err_code* err);

/* Destroy the given capabilities structure. */
imgdnn_err_code imgdnnDeviceCapabilitiesDestroy(imgdnn_device_capabilities capabilities);

#endif /* IMGDNN_MINOR_VERSION >= 7 */

#if IMGDNN_MINOR_VERSION >= 9
/**
 * Utility function to obtain the name of an input.
 *
 * @Input  input  An input previously created.
 * @Output err    IMGDNN_SUCCESS or IMGDNN_INVALID_VALUE for invalid tensor.
 * @Return        The given name on success, NULL on failure.
 */
const char *imgdnnGetInputName(imgdnn_input input, imgdnn_err_code* errcode_ret);

/**
 * Utility function to obtain the name of an output.
 *
 * @Input  output An output previously created.
 * @Output err    IMGDNN_SUCCESS or IMGDNN_INVALID_VALUE for invalid tensor.
 * @Return        The given name on success, NULL on failure.
 */
const char *imgdnnGetOutputName(imgdnn_output output, imgdnn_err_code* errcode_ret);

#endif /* IMGDNN_MINOR_VERSION >= 9 */

/** @} */
#ifdef __cplusplus
}
#endif
#endif /* _IMG_DNN_H */
