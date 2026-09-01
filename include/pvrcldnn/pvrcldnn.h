/*************************************************************************/ /*!
																			@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
																			@License        Strictly Confidential.
																			*/ /**************************************************************************/

#ifndef _PVR_CLDNN_H
#define _PVR_CLDNN_H

#include <stdint.h>
#include "CL/cl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _pvrcldnn_context_t* pvrcldnn_context;
typedef struct _pvrcldnn_input_t* pvrcldnn_input;
typedef struct _pvrcldnn_tensor_t* pvrcldnn_tensor;
typedef struct _pvrcldnn_graph_t* pvrcldnn_graph;
typedef struct _pvrcldnn_binding_descriptor_t* pvrcldnn_binding_descriptor;

typedef enum _pvrcldnn_flags_t
{
	PVRCLDNN_FLAGS_NONE = 0,
	PVRCLDNN_FLAGS_ALL = UINT32_MAX
} pvrcldnn_flags;

typedef enum _pvrcldnn_type_t
{
	PVRCLDNN_TYPE_I8,
	PVRCLDNN_TYPE_U8,
	PVRCLDNN_TYPE_I16,
	PVRCLDNN_TYPE_U16,
	PVRCLDNN_TYPE_I32,
	PVRCLDNN_TYPE_U32,
	PVRCLDNN_TYPE_F16,
	PVRCLDNN_TYPE_F32,
	PVRCLDNN_COMPLEX_TYPE_F16,
	PVRCLDNN_COMPLEX_TYPE_F32,
	PVRCLDNN_TYPE_MAX
} pvrcldnn_type;

#define _PVRCLDNN_DESCRIPTOR_MAX_DIM (6)
typedef struct _pvrcldnn_tensor_descriptor_t
{
	int dimensions;
	pvrcldnn_type type;
	// Will actually be of size 'dimensions'
	size_t size[_PVRCLDNN_DESCRIPTOR_MAX_DIM];
} pvrcldnn_tensor_descriptor;

typedef enum _pvrcldnn_operation_unary_t
{
	PVRCLDNN_OPERATION_SIGN,
	PVRCLDNN_OPERATION_SIGMOID,
	PVRCLDNN_OPERATION_SOFTMAX,
	PVRCLDNN_OPERATION_TANH,
	PVRCLDNN_OPERATION_RELU,
	PVRCLDNN_OPERATION_NEGATE,
	PVRCLDNN_OPERATION_ABS,
	PVRCLDNN_OPERATION_NOT,
	PVRCLDNN_OPERATION_LOG,
	PVRCLDNN_OPERATION_SQRT,
	PVRCLDNN_OPERATION_EXP
} pvrcldnn_operation_unary;

typedef enum _pvrcldnn_operation_binary_t
{
	PVRCLDNN_OPERATION_ADD,
	PVRCLDNN_OPERATION_SUB,
	PVRCLDNN_OPERATION_MUL,
	PVRCLDNN_OPERATION_DIV,
	PVRCLDNN_OPERATION_AND,
	PVRCLDNN_OPERATION_OR,
	PVRCLDNN_OPERATION_XOR,
	PVRCLDNN_OPERATION_MAX,
	PVRCLDNN_OPERATION_MIN,
	PVRCLDNN_OPERATION_MATMUL
} pvrcldnn_operation_binary;

typedef enum _pvrcldnn_pooling_type_t
{
	/* Not all values used, output_size = floor((input_size + 2 * pad_size - pool_size) / stride) + 1 */
	PVRCLDNN_POOLING_MAX,
	/* All values used, output_size = ceil((input_size + 2 * pad_size - pool_size) / stride) + 1 */
	PVRCLDNN_POOLING_MAX_WITH_PARTIAL,

	PVRCLDNN_POOLING_AVERAGE,
	PVRCLDNN_POOLING_MIN,
	PVRCLDNN_POOLING_L2
} pvrcldnn_pooling_type;

typedef enum _pvrcldnn_lrn_type_t
{
	PVRCLDNN_LRN_ACROSS,
	PVRCLDNN_LRN_WITHIN
} pvrcldnn_lrn_type;

