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
	uint16 index;			//!< Attribute index
	types::DataType format;	//!< Data type of each element of the attribute
	uint8 width;			//!< Number of elements in attribute, e.g 1,2,3,4
	uint32 offsetInBytes;	//!< Offset of the first element in the buffer
	std::string attribName;		//!< Optional: Name(in the shader) of the attribute

	/*!*********************************************************************************************************
	\brief Default  constructor. Uninitialized values, except for AttributeName.
	************************************************************************************************************/
	VertexAttributeInfo(): index(types::PipelineDefaults::VertexAttributeInfo::Index),
		format(types::PipelineDefaults::VertexAttributeInfo::Format),
		width(types::PipelineDefaults::VertexAttributeInfo::Width),
		offsetInBytes(types::PipelineDefaults::VertexAttributeInfo::OffsetInBytes),
		attribName(types::PipelineDefaults::VertexAttributeInfo::AttribName) {}

	/*!*********************************************************************************************************
	\brief Create a new VertexAttributeInfo object.
	\param index Attribute binding index
	\param format Attribute data type
	\param width Number of elements in attribute
	\param offsetInBytes Interleaved: offset of the attribute from the start of data of each vertex
	\param attribName Name of the attribute in the shader.
	************************************************************************************************************/
	VertexAttributeInfo(uint16 index, types::DataType format, uint8 width,
	                    uint32 offsetInBytes,
	                    const char* attribName = types::PipelineDefaults::VertexAttributeInfo::AttribName.c_str()) :
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
	bool operator!=(VertexAttributeInfo const& rhs)const { return !((*this) == rhs); }

};

/*!****************************************************************************************************************
\brief Information about a Buffer binding: Binding index, stride, (instance) step rate.
*******************************************************************************************************************/
struct VertexInputBindingInfo
{
	uint16 bindingId;//< buffer binding index
	uint32 strideInBytes; //< buffer stride in bytes
	types::StepRate stepRate;//< buffer step rate

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
	VertexInputBindingInfo(uint16 bindId, uint32 strideInBytes,  types::StepRate stepRate = types::StepRate::Default) :
		bindingId(bindId), strideInBytes(strideInBytes), stepRate(stepRate) {}
};

struct VertexAttributeInfoWithBinding : public VertexAttributeInfo
{
	uint16 binding;
	VertexAttributeInfoWithBinding() {}

	VertexAttributeInfoWithBinding(const VertexAttributeInfo& nfo, uint16 binding) :
		VertexAttributeInfo(nfo), binding(binding) {}

	VertexAttributeInfoWithBinding(
	  uint16 index, types::DataType format, uint8 width, uint32 offsetInBytes, uint16 binding,
	  const char* attribName = types::PipelineDefaults::VertexAttributeInfo::AttribName.c_str())
		: VertexAttributeInfo(index, format, width, offsetInBytes, attribName), binding(binding) {}
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
	Viewport(float32 x = types::PipelineDefaults::ViewportScissor::OffsetX,
	         float32 y = types::PipelineDefaults::ViewportScissor::OffsetY,
	         float32 width = types::PipelineDefaults::ViewportScissor::Width,
	         float32 height = types::PipelineDefaults::ViewportScissor::Height,
	         float32 minDepth = types::PipelineDefaults::ViewportScissor::MinDepth,
	         float32 maxDepth = types::PipelineDefaults::ViewportScissor::MaxDepth) :
		x(x), y(y), width(width), height(height), minDepth(minDepth), maxDepth(maxDepth) {}

	/*!*********************************************************************************************************************
	\brief ctor
	\param rect	viewport
	\param minDepth depth min
	\param maxDepth depth max
	***********************************************************************************************************************/
	Viewport(const Rectanglei& rect, float32 minDepth = types::PipelineDefaults::ViewportScissor::MinDepth,
	         float32 maxDepth = types::PipelineDefaults::ViewportScissor::MaxDepth) :
		x((float32)rect.x), y((float32)rect.y), width((float32)rect.width), height((float32)rect.height),
		minDepth(minDepth), maxDepth(maxDepth) {}
};

typedef std::vector<VertexInputBindingInfo> VertexInputBindingMap;// map buffer binding -> VertexAttributes
typedef std::vector<VertexAttributeInfoWithBinding>	VertexAttributeMap;
typedef types::StencilState StencilState;

namespace pipelineCreation {
/*!********************************************************************************************
\brief  Contains parameters needed to set depth stencil states to a pipeline create params.
        This object can be added to a PipelineCreateParam to set a depth-stencil state to
		values other than their defaults.
\details --- Defaults: ---
        depthWrite:enabled,    depthTest:enabled,    DepthComparison:Less,
        Stencil Text: disabled,    All stencil ops:Keep,
***********************************************************************************************/
struct DepthStencilStateCreateParam
{
public:
	typedef api::StencilState StencilState;
private:
	// stencil
	bool depthTest;  //!< Enable/disable depth test. Default false
	bool depthWrite; //!< Enable/ disable depth write
	bool stencilTestEnable; //!< Enable/disable stencil test. Default false
	bool depthBoundTest;//!< Enable/ disable depth bound test
	bool enableDepthStencilState;//!< Enable/ Disable this state
	float32 minDepth;//!< Depth minimum
	float32 maxDepth;//!< Depth maximum

	StencilState stencilFront;//!< Stencil state front
	StencilState stencilBack;//!< Stencil state back

