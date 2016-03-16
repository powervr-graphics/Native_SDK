/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\PipelineConfig.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the CreateParameters for the Pipeline Config States, used to set states to the PipelineCreateParam objects.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRCore/SortedArray.h"
#include <map>
#include <vector>
namespace pvr {
namespace api {

/*!****************************************************************************************************************
\brief Contains a full description of a Vertex Attribute: Index, format, number of elements, offset in the buffer,
       optionally name. All values (except attributeName) must be set explicitly.
*******************************************************************************************************************/
struct VertexAttributeInfo
{
	pvr::uint16 index;			//!< Attribute index
	types::DataType::Enum format;	//!< Data type of each element of the attribute
	pvr::uint8 width;			//!< Number of elements in attribute, e.g 1,2,3,4
	pvr::uint32 offsetInBytes;	//!< Offset of the first element in the buffer
	std::string attribName;		//!< Optional: Name(in the shader) of the attribute

	/*!*********************************************************************************************************
	\brief Default  constructor. Uninitialized values, except for AttributeName.
	************************************************************************************************************/
	VertexAttributeInfo(): index(0), format(types::DataType::None), width(0), offsetInBytes(0), attribName("") {}

	/*!*********************************************************************************************************
	\brief Create a new VertexAttributeInfo object.
	\param index Attribute binding index
	\param format Attribute data type
	\param width Number of elements in attribute
	\param offsetInBytes Interleaved: offset of the attribute from the start of data of each vertex
	\param attribName Name of the attribute in the shader.
	************************************************************************************************************/
	VertexAttributeInfo(pvr::uint16 index, types::DataType::Enum format, pvr::uint8 width,
	                    pvr::uint32 offsetInBytes, const char* attribName = "") :
		index(index), format(format), width(width), offsetInBytes(offsetInBytes), attribName(attribName) {}

	/*!*********************************************************************************************************************
    \brief Return true if the right hand object is equal to this
    ***********************************************************************************************************************/
	bool operator==(VertexAttributeInfo const& rhs)const
	{
		return ((index == rhs.index) && (format == rhs.format) &&
		        (width == rhs.width) && (offsetInBytes == rhs.offsetInBytes));
	}
	
	/*!*********************************************************************************************************************
    \brief Return true if the right hand object is not equal to this
    ***********************************************************************************************************************/
	bool operator!=(VertexAttributeInfo const& rhs)const {	return !((*this) == rhs);  }

};

/*!****************************************************************************************************************
\brief Information about a Buffer binding: Binding index, stride, (instance) step rate.
*******************************************************************************************************************/
struct VertexInputBindingInfo
{
	pvr::uint16 bindingId;//< buffer binding index
	pvr::uint32 strideInBytes; //< buffer stride in bytes
	types::StepRate::Enum stepRate;//< buffer step rate

	/*!*********************************************************************************************************
	\brief Construct with Uninitialized values.
	************************************************************************************************************/
	VertexInputBindingInfo() {}

	/*!*********************************************************************************************************
	\brief  Add a buffer binding.
	\param[in] bindId Buffer binding point
	\param[in] strideInBytes Buffer stride of each vertex attribute to the next
	\param[in] stepRate Vertex Attribute Step Rate
	************************************************************************************************************/
	VertexInputBindingInfo(pvr::uint16 bindId, pvr::uint32 strideInBytes, types::StepRate::Enum stepRate = types::StepRate::Vertex) :
		bindingId(bindId), strideInBytes(strideInBytes), stepRate(stepRate) {}
};

struct VertexAttributeInfoWithBinding : public VertexAttributeInfo
{
	uint16 binding;
	VertexAttributeInfoWithBinding() {}
	VertexAttributeInfoWithBinding(const VertexAttributeInfo& nfo, uint16 binding) : VertexAttributeInfo(nfo), binding(binding) {}
	VertexAttributeInfoWithBinding(pvr::uint16 index, types::DataType::Enum format, pvr::uint8 width, pvr::uint32 offsetInBytes, uint16 binding, const char* attribName = "") :
		VertexAttributeInfo(index, format, width, offsetInBytes, attribName), binding(binding) {}
};

struct VertexAttributeInfoPred_IndexEquals
{
	VertexAttributeInfoPred_IndexEquals(uint16 attributeIndex) : attributeIndex(attributeIndex) {}
	uint16 attributeIndex;
	bool operator()(const VertexAttributeInfo& nfo) const
	{
		return nfo.index == attributeIndex;
	}
};

struct VertexAttributeInfoCmp_IndexLess
{
	bool operator()(const VertexAttributeInfo& lhs, const VertexAttributeInfo& rhs) const
	{
		return lhs.index < rhs.index;
	}
};
struct VertexAttributeInfoCmp_BindingLess_IndexLess
{
	bool operator()(const VertexAttributeInfoWithBinding& lhs, const VertexAttributeInfoWithBinding& rhs) const
	{
		return lhs.binding < rhs.binding || (lhs.binding == rhs.binding && lhs.index < rhs.index);
	}
};

struct VertexAttributeInfoPred_BindingEquals
{
	uint16 binding;
	VertexAttributeInfoPred_BindingEquals(uint16 binding) : binding(binding) {}
	bool operator()(const VertexAttributeInfoWithBinding& nfo) const
	{
		return nfo.binding == binding;
	}
};

struct VertexBindingInfoCmp_BindingLess
{
	bool operator()(const VertexInputBindingInfo& lhs, const VertexInputBindingInfo& rhs) const
	{
		return lhs.bindingId < rhs.bindingId;
	}
};
struct VertexBindingInfoPred_BindingLess
{
	bool operator()(uint16 lhs, const VertexInputBindingInfo& rhs) const
	{
		return lhs < rhs.bindingId;
	}
};

struct VertexBindingInfoPred_BindingEqual
{
	uint16 binding;
	VertexBindingInfoPred_BindingEqual(uint16 binding) : binding(binding) {}
	bool operator()(const VertexInputBindingInfo& nfo) const
	{
		return nfo.bindingId == binding;
	}
};

/*!*********************************************************************************************************************
\brief Viewport specifes the drawing region, min and  max depth 
***********************************************************************************************************************/
struct Viewport
{
	float32 x;//!< region x
	float32 y;//!< region y
	float32 width;//!< region width
	float32 height;//!< region height
	float32 minDepth;//!< min depth
	float32 maxDepth;//!< max depth
	