/* General setup and teardown *************************************************/

/*! Create a PVRCLDNN context from a CL device.
 *
 * @Input  clcontext  The OpenCL context to create a PVRCLDNN context for.
 * @Input  device  The OpenCL device to create a PVRCLDNN context for.
 * @Input  flags  Flags to modify how the context behaves.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_CONTEXT, CL_INVALID_DEVICE,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 * @Return The returned PVRCLDNN context created or NULL on failure.
 */
pvrcldnn_context pvrcldnnContextCreate(cl_context clcontext, cl_device_id device, uint32_t flags, cl_int* errcode_ret);

/*! Destroy a previously created PVRCLDNN context.
 *
 * @Input  context  A previously created PVRCLDNN context
 * @Return CL_SUCCESS, CL_INVALID_CONTEXT, CL_OUT_OF_RESOURCES, or
 *         CL_OUT_OF_HOST_MEMORY.
 *! */
cl_int pvrcldnnContextDestroy(pvrcldnn_context context);

/* Graph preparation **********************************************************/

/* A tensor is an opaque memory region. It may correspond to actual memory, or
 * it may be elided in whole graph optimisation and hence not actually exist
 * when executing. In either case its use is to connect nodes together
 * logically.
 *
 * A tensor layout is described by a tensor descriptor which fully defines the
 * layout. This allows the user to access data in a cl_mem without necessarily
 * needing to pass through an opaque tensor. This is useful for custom kernels
 * in a graph.
 *
 * The layout of all tensors can be considered to be linearly ordered in row
 * major order and alignment of 16 bytes.
 */

/*! Utility function to get total size of a descriptor
 *
 * @Input  descriptor  The descriptor to find total size of
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_VALUE for invalid descriptor.
 *                      May be NULL.
 * @Return The total size of the data referenced by this descriptor.
 */
size_t pvrcldnnDescriptorTotalSize(const pvrcldnn_tensor_descriptor* descriptor, cl_int* errcode_ret);

/*! Create an input to the network.
 *
 * @Input  descriptor  The description of the input to create.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_VALUE for invalid descriptor,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 * @Return The created input or NULL on failure.
 */
pvrcldnn_input pvrcldnnInputCreate(const pvrcldnn_tensor_descriptor* descriptor, cl_int* errcode_ret);

/*! Destroy an input object.
 *
 * It is undefined behaviour for an input to be destroyed that is still used in
 * an output graph.
 *
 * @Input  input  An input previously created.
 * @Return CL_SUCCESS, CL_INVALID_MEM_OBJECT for an invalid input,
 *         CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY.
 */
cl_int pvrcldnnInputDestroy(pvrcldnn_input input);

/*! Create a tensor from an input.
 *
 * @Input  input  The input object to turn into a tensor.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid input,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 * @Return The created tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnTensorFromInput(pvrcldnn_input input, cl_int* errcode_ret);

/*! Create a tensor with fixed data.
 *
 * @Input  descriptor  The description of the tensor to create.
 * @Input  initial_size  Size in bytes of data in initial.
 * @Input  initial  The data to use for the tensor. This data is fixed and will
 *                  likely be optimised by the implementation. May not be NULL.
 *                  Must stay valid until the graph is created.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_VALUE for invalid descriptor,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 * @Return The created tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnFixedTensorCreate(const pvrcldnn_tensor_descriptor* descriptor, size_t initial_size, const void* initial, cl_int* errcode_ret);

/*! Destroy a tensor created directly, or through adding a node.
 *
 * It is undefined if a tensor is destroyed that is still used in an output
 * graph.
 *
 * @Input  tensor  A tensor previously created.
 * @Return CL_SUCCESS, CL_INVALID_MEM_OBJECT for an invalid tensor,
 *         CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY.
 */
cl_int pvrcldnnTensorDestroy(pvrcldnn_tensor tensor);