	types::ComparisonMode depthCmpOp; //!< Depth compare operation. Default LESS
public:
	/*!*********************************************************************************************************************
	\brief  Set all Depth and Stencil parameters.
	***********************************************************************************************************************/
	DepthStencilStateCreateParam(bool depthWrite = types::PipelineDefaults::DepthStencilStates::DepthWriteEnabled,
	                             bool depthTest = types::PipelineDefaults::DepthStencilStates::DepthTestEnabled,
	                             types::ComparisonMode depthCompareFunc = types::ComparisonMode::DefaultDepthFunc,
	                             bool stencilTest = types::PipelineDefaults::DepthStencilStates::StencilTestEnabled,
	                             bool depthBoundTest = types::PipelineDefaults::DepthStencilStates::DepthBoundTestEnabled,
	                             const StencilState& stencilFront = StencilState(),
	                             const StencilState& stencilBack = StencilState(),
	                             float32 minDepth = types::PipelineDefaults::DepthStencilStates::DepthMin,
	                             float32 maxDepth = types::PipelineDefaults::DepthStencilStates::DepthMax)
		:
		depthTest(depthTest), depthWrite(depthWrite), stencilTestEnable(stencilTest),
		depthBoundTest(depthBoundTest), enableDepthStencilState(types::PipelineDefaults::DepthStencilStates::UseDepthStencil),
		minDepth(minDepth), maxDepth(maxDepth), stencilFront(stencilFront), stencilBack(stencilBack), depthCmpOp(depthCompareFunc) {}

	/*!*********************************************************************************************************************
	\brief  Return true if depth test is enable
	***********************************************************************************************************************/
	bool isDepthTestEnable()const
	{
		return depthTest;
	}

	/*!*********************************************************************************************************************
	\brief  Return true if depth write is enable
	***********************************************************************************************************************/
	bool isDepthWriteEnable()const
	{
		return depthWrite;
	}

	/*!*********************************************************************************************************************
	\brief  Return true if depth bound is enable
	***********************************************************************************************************************/
	bool isDepthBoundTestEnable()const
	{
		return depthBoundTest;
	}

	/*!*********************************************************************************************************************
	\brief  Return true if stencil test is enable
	***********************************************************************************************************************/
	bool isStencilTestEnable()const
	{
		return stencilTestEnable;
	}

	/*!*********************************************************************************************************************
	\brief  Return minimum depth value
	***********************************************************************************************************************/
	float32 getMinDepth()const
	{
		return minDepth;
	}

	/*!*********************************************************************************************************************
	\brief  Return maximum depth value
	***********************************************************************************************************************/
	float32 getMaxDepth()const
	{
		return maxDepth;
	}

	/*!*********************************************************************************************************************
	\brief  Return depth comparison operator
	***********************************************************************************************************************/
	types::ComparisonMode getDepthComapreOp()const
	{
		return depthCmpOp;
	}

	/*!*********************************************************************************************************************
	\brief  Return true if this state is enabled.
	***********************************************************************************************************************/
	bool isStateEnable()const
	{
		return enableDepthStencilState;
	}

	/*!*********************************************************************************************************************
	\brief  Enable/ Disale this state
	\param flag True:enable, False:disable
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	DepthStencilStateCreateParam& enableState(bool flag)
	{
		enableDepthStencilState = flag;
		return *this;
	}

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
	DepthStencilStateCreateParam& setDepthCompareFunc(types::ComparisonMode compareFunc)
	{
		this->depthCmpOp = compareFunc;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/disable stencil test.
	\param stencilTest True:enable, False:disable
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilTest(bool stencilTest)
	{
		this->stencilTestEnable = stencilTest;
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Set the stencil front state
	\return this object (allows chained calls)
	\param	stencil Stencil state
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilFront(const StencilState& stencil)
	{
		stencilFront = stencil;
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Set the stencil back state
	\return this object (allows chained calls)
	\param	stencil Stencil state
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilBack(const StencilState& stencil)
	{
		stencilBack = stencil;
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Set the stencil front and back state
	\return this object (allows chained calls)
	\param	stencil Stencil state
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilFrontBack(StencilState& stencil)
	{
		stencilFront = stencil, stencilBack = stencil;
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Return stencil front state
	*******************************************************************************************************************/
	const StencilState& getStencilFront()const
	{
		return stencilFront;
	}

	/*!****************************************************************************************************************
	\brief	Return stencil back state
	*******************************************************************************************************************/
	const StencilState& getStencilBack()const
	{
		return stencilBack;
	}
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
	const VertexInputBindingMap& getInputBindings() const
	{
		return inputBindings;
	}

	/*!********************************************************************************************************************
	\brief Return the vertex attributes
	**********************************************************************************************************************/
	const VertexAttributeMap& getAttributes() const
	{
		return attributes;
	}