	/*!*********************************************************************************************************************
	\brief ctor
	\param x	viewport x
	\param y	viewport y
	\param width viewport width
	\param height viewport height
	\param minDepth depth min
	\param maxDepth depth max
	***********************************************************************************************************************/
	Viewport(pvr::float32 x = 0, pvr::float32 y = 0, pvr::float32 width = 0, pvr::float32 height = 0, pvr::float32 minDepth = 0.0f,
			 pvr::float32 maxDepth = 1.f) :
		x(x), y(y), width(width), height(height), minDepth(minDepth), maxDepth(maxDepth) {}

	/*!*********************************************************************************************************************
	\brief ctor
	\param rect	viewport
	\param minDepth depth min
	\param maxDepth depth max
	***********************************************************************************************************************/
	Viewport(const Rectanglei& rect, pvr::float32 minDepth = 0.0f, pvr::float32 maxDepth = 1.f) :
		x((float32)rect.x), y((float32)rect.y), width((float32)rect.width), height((float32)rect.height), minDepth(minDepth), maxDepth(maxDepth) {}
};

typedef std::vector<VertexInputBindingInfo> VertexInputBindingMap;// map buffer binding -> VertexAttributes
typedef std::vector<VertexAttributeInfoWithBinding>	VertexAttributeMap;

/*!*********************************************************************************************************************
\brief Pipeline Stencil state
***********************************************************************************************************************/
struct StencilState
{
	types::StencilOp::Enum opDepthPass;//!< Action performed on samples that pass both the depth and stencil tests.
	types::StencilOp::Enum opDepthFail;//!< Action performed on samples that pass the stencil test and fail the depth test.
	types::StencilOp::Enum opStencilFail;//!< Action performed on samples that fail the stencil test.
	uint32 compareMask;//!< Selects the bits of the unsigned integer stencil values during in the stencil test.
	uint32 writeMask;//!<  Selects the bits of the unsigned integer stencil values updated by the stencil test in the stencil framebuffer attachment.
	uint32 reference;//!< Integer reference value that is used in the unsigned stencil comparison.
	types::ComparisonMode::Enum compareOp;//!<  Comparison operator used in the stencil test.
	
	/*!*********************************************************************************************************************
	\brief ctor
	\param depthPass Action performed on samples that pass both the depth and stencil tests.
	\param depthFail Action performed on samples that pass the stencil test and fail the depth test.
	\param stencilFail Action performed on samples that fail the stencil test.
	\param compareOp Comparison operator used in the stencil test.
	\param compareMask Selects the bits of the unsigned integer stencil values during in the stencil test.
	\param 	writeMask Selects the bits of the unsigned integer stencil values updated by the stencil test in the stencil framebuffer attachment
	\param 	reference Integer reference value that is used in the unsigned stencil comparison.
	***********************************************************************************************************************/
	StencilState(types::StencilOp::Enum depthPass = types::StencilOp::Keep,
	             types::StencilOp::Enum depthFail = types::StencilOp::Keep,
	             types::StencilOp::Enum stencilFail = types::StencilOp::Keep,
	             types::ComparisonMode::Enum compareOp = types::ComparisonMode::Default,
	             uint32 compareMask = 0xff, uint32 writeMask = 0xff, uint32 reference = 0)
		: opDepthPass(depthPass), opDepthFail(depthFail),
		  opStencilFail(stencilFail), compareMask(compareMask),
		  writeMask(writeMask), reference(reference), compareOp(compareOp) {}
};

namespace pipelineCreation {
/*!********************************************************************************************
\brief  Contains parameters needed to set depth stencil states to a pipeline create params.
        This object can be added to a PipelineCreateParam to set a depth-stencil state to
		values other than their defaults.
\description --- Defaults: ---
        depthWrite:enabled,    depthTest:enabled,    DepthComparison:Less,
        Stencil Text: disabled,    All stencil ops:Keep,
***********************************************************************************************/
struct DepthStencilStateCreateParam
{
public:
	typedef ::pvr::api::StencilState StencilState;
private:
	// stencil
	bool depthTest;  //!< Enable/disable depth test. Default false
	bool depthWrite; //!< Enable/ disable depth write
	bool stencilTestEnable; //!< Enable/disable stencil test. Default false
	bool depthBoundTest;//!< Enable/ disable depth bound test
	float32 minDepth;//!< Depth minimum
	float32 maxDepth;//!< Depth maximum

	StencilState stencilFront;//!< Stencil state front
	StencilState stencilBack;//!< Stencil state back

	types::ComparisonMode::Enum depthCmpOp; //!< Depth compare operation. Default LESS
public:
	/*!*********************************************************************************************************************
	\brief  Set all Depth and Stencil parameters.
	***********************************************************************************************************************/
	DepthStencilStateCreateParam(bool depthWrite = true, bool depthTest = true,
	                             types::ComparisonMode::Enum depthCompareFunc = types::ComparisonMode::Less,
	                             bool stencilTest = false, bool depthBoundTest = false,
	                             const StencilState& stencilFront = StencilState(),
	                             const StencilState& stencilBack = StencilState(),
	                             float32 minDepth = 0.0f,
	                             float32 maxDepth = 1.0f) :
		depthTest(depthTest), depthWrite(depthWrite), stencilTestEnable(stencilTest),
		depthBoundTest(depthBoundTest), minDepth(minDepth), maxDepth(maxDepth),
		stencilFront(stencilFront), stencilBack(stencilBack), depthCmpOp(depthCompareFunc) {}