/*! Reshape a tensor to a different layout.
 *
 * @Input  tensor  The input tensor to reshape. May not be NULL.
 * @Input  descriptor  The tensor descriptor of the new tensor. Must contain the
 *                     same number of elements as the input tensor. May not be
 *                     NULL.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid tensor,
 *                      CL_INVALID_VALUE for invalid descriptor,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 * @Return The output tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnTensorReshape(pvrcldnn_tensor tensor, const pvrcldnn_tensor_descriptor* descriptor, cl_int* errcode_ret);

/*! Perform data-type cast operation.
 *
 * @Input  in_tensor   The input data to cast. May not be NULL.
 * @Input  dst_type    pvrcldnn_type to cast to
 * @Output errcode_ret CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid in_tensor,
 *                     CL_INVALID_VALUE for invalid dst_type,
 *                     CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                     NULL.
 * @Return The output tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnTensorCast(pvrcldnn_tensor tensor, const pvrcldnn_type dst_type, cl_int* errcode_ret);

/*! Broadcast a tensor along a new dimension.
 *
 * e.g.
 * tensor=[1], dim=0, size=2 => [[1], [1]]
 * tensor=[1], dim=1, size=3 => [[1, 1, 1]]
 * tensor=[[1,2],[3,4],[5,6]], dim=0, size=2 => [[[1,2],[3,4],[5,6]], [[1,2],[3,4],[5,6]]]
 * tensor=[[1,2],[3,4],[5,6]], dim=2, size=2 => [[[1,1],[2,2]],[[3,3],[4,4]],[[5,5],[6,6]]]
 * tensor=1, dim=0, size=5 => [1,1,1,1,1]
 *
 * @Input  input  The input tensor to broadcast. May not be NULL.
 * @Input  dimension  The dimension of the new broadcasted dimension in the
 * resulting tensor.
 * @Input  size  The size of the newly broadcast dimension.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid tensor,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 * @Return The output tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnTensorBroadcast(pvrcldnn_tensor tensor, int dimension, size_t size, cl_int* errcode_ret);

/*! Obtain a subset of a tensor.
 *
 * start, end and stride must have the same length as number of dimensions in
 * input tensor. start and end are inclusive.
 * @Input  tensor  The input tensor. May not be NULL.
 * @Input  start  The start of the sub-tensor in each dimension.
 * @Input  end  The end of the sub-tensor in each dimension.
 * @Input  stride  The stride of the sub-tensor in each dimension.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid tensor,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 * @Return The sub-tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnSubTensor(pvrcldnn_tensor tensor, size_t start[], size_t end[], size_t stride[], cl_int* errcode_ret);

/*! Join tensors
 *
 * Splice tensor2 into tensor1 along one dimension. Place initial element of
 * tensor2 at start[] index of tensor1, shifting tensor1 values higher by one.
 * Skip stride[] elements and place the next element of tensor2, etc. Extra
 * elements are appended.
 * Dimensions other than join dimension must have the same sizes.
 * e.g.
 * tensor1=[0,1,2,3,4,5,6,7,8], tensor2=[10,11,12,13], dim=0, start=4, stride=2 => [0,1,2,3,10,4,5,11,6,7,12,8,13]
 * tensor1=[[0,1,2],[3,4,5]], tensor2=[[6,7],[8,9]], dim=1, start=2, stride=0 => [[0,1,6,7,2],[3,4,8,9,5]]
 * @Input  tensor1  First tensor. May not be NULL
 * @Input  tensor2  Second tensor. Must have same number of dimensions as
 * tensor1. May not be NULL
 * @Input  dimension  The dimension to join along.
 * @Input  start  The starting element to place tensor2
 * @Input  stride  The stride of splicing in tensor2 (tensor1 elements between
 * tensor2 elements).
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid tensor,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 * @Return The joined tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnJoinTensor(pvrcldnn_tensor tensor1, pvrcldnn_tensor tensor2, int dimension, size_t start, size_t stride, cl_int* errcode_ret);

/*! Obtains the descriptor of a tensor.
 *
 * @Input  tensor  A tensor previously created.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_VALUE for invalid descriptor.
 *                      May be NULL.
 * @Return The descriptor of the given tensor.
 */
pvrcldnn_tensor_descriptor pvrcldnnTensorDescriptor(pvrcldnn_tensor tensor, cl_int* errcode_ret);

