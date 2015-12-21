/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\GraphicsStateCreateParam.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief			Contains internal classes used by the pipeline
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjectTypes.h"
namespace pvr {
namespace api {
namespace impl {
//!\cond NO_DOXYGEN
namespace GraphicsStateType {
enum Enum
{
	ShaderProgram, VertexShader, FragmentShader, GeometryShader, TessellationControlShader,
	TessellationEvaluationShader, DepthTest, DepthClear, DepthWrite, PolygonCulling, PolygonWindingOrder, BlendRgba, BlendTest, PolygonFill,
	ScissorTest, StencilOpFront, StencilOpBack, FrameBufferClear, FrameBufferWrite, DepthFunc, BlendEq, StencilTest, StencilClear,
	VertexAttributeFormatState, VertexAttributeLocation, Count
};
};
//!\endcond

// Forward declaration
class GraphicsPipelineImpl;
class GraphicsPipelineImplState;
}//namespace impl

/*!****************************************************************************************************************
\brief Contains a full description of a Vertex Attribute: Index, format, number of elements, offset in the buffer,
       optionally name. All values (except attributeName) must be set explicitly.
*******************************************************************************************************************/
struct VertexAttributeInfo
{
	pvr::uint16 index;			//!< Attribute index
	pvr::DataType::Enum format;	//!< Data type of each element of the attribute
	pvr::uint8 width;			//!< Number of elements in attribute, e.g 1,2,3,4
	pvr::uint32 offsetInBytes;	//!< Offset of the first element in the buffer
	std::string attribName;		//!< Optional: Name(in the shader) of the attribute

	/*!*********************************************************************************************************
	\brief Default  constructor. Uninitialized values, except for AttributeName.
	************************************************************************************************************/
	VertexAttributeInfo(): index(0), format(DataType::None), width(0), offsetInBytes(0), attribName("") {}

	/*!*********************************************************************************************************
	\brief Create a new VertexAttributeInfo object.
	\param index Attribute binding index
	\param format Attribute data type
	\param width Number of elements in attribute
	\param offsetInBytes Interleaved: offset of the attribute from the start of data of each vertex
	\param attribName Name of the attribute in the shader.
	************************************************************************************************************/
	VertexAttributeInfo(pvr::uint16 index, pvr::DataType::Enum format, pvr::uint8 width,
	                    pvr::uint32 offsetInBytes, const char* attribName = "") :
		index(index), format(format), width(width), offsetInBytes(offsetInBytes),attribName(attribName) {}

	bool operator==(VertexAttributeInfo const& rhs)const
	{
		return ((index == rhs.index) && (format == rhs.format) &&
		        (width == rhs.width) && (offsetInBytes == rhs.offsetInBytes));
	}
	bool operator!=(VertexAttributeInfo const& rhs)const {	return !((*this) == rhs);  }
};

/*!****************************************************************************************************************
\brief Information about a Buffer binding: Binding index, stride, (instance) step rate.
*******************************************************************************************************************/
struct VertexInputBindingInfo
{
	pvr::uint16 bindingId;//< buffer binding index
	pvr::uint32 strideInBytes; //< buffer stride in bytes
	StepRate::Enum stepRate;//< buffer step rate

	/*!*********************************************************************************************************
	\brief Construct with Uninitialised values.
	************************************************************************************************************/
	VertexInputBindingInfo() {}

	/*!*********************************************************************************************************
	\brief  Add a buffer binding.
	\param[in] bindId Buffer binding point
	\param[in] strideInBytes Buffer stride of each vertex attribute to the next
	\param[in] stepRate Vertex Attribute Step Rate
	************************************************************************************************************/
	VertexInputBindingInfo(pvr::uint16 bindId, pvr::uint32 strideInBytes, StepRate::Enum stepRate = StepRate::Vertex) :
		bindingId(bindId), strideInBytes(strideInBytes), stepRate(stepRate) {}
};

}
}