	/*!*********************************************************************************************************************
	\brief  Return true if depth test is enable
	***********************************************************************************************************************/	
	bool isDepthTestEnable()const { return depthTest; }
	
	/*!*********************************************************************************************************************
	\brief  Return true if depth write is enable
	***********************************************************************************************************************/
	bool isDepthWriteEnable()const { return depthWrite; }
	
	/*!*********************************************************************************************************************
	\brief  Return true if depth bound is enable
	***********************************************************************************************************************/
	bool isDepthBoundTestEnable()const { return depthBoundTest; }
	
	/*!*********************************************************************************************************************
	\brief  Return true if stencil test is enable
	***********************************************************************************************************************/
	bool isStencilTestEnable()const { return stencilTestEnable; }
	
	/*!*********************************************************************************************************************
	\brief  Return minimum depth value
	***********************************************************************************************************************/
	float32 getMinDepth()const { return minDepth; }
	
	/*!*********************************************************************************************************************
	\brief  Return maximum depth value
	***********************************************************************************************************************/
	float32 getMaxDepth()const { return maxDepth; }
	
	/*!*********************************************************************************************************************
	\brief  Return depth comparison operator
	***********************************************************************************************************************/
	types::ComparisonMode::Enum getDepthComapreOp()const { return depthCmpOp; }
	
	/*!*********************************************************************************************************************
	\brief Enable/disable writing into the Depth Buffer.
	\param depthWrite True:enable, False:disable
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	DepthStencilStateCreateParam& setDepthWrite(bool depthWrite)
	{
		this->depthWrite = depthWrite;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/disable depth test (initial state: enabled)
	\param depthTest True:enable, False:disable
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	DepthStencilStateCreateParam& setDepthTestEnable(bool depthTest)
	{
		this->depthTest = depthTest;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the depth compare function (initial state: LessEqual)
	\param compareFunc A ComparisonMode (Less, Greater, Less etc.)
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	DepthStencilStateCreateParam& setDepthCompareFunc(types::ComparisonMode::Enum compareFunc)
	{
		this->depthCmpOp = compareFunc;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/disable stencil test.
	\param stencilTest True:enable, False:disable
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilTest(bool stencilTest){	this->stencilTestEnable = stencilTest;	return *this; }

	/*!****************************************************************************************************************
	\brief	Set the stencil front state
	\return this object (allows chained calls)
	\param	stencil Stencil state
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilFront(StencilState& stencil){	stencilFront = stencil;	return *this;	}

	/*!****************************************************************************************************************
	\brief	Set the stencil back state
	\return this object (allows chained calls)
	\param	stencil Stencil state
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilBack(StencilState& stencil){	stencilBack = stencil;	return *this;	}

	/*!****************************************************************************************************************
	\brief	Set the stencil front and back state
	\return this object (allows chained calls)
	\param	stencil Stencil state
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilFrontBack(StencilState& stencil)
	{
		stencilFront = stencil, stencilBack = stencil;	return *this;
	}

	/*!****************************************************************************************************************
	\brief	Return stencil front state
	*******************************************************************************************************************/
	const StencilState& getStencilFront()const	{ return stencilFront; }

	/*!****************************************************************************************************************
	\brief	Return stencil back state
	*******************************************************************************************************************/
	const StencilState& getStencilBack()const{	return stencilBack;	}
};

/*!********************************************************************************************************************
\brief  Contains parameters needed to configure the Vertex Input for a pipeline object.
        (vertex attrubutes, input bindings etc.). Use by adding the buffer bindings with (setInputBinding) and
		then configure the attributes with (addVertexAttribute).
		Default settings: 0 Vertext buffers, 0 vertex attributes.
**********************************************************************************************************************/
struct VertexInputCreateParam
{
private:
	friend class ::pvr::api::impl::GraphicsPipeline_;

	VertexInputBindingMap inputBindings;
	VertexAttributeMap attributes;
public:

	/*!********************************************************************************************************************
	\brief Return the input bindings
	**********************************************************************************************************************/
	const VertexInputBindingMap& getInputBindings() const { return inputBindings; }
	
	/*!********************************************************************************************************************
	\brief Return the vertex attributes
	**********************************************************************************************************************/
	const VertexAttributeMap& getAttributes() const { return attributes; }

	/*!********************************************************************************************************************
	\brief Clear this object.
	\return this object (allows chained calls)
	**********************************************************************************************************************/
	VertexInputCreateParam& clear(){	inputBindings.clear();	attributes.clear();	return *this;	}

	/*!****************************************************************************************************************
	\brief	Set the vertex input buffer bindings.
	\return this object (allows chained calls)
	\param	bufferBinding Vertex buffer binding index
	\param	strideInBytes specifies the byte offset between consecutive generic vertex attributes.
			If stride is 0, the generic vertex attributes are understood to be tightly packed in the array.
			The initial value is 0.
	\param	stepRate The rate at which this binding is incremented (used for Instancing).
	*******************************************************************************************************************/
	VertexInputCreateParam& setInputBinding(pvr::uint16 bufferBinding, pvr::uint16 strideInBytes = 0,
	                                        types::StepRate::Enum stepRate = types::StepRate::Vertex)
	{
		pvr::utils::insertSorted_overwrite(inputBindings, VertexInputBindingInfo(bufferBinding, strideInBytes, stepRate), VertexBindingInfoCmp_BindingLess());
		return *this;
	}