/*! Create a unary operation node
 *
 * @Input  in_tensor  The input tensor to operate on
 * @Input  operation  The operation to perform on the input tensor.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid tensor,
 *                      CL_INVALID_OPERATION, CL_OUT_OF_RESOURCES or
 *                      CL_OUT_OF_HOST_MEMORY. May be NULL.
 * @Return The output tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnNodeAddUnary(pvrcldnn_tensor in_tensor, pvrcldnn_operation_unary operation, cl_int* errcode_ret);

/*! Create a binary operation node
 *
 * @Input  in_tensor1  The LHS input tensor to operate on
 * @Input  in_tensor2  The RHS input tensor to operate on
 * @Input  operation  The operation to perform on the input tensors.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid tensor,
 *                      CL_INVALID_OPERATION, CL_OUT_OF_RESOURCES or
 *                      CL_OUT_OF_HOST_MEMORY. May be NULL.
 * @Return The output tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnNodeAddBinary(pvrcldnn_tensor in_tensor1, pvrcldnn_tensor in_tensor2, pvrcldnn_operation_binary operation, cl_int* errcode_ret);

/*! Perform a 2D convolution
 *
 * @Input  in_tensor  The input data to convolve. 4D: [N, C, H, W]
 * @Input  filter  The filter to use. 4D: [Ci, Co, H, W]
 * @Input  stride  The stride in each dimension. 2D: [H, W]
 * @Input  pad  The padding in each dimension. 2D: [H, W]
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid in_tensor or
 *                      filter, CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY.
 *                      May be NULL.
 * @Return The output tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnNodeAddConvolution2d(pvrcldnn_tensor in_tensor, pvrcldnn_tensor filter, cl_uint stride[2], cl_uint pad[2], cl_int* errcode_ret);

/*! Perform pooling
 *
 * @Input  in_tensor  The input data to pool.
 * @Input  size  The size of pooling in each dimension. Must have the same
 *               dimensions as in_tensor.
 * @Input  stride  The pooling stride in each dimension. Must have the same
 *                 dimensions as in_tensor.
 * @Input  pad  The padding in each dimension. Must have the same dimensions as
 *				in_tensor.
 * @Input  type  The type of pooling to perform.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid in_tensor,
 *                      CL_INVALID_VALUE for other invalid argument,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 * @Return The output tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnNodeAddPooling(pvrcldnn_tensor in_tensor, cl_uint size[], cl_uint stride[], cl_uint pad[], pvrcldnn_pooling_type type, cl_int* errcode_ret);

/*! Perform local response normalisation.
 *
 * @Input  in_tensor   The input data to normalise. 4-dimensions: (N, C, H, W)
 * @Input  lrn_type    across/within channel
 * @Input  local_size  Number of channels to sum over (for cross channel) or
 *                     the side length of the square region to sum over (for
 *                     within channel)
 * @Input  alpha       Scaling Parameter
 * @Input  beta        Exponent
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_MEM_OBJECT for invalid in_tensor,
 *                      CL_INVALID_VALUE for invalid window_size or lrn_type,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 * @Return The output tensor or NULL on failure.
 */
pvrcldnn_tensor pvrcldnnNodeAddLRN(pvrcldnn_tensor in_tensor, pvrcldnn_lrn_type type, size_t window_size, float alpha, float beta, cl_int* errcode_ret);

/*! Create an output point from the graph.
 *
 * Produces a graph used for execution.
 *
 * The difference between using multiple outputs here, or multiple calls with
 * single outputs is the graph optimisation. A tensor that is not an output
 * may be elided completely.
 *
 * @Input  context  The PVRCLDNN context in which to create this graph output.
 * @Input  num_output  The number of elements in the tensor array.
 * @Input  tensor  Array of tensors to create graph outputs from.
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_VALUE for invalid tensor,
 *                      CL_BUILD_PROGRAM_FAILURE, CL_OUT_OF_RESOURCES or
 *                      CL_OUT_OF_HOST_MEMORY. May be NULL.
 * @Return The output graph node for execution or NULL on failure.
 */
pvrcldnn_graph pvrcldnnGraphCreate(pvrcldnn_context context, cl_uint num_output, pvrcldnn_tensor* tensor, cl_int* errcode_ret);