	/*!********************************************************************************************************************
	\brief Clear this object.
	\return this object (allows chained calls)
	**********************************************************************************************************************/
	VertexInputCreateParam& clear()
	{
		inputBindings.clear();
		attributes.clear();
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Set the vertex input buffer bindings.
	\return this object (allows chained calls)
	\param	bufferBinding Vertex buffer binding index
	\param	strideInBytes specifies the byte offset between consecutive generic vertex attributes.
			If stride is 0, the generic vertex attributes are understood to be tightly packed in the array.
			The initial value is 0.
	\param	stepRate The rate at which this binding is incremented (used for Instancing).
	*******************************************************************************************************************/
	VertexInputCreateParam& setInputBinding(
	  uint16 bufferBinding, uint16 strideInBytes = types::PipelineDefaults::VertexInput::StrideInBytes,
	  types::StepRate stepRate = types::StepRate::Default)
	{
		utils::insertSorted_overwrite(inputBindings, VertexInputBindingInfo(bufferBinding, strideInBytes,
		                              stepRate), VertexBindingInfoCmp_BindingLess());
		return *this;
	}

	/*!
	   \brief Return a VertexBindingInfo for a buffer binding index, else return NULL if not found
	   \param bufferBinding Buffer binding index
	 */
	const VertexInputBindingInfo* getInputBinding(uint32 bufferBinding)
	{
		for (const VertexInputBindingInfo& it : inputBindings)
		{
			if (it.bindingId == bufferBinding) { return &it; }
		}
		return NULL;
	}


	/*!****************************************************************************************************************
	\brief Add vertex layout information to a buffer binding index using a VertexAttributeInfo object.
	\param bufferBinding The binding index to add the vertex attribute information.
	\param attrib Vertex Attribute information object.
	\return this object (allows chained calls)
	*******************************************************************************************************************/
	VertexInputCreateParam& addVertexAttribute(uint16 bufferBinding, const VertexAttributeInfo& attrib)
	{
		utils::insertSorted_overwrite(attributes, VertexAttributeInfoWithBinding(attrib, bufferBinding),
		                              VertexAttributeInfoCmp_BindingLess_IndexLess());
		return *this;
	}

	/*!
	   \brief Add vertex layout information to a buffer binding index using an array of VertexAttributeInfo object.
	   \param bufferBinding The binding index to add the vertex attribute information.
	   \param attrib Attribute information object.
	   \param numAttributes Number of attributues in the array
	   \return this object (allows chained calls)
	 */
	VertexInputCreateParam& addVertexAttributes(uint16 bufferBinding, const VertexAttributeInfo* attrib,
	    uint32 numAttributes)
	{
		for (uint32 i = 0; i < numAttributes; ++i)
		{
			utils::insertSorted_overwrite(attributes, VertexAttributeInfoWithBinding(attrib[i], bufferBinding),
			                              VertexAttributeInfoCmp_BindingLess_IndexLess());
		}
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
	VertexInputCreateParam& addVertexAttribute(
	  uint16 index, uint16 bufferBinding, const assets::VertexAttributeLayout& layout,
	  const char* attributeName = types::PipelineDefaults::VertexInput::AttribName.c_str())
	{
		utils::insertSorted_overwrite(attributes, VertexAttributeInfoWithBinding(index, layout.dataType,
		                              layout.width, layout.offset, bufferBinding, attributeName),
		                              VertexAttributeInfoCmp_BindingLess_IndexLess());
		return *this;
	}
};

/*!*********************************************************************************************************************
\brief Add Input Assembler configuration to this buffer object (primitive topology, vertex restart, vertex reuse etc).
\details --- Default settings ---
             Primitive Topology: TriangleList,   Primitive Restart: False, Vertex Reuse: Disabled,
			 Primitive Restart Index: 0xFFFFFFFF
***********************************************************************************************************************/
struct InputAssemblerStateCreateParam
{
public:
	friend class ::pvr::api::impl::GraphicsPipeline_;
	mutable types::PrimitiveTopology topology;
	bool	disableVertexReuse;
	bool	primitiveRestartEnable;
	uint32 primitiveRestartIndex;
	/*!*********************************************************************************************************************
	\brief Create and configure an InputAssembler configuration.
	\param topology Primitive Topology (default: TriangleList)
	\param disableVertexReuse   Disable Vertex Reuse (true:disabled false:enabled). Default:true
	\param primitiveRestartEnable (true: enabled, false: disabled) Default:false
	\param primitiveRestartIndex Primitive Restart Index. Default  0xFFFFFFFF
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	InputAssemblerStateCreateParam(types::PrimitiveTopology topology = types::PipelineDefaults::InputAssembler::Topology,
	                               bool disableVertexReuse = types::PipelineDefaults::InputAssembler::DisableVertexReuse,
	                               bool primitiveRestartEnable = types::PipelineDefaults::InputAssembler::PrimitiveRestartEnabled,
	                               uint32 primitiveRestartIndex = types::PipelineDefaults::InputAssembler::PrimitiveRestartIndex):
		topology(topology), disableVertexReuse(disableVertexReuse), primitiveRestartEnable(primitiveRestartEnable),
		primitiveRestartIndex(primitiveRestartIndex) {}

	/*!*********************************************************************************************************************
	\brief Enable/ disable primitive restart.
	\param enable true for enable, false for disable.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	InputAssemblerStateCreateParam& setPrimitiveRestartEnable(bool enable)
	{
		primitiveRestartEnable = enable;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/ disable vertex reuse.
	\param disable true for disable, false for enable.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	InputAssemblerStateCreateParam& setVertexReuseDisable(bool disable)
	{
		disableVertexReuse = disable;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set primitive topology.
	\param topology The primitive topology to interpret the vertices as (TriangleList, Lines etc)
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	InputAssemblerStateCreateParam& setPrimitiveTopology(types::PrimitiveTopology topology)
	{
		this->topology = topology;
		return *this;
	}
};

/*!****************************************************************************************************************
\brief  Pipeline Color blending state configuration (alphaToCoverage, logicOp).
\details Defaults: Enable alpha to coverage:false,  Enable logic op: false,  Logic Op: Set,	Attachments: 0
*******************************************************************************************************************/
struct ColorBlendStateCreateParam
{
	std::vector<types::BlendingConfig> attachmentStates;
	friend class ::pvr::api::impl::GraphicsPipeline_;
	bool alphaToCoverageEnable;
	bool logicOpEnable;
	types::LogicOp logicOp;
	glm::vec4 colorBlendConstants;
	const std::vector<types::BlendingConfig>& getAttachmentStates() const
	{
		return attachmentStates;
	}
public:
	/*!*********************************************************************************************************************
	\brief Create a Color Blend state object.
	\param alphaToCoverageEnable enable/ disable alpa to coverage (default:disable)
	\param colorBlendConstants Blending constants
	\param logicOpEnable enable/disable logicOp (default:disable)
	\param logicOp Select logic operation (default:Set)
	\param attachmentStates An array of color blend attachment states (default: NULL)
	\param numAttachmentStates Number of color attachment states in array (default: 0)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam(bool alphaToCoverageEnable, bool logicOpEnable, types::LogicOp logicOp,
	                           glm::vec4 colorBlendConstants, types::BlendingConfig* attachmentStates, uint32 numAttachmentStates) :
		alphaToCoverageEnable(alphaToCoverageEnable), logicOpEnable(logicOpEnable), logicOp(logicOp),
		colorBlendConstants(colorBlendConstants)
	{
		this->attachmentStates.assign(attachmentStates, attachmentStates + numAttachmentStates);
	}

	/*!
	   \brief Create a Color Blend state object.
	   \param alphaToCoverageEnable enable/ disable alpa to coverage (default:disable
	   \param logicOpEnable enable/disable logicOp (default:disable)
	   \param logicOp Select logic operation (default:Set)
	   \param colorBlendConstants Set color blend constants. Default (0,0,0,0)
	 */
	ColorBlendStateCreateParam(bool alphaToCoverageEnable = types::PipelineDefaults::ColorBlend::AlphaCoverageEnable,
	                           bool logicOpEnable = types::PipelineDefaults::ColorBlend::LogicOpEnable,
	                           types::LogicOp logicOp = types::PipelineDefaults::ColorBlend::LogicOp,
	                           glm::vec4 colorBlendConstants = types::PipelineDefaults::ColorBlend::BlendConstantRGBA) :
		alphaToCoverageEnable(alphaToCoverageEnable), logicOpEnable(logicOpEnable),
		logicOp(logicOp), colorBlendConstants(colorBlendConstants)
	{}

	/*!
	   \brief Set color blend constant
	   \param blendConst
	   \return Return this object (allows chained calls)
	 */
	ColorBlendStateCreateParam& setColorBlendConst(glm::vec4& blendConst)
	{
		colorBlendConstants = blendConst;
		return *this;
	}

	/*!
	   \brief Get color blend const
	   \return Blend constants
	 */
	const glm::vec4& getColorBlendConst()
	{
		return colorBlendConstants;
	}

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
	ColorBlendStateCreateParam& setLogicOp(types::LogicOp logicOp)
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
	\brief Add a color attachment state blend configuration to a specified index.
	\param index Which index this color attachment will be
	\param state The color attachment state to add
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam& setAttachmentState(uint32 index, const types::BlendingConfig& state)
	{
		if (index >= attachmentStates.size())
		{
			attachmentStates.resize(index + 1);
		}
		attachmentStates[index] = state;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set all color attachment states as an array. Replaces any that had already been added.
	\param state An array of color attachment states
	\param count The number of color attachment states in (state)
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam& setAttachmentStates(uint32 count, types::BlendingConfig const* state)
	{
		attachmentStates.assign(state, state + count);
		return *this;
	}
};

/*!****************************************************************************************************************
\brief  Pipeline Viewport state descriptor. Sets the base configuration of all viewports.
\details Defaults:  Number of Viewports:1, Clip Origin: lower lef, Depth range: 0..1
*******************************************************************************************************************/
struct ViewportStateCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
public:

	std::vector<std::pair<Rectanglei, Viewport>/**/> scissorViewport;
	/*!*********************************************************************************************************************
	\brief Constructor.
	***********************************************************************************************************************/
	ViewportStateCreateParam() {}

	/*!
	   \brief Set viewport and scissor
	   \param index View port and scissor index
	   \param viewport Viewport
	   \param scissor Scissor
	   \return return this object (allows chained calls)
	 */
	ViewportStateCreateParam& setViewportAndScissor(uint32 index, const Viewport& viewport,
	    const Rectanglei& scissor)
	{
		if (index >= scissorViewport.size())
		{
			scissorViewport.resize(index + 1);
		}
		scissorViewport[index].first = scissor;
		scissorViewport[index].second = viewport;
		return *this;
	}

	/*!
	   \brief Return scissor
	   \param index
	 */
	Rectanglei& getScissor(uint32 index)
	{
		assertion(index < scissorViewport.size(), "Array index out of bound");
		return scissorViewport[index].first;
	}

	/*!
	   \brief Return scissor (const)
	   \param index
	 */
	const Rectanglei& getScissor(uint32 index)const
	{
		assertion(index < scissorViewport.size(), "Array index out of bound");
		return scissorViewport[index].first;
	}

	/*!
	   \brief Return viewport
	   \param index
	 */
	Viewport& getViewport(uint32 index)
	{
		assertion(index < scissorViewport.size(),  "Array index out of bound");
		return scissorViewport[index].second;
	}

	/*!
	   \brief Return viewport (const)
	   \param index
	 */
	const Viewport& getViewport(uint32 index)const
	{
		assertion(index < scissorViewport.size(),  "Array index out of bound");
		return scissorViewport[index].second;
	}

	/*!
	   \brief Return number of viewport and scissor
	 */
	uint32 getNumViewportScissor() const
	{
		return (uint32)scissorViewport.size();
	}
};

/*!*********************************************************************************************************************
\brief  Pipeline Rasterisation, clipping and culling state configuration. Culling, winding order, depth clipping, raster discard,
        point size, fill mode, provoking vertex.
\details Defaults: Cull face: back, Front face: CounterClockWise, Depth Clipping: true, Rasterizer Discard: false, Program Point Size: false,
		Point Origin: Lower left,  Fill Mode: Front&Back,  Provoking Vertex: First
***********************************************************************************************************************/
struct RasterStateCreateParam
{
	types::Face cullFace;
	types::PolygonWindingOrder frontFaceWinding;
	bool enableDepthClip;
	bool enableRasterizerDiscard;
	bool enableProgramPointSize;
	bool enableDepthBias;
	bool enableDepthBiasClamp;

	types::FillMode fillMode;
	types::ProvokingVertex provokingVertex;
	float32 lineWidth;
	friend class ::pvr::api::impl::GraphicsPipeline_;
public:
	/*!*********************************************************************************************************************
	\brief Create a rasterization and polygon state configuration.
	\param cullFace Face culling (default: Back)
	\param frontFaceWinding The polygon winding order (default: Front face is counterclockwise)
	\param enableDepthClip Enable depth clipping (default: true)
	\param enableRasterizerDiscard Enable rasterizer discard (default:false)
	\param enableProgramPointSize Enable program point size (default:true)
	\param fillMode Polygon fill mode (default: Front and Back)
	\param provokingVertex Provoking Vertex (default: First)
	\param lineWidth Width of rendered lines (default: One)
	\param enableDepthBias Enable depth bias (default: false)
	\param enableDepthBiasClamp Enable depth bias clamp (default: false)
	***********************************************************************************************************************/
	RasterStateCreateParam(types::Face cullFace = types::PipelineDefaults::Rasterizer::CullFace,
	                       types::PolygonWindingOrder frontFaceWinding = types::PipelineDefaults::Rasterizer::WindingOrder,
	                       bool enableDepthClip = types::PipelineDefaults::Rasterizer::DepthClipEnabled,
	                       bool enableRasterizerDiscard = types::PipelineDefaults::Rasterizer::RasterizerDiscardEnabled,
	                       bool enableProgramPointSize = types::PipelineDefaults::Rasterizer::ProgramPointSizeEnabled,
	                       types::FillMode fillMode = types::PipelineDefaults::Rasterizer::FillMode,
	                       types::ProvokingVertex provokingVertex = types::PipelineDefaults::Rasterizer::ProvokingVertex,
	                       float32 lineWidth = types::PipelineDefaults::Rasterizer::LineWidth,
	                       bool enableDepthBias = types::PipelineDefaults::Rasterizer::DepthBiasEnabled,
	                       bool enableDepthBiasClamp = types::PipelineDefaults::Rasterizer::DepthBiasClampEnabled) :
		cullFace(cullFace), frontFaceWinding(frontFaceWinding), enableDepthClip(enableDepthClip),
		enableRasterizerDiscard(enableRasterizerDiscard), enableProgramPointSize(enableProgramPointSize),
		enableDepthBias(enableDepthBias), enableDepthBiasClamp(enableDepthBiasClamp), fillMode(fillMode),
		provokingVertex(provokingVertex), lineWidth(lineWidth) {}

	/*!
	   \brief Return true if depth bias clamp is enabled
	 */
	bool isDepthBiasClampEnable()const
	{
		return enableDepthBiasClamp;
	}

	/*!
	   \brief Return true if depth bias is enabled
	 */
	bool isDepthBiasEnable()const
	{
		return enableDepthBias;
	}

	/*!
	   \brief Return the line width
	 */
	float32 getLineWidth()const
	{
		return lineWidth;
	}

	/*!*********************************************************************************************************************
	\brief Set the face that will be culled (front/back/both/none).
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setCullFace(types::Face face)
	{
		this->cullFace = face;
		return *this;
	}

	/*!
	   \brief Set line width
	   \param lineWidth
	   \return Return this object (allows chained calls)
	 */
	RasterStateCreateParam& setLineWidth(float32 lineWidth)
	{
		this->lineWidth = lineWidth;
		return *this;
	}

	/*!
	   \brief Set depth clip
	   \param enableDepthClip
	   \return Return this object (allows chained calls)
	 */
	RasterStateCreateParam& setDepthClip(bool enableDepthClip)
	{
		this->enableDepthClip = enableDepthClip;
		return *this;
	}

	/*!
	   \brief Set depth bias
	   \param enableDepthBias
	   \return Return this object (allows chained calls)
	 */
	RasterStateCreateParam& setDepthBias(bool enableDepthBias)
	{
		this->enableDepthBias = enableDepthBias;
		return *this;
	}

	/*!
	   \brief Set depth bias clamp
	   \param enableDepthBiasClamp
	   \return Return this object (allows chained calls)
	 */
	RasterStateCreateParam& setDepthBiasClamp(bool enableDepthBiasClamp)
	{
		this->enableDepthBiasClamp = enableDepthBiasClamp;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set polygon winding order.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setFrontFaceWinding(types::PolygonWindingOrder frontFaceWinding)
	{
		this->frontFaceWinding = frontFaceWinding;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/disable rasterizer discard.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setRasterizerDiscard(bool enable)
	{
		this->enableRasterizerDiscard = enable;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/disable Program Point Size.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setProgramPointSize(bool enable)
	{
		enableProgramPointSize = enable;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set polygon fill mode.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setFillMode(types::FillMode mode)
	{
		fillMode = mode;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the Provoking Vertex.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setProvokingVertex(types::ProvokingVertex provokingVertex)
	{
		this->provokingVertex = provokingVertex;
		return *this;
	}
};

struct MultiSampleStateCreateParam
{
private:
	friend class ::pvr::api::impl::GraphicsPipeline_;
	bool stateEnabled;
	bool sampleShadingEnable;
	bool alphaToCoverageEnable;
	bool alphaToOneEnable;
	types::SampleCount rasterizationSamples;
	float32 minSampleShading;
	uint32 sampleMask;

public:
	/*!*********************************************************************************************************************
	\brief Constructor. Create a multisampling configuration.
	\param stateEnabled Enable/disable multisampling (default false)
	\param sampleShadingEnable Enable/disable sample shading (defalt false)
	\param alphaToCoverageEnable Enable/disable alpha-to-coverage
	\param alphaToOneEnable Enable/disable alpha-to-one
	\param rasterizationSamples The number of rasterization samples (default 1)
	\param minSampleShading The minimum sample Shading (default 0)
	\param sampleMask sampleMask (default 0)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam(bool stateEnabled = types::PipelineDefaults::MultiSample::Enabled,
	                            bool sampleShadingEnable = types::PipelineDefaults::MultiSample::SampleShading,
	                            bool alphaToCoverageEnable = types::PipelineDefaults::MultiSample::AlphaToCoverageEnable,
	                            bool alphaToOneEnable = types::PipelineDefaults::MultiSample::AlphaToOnEnable,
	                            types::SampleCount rasterizationSamples = types::PipelineDefaults::MultiSample::RasterizationSamples,
	                            float32 minSampleShading = types::PipelineDefaults::MultiSample::MinSampleShading,
	                            uint32 sampleMask = types::PipelineDefaults::MultiSample::SampleMask) :
		stateEnabled(stateEnabled), sampleShadingEnable(sampleShadingEnable),
		alphaToCoverageEnable(alphaToCoverageEnable), alphaToOneEnable(alphaToOneEnable),
		rasterizationSamples(rasterizationSamples),	minSampleShading(minSampleShading),	sampleMask(sampleMask) {}

	/*!*********************************************************************************************************************
	\brief Enable/disable multisampling
	\param active true enable, false disable if the pipeline has rasterization disabled.
	\return this (allow chaining)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam& enableState(bool active)
	{
		stateEnabled = active;
		return *this;
	}

	/*!
	   \brief Enable/ Disable alpha to coverage
	   \param enable
	   \return Return this object (allows chained calls)
	 */
	MultiSampleStateCreateParam& setAlphaToCoverage(bool enable)
	{
		alphaToCoverageEnable = enable;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Enable/ disable sampler shading.
	\param enable true enable, false disable
	\return this (allow chaining)
	************************************************************************************************************************/
	MultiSampleStateCreateParam& setSampleShading(bool enable)
	{
		sampleShadingEnable = enable;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Controls whether the alpha component of the fragment’s first color output is replaced with one
	\param enable true enable.false disable
	\return this (allow chaining)
	************************************************************************************************************************/
	MultiSampleStateCreateParam& setAlphaToOne(bool enable)
	{
		alphaToOneEnable = enable;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief  Set the number of samples per pixel used in rasterization
	\param numSamples The number of samples.
	\return this (allow chaining)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam& setNumRasterizationSamples(types::SampleCount numSamples)
	{
		this->rasterizationSamples = numSamples;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set minimum sample shading.
	\param minSampleShading The number of minimum samples to shade
	\return this (allow chaining)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam& setMinSampleShading(float32 minSampleShading)
	{
		this->minSampleShading = minSampleShading;
		return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set sample mask.
	\param mask The sample mask.
	\return this (allow chaining)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam& setSampleMask(uint32 mask)
	{
		sampleMask = mask;
		return *this;
	}

	/*!
	   \brief Return sample mask
	 */
	uint32 getSampleMask()const { return sampleMask; }

	/*!
	   \brief Return number of rasterization samples
	 */
	uint32 getNumRasterizationSamples()const {	return (uint32)rasterizationSamples;  }

	/*!
	   \brief Return min sample shading
	 */
	float32 getMinSampleShading()const { return minSampleShading; }

	/*!
	   \brief Return true if sample shading enabled
	 */
	bool isSampleShadingEnabled()const { return sampleShadingEnable; }

	/*!
	   \brief Return true if alpha to coverage is enabled.
	 */
	bool isAlphaToCoverageEnabled()const { return alphaToCoverageEnable; }

	/*!
	   \brief Return true if alpha to one is enabled.
	 */
	bool isAlphaToOneEnabled()const { return alphaToOneEnable; }

	/*!
	   \brief Return true if this state is enabled
	 */
	bool isStateEnabled()const { return stateEnabled; }
};

/*!
   \brief Pipeline Dynamic states.
 */
struct DynamicStatesCreateParam
{
private:
	types::DynamicState dynamicStates[(uint32)types::DynamicState::Count];
public:
	/*!
	   \brief DynamicStatesCreateParam
	 */
	DynamicStatesCreateParam() { memset(dynamicStates, -1, sizeof(dynamicStates)); }

	/*!
	   \brief Return true if a dynamic static is enabled.
	   \param state
	 */
	bool isDynamicStateEnabled(types::DynamicState state)const { return (dynamicStates[(uint32)state] == state); }

	/*!
	   \brief Set dynamic state
	   \param state
	   \return Return this object(allows chained calls)
	 */
	DynamicStatesCreateParam& setDynamicState(types::DynamicState state)
	{
		dynamicStates[(uint32)state] = state;
		return *this;
	}
};

struct ShaderConstantInfo
{
	uint32 constantId;
	byte   data[16];// max can hold 4x4 matrix
	types::GpuDatatypes::Enum gpuDataType;
	uint32 sizeInBytes;
	ShaderConstantInfo() {}

	ShaderConstantInfo(uint32 constantId, int32 data) : constantId(constantId),
		gpuDataType(types::GpuDatatypes::integer), sizeInBytes(sizeof(data))
	{
		memcpy(this->data, &data, sizeof(data));
	}

	ShaderConstantInfo(uint32 constantId, float32 data) : constantId(constantId),
		gpuDataType(types::GpuDatatypes::float32), sizeInBytes(sizeof(data))
	{
		memcpy(this->data, &data, sizeof(data));
	}

	ShaderConstantInfo(uint32 constantId, const glm::vec3& data) : constantId(constantId),
		gpuDataType(types::GpuDatatypes::vec3), sizeInBytes(sizeof(data))
	{
		memcpy(this->data, &data, sizeof(data));
	}

	ShaderConstantInfo(uint32 constantId, const glm::vec4& data) : constantId(constantId),
		gpuDataType(types::GpuDatatypes::vec4), sizeInBytes(sizeof(data))
	{
		memcpy(this->data, &data, sizeof(data));
	}

	ShaderConstantInfo(uint32 constantId, const glm::mat4x4& data) : constantId(constantId),
		gpuDataType(types::GpuDatatypes::mat4x4), sizeInBytes(sizeof(data))
	{
		memcpy(this->data, &data, sizeof(data));
	}

	ShaderConstantInfo(uint32 constantId, const glm::mat3x3& data) : constantId(constantId),
		gpuDataType(types::GpuDatatypes::mat3x3), sizeInBytes(sizeof(data))
	{
		memcpy(this->data, &data, sizeof(data));
	}
};

/*!*********************************************************************************************************************
\brief Pipeline vertex Shader stage create param.
***********************************************************************************************************************/
struct ShaderStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
private:
	api::Shader shader;
	std::vector<ShaderConstantInfo> shaderConsts;
	std::string entryPoint;
public:

	/*!*********************************************************************************************************************
	\brief Constructor.
	***********************************************************************************************************************/
	ShaderStageCreateParam() : entryPoint(types::PipelineDefaults::ShaderStage::EntryPoint) {}

	/*!*********************************************************************************************************************
	\brief Construct from a api::Shader object
	\param shader A vertex shader
	***********************************************************************************************************************/
	ShaderStageCreateParam(const api::Shader& shader) : shader(shader),
		entryPoint(types::PipelineDefaults::ShaderStage::EntryPoint) {}

	/*!
	   \brief Return the shader
	 */
	const api::Shader& getShader() const { return shader; }

	/*!
	   \brief Return true if this state is active
	 */
	bool isActive()const { return shader.isValid(); }


	/*!*********************************************************************************************************************
	\brief Set vertex shader.
	\param shader A vertex shader
	***********************************************************************************************************************/
	void setShader(const api::Shader& shader) { this->shader = shader; }

	/*!
	   \brief Set shader entry point (default: main)
	   \param entryPoint
	 */
	void setEntryPoint(const char* entryPoint) { this->entryPoint = entryPoint; }

	/*!
	   \brief Return shader entry point
	 */
	const char* getEntryPoint()const { return entryPoint.c_str(); }

	/*!
	   \brief operator =
	   \param shader
	   \return Return true if equal
	 */
	ShaderStageCreateParam& operator=(const api::Shader& shader) { setShader(shader); return *this; }

	/*!*
	\brief Set shader constants
	\param index
	\param shaderConst
	\return Return this for chaining
	 */
	ShaderStageCreateParam& setShaderConstant(uint32 index, const ShaderConstantInfo& shaderConst)
	{
		if (shaderConsts.size() <= index)
		{
			shaderConsts.resize(index + 1);
		}
		shaderConsts[index] = shaderConst;
		return *this;
	}

	/*!*
	\brief Set all the shader constants.
	\details Uses better memory reservation than the setShaderConstant counterpart.
	\param shaderConsts A c-style array containing the shader constants
	\param numConstants The number of shader constants in \p shaderConsts
	\return Return this (allow chaining)
	*/
	ShaderStageCreateParam& setShaderConstants(const ShaderConstantInfo* shaderConsts, uint32 numConstants)
	{
		this->shaderConsts.insert(this->shaderConsts.begin(), shaderConsts, shaderConsts + numConstants);
		return *this;
	}

	/*!*
	\brief Retrieve a ShaderConstant
	\param index The index of the ShaderConstant to retrieve
	\return The shader constant
	 */
	const ShaderConstantInfo& getShaderConstant(uint32 index)const
	{
		assertion(index < shaderConsts.size());
		return shaderConsts[index];
	}

	/*!*
	\brief Get all shader constants
	\return Return an array of shader of constants
	 */
	const ShaderConstantInfo* getAllShaderConstants()const
	{
		return shaderConsts.data();
	}

	/*!*
	\brief Get number of shader constants
	\return Number of shader constants
	 */
	uint32 getNumShaderConsts()const
	{
		return (uint32)shaderConsts.size();
	}
};

/*!
   \brief The VertexShaderStageCreateParam struct
 */
struct VertexShaderStageCreateParam : public ShaderStageCreateParam
{
	ShaderStageCreateParam& operator=(const api::Shader& shader)
	{
		setShader(shader);
		return *this;
	}
};

/*!
   \brief The FragmentShaderStageCreateParam struct
 */
struct FragmentShaderStageCreateParam : public ShaderStageCreateParam
{
	ShaderStageCreateParam& operator=(const api::Shader& shader)
	{
		setShader(shader);
		return *this;
	}
};

/*!
   \brief The GeometryShaderStageCreateParam struct
 */
struct GeometryShaderStageCreateParam : public ShaderStageCreateParam
{
	ShaderStageCreateParam& operator=(const api::Shader& shader)
	{
		setShader(shader);
		return *this;
	}
};

/*!
   \brief The ComputeShaderStageCreateParam struct
 */
struct ComputeShaderStageCreateParam : public ShaderStageCreateParam
{
	ShaderStageCreateParam& operator=(const api::Shader& shader)
	{
		setShader(shader);
		return *this;
	}
};

/*!*********************************************************************************************************************
\brief: Pipeline Tesselation Control shader stage create param.
***********************************************************************************************************************/
struct TesselationStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
private:
	api::Shader controlShader, evalShader;
	uint32 patchControlPoints;
	std::vector<ShaderConstantInfo> shaderConstsTessCtrl, shaderConstTessEval;

	/*!*********************************************************************************************************************
	\brief: Create pipeline state object.
	***********************************************************************************************************************/
public:
	/*!
	   \brief Return control shader
	 */
	const api::Shader& getControlShader() const { return controlShader; }

	/*!
	   \brief Return evaluation shader
	 */
	const api::Shader& getEvaluationShader()const { return evalShader; }

	/*!
	   \brief Return true if the control shader is active
	 */
	bool isControlShaderActive()const { return controlShader.isValid(); }

	/*!
	   \brief Return true if the evaluation shader is active
	 */
	bool isEvaluationShaderActive()const { return evalShader.isValid();}

	/*!
	   \brief Constructor
	 */
	TesselationStageCreateParam() : patchControlPoints(types::PipelineDefaults::Tesselation::NumControlPoints) {}

	/*!*********************************************************************************************************************
	\brief: Set control shader.
	***********************************************************************************************************************/
	TesselationStageCreateParam& setControlShader(const api::Shader& shader)
	{
		controlShader = shader;
		return *this;
	}

	/*!*
	\brief Set evaluation shader
	\param shader The shader
	\return this for chaining
	 */
	TesselationStageCreateParam& setEvaluationShader(const api::Shader& shader)
	{
		evalShader = shader;
		return *this;
	}

	/*!*
	\brief Set number of control points
	\param controlPoints
	\return this for chaining
	 */
	TesselationStageCreateParam& setNumPatchControlPoints(uint32 controlPoints)
	{
		patchControlPoints = controlPoints;
		return *this;
	}

	/*!*
	\brief Get number of control points
	\return The number of patch control points
	 */
	uint32 getNumPatchControlPoints()const { return patchControlPoints; }

	/*!*
	\brief Set control shader constans
	\param index
	\param shaderConst
	\return Return this for chaining
	 */
	TesselationStageCreateParam& setControlShaderConstant(uint32 index, const ShaderConstantInfo& shaderConst)
	{
		if (shaderConstsTessCtrl.size() <= index)
		{
			shaderConstsTessCtrl.resize(index + 1);
		}
		shaderConstsTessCtrl[index] = shaderConst;
		return *this;
	}


	/*!*
	\brief Set all the shader constants.
	\details Uses better memory reservation than the setShaderConstant counterpart.
	\param shaderConsts A c-style array containing the shader constants
	\param numConstants The number of shader constants in \p shaderConsts
	\return Return this (allow chaining)
	*/
	TesselationStageCreateParam& setControlShaderConstants(const ShaderConstantInfo* shaderConsts, uint32 numConstants)
	{
		this->shaderConstsTessCtrl.insert(this->shaderConstsTessCtrl.begin(), shaderConsts, shaderConsts + numConstants);
		return *this;
	}

	/*!*
	\brief Get Control shader constant
	\param index
	\return ShaderConstantInfo
	 */
	const ShaderConstantInfo& getControlShaderConstant(uint32 index)const
	{
		assertion(index < shaderConstsTessCtrl.size());
		return shaderConstsTessCtrl[index];
	}

	/*!
	   \brief Return all control shader constants
	   \return C-style array of all shader constants
	 */
	const ShaderConstantInfo* getAllControlShaderConstants()const { return shaderConstsTessCtrl.data(); }

	/*!
	   \brief Return number of control shader constants
	 */
	uint32 getNumControlShaderConstants()const { return (uint32)shaderConstsTessCtrl.size(); }

	/*!*
	\brief Set evaluation shader constants
	\param index
	\param shaderConst
	\return Return this for chaining
	 */
	void setEvaluationShaderConstant(uint32 index, const ShaderConstantInfo& shaderConst)
	{
		if (shaderConstTessEval.size() <= index)
		{
			shaderConstTessEval.resize(index + 1);
		}
		shaderConstTessEval[index] = shaderConst;
	}

	/*!*
	\brief Set all the shader constants.
	\details Uses better memory reservation than the setShaderConstant counterpart.
	\param shaderConsts A c-style array containing the shader constants
	\param numConstants The number of shader constants in \p shaderConsts
	\return Return this (allow chaining)
	*/
	TesselationStageCreateParam& setEvaluationShaderConstants(const ShaderConstantInfo* shaderConsts, uint32 numConstants)
	{
		this->shaderConstTessEval.insert(this->shaderConstTessEval.begin(), shaderConsts, shaderConsts + numConstants);
		return *this;
	}

	/*!*
	\brief Get Evaluation shader constants
	\param index The
	\return ShaderConstantInfo
	 */
	const ShaderConstantInfo& getEvaluationlShaderConstant(uint32 index)const
	{
		assertion(index < shaderConstTessEval.size());
		return shaderConstTessEval[index];
	}

	/*!
	   \brief Return all evaluationshader constants
	 */
	const ShaderConstantInfo* getAllEvaluationShaderConstants()const { return shaderConstTessEval.data(); }

	/*!
	   \brief Return number of evaluatinon shader constants
	 */
	uint32 getNumEvaluatinonShaderConstants()const { return (uint32)shaderConstTessEval.size(); }
};

/*!************************************************************************************************************
\brief	OGLES2TextureUnitBindings struct.
		This struct does the shader's texture unit reflection as the shader does not support layout qualifiers.
		ONLY takes effect for OPENGLES.
**************************************************************************************************************/
struct OGLES2TextureUnitBindings
{
private:
	std::vector<std::string> texUnit;
public:
	/*!*******************************************************************************************************
	\brief Set texture unit.
	\param unit Texture binding unit. Unit must be consecutive
	\param name Texture binding name
	\return This object
	**********************************************************************************************************/
	OGLES2TextureUnitBindings& setTextureUnit(uint32 unit, const char* name)
	{
		if (unit >= texUnit.size())
		{
			texUnit.resize(unit + 1);
		}
		texUnit[unit] = name;
		return *this;
	}

	/*!*******************************************************************************************************
	\brief Return texture unit binding name
	\param unit Texture unit
	**********************************************************************************************************/
	const char* getTextureUnitName(uint32 unit)const
	{
		assertion(unit < texUnit.size(), "Invalid binding id");
		return texUnit[unit].c_str();
	}

	/*!*******************************************************************************************************
	\brief Return texture unit binding id
	\param name Texture binding name
	**********************************************************************************************************/
	int32 getTextureUnitId(const char* name)const
	{
		std::vector<std::string>::const_iterator found = std::find(texUnit.cbegin(), texUnit.cend(), string(name));
		return (int32)(found != texUnit.end() ? texUnit.end() - found : -1);
	}

	/*!*******************************************************************************************************
	\brief Return number of bindings
	**********************************************************************************************************/
	uint32 getNumBindings()const
	{
		return (uint32)texUnit.size();
	}
};
}
}
}