	/*!****************************************************************************************************************
	\brief Add vertex layout information to a buffer binding index using a VertexAttributeInfo object.
	\param bufferBinding The binding index to add the vertex attribute information.
	\param attrib Vertex Attribute information object.
	\return this object (allows chained calls)
	*******************************************************************************************************************/
	VertexInputCreateParam& addVertexAttribute(pvr::uint16 bufferBinding, const VertexAttributeInfo& attrib)
	{
		pvr::utils::insertSorted_overwrite(attributes, VertexAttributeInfoWithBinding(attrib, bufferBinding), VertexAttributeInfoCmp_BindingLess_IndexLess());
		return *this;
	}

	/*!****************************************************************************************************************
	\brief Add vertex layout information to a buffer binding index using a VertexAttributeLayout object and an attrib name.
	\param index The index of the vertex attribute
	\param bufferBinding The binding index of the buffer from which vertex data will be read.
	\param layout Vertex Attribute Layout object
	\param attributeName The name of the variable in shader code. Required for API's that only support Reflective attribute
	       binding and not Explicit binding of attributes to indexes in shader code.
	\return this object (allows chained calls)
	*******************************************************************************************************************/
	VertexInputCreateParam& addVertexAttribute(pvr::uint16 index, pvr::uint16 bufferBinding,
	        const assets::VertexAttributeLayout& layout, const char* attributeName = "")
	{
		pvr::utils::insertSorted_overwrite(attributes,
		                                   VertexAttributeInfoWithBinding(index, layout.dataType, layout.width, layout.offset, bufferBinding, attributeName),
		                                   VertexAttributeInfoCmp_BindingLess_IndexLess());
		return *this;
	}
};

/*!*********************************************************************************************************************
\brief Add Input Assembler configuration to this buffer object (primitive topology, vertex restart, vertex reuse etc).
\description --- Default settings ---
             Primitive Topology: TriangleList,   Primitive Restart: False, Vertex Reuse: Disabled,
			 Primitive Restart Index: 0xFFFFFFFF
***********************************************************************************************************************/
struct InputAssemblerStateCreateParam
{
public:
	friend class ::pvr::api::impl::GraphicsPipeline_;
	mutable types::PrimitiveTopology::Enum topology;
	bool							  disableVertexReuse;
	bool							  primitiveRestartEnable;

	/*!*********************************************************************************************************************
	\brief Create and configure an InputAssembler configuration.
	\param topology Primitive Topology (default: TriangleList)
	\param disableVertexReuse   Disable Vertex Reuse (true:disabled false:enabled). Default:true
	\param primitiveRestartEnable (true: enabled, false: disabled) Default:false
	\param primitiveRestartIndex Primitive Restart Index. Default  0xFFFFFFFF
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	InputAssemblerStateCreateParam(types::PrimitiveTopology::Enum topology = types::PrimitiveTopology::TriangleList,
	                               bool disableVertexReuse = true, bool primitiveRestartEnable = false, pvr::uint32 primitiveRestartIndex = 0xFFFFFFFF) :
		topology(topology), disableVertexReuse(disableVertexReuse), primitiveRestartEnable(primitiveRestartEnable) {}

	/*!*********************************************************************************************************************
	\brief Enable/ disable primitive restart.
	\param enable true for enable, false for disable.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	InputAssemblerStateCreateParam& setPrimitiveRestartEnable(bool enable)
	{
		primitiveRestartEnable = enable; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/ disable vertex reuse.
	\param disable true for disable, false for enable.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	InputAssemblerStateCreateParam& setVertexReuseDisable(bool disable)
	{
		disableVertexReuse = disable; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set primitive topology.
	\param topology The primitive topology to interpret the vertices as (TriangleList, Lines etc)
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	InputAssemblerStateCreateParam& setPrimitiveTopology(types::PrimitiveTopology::Enum topology)
	{
		this->topology = topology; return *this;
	}
};


/*!*********************************************************************************************************************
\brief Add blending configuration for a color attachment. Some API's only support one blending state for all attachments,
       in which case the 1st such configuration will be used for all.
\description --- Defaults ---
	Blend Enabled:false,    Source blend Color factor: false,    Destination blend Color factor: Zero,
	Source blend Alpha factor: Zero,    Destination blending Alpha factor :Zero,
	Blending operation color: Add,    Blending operation alpha: Add,     Channel writing mask: All
***********************************************************************************************************************/
struct ColorBlendAttachmentState
{
	bool				blendEnable; 	//!< Enable blending
	types::BlendFactor::Enum	srcBlendColor;	//!< Source Blending color factor
	types::BlendFactor::Enum	destBlendColor;	//!< Destination blending color factor
	types::BlendFactor::Enum	srcBlendAlpha;	//!< Source blending alpha factor
	types::BlendFactor::Enum	destBlendAlpha;	//!< Destination blending alpha factor
	types::BlendOp::Enum		blendOpColor;	//!< Blending operation color
	types::BlendOp::Enum		blendOpAlpha;	//!< Blending operation alpha
	types::ColorChannel::Bits	channelWriteMask;//!<Channel writing mask