/*! Destroy an output graph.
 *
 * @Input  graph  A graph output created previously.
 * @Return CL_SUCCESS, CL_INVALID_KERNEL for invalid graph,
 *         CL_OUT_OF_RESOURCES, or CL_OUT_OF_HOST_MEMORY.
 */
cl_int pvrcldnnGraphDestroy(pvrcldnn_graph graph);

/* Graph execution ************************************************************/

/*! Create a binding descriptor.
 * Effectively a map of cl_mem to input objects.
 *
 * @Output errcode_ret  CL_SUCCESS, CL_OUT_OF_RESOURCES or
 *                      CL_OUT_OF_HOST_MEMORY. May be NULL.
 * @Return A new binding descriptor or NULL on failure.
 */
pvrcldnn_binding_descriptor pvrcldnnBindingDescriptorCreate(cl_int* errcode_ret);

/*! Destroy a binding descriptor.
 *
 * @Input  binding  The binding descriptor to destroy
 * @Output errcode_ret  CL_SUCCESS, CL_INVALID_VALID for invalid descriptor,
 *                      CL_OUT_OF_RESOURCES or CL_OUT_OF_HOST_MEMORY. May be
 *                      NULL.
 */
cl_int pvrcldnnBindingDescriptorDestroy(pvrcldnn_binding_descriptor binding);

/*! Add a binding to a descriptor.
 * If a binding already exists for this input in the descriptor, then it is
 * updated. The same cl_mem may be used for multiple inputs.
 *
 * @Input  descriptor  The binding descriptor to add to.
 * @Input  input  The input object to bind memory to.
 * @Input  memory  The memory to bind to a tensor.
 * @Return CL_SUCCESS, CL_INVALID_KERNEL_ARGS for invalid descriptor,
 *         CL_INVALID_VALUE for invalid input,
 *         CL_INVALID_MEM_OBJECT for invalid memory,
 *         CL_OUT_OF_RESOURCES, or CL_OUT_OF_HOST_MEMORY.
 */
cl_int pvrcldnnBindingAdd(pvrcldnn_binding_descriptor descriptor, pvrcldnn_input input, cl_mem memory);

/*! Enqueue the graph with the given bindings.
 *
 * When the graph is executed, the event object follows the graph's execution
 * and when complete the given cl_mem here contains the output tensor. It may be
 * required to map/read the cl_mem to access it from the host depending on how
 * it is allocated.
 *
 * A binding descriptor is allowed to have bindings for inputs that do not exist
 * in the graph.
 *
 * After this, the user still will need to flush and finish the appropriate
 * command queue.
 *
 * @Input  queue  The command queue to enqueue this graph. Must be for the
 *                same device as the context that created this graph. May be
 *                NULL if the given cl_device_id to the PVRCLDNN context was
 *                NULL.
 * @Input  graph  The graph to execute.
 * @Input  bindings  The bindings to make to the input objects in the graph.
 * @Input  mem  Array of cl_mem objects to use as output for the respective
 *              tensor. Must have the same length as the list of tensors passed
 *              to graph creation. Must be created previously by the user
 *              through the OpenCL API. May not be NULL.
 * @Input  wait_event  The event object to wait on before this graph can be
 *                     executed. May be NULL.
 * @Output out_event  The event object that represents the status of this enqueue.
 *                    May be NULL.
 * @Return CL_SUCCESS, CL_INVALID_COMMAND_QUEUE for an invalid queue,
 *         CL_INVALID_KERNEL for invalid graph,
 *         CL_INVALID_KERNEL_ARGS for invalid binding descriptor,
 *         CL_INVALID_EVENT for invalid wait_event,
 *         CL_OUT_OF_RESOURCES, or CL_OUT_OF_HOST_MEMORY.
 */
cl_int pvrcldnnGraphEnqueue(cl_command_queue queue, pvrcldnn_graph graph, pvrcldnn_binding_descriptor bindings, cl_mem* mem, cl_event wait_event, cl_event* out_event);

#ifdef __cplusplus
}
#endif

#endif /* _PVR_CLDNN_H */