	/*!*********************************************************************************************************************
	\brief Create a blending state. Separate color/alpha factors.
	\param blendEnable Enable blending (default false)
	\param srcBlendColor Source Blending color factor (default:Zero)
	\param destBlendColor Destination blending color factor (default:Zero)
	\param srcBlendAlpha Source blending alpha factor (default:Zero)
	\param destBlendAlpha Destination blending alpha factor (default:Zero)
	\param blendOpColor Blending operation color (default:Add)
	\param blendOpAlpha Blending operation alpha (default:Add)
	\param channelWriteMask  Channel writing mask (default:All)
	***********************************************************************************************************************/
	ColorBlendAttachmentState(bool blendEnable = false, types::BlendFactor::Enum srcBlendColor = types::BlendFactor::One,
	                          types::BlendFactor::Enum destBlendColor = types::BlendFactor::Zero, types::BlendFactor::Enum srcBlendAlpha = types::BlendFactor::One,
	                          types::BlendFactor::Enum destBlendAlpha = types::BlendFactor::Zero, types::BlendOp::Enum blendOpColor = types::BlendOp::Add,
	                          types::BlendOp::Enum blendOpAlpha = types::BlendOp::Add, pvr::uint32 channelWriteMask = types::ColorChannel::All) :
		blendEnable(blendEnable), srcBlendColor(srcBlendColor), destBlendColor(destBlendColor),
		srcBlendAlpha(srcBlendAlpha), destBlendAlpha(destBlendAlpha), blendOpColor(blendOpColor),
		blendOpAlpha(blendOpAlpha), channelWriteMask(channelWriteMask) {}

	/*!*********************************************************************************************************************
	\brief Create a blending state. Color and alpha factors together.
	\param blendEnable Enable blending (default false)
	\param srcBlendColorAlpha Source Blending color & alpha factor (default:Zero)
	\param dstBlendColorAlpha Destination blending color & alpha factor (default:Zero)
	\param blendOpColorAlpha Blending operation color & alpha (default:Add)
	\param channelWriteMask  Channel writing mask (default:All)
	***********************************************************************************************************************/
	ColorBlendAttachmentState(bool blendEnable, types::BlendFactor::Enum srcBlendColorAlpha, types::BlendFactor::Enum dstBlendColorAlpha,
	                          types::BlendOp::Enum blendOpColorAlpha, pvr::uint32 channelWriteMask = types::ColorChannel::All) :
		blendEnable(blendEnable), srcBlendColor(srcBlendColorAlpha), destBlendColor(dstBlendColorAlpha),
		srcBlendAlpha(srcBlendColorAlpha), destBlendAlpha(dstBlendColorAlpha),
		blendOpColor(blendOpColorAlpha), blendOpAlpha(blendOpColorAlpha), channelWriteMask(channelWriteMask) {}
};

/*!****************************************************************************************************************
\brief  Pipeline Color blending state configuration (alphaToCoverage, logicOp).
\description --- Defaults ---
        Enable alpha to coverage:false,  Enable logic op: false,  Logic Op: Set,	Attachments: 0
*******************************************************************************************************************/
struct ColorBlendStateCreateParam
{
	std::vector<ColorBlendAttachmentState> attachmentStates;
	friend class ::pvr::api::impl::GraphicsPipeline_;
	bool alphaToCoverageEnable;
	bool logicOpEnable;
	types::LogicOp::Enum logicOp;
	glm::vec4 colorBlendConstants;
	const std::vector<ColorBlendAttachmentState>& getAttachmentStates() const { return attachmentStates; }
public:
	/*!*********************************************************************************************************************
	\brief Create a Color Blend state object.
	\param alphaToCoverageEnable enable/ disable alpa to coverage (default:disable)
	\param logicOpEnable enable/disable logicOp (default:disable)
	\param logicOp Select logic operation (default:Set)
	\param attachmentStates An array of color blend attachment states (default: NULL)
	\param numAttachmentStates Number of color attachment states in array (default: 0)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam(bool alphaToCoverageEnable,
	                           bool logicOpEnable,
	                           types::LogicOp::Enum logicOp,
	                           glm::vec4 colorBlendConstants,
	                           ColorBlendAttachmentState* attachmentStates,
	                           pvr::uint32 numAttachmentStates) :
		alphaToCoverageEnable(alphaToCoverageEnable), logicOpEnable(logicOpEnable), logicOp(logicOp),
		colorBlendConstants(colorBlendConstants)
	{
		this->attachmentStates.assign(attachmentStates, attachmentStates + numAttachmentStates);
	}

	ColorBlendStateCreateParam(bool alphaToCoverageEnable = false,
	                           bool logicOpEnable = false,
	                           types::LogicOp::Enum logicOp = types::LogicOp::Set,
	                           glm::vec4 colorBlendConstants = glm::vec4(0.0f)) :
		alphaToCoverageEnable(alphaToCoverageEnable), logicOpEnable(logicOpEnable),
		logicOp(logicOp), colorBlendConstants(colorBlendConstants)
	{}

	ColorBlendStateCreateParam& setColorBlend(glm::vec4& blendConst)
	{
		colorBlendConstants = blendConst;
		return *this;
	}

	const glm::vec4& getColorBlendConst() {	return colorBlendConstants;	}

	/*!*********************************************************************************************************************
	\brief Enable/ disable alpha to coverage.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam& setAlphaToCoverageEnable(bool alphaToCoverageEnable)
	{
		this->alphaToCoverageEnable = alphaToCoverageEnable;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/ disable logic op.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam& setLogicOpEnable(bool logicOpEnable)
	{
		this->logicOpEnable = logicOpEnable;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief  Set the logic op.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam& setLogicOp(types::LogicOp::Enum logicOp)
	{
		this->logicOp = logicOp;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Append a color attachment blend configuration (appended to the end of the attachments list).
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam& clearAttachments()
	{
		attachmentStates.clear();
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Append a color attachment blend configuration (appended to the end of the attachments list).
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam& addAttachmentState(const ColorBlendAttachmentState& state)
	{
		attachmentStates.push_back(state);
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Add a color attachment state blend configuration to a specified index.
	\param index Which index this color attachment will be
	\param state The color attachment state to add
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam& setAttachmentState(pvr::uint32 index, const ColorBlendAttachmentState& state)
	{
		if (index >= attachmentStates.size()) {	attachmentStates.resize(index + 1);	}
		attachmentStates[index] = state;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set all color attachment states as an array. Replaces any that had already been added.
	\param state An array of color attachment states
	\param count The number of color attachment states in (state)
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam& addAttachmentState(pvr::uint32 count, ColorBlendAttachmentState const* state)
	{
		attachmentStates.assign(state, state + count);
		return *this;
	}
};

/*!****************************************************************************************************************
\brief  Pipeline Viewport state descriptor. Sets the base configuration of all viewports.
\description --- Defaults ---
        Number of Viewports:1, Clip Origin: lower lef, Depth range: 0..1
*******************************************************************************************************************/
struct ViewportStateCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
public:

	std::vector<std::pair<Rectanglei, Viewport>/**/> scissorViewport;
	/*!*********************************************************************************************************************
	\brief Constructs a Viewport State object.
	\param numViewport The total number of viewports (default 1)
	\param clipOrigin The Clip Origin of all viewports (default LowerLeft)
	\param depthMode  The depth mode of all viewports (default 0..1)
	***********************************************************************************************************************/
	ViewportStateCreateParam() {}


	ViewportStateCreateParam& addViewportScissor(const Viewport& viewport,
	        const Rectanglei& scissor)
	{
		scissorViewport.push_back(std::make_pair(scissor, viewport));
		return *this;
	}

	ViewportStateCreateParam& setViewportScissor(pvr::uint32 index, Viewport& viewport,
	        const Rectanglei& scissor)
	{
		if (index >= scissorViewport.size()) { scissorViewport.resize(index + 1); }
		scissorViewport[index].first = scissor;
		scissorViewport[index].second = viewport;
		return *this;
	}

	Rectanglei& getScissor(pvr::uint32 index)
	{
		assertion(index < scissorViewport.size(), "Array index out of bound");
		return scissorViewport[index].first;
	}

	const Rectanglei& getScissor(pvr::uint32 index)const
	{
		assertion(index < scissorViewport.size(), "Array index out of bound");
		return scissorViewport[index].first;
	}

	Viewport& getViewport(pvr::uint32 index)
	{
		assertion(index < scissorViewport.size(),  "Array index out of bound");
		return scissorViewport[index].second;
	}

	const Viewport& getViewport(pvr::uint32 index)const
	{
		assertion(index < scissorViewport.size(),  "Array index out of bound");
		return scissorViewport[index].second;
	}

	uint32 getNumViewportScissor() const { return (uint32)scissorViewport.size(); }

};

/*!*********************************************************************************************************************
\brief  Pipeline Rasterisation, clipping and culling state configuration. Culling, winding order, depth clipping, raster discard,
        point size, fill mode, provoking vertex.
\description ---  Defaults  ---
        Cull face: back, Front face: CounterClockWise, Depth Clipping: true, Rasterizer Discard: false, Program Point Size: false,
		Point Origin: Lower left,  Fill Mode: Front&Back,  Provoking Vertex: First
***********************************************************************************************************************/
struct RasterStateCreateParam
{
	types::Face::Enum cullFace;
	types::PolygonWindingOrder::Enum frontFaceWinding;
	bool enableDepthClip;
	bool enableRasterizerDiscard;
	bool enableProgramPointSize;
	bool enableDepthBias;
	bool enableDepthBiasClamp;

	types::FillMode::Enum fillMode;
	types::ProvokingVertex::Enum provokingVertex;
	float32 lineWidth;
	friend class ::pvr::api::impl::GraphicsPipeline_;
public:

	/*!*********************************************************************************************************************
	\brief Create a rasterization and polygon state configuration.
	\param cullFace Face culling (default: Back)
	\param windingOrder The polygon winding order (default: Front face is counterclockwise)
	\param enableDepthClip Enable depth clipping (default: true)
	\param enableRasterizerDiscard Enable rasterizer discard (default:false)
	\param enableProgramPointSize Enable program point size (default:true)
	\param pointOrigin Point Origin (default: Lower left corner)
	\param fillMode Polygon fill mode (default: Front and Back)
	\param provokingVertex Provoking Vertex (default: First)
	***********************************************************************************************************************/
	RasterStateCreateParam(types::Face::Enum cullFace = types::Face::Back,
	                       types::PolygonWindingOrder::Enum frontFaceWinding = types::PolygonWindingOrder::FrontFaceCCW,
	                       bool enableDepthClip = true,
	                       bool enableRasterizerDiscard = false,
	                       bool enableProgramPointSize = false,
	                       types::FillMode::Enum fillMode = types::FillMode::Fill,
	                       types::ProvokingVertex::Enum provokingVertex = types::ProvokingVertex::First,
	                       float32 lineWidth = 1.f,
	                       bool enableDepthBias = false,
	                       bool enableDepthBiasClamp = false) :
		cullFace(cullFace),
		frontFaceWinding(frontFaceWinding),
		enableDepthClip(enableDepthClip),
		enableRasterizerDiscard(enableRasterizerDiscard),
		enableProgramPointSize(enableProgramPointSize),
		enableDepthBias(enableDepthBias),
		enableDepthBiasClamp(enableDepthBiasClamp),
		fillMode(fillMode),
		provokingVertex(provokingVertex),
		lineWidth(lineWidth) {}

	bool isDepthBiasClampEnable()const { return enableDepthBiasClamp; }

	bool isDepthBiasEnable()const { return enableDepthBias;  }

	float32 getLineWidth()const { return lineWidth; }

	/*!*********************************************************************************************************************
	\brief Set the face that will be culled (front/back/both/none).
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setCullFace(types::Face::Enum face)
	{
		this->cullFace = face; return *this;
	}

	RasterStateCreateParam& setLineWidth(float32 lineWidth)
	{
		this->lineWidth = lineWidth; return *this;
	}

	RasterStateCreateParam& setDepthClipEnable(bool enableDepthClip)
	{
		this->enableDepthClip = enableDepthClip; return *this;
	}

	RasterStateCreateParam& setDepthBiasEnable(bool enableDepthBias)
	{
		this->enableDepthBias = enableDepthBias; return *this;
	}

	RasterStateCreateParam& setDepthBiasClampEnable(bool enableDepthBiasClamp)
	{
		this->enableDepthBiasClamp = enableDepthBiasClamp; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set polygon winding order.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setFrontFaceWinding(types::PolygonWindingOrder::Enum frontFaceWinding)
	{
		this->frontFaceWinding = frontFaceWinding; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/ disable depth clip.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setDepthclipEnable(bool enable)
	{
		this->enableDepthClip = enable; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/disable rasterizer discard.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setRasterizerDiscardEnanle(bool enable)
	{
		this->enableRasterizerDiscard = enable; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/disable Program Point Size.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setProgramPointSizeEnable(bool enable)
	{
		enableProgramPointSize = enable; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set polygon fill mode.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setFillMode(types::FillMode::Enum mode)
	{
		fillMode = mode; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the Provoking Vertex.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setProvokingVertex(types::ProvokingVertex::Enum provokingVertex)
	{
		this->provokingVertex = provokingVertex; return *this;
	}
};
//!\cond NO_DOXYGEN
struct MultiSampleStateCreateParam
{
private:
	friend class ::pvr::api::impl::GraphicsPipeline_;
	bool multisampleEnable;
	bool sampleShadingEnable;
	bool alphaToCoverageEnable;
	bool alphaToOnEnable;
	pvr::uint32 rasterizationSamples;
	pvr::float32 minSampleShading;
	pvr::uint32 sampleMask;

public:
	/*!*********************************************************************************************************************
	\brief Constructor. Create a multisampling configuration.
	\param multisampleEnable Enable/disable multisampling (default false)
	\param sampleShadingEnable Enable/disable sample shading (defalt false)
	\param numSamples The number of (multisampling) samples (default 1)
	\param minSampleShading The minimum sample Shading (default 0)
	\param sampleMask sampleMask (default 0)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam(bool multisampleEnable = false, bool sampleShadingEnable = false,
	                            bool alphaToCoverageEnable = false, bool alphaToOnEnable = false,
	                            pvr::uint32 rasterizationSamples = 1,  pvr::float32 minSampleShading = 0.0f,
	                            pvr::uint32 sampleMask = 0xffffffff) :
		multisampleEnable(multisampleEnable), sampleShadingEnable(sampleShadingEnable),
		alphaToCoverageEnable(alphaToCoverageEnable), alphaToOnEnable(alphaToOnEnable),
		rasterizationSamples(rasterizationSamples),	minSampleShading(minSampleShading),	sampleMask(sampleMask) {}

	/*!*********************************************************************************************************************
	\brief Enable/disable multisampling
	\param enable true enable, false disable
	\return this (allow chaining)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam& setMultiSampleEnable(bool enable)
	{
		multisampleEnable = enable; return *this;
	}

	MultiSampleStateCreateParam& setAlphaToCoverageEnable(bool enable)
	{
		alphaToCoverageEnable = enable; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/ disable sampler shading.
	\param enable true enable, false disable
	\return this (allow chaining)
	*************************************************************************************************/
	MultiSampleStateCreateParam& setSampleShadingEnabe(bool enable)
	{
		sampleShadingEnable = enable; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set number of samples.
	\param numSamples The number of samples.
	\return this (allow chaining)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam& setNumRasterizationSamples(pvr::uint32 numSamples)
	{
		this->rasterizationSamples = numSamples; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set minimum sample shading.
	\param minSampleShading The number of minimum samples to shade
	\return this (allow chaining)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam& setMinSampleShading(pvr::float32 minSampleShading)
	{
		this->minSampleShading = minSampleShading; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set sample mask.
	\param mask The sample mask.
	\return this (allow chaining)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam& setSampleMask(pvr::uint32 mask)
	{
		sampleMask = mask; return *this;
	}
	uint32 getNumRasterizationSamples()const {	return rasterizationSamples;  }
	bool isSampleShadingEnabled()const { return sampleShadingEnable; }
	float32 getMinSanpleShading()const { return minSampleShading; }
	bool isAlphaToCoverageEnabled()const { return alphaToCoverageEnable; }
	bool isAlphaToOneEnabled()const { return alphaToOnEnable; }
	uint32 getSampleMask()const { return sampleMask; }
};
//!\endcond


struct DynamicStatesCreateParam
{
private:
	int32 dynamicStates[types::DynamicState::Count];
public:
	DynamicStatesCreateParam() { memset(dynamicStates, -1, sizeof(dynamicStates)); }
	bool isDynamicStateEnabled(types::DynamicState::Enum state)const { return (dynamicStates[state] == state); }
	DynamicStatesCreateParam& setDynamicState(types::DynamicState::Enum state)
	{
		dynamicStates[state] = state; return *this;
	}
};

/*!*********************************************************************************************************************
\brief Pipeline vertex Shader stage create param.
***********************************************************************************************************************/
struct VertexShaderStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
private:
	api::Shader shader;
public:
	const api::Shader& getShader() const { return shader; }

	bool isActive()const { return shader.isValid(); }

	/*!*********************************************************************************************************************
	\brief Constructor.
	***********************************************************************************************************************/
	VertexShaderStageCreateParam() {}

	/*!*********************************************************************************************************************
	\brief Construct from a pvr::api::Shader object
	\param shader A vertex shader
	***********************************************************************************************************************/
	VertexShaderStageCreateParam(const api::Shader& shader) : shader(shader) {}

	/*!*********************************************************************************************************************
	\brief Set vertex shader.
	\param shader A vertex shader
	***********************************************************************************************************************/
	void setShader(const api::Shader& shader) { this->shader = shader; }

	VertexShaderStageCreateParam& operator=(const api::Shader& shader) { setShader(shader); return *this; }
};

/*!*********************************************************************************************************************
\brief Pipeline Fragement shader stage create param.
***********************************************************************************************************************/
struct FragmentShaderStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
private:
	api::Shader shader;

	/*!*********************************************************************************************************************
	\brief Create pipeline state object.
	***********************************************************************************************************************/
public:
	const api::Shader& getShader() const { return shader; }

	bool isActive()const { return shader.isValid(); }

	/*!*********************************************************************************************************************
	\brief
	\return
	***********************************************************************************************************************/
	FragmentShaderStageCreateParam() {}

	/*!*********************************************************************************************************************
	\brief  ctor
	***********************************************************************************************************************/
	FragmentShaderStageCreateParam(const api::Shader& shader) : shader(shader) {}

	/*!*********************************************************************************************************************
	\brief Set fragment shader.
	***********************************************************************************************************************/
	void setShader(const api::Shader& shader) { this->shader = shader; }

	FragmentShaderStageCreateParam& operator=(const api::Shader& shader) { setShader(shader); return *this; }
};


/*!*********************************************************************************************************************
\brief Pipeline Geometry shader stage create param.
***********************************************************************************************************************/
struct GeometryShaderStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
private:
	api::Shader shader;

	/*!*********************************************************************************************************************
	\brief Create pipeline state object.
	***********************************************************************************************************************/
public:
	const api::Shader& getShader() const { return shader; }

	bool isActive()const { return shader.isValid(); }

	/*!*********************************************************************************************************************
	\brief
	\return
	***********************************************************************************************************************/
	GeometryShaderStageCreateParam() {}

	/*!*********************************************************************************************************************
	\brief  ctor
	***********************************************************************************************************************/
	GeometryShaderStageCreateParam(const api::Shader& shader) : shader(shader) {}

	/*!*********************************************************************************************************************
	\brief Set fragment shader.
	***********************************************************************************************************************/
	void setShader(const api::Shader& shader) { this->shader = shader; }

	GeometryShaderStageCreateParam& operator=(const api::Shader& shader) { setShader(shader); return *this; }
};



/*!*********************************************************************************************************************
\brief Pipeline Tesselation Control shader stage create param.
***********************************************************************************************************************/
struct TessControlShaderStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
private:
	api::Shader shader;

	/*!*********************************************************************************************************************
	\brief Create pipeline state object.
	***********************************************************************************************************************/
public:
	const api::Shader& getShader() const { return shader; }

	bool isActive()const { return shader.isValid(); }

	/*!*********************************************************************************************************************
	\brief
	\return
	***********************************************************************************************************************/
	TessControlShaderStageCreateParam() {}

	/*!*********************************************************************************************************************
	\brief  ctor
	***********************************************************************************************************************/
	TessControlShaderStageCreateParam(const api::Shader& shader) : shader(shader) {}

	/*!*********************************************************************************************************************
	\brief Set fragment shader.
	***********************************************************************************************************************/
	void setShader(const api::Shader& shader) { this->shader = shader; }

	TessControlShaderStageCreateParam& operator=(const api::Shader& shader) { setShader(shader); return *this; }
};


/*!*********************************************************************************************************************
\brief Pipeline Tesselation Evaluation shader stage create param.
***********************************************************************************************************************/
struct TessEvalShaderStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
private:
	api::Shader shader;

	/*!*********************************************************************************************************************
	\brief Create pipeline state object.
	***********************************************************************************************************************/
public:
	const api::Shader& getShader() const { return shader; }

	bool isActive()const { return shader.isValid(); }

	/*!*********************************************************************************************************************
	\brief
	\return
	***********************************************************************************************************************/
	TessEvalShaderStageCreateParam() {}

	/*!*********************************************************************************************************************
	\brief  ctor
	***********************************************************************************************************************/
	TessEvalShaderStageCreateParam(const api::Shader& shader) : shader(shader) {}

	/*!*********************************************************************************************************************
	\brief Set fragment shader.
	***********************************************************************************************************************/
	void setShader(const api::Shader& shader) { this->shader = shader; }

	TessEvalShaderStageCreateParam& operator=(const api::Shader& shader) { setShader(shader); return *this; }
};


/*!*********************************************************************************************************************
\brief Computer shader stage creator.
***********************************************************************************************************************/
struct ComputeShaderStageCreateParam
{
public:
	/*!*********************************************************************************************************************
	\brief Set the compute shader object
	\param[in] shader The compute shader object.
	***********************************************************************************************************************/
	void setShader(const pvr::api::Shader& shader) { m_shader = shader; }

	bool isActive()const { return m_shader.isValid(); }

	/*!*********************************************************************************************************************
	\brief Return if it has valid compute shader.
	\return true If the compute shader exists, false otherwise.
	***********************************************************************************************************************/
	bool hasComputeShader() { return m_shader.isValid(); }

	const pvr::api::Shader& getShader() const { return m_shader; }
	void setShaderEntrypoint(const char* entryPoint)
	{
		this->entryPoint.assign(entryPoint);
	}
	ComputeShaderStageCreateParam() : entryPoint("main") {}
	const char* getShaderEntrypoint() { return entryPoint.c_str(); }
	ComputeShaderStageCreateParam& operator=(const api::Shader& shader) { setShader(shader); return *this; }
private:
	pvr::api::Shader m_shader;
	std::string entryPoint;
};
}
}
}
