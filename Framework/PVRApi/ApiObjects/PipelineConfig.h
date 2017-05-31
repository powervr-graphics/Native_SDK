/*!
\brief Contains the CreateParameters for the Pipeline Config States, used to set states to the PipelineCreateParam
objects.
\file PVRApi/ApiObjects/PipelineConfig.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/Texture.h"
#include "PVRCore/DataStructures/SortedArray.h"
#include <map>
#include <vector>
namespace pvr {
namespace api {

/// <summary>Contains a full description of a Vertex Attribute: Index, format, number of elements, offset in the
/// buffer, optionally name. All values (except attributeName) must be set explicitly.</summary>
struct VertexAttributeInfo
{
	uint16 index;     //!< Attribute index
	types::DataType format; //!< Data type of each element of the attribute
	uint8 width;      //!< Number of elements in attribute, e.g 1,2,3,4
	uint32 offsetInBytes; //!< Offset of the first element in the buffer
	std::string attribName;   //!< Optional: Name(in the shader) of the attribute

	/// <summary>Default constructor. Uninitialized values, except for AttributeName.</summary>
	VertexAttributeInfo(): index(0),
		format(types::DataType::None),
		width(0),
		offsetInBytes(0),
		attribName("") {}

	/// <summary>Create a new VertexAttributeInfo object.</summary>
	/// <param name="index">Attribute binding index</param>
	/// <param name="format">Attribute data type</param>
	/// <param name="width">Number of elements in attribute</param>
	/// <param name="offsetInBytes">Interleaved: offset of the attribute from the start of data of each vertex</param>
	/// <param name="attribName">Name of the attribute in the shader.</param>
	VertexAttributeInfo(uint16 index, types::DataType format, uint8 width,
	                    uint32 offsetInBytes,
	                    const char* attribName = "") :
		index(index), format(format), width(width), offsetInBytes(offsetInBytes), attribName(attribName) {}

	/// <summary>Return true if the right hand object is equal to this</summary>
	bool operator==(VertexAttributeInfo const& rhs)const
	{
		return ((index == rhs.index) && (format == rhs.format) &&
		        (width == rhs.width) && (offsetInBytes == rhs.offsetInBytes));
	}

	/// <summary>Return true if the right hand object is not equal to this</summary>
	bool operator!=(VertexAttributeInfo const& rhs)const { return !((*this) == rhs); }

};

/// <summary>Information about a Buffer binding: Binding index, stride, (instance) step rate.</summary>
struct VertexInputBindingInfo
{
	uint16 bindingId;//< buffer binding index
	uint32 strideInBytes; //< buffer stride in bytes
	types::StepRate stepRate;//< buffer step rate

	/// <summary>Construct with Uninitialized values.</summary>
	VertexInputBindingInfo() {}

	/// <summary>Add a buffer binding.</summary>
	/// <param name="bindId">Buffer binding point</param>
	/// <param name="strideInBytes">Buffer stride of each vertex attribute to the next</param>
	/// <param name="stepRate">Vertex Attribute Step Rate</param>
	VertexInputBindingInfo(uint16 bindId, uint32 strideInBytes,  types::StepRate stepRate = types::StepRate::Vertex) :
		bindingId(bindId), strideInBytes(strideInBytes), stepRate(stepRate) {}
};

/// <summary>A container struct carrying Vertex Attribute information (vertex layout, plus binding point)
struct VertexAttributeInfoWithBinding : public VertexAttributeInfo
{
	/// <summary>The Vertex Buffer binding point this attribute is bound to
	uint16 binding;
	VertexAttributeInfoWithBinding() {}

	VertexAttributeInfoWithBinding(const VertexAttributeInfo& nfo, uint16 binding) :
		VertexAttributeInfo(nfo), binding(binding) {}

	VertexAttributeInfoWithBinding(
	  uint16 index, types::DataType format, uint8 width, uint32 offsetInBytes, uint16 binding,
	  const char* attribName = "")
		: VertexAttributeInfo(index, format, width, offsetInBytes, attribName), binding(binding) {}
};

//!\cond NO_DOXYGEN

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

//!\endcond

/// <summary>Viewport specifes the drawing region, min and max depth.
/// The viewport region x and y starts bottom left as similar to opengl
/// </summary>
struct Viewport
{
	float32 x;//!< region x
	float32 y;//!< region y
	float32 width;//!< region width
	float32 height;//!< region height
	float32 minDepth;//!< min depth
	float32 maxDepth;//!< max depth

	/// <summary>ctor</summary>
	/// <param name="x">viewport x</param>
	/// <param name="y">viewport y</param>
	/// <param name="width">viewport width</param>
	/// <param name="height">viewport height</param>
	/// <param name="minDepth">depth min</param>
	/// <param name="maxDepth">depth max</param>
	Viewport(float32 x = 0, float32 y = 0, float32 width = 1, float32 height = 1,
	         float32 minDepth = 0.f, float32 maxDepth = 1.f) :
		x(x), y(y), width(width), height(height), minDepth(minDepth), maxDepth(maxDepth) {}

	/// <summary>ctor</summary>
	/// <param name="rect">viewport</param>
	/// <param name="minDepth">depth min</param>
	/// <param name="maxDepth">depth max</param>
	Viewport(const Rectanglei& rect, float32 minDepth = 0.f, float32 maxDepth = 1.f) :
		x((float32)rect.x), y((float32)rect.y), width((float32)rect.width), height((float32)rect.height),
		minDepth(minDepth), maxDepth(maxDepth) {}

	Rectanglef getRegion()const { return Rectanglef(x, y, width, height); }

};
//!\cond NO_DOXYGEN
typedef std::vector<VertexInputBindingInfo> VertexInputBindingMap;
typedef std::vector<VertexAttributeInfoWithBinding> VertexAttributeMap;
typedef types::StencilState StencilState;
//!\endcond NO_DOXYGEN

namespace pipelineCreation {
/// <summary>Contains parameters needed to set depth stencil states to a pipeline create params. This object can be
/// added to a PipelineCreateParam to set a depth-stencil state to values other than their defaults.</summary>
/// <remarks>--- Defaults: --- depthWrite:enabled, depthTest:enabled, DepthComparison:Less, Stencil Text:
/// disabled, All stencil ops:Keep,</remarks>
struct DepthStencilStateCreateParam
{
public:
	typedef api::StencilState StencilState;
private:
	// stencil
	bool _depthTest;  //!< Enable/disable depth test. Default false
	bool _depthWrite; //!< Enable/ disable depth write
	bool _stencilTestEnable; //!< Enable/disable stencil test. Default false
	bool _depthBoundTest;//!< Enable/ disable depth bound test
	bool _enableDepthStencilState;//!< Enable/ Disable this state
	float32 _minDepth;//!< Depth minimum
	float32 _maxDepth;//!< Depth maximum

	StencilState _stencilFront;//!< Stencil state front
	StencilState _stencilBack;//!< Stencil state back

	types::ComparisonMode _depthCmpOp; //!< Depth compare operation. Default LESS
public:
	/// <summary>Set all Depth and Stencil parameters.</summary>
	DepthStencilStateCreateParam(bool depthWrite = true,
	                             bool depthTest = false,
	                             types::ComparisonMode depthCompareFunc = types::ComparisonMode::Less,
	                             bool stencilTest = false,
	                             bool depthBoundTest = false,
	                             const StencilState& stencilFront = StencilState(),
	                             const StencilState& stencilBack = StencilState(),
	                             float32 minDepth = 0.f,
	                             float32 maxDepth = 1.f)
		:
		_depthTest(depthTest), _depthWrite(depthWrite), _stencilTestEnable(stencilTest),
		_depthBoundTest(depthBoundTest), _enableDepthStencilState(true),
		_minDepth(minDepth), _maxDepth(maxDepth),
		_stencilFront(stencilFront), _stencilBack(stencilBack), _depthCmpOp(depthCompareFunc) {}

	/// <summary>Return true if depth test is enable</summary>
	bool isDepthTestEnable()const
	{
		return _depthTest;
	}

	/// <summary>Return true if depth write is enable</summary>
	bool isDepthWriteEnable()const
	{
		return _depthWrite;
	}

	/// <summary>Return true if depth bound is enable</summary>
	bool isDepthBoundTestEnable()const
	{
		return _depthBoundTest;
	}

	/// <summary>Return true if stencil test is enable</summary>
	bool isStencilTestEnable()const
	{
		return _stencilTestEnable;
	}

	/// <summary>Return minimum depth value</summary>
	float32 getMinDepth()const
	{
		return _minDepth;
	}

	/// <summary>Return maximum depth value</summary>
	float32 getMaxDepth()const
	{
		return _maxDepth;
	}

	/// <summary>Return depth comparison operator</summary>
	types::ComparisonMode getDepthComapreOp()const
	{
		return _depthCmpOp;
	}

	/// <summary>Return true if this state is enabled.</summary>
	bool isStateEnable()const
	{
		return _enableDepthStencilState;
	}

	/// <summary>Enable/ Disale this state</summary>
	/// <param name="flag">True:enable, False:disable</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& enableState(bool flag)
	{
		_enableDepthStencilState = flag;
		return *this;
	}

	/// <summary>Enable/disable writing into the Depth Buffer.</summary>
	/// <param name="depthWrite">True:enable, False:disable</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& setDepthWrite(bool depthWrite)
	{
		_depthWrite = depthWrite;
		return *this;
	}

	/// <summary>Enable/disable depth test (initial state: enabled)</summary>
	/// <param name="depthTest">True:enable, False:disable</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& setDepthTestEnable(bool depthTest)
	{
		_depthTest = depthTest;
		return *this;
	}

	/// <summary>Set the depth compare function (initial state: LessEqual)</summary>
	/// <param name="compareFunc">A ComparisonMode (Less, Greater, Less etc.)</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& setDepthCompareFunc(types::ComparisonMode compareFunc)
	{
		_depthCmpOp = compareFunc;
		return *this;
	}

	/// <summary>Enable/disable stencil test.</summary>
	/// <param name="stencilTest">True:enable, False:disable</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& setStencilTest(bool stencilTest)
	{
		_stencilTestEnable = stencilTest;
		return *this;
	}

	/// <summary>Set the stencil front state</summary>
	/// <param name="stencil">Stencil state</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& setStencilFront(const StencilState& stencil)
	{
		_stencilFront = stencil;
		return *this;
	}

	/// <summary>Set the stencil back state</summary>
	/// <param name="stencil">Stencil state</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& setStencilBack(const StencilState& stencil)
	{
		_stencilBack = stencil;
		return *this;
	}

	/// <summary>Set the stencil front and back state</summary>
	/// <param name="stencil">Stencil state</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& setStencilFrontBack(StencilState& stencil)
	{
		_stencilFront = stencil, _stencilBack = stencil;
		return *this;
	}

	/// <summary>Return stencil front state</summary>
	const StencilState& getStencilFront()const
	{
		return _stencilFront;
	}

	/// <summary>Return stencil back state</summary>
	const StencilState& getStencilBack()const
	{
		return _stencilBack;
	}


	/// <summary>Enable/ Disable depth bound testing</summary>
	/// <param name="enabled">True:enable, False:disable</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& setDepthBoundEnabled(bool enabled)
	{
		_depthBoundTest = enabled;
		return *this;
	}

	/// <summary>Set the minimum depth bound</summary>
	/// <param name="minDepth">The minimum depth bound</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& setMinDepthBound(float minDepth)
	{
		_minDepth = minDepth;
		return *this;
	}

	/// <summary>Set the maximum depth bound</summary>
	/// <param name="maxDepth">The maximum depth bound</param>
	/// <returns>this object (allows chained calls)</returns>
	DepthStencilStateCreateParam& setMaxDepthBound(float maxDepth)
	{
		_maxDepth = maxDepth;
		return *this;
	}
};

/// <summary>Contains parameters needed to configure the Vertex Input for a pipeline object. (vertex attrubutes, input
/// bindings etc.). Use by adding the buffer bindings with (setInputBinding) and then configure the attributes with
/// (addVertexAttribute). Default settings: 0 Vertext buffers, 0 vertex attributes.</summary>
struct VertexInputCreateParam
{
private:
	friend class ::pvr::api::impl::GraphicsPipeline_;

	VertexInputBindingMap _inputBindings;
	VertexAttributeMap _attributes;
public:

	/// <summary>Return the input bindings</summary>
	const VertexInputBindingMap& getInputBindings() const
	{
		return _inputBindings;
	}

	/// <summary>Return the vertex attributes</summary>
	const VertexAttributeMap& getAttributes() const
	{
		return _attributes;
	}

	/// <summary>Clear this object.</summary>
	/// <returns>this object (allows chained calls)</returns>
	VertexInputCreateParam& clear()
	{
		_inputBindings.clear();
		_attributes.clear();
		return *this;
	}

	/// <summary>Set the vertex input buffer bindings.</summary>
	/// <param name="bufferBinding">Vertex buffer binding index</param>
	/// <param name="strideInBytes">specifies the byte offset between consecutive generic vertex attributes. If stride is 0,
	/// the generic vertex attributes are understood to be tightly packed in the array. The initial value is 0.
	/// </param>
	/// <param name="stepRate">The rate at which this binding is incremented (used for Instancing).</param>
	/// <returns>this object (allows chained calls)</returns>
	VertexInputCreateParam& setInputBinding(
	  uint16 bufferBinding, uint16 strideInBytes = 0, types::StepRate stepRate = types::StepRate::Vertex)
	{
		utils::insertSorted_overwrite(_inputBindings, VertexInputBindingInfo(bufferBinding, strideInBytes,
		                              stepRate), VertexBindingInfoCmp_BindingLess());
		return *this;
	}

	/// <summary>Return a VertexBindingInfo for a buffer binding index, else return NULL if not found</summary>
	/// <param name="bufferBinding">Buffer binding index</param>
	const VertexInputBindingInfo* getInputBinding(uint32 bufferBinding) const
	{
		for (const VertexInputBindingInfo& it : _inputBindings)
		{
			if (it.bindingId == bufferBinding) { return &it; }
		}
		return NULL;
	}


	/// <summary>Add vertex layout information to a buffer binding index using a VertexAttributeInfo object.
	/// </summary>
	/// <param name="bufferBinding">The binding index to add the vertex attribute information.</param>
	/// <param name="attrib">Vertex Attribute information object.</param>
	/// <returns>this object (allows chained calls)</returns>
	VertexInputCreateParam& addVertexAttribute(uint16 bufferBinding, const VertexAttributeInfo& attrib)
	{
		utils::insertSorted_overwrite(_attributes, VertexAttributeInfoWithBinding(attrib, bufferBinding),
		                              VertexAttributeInfoCmp_BindingLess_IndexLess());
		return *this;
	}

	/// <summary>Add vertex layout information to a buffer binding index using an array of VertexAttributeInfo object.
	/// </summary>
	/// <param name="bufferBinding">The binding index to add the vertex attribute information.</param>
	/// <param name="attrib">Attribute information object.</param>
	/// <param name="numAttributes">Number of attributues in the array</param>
	/// <returns>this object (allows chained calls)</returns>
	VertexInputCreateParam& addVertexAttributes(uint16 bufferBinding, const VertexAttributeInfo* attrib,
	    uint32 numAttributes)
	{
		for (uint32 i = 0; i < numAttributes; ++i)
		{
			utils::insertSorted_overwrite(_attributes, VertexAttributeInfoWithBinding(attrib[i], bufferBinding),
			                              VertexAttributeInfoCmp_BindingLess_IndexLess());
		}
		return *this;
	}

	/// <summary>Add vertex layout information to a buffer binding index using a VertexAttributeLayout object and an
	/// attrib name.</summary>
	/// <param name="index">The index of the vertex attribute</param>
	/// <param name="bufferBinding">The binding index of the buffer from which vertex data will be read.</param>
	/// <param name="layout">Vertex Attribute Layout object</param>
	/// <param name="attributeName">The name of the variable in shader code. Required for API's that only support
	/// Reflective attribute binding and not Explicit binding of attributes to indexes in shader code.</param>
	/// <returns>this object (allows chained calls)</returns>
	VertexInputCreateParam& addVertexAttribute(
	  uint16 index, uint16 bufferBinding, const types::VertexAttributeLayout& layout, const char* attributeName = "")
	{
		utils::insertSorted_overwrite(_attributes, VertexAttributeInfoWithBinding(index, layout.dataType,
		                              layout.width, layout.offset, bufferBinding, attributeName),
		                              VertexAttributeInfoCmp_BindingLess_IndexLess());
		return *this;
	}
};

/// <summary>Add Input Assembler configuration to this buffer object (primitive topology, vertex restart, vertex
/// reuse etc).</summary>
/// <remarks>--- Default settings --- Primitive Topology: TriangleList, Primitive Restart: False, Vertex Reuse:
/// Disabled, Primitive Restart Index: 0xFFFFFFFF</remarks>
struct InputAssemblerStateCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
	mutable types::PrimitiveTopology topology;
private:
	bool _disableVertexReuse;
	bool _primitiveRestartEnable;
	uint32 _primitiveRestartIndex;
public:
	/// <summary>Create and configure an InputAssembler configuration.</summary>
	/// <param name="topology">Primitive Topology (default: TriangleList)</param>
	/// <param name="disableVertexReuse">Disable Vertex Reuse (true:disabled false:enabled). Default:true</param>
	/// <param name="primitiveRestartEnable">(true: enabled, false: disabled) Default:false</param>
	/// <param name="primitiveRestartIndex">Primitive Restart Index. Default 0xFFFFFFFF</param>
	/// <returns>this object (allows chained calls)</returns>
	InputAssemblerStateCreateParam(types::PrimitiveTopology topology = types::PrimitiveTopology::TriangleList,
	                               bool disableVertexReuse = true,
	                               bool primitiveRestartEnable = false,
	                               uint32 primitiveRestartIndex = 0xFFFFFFFF):
		topology(topology), _disableVertexReuse(disableVertexReuse), _primitiveRestartEnable(primitiveRestartEnable),
		_primitiveRestartIndex(primitiveRestartIndex) {}

	/// <summary>Enable/ disable primitive restart.</summary>
	/// <param name="enable">true for enable, false for disable.</param>
	/// <returns>this object (allows chained calls)</returns>
	InputAssemblerStateCreateParam& setPrimitiveRestartEnable(bool enable)
	{
		_primitiveRestartEnable = enable;
		return *this;
	}

	/// <summary>Enable/ disable vertex reuse.</summary>
	/// <param name="disable">true for disable, false for enable.</param>
	/// <returns>this object (allows chained calls)</returns>
	InputAssemblerStateCreateParam& setVertexReuseDisable(bool disable)
	{
		_disableVertexReuse = disable;
		return *this;
	}

	/// <summary>Set primitive topology.</summary>
	/// <param name="topology">The primitive topology to interpret the vertices as (TriangleList, Lines etc)</param>
	/// <returns>this object (allows chained calls)</returns>
	InputAssemblerStateCreateParam& setPrimitiveTopology(types::PrimitiveTopology topology)
	{
		this->topology = topology;
		return *this;
	}

	/// <summary>Check if Vertex Reuse is disabled</summary>
	/// <returns>True if vertex reuse is disabled, otherwise false</returns>
	bool isVertexReuseDisabled() const { return _disableVertexReuse; }

	/// <summary>Check if primitive restart is enabled</summary>
	/// <returns>True if primitive restart is enabled, otherwise false</returns>
	bool isPrimitiveRestartEnabled() const { return _primitiveRestartEnable; }

	/// <summary>Get the primitive restart index</summary>
	/// <returns>The primitive restart index</returns>
	uint32 getPrimitiveRestartIndex() const { return _primitiveRestartIndex; }
};

/// <summary>Pipeline Color blending state configuration (alphaToCoverage, logicOp).</summary>
/// <remarks>Defaults: Enable alpha to coverage:false, Enable logic op: false, Logic Op: Set, Attachments: 0</remarks>
struct ColorBlendStateCreateParam
{
private:
	types::BlendingConfig _attachmentStates[types::PipelineDefaults::ColorBlend::MaxBlendAttachments];
	pvr::uint32 _attachmentStatesCount;
	friend class ::pvr::api::impl::GraphicsPipeline_;
	bool _alphaToCoverageEnable;
	bool _logicOpEnable;
	types::LogicOp _logicOp;
	glm::vec4 _colorBlendConstants;
public:
	const types::BlendingConfig* getAttachmentStates() const
	{
		return _attachmentStates;
	}
	/// <summary>Create a Color Blend state object.</summary>
	/// <param name="alphaToCoverageEnable">enable/ disable alpa to coverage (default:disable)</param>
	/// <param name="colorBlendConstants">Blending constants</param>
	/// <param name="logicOpEnable">enable/disable logicOp (default:disable)</param>
	/// <param name="logicOp">Select logic operation (default:Set)</param>
	/// <param name="attachmentStates">An array of color blend attachment states (default: NULL)</param>
	/// <param name="attachmentStatesCount">Number of color attachment states in array (default: 0)</param>
	ColorBlendStateCreateParam(bool alphaToCoverageEnable, bool logicOpEnable, types::LogicOp logicOp,
	                           glm::vec4 colorBlendConstants, types::BlendingConfig* attachmentStates, uint32 attachmentStatesCount) :
		_alphaToCoverageEnable(alphaToCoverageEnable), _logicOpEnable(logicOpEnable), _logicOp(logicOp),
		_attachmentStatesCount(0), _colorBlendConstants(colorBlendConstants)
	{
		debug_assertion(attachmentStatesCount < types::PipelineDefaults::ColorBlend::MaxBlendAttachments, "Blend Attachments out of range.");
		for (pvr::uint32 i = 0; i < attachmentStatesCount; i++)
		{
			_attachmentStates[i] = attachmentStates[i];
		}
		_attachmentStatesCount = attachmentStatesCount;
	}

	/// <summary>Create a Color Blend state object.</summary>
	/// <param name="alphaToCoverageEnable">enable/ disable alpa to coverage (default:disable</param>
	/// <param name="logicOpEnable">enable/disable logicOp (default:disable)</param>
	/// <param name="logicOp">Select logic operation (default:Set)</param>
	/// <param name="colorBlendConstants">Set color blend constants. Default (0,0,0,0)</param>
	ColorBlendStateCreateParam(bool alphaToCoverageEnable = false,
	                           bool logicOpEnable = false,
	                           types::LogicOp logicOp = types::LogicOp::Set,
	                           glm::vec4 colorBlendConstants = glm::vec4(0., 0., 0., 0.)) :
		_alphaToCoverageEnable(alphaToCoverageEnable), _logicOpEnable(logicOpEnable),
		_logicOp(logicOp), _colorBlendConstants(colorBlendConstants), _attachmentStatesCount(0)
	{}

	/// <summary>Set a constant for color blending</summary>
	/// <param name="blendConst">The color blend constant</param>
	/// <returns>Return this object (allows chained calls)</returns>
	ColorBlendStateCreateParam& setColorBlendConst(glm::vec4& blendConst)
	{
		_colorBlendConstants = blendConst;
		return *this;
	}

	/// <summary>Get the constant for color blending</summary>
	/// <returns>The color blend constant</returns>
	const glm::vec4& getColorBlendConst() const
	{
		return _colorBlendConstants;
	}

	types::BlendingConfig getAttachmentState(uint32 index) const
	{
		return _attachmentStates[index];
	}

	uint32 getAttachmentStatesCount()const
	{
		return _attachmentStatesCount;
	}

	/// <summary>Enable/ disable alpha to coverage.</summary>
	/// <returns>this object (allows chained calls)</returns>
	ColorBlendStateCreateParam& setAlphaToCoverageEnable(bool alphaToCoverageEnable)
	{
		_alphaToCoverageEnable = alphaToCoverageEnable;
		return *this;
	}

	/// <summary>Enable/ disable logic op.</summary>
	/// <returns>this object (allows chained calls)</returns>
	ColorBlendStateCreateParam& setLogicOpEnable(bool logicOpEnable)
	{
		_logicOpEnable = logicOpEnable;
		return *this;
	}

	/// <summary>Set the logic op.</summary>
	/// <returns>this object (allows chained calls)</returns>
	ColorBlendStateCreateParam& setLogicOp(types::LogicOp logicOp)
	{
		_logicOp = logicOp;
		return *this;
	}

	/// <summary>Append a color attachment blend configuration (appended to the end of the attachments list).</summary>
	/// <returns>this object (allows chained calls)</returns>
	ColorBlendStateCreateParam& clearAttachments()
	{
		for (pvr::uint32 i = 0; i < types::PipelineDefaults::ColorBlend::MaxBlendAttachments; i++)
		{
			_attachmentStates[i] = types::BlendingConfig();
		}
		_attachmentStatesCount = 0;
		return *this;
	}

	/// <summary>Add a color attachment state blend configuration to a specified index.</summary>
	/// <param name="index">Which index this color attachment will be</param>
	/// <param name="state">The color attachment state to add</param>
	/// <returns>this object (allows chained calls)</returns>
	ColorBlendStateCreateParam& setAttachmentState(uint32 index, const types::BlendingConfig& state)
	{
		debug_assertion(index < types::PipelineDefaults::ColorBlend::MaxBlendAttachments, "Blend config out of range.");
		_attachmentStates[index] = state;
		if (index >= _attachmentStatesCount)
		{
			_attachmentStatesCount = index + 1;
		}
		return *this;
	}

	/// <summary>Set all color attachment states as an array. Replaces any that had already been added.</summary>
	/// <param name="state">An array of color attachment states</param>
	/// <param name="count">The number of color attachment states in (state)</param>
	/// <returns>this object (allows chained calls)</returns>
	ColorBlendStateCreateParam& setAttachmentStates(uint32 count, types::BlendingConfig const* state)
	{
		debug_assertion(count < types::PipelineDefaults::ColorBlend::MaxBlendAttachments, "Blend config out of range.");
		for (pvr::uint32 i = 0; i < count; i++)
		{
			_attachmentStates[i] = state[i];
		}
		_attachmentStatesCount = count;
		return *this;
	}

	/// <summary>Check if Alpha to Coverage is enabled</summary>
	/// <returns>True if enabled, otherwise false</returns>
	bool isAlphaToCoverageEnabled() const { return _alphaToCoverageEnable; }

	/// <summary>Check if Logic Op is enabled</summary>
	/// <returns>True if enabled, otherwise false</returns>
	bool isLogicOpEnabled() const { return _logicOpEnable; }

	/// <summary>Get the Logic Op (regardless if enabled or not)</summary>
	/// <returns>The logic op</returns>
	types::LogicOp getLogicOp() const { return _logicOp; }

};

/// <summary>Pipeline Viewport state descriptor. Sets the base configuration of all viewports.</summary>
/// <remarks>Defaults: Number of Viewports:1, Clip Origin: lower lef, Depth range: 0..1</remarks>
struct ViewportStateCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
private:
	std::pair<Rectanglei, Viewport> _scissorViewports[types::PipelineDefaults::ViewportScissor::MaxScissorViewports];
	glm::ivec2 _renderSurfaceDimensions;
	pvr::uint32 _scissorViewportsCount;
public:
	/// <summary>Constructor.</summary>
	ViewportStateCreateParam() :
		_renderSurfaceDimensions(types::PipelineDefaults::ViewportScissor::SurfaceDimensions),
		_scissorViewportsCount(0)
	{}

	/// <summary>Configure the viewport with its corresponding scissor rectangle for an attachment</summary>
	/// <param name="index">The index of the attachment for which to set the viewport and scissor rectangle</param>
	/// <param name="viewport">The viewport to set for attachment <paramref name="index"/></param>
	/// <param name="scissor">The scissor rectangle of the viewport</param>
	/// <param name="renderSurfaceDimensions">Only set this value in order to render to an FBO that is NOT screen sized.
	/// The default is screen size. This value must always be set to the actual dimension of the entire render surface,
	/// NOT the viewport being rendered to. It is being used to convert NDC to pixel coordinates and back, so that
	/// API-independent code can be written across OGLES and Vulkan code.</param>
	/// <returns>return this object (allows chained calls)</returns>
	ViewportStateCreateParam& setViewportAndScissor(uint32 index, const Viewport& viewport,
	    const Rectanglei& scissor, glm::ivec2 renderSurfaceDimensions = types::PipelineDefaults::ViewportScissor::SurfaceDimensions)
	{
		debug_assertion(index < types::PipelineDefaults::ViewportScissor::MaxScissorViewports, "Scissor Viewport out of range.");

		_scissorViewports[index].first = scissor;
		_scissorViewports[index].second = viewport;
		_scissorViewportsCount++;

		_renderSurfaceDimensions = renderSurfaceDimensions;
		return *this;
	}

	void clear()
	{
		for (pvr::uint32 i = 0; i < types::PipelineDefaults::ViewportScissor::MaxScissorViewports; i++)
		{
			_scissorViewports[i].first = Rectanglei();
			_scissorViewports[i].second = Viewport();
		}
		_scissorViewportsCount = 0;
	}

	/// <summary>Get the scissor rectangle for the specified attachment intex</summary>
	/// <param name="index">The index for which to return the scissor rectangle</param>
	/// <returns>Get the scissor rectangle for the specified attachment intex</returns>
	const Rectanglei& getScissor(uint32 index)const
	{
		return _scissorViewports[index].first;
	}

	/// <summary>Get the viewport for the specified attachment intex</summary>
	/// <param name="index">The index for which to return the scissor rectangle</param>
	/// <returns>Get the scissor rectangle for the specified attachment intex</returns>
	const Viewport& getViewport(uint32 index)const
	{
		return _scissorViewports[index].second;
	}

	/// <summary>Get the render surface dimensions</summary>
	/// <returns>Get the render surface dimensions</returns>
	const glm::ivec2& getRenderSurfaceDimensions()const
	{
		return _renderSurfaceDimensions;
	}

	/// <summary>Return number of viewport and scissor</summary>
	uint32 getNumViewportScissor() const
	{
		return _scissorViewportsCount;
	}
};

/// <summary>Pipeline Rasterisation, clipping and culling state configuration. Culling, winding order, depth clipping,
/// raster discard, point size, fill mode, provoking vertex.</summary>
/// <remarks>Defaults: Cull face: back, Front face: CounterClockWise, Depth Clipping: true, Rasterizer Discard: false,
/// Program Point Size: false, Point Origin: Lower left, Fill Mode: Front&Back, Provoking Vertex: First</remarks>
struct RasterStateCreateParam
{
private:
	types::Face _cullFace;
	types::PolygonWindingOrder _frontFaceWinding;
	bool _enableDepthClip;
	bool _enableRasterizerDiscard;
	bool _enableProgramPointSize;
	bool _enableDepthBias;
	float32 _depthBiasClamp;
	float32 _depthBiasConstantFactor;
	float32 _depthBiasSlopeFactor;

	types::FillMode _fillMode;
	float32 _lineWidth;
	friend class ::pvr::api::impl::GraphicsPipeline_;
public:
	/// <summary>Create a rasterization and polygon state configuration.</summary>
	/// <param name="cullFace">Face culling (default: Back)</param>
	/// <param name="frontFaceWinding">The polygon winding order (default: Front face is counterclockwise)</param>
	/// <param name="enableDepthClip">Enable depth clipping. If set to false , depth Clamping happens instead of clipping
	/// (default: true)</param>
	/// <param name="enableRasterizerDiscard">Enable rasterizer discard (default:false)</param>
	/// <param name="enableProgramPointSize">Enable program point size (default:true)</param>
	/// <param name="fillMode">Polygon fill mode (default: Front and Back)</param>
	/// <param name="lineWidth">Width of rendered lines (default: One)</param>
	/// <param name="enableDepthBias">Enable depth bias (default: false)</param>
	/// <param name="provokingVertex">The provoking vertex to use</param>
	/// <param name="name="enableDepthBias">Enable">depth bias (default: false)</param>
	/// <param name="name="depthBiasClamp">If">depth bias is enabled, the clamping value for depth bias (default:0)
	/// </param>
	/// <param name="name="depthBiasConstantFactor">If">depth bias is enabled, the constant value by which to bias
	/// depth(default:0)</param>
	/// <param name="name="depthBiasSlopeFactor">If">depth bias is enabled, the slope value by which to bias
	/// depth(default:0)</param>
	RasterStateCreateParam(types::Face cullFace = types::Face::None,
	                       types::PolygonWindingOrder frontFaceWinding = types::PolygonWindingOrder::FrontFaceCCW,
	                       bool enableDepthClip = true,
	                       bool enableRasterizerDiscard = false,
	                       bool enableProgramPointSize = false,
	                       types::FillMode fillMode = types::FillMode::Fill,
	                       types::ProvokingVertex provokingVertex = types::ProvokingVertex::First,
	                       float32 lineWidth = 1.0f,
	                       bool enableDepthBias = false,
	                       float32 depthBiasClamp = 0.f,
	                       float32 depthBiasConstantFactor = 0.f,
	                       float32 depthBiasSlopeFactor = 0.f) :
		_cullFace(cullFace), _frontFaceWinding(frontFaceWinding), _enableDepthClip(enableDepthClip),
		_enableRasterizerDiscard(enableRasterizerDiscard), _enableProgramPointSize(enableProgramPointSize),
		_enableDepthBias(enableDepthBias), _depthBiasClamp(depthBiasClamp), _depthBiasConstantFactor(depthBiasConstantFactor),
		_depthBiasSlopeFactor(depthBiasSlopeFactor), _fillMode(fillMode), _lineWidth(lineWidth) {}

	/// <summary>Set the face that will be culled (front/back/both/none).</summary>
	/// <returns>this object (allows chained calls)</returns>
	RasterStateCreateParam& setCullFace(types::Face face)
	{
		_cullFace = face;
		return *this;
	}

	/// <summary>Set the line width</summary>
	/// <param name="lineWidth">The width of lines (in pixels) when drawing line primitives.</param>
	/// <returns>Return this object (allows chained calls)</returns>
	RasterStateCreateParam& setLineWidth(float32 lineWidth)
	{
		_lineWidth = lineWidth;
		return *this;
	}

	/// <summary>Select between depth Clipping and depth Clamping</summary>
	/// <param name="enableDepthClip">Set to true to clip polygons at the Z-sides of the view frustum Set to false to
	/// clamp the depth to the min/max values and not clip based on depth</param>
	/// <returns>Return this object (allows chained calls)</returns>
	RasterStateCreateParam& setDepthClip(bool enableDepthClip)
	{
		_enableDepthClip = enableDepthClip;
		return *this;
	}

	/// <summary>Enable depth bias (add a value to the calculated fragment depth)</summary>
	/// <param name="enableDepthBias">Set to true to enable depth bias, false to disable</param>
	/// <param name="depthBiasClamp">The maximum (or minimum) value of depth biasing</param>
	/// <param name="depthBiasConstantFactor">A constant value added to all fragment depths</param>
	/// <param name="depthBiasSlopeFactor">Depth slope factor for multiply fragment depths</param>
	/// <returns>Return this object (allows chained calls)</returns>
	RasterStateCreateParam& setDepthBias(bool enableDepthBias, bool depthBiasClamp = 0.f, bool depthBiasConstantFactor = 0.f, bool depthBiasSlopeFactor = 0.f)
	{
		_enableDepthBias = enableDepthBias;
		_depthBiasClamp = depthBiasClamp;
		_depthBiasConstantFactor = depthBiasConstantFactor;
		_depthBiasSlopeFactor = depthBiasSlopeFactor;
		return *this;
	}

	/// <summary>Set which polygon winding order is considered the "front" face (The opposite order is considered back
	/// face).</summary>
	/// <param name="frontFaceWinding">The winding order that will represent front faces</param>
	/// <returns>this object (allows chained calls)</returns>
	RasterStateCreateParam& setFrontFaceWinding(types::PolygonWindingOrder frontFaceWinding)
	{
		_frontFaceWinding = frontFaceWinding;
		return *this;
	}

	/// <summary>Disable all phases after transform feedback (rasterization and later)</summary>
	/// <param name="enable">Set to "false" for normal rendering, "true" to enable rasterization discard</param>
	/// <returns>this object (allows chained calls)</returns>
	RasterStateCreateParam& setRasterizerDiscard(bool enable)
	{
		_enableRasterizerDiscard = enable;
		return *this;
	}

	/// <summary>Enable/disable Program Point Size.</summary>
	/// <param name="enable">Set to "true" to control point size for the entire program</param>
	/// <returns>this object (allows chained calls)</returns>
	RasterStateCreateParam& setProgramPointSize(bool enable)
	{
		_enableProgramPointSize = enable;
		return *this;
	}

	/// <summary>Set polygon fill mode.</summary>
	/// <returns>this object (allows chained calls)</returns>
	RasterStateCreateParam& setFillMode(types::FillMode mode)
	{
		_fillMode = mode;
		return *this;
	}

	/// <summary>Get which of the faces (Front/Back/None/Both) will not be rendered (will be culled)</summary>
	/// <returns>The faces that will be culled ("None" means everything drawn, "Both" means nothing drawn)</returns>
	types::Face getCullFace() const { return _cullFace; }

	/// <summary>Get which winding order is considered the FRONT face (CCW means front faces are the counterclockwise,
	/// CW clockwise)</summary>
	/// <returns>The winding order that is considered FRONT facing</returns>
	types::PolygonWindingOrder getFrontFaceWinding() const { return _frontFaceWinding; }

	/// <summary>Check if depth clipping is enabled (i.e. depth clamping disabled). If disabled, polygons will not be
	/// clipped against the front and back clipping plane clipping planes: instead, the primitives' depth will be
	/// clamped to min and max depth instead.</summary>
	/// <returns>True if depth clipping is enabled (depth clamping is disabled). False if depth clamping is enabled
	/// (depth clipping disabled).</returns>
	bool isDepthClipEnabled() const { return _enableDepthClip; }

	/// <summary>Check if rasterization is skipped (so all parts of the pipeline after transform feedback)</summary>
	/// <returns>True if rasterization discard is enabled, false for normal rendering.</returns>
	bool isRasterizerDiscardEnabled() const { return _enableRasterizerDiscard; }

	/// <summary>Check if program point size is enabled</summary>
	/// <returns>True if program point size is enabled, otherwise false</returns>
	bool isProgramPointSizeEnabled() const { return _enableProgramPointSize; }

	/// <summary>Check if depth bias is enabled. (If enabled, clamp, constant factor and slope factor can be checked)
	/// </summary>
	/// <returns>True if depth bias is enabled, otherwise false.</returns>
	bool isDepthBiasEnabled() const { return _enableDepthBias; }

	/// <summary>Get the maximum(minimum) value of depth bias</summary>
	/// <returns>The maximum(minimum) value of depth bias</returns>
	float getDepthBiasClamp() const { return _depthBiasClamp; }

	/// <summary>Get the constant factor of depth bias</summary>
	/// <returns>The constant factor depth bias</returns>
	float getDepthBiasConstantFactor() const { return _depthBiasConstantFactor; }

	/// <summary>Get the slope factor of depth bias</summary>
	/// <returns>The slope factor depth bias</returns>
	float getDepthBiasSlopeFactor() const { return _depthBiasSlopeFactor; }

	/// <summary>Get the slope factor of depth bias</summary>
	/// <returns>The slope factor depth bias</returns>
	types::FillMode getFillMode() const { return _fillMode; }

	/// <summary>Get the slope factor of depth bias</summary>
	/// <returns>The slope factor depth bias</returns>
	float getLineWidth() const { return _lineWidth; }
};

/// <summary>Pipeline Multisampling state configuration: Number of samples, alpha to coverage, alpha to one,
/// sampling mask.</summary>
/// <remarks>Defaults: No multisampling</remarks>
struct MultiSampleStateCreateParam
{
private:
	friend class ::pvr::api::impl::GraphicsPipeline_;
	bool _stateEnabled;
	bool _sampleShadingEnable;
	bool _alphaToCoverageEnable;
	bool _alphaToOneEnable;
	types::SampleCount _rasterizationSamples;
	float32 _minSampleShading;
	uint32 _sampleMask;

public:
	/// <summary>Constructor. Create a multisampling configuration.</summary>
	/// <param name="stateEnabled">Enable/disable multisampling (default false)</param>
	/// <param name="sampleShadingEnable">Enable/disable sample shading (defalt false)</param>
	/// <param name="alphaToCoverageEnable">Enable/disable alpha-to-coverage</param>
	/// <param name="alphaToOneEnable">Enable/disable alpha-to-one</param>
	/// <param name="rasterizationSamples">The number of rasterization samples (default 1)</param>
	/// <param name="minSampleShading">The minimum sample Shading (default 0)</param>
	/// <param name="sampleMask">sampleMask (default 0)</param>
	MultiSampleStateCreateParam(
	  bool stateEnabled = false, bool sampleShadingEnable = false, bool alphaToCoverageEnable = false,
	  bool alphaToOneEnable = false, types::SampleCount rasterizationSamples = types::SampleCount::Count1,
	  float32 minSampleShading = 0.f, uint32 sampleMask = 0xffffffff) :
		_stateEnabled(stateEnabled), _sampleShadingEnable(sampleShadingEnable),
		_alphaToCoverageEnable(alphaToCoverageEnable), _alphaToOneEnable(alphaToOneEnable),
		_rasterizationSamples(rasterizationSamples), _minSampleShading(minSampleShading), _sampleMask(sampleMask) {}

	/// <summary>Enable/disable multisampling</summary>
	/// <param name="active">true enable, false disable if the pipeline has rasterization disabled.</param>
	/// <returns>this (allow chaining)</returns>
	MultiSampleStateCreateParam& enableState(bool active)
	{
		_stateEnabled = active;
		return *this;
	}

	/// <summary>Enable/ Disable alpha to coverage</summary>
	/// <param name="enable">True to enable Alpha to Coverage, false to Disable</param>
	/// <returns>Return this object (allows chained calls)</returns>
	MultiSampleStateCreateParam& setAlphaToCoverage(bool enable)
	{
		_alphaToCoverageEnable = enable;
		return *this;
	}

	/// <summary>Enable/ disable sampler shading (Multi Sampling Anti Aliasing).</summary>
	/// <param name="enable">true enable per-sample shading, false to disable</param>
	/// <returns>this (allow chaining)</returns>
	MultiSampleStateCreateParam& setSampleShading(bool enable)
	{
		_sampleShadingEnable = enable;
		return *this;
	}

	/// <summary>Controls whether the alpha component of the fragment’s first color output is replaced with one
	/// </summary>
	/// <param name="enable">true enable alpha to one, false disable</param>
	/// <returns>this (allow chaining)</returns>
	MultiSampleStateCreateParam& setAlphaToOne(bool enable)
	{
		_alphaToOneEnable = enable;
		return *this;
	}

	/// <summary>Set the number of samples per pixel used in rasterization (Multi sample anti aliasing)</summary>
	/// <param name="samplesCount">The number of samples</param>
	/// <returns>this (allow chaining)</returns>
	MultiSampleStateCreateParam& setNumRasterizationSamples(types::SampleCount samplesCount)
	{
		_rasterizationSamples = samplesCount;
		return *this;
	}

	/// <summary>Set minimum sample shading.</summary>
	/// <param name="minSampleShading">The number of minimum samples to shade</param>
	/// <returns>this (allow chaining)</returns>
	MultiSampleStateCreateParam& setMinSampleShading(float32 minSampleShading)
	{
		_minSampleShading = minSampleShading;
		return *this;
	}

	/// <summary>Set a bitmask of static coverage information that is ANDed with the coverage information generated
	/// during rasterization.</summary>
	/// <param name="mask">The sample mask. See the corresponding API spec for exact bit usage of the mask.</param>
	/// <returns>this (allow chaining)</returns>
	MultiSampleStateCreateParam& setSampleMask(uint32 mask)
	{
		_sampleMask = mask;
		return *this;
	}

	/// <summary>Get the sample mask</summary>
	/// <returns>The sample mask</returns>
	uint32 getSampleMask()const { return _sampleMask; }

	/// <summary>Return the number of rasterization (MSAA) samples</summary>
	/// <returns>The number of rasterization samples</returns>
	uint32 getNumRasterizationSamples()const {  return (uint32)_rasterizationSamples;  }

	/// <summary>Get the number of minimum samples</summary>
	/// <returns>The number of minimum samples</returns>
	float32 getMinSampleShading()const { return _minSampleShading; }

	/// <summary>Get the sample shading state</summary>
	/// <returns>true if sample shading enabled, false if disabled</returns>
	bool isSampleShadingEnabled()const { return _sampleShadingEnable; }

	/// <summary>Get alpha to coverage state</summary>
	/// <returns>True if enabled, false if disabled</returns>
	bool isAlphaToCoverageEnabled()const { return _alphaToCoverageEnable; }

	/// <summary>Get alpha to one state</summary>
	/// <summary>Return true if alpha to one is enabled, false if disabled</summary>
	bool isAlphaToOneEnabled()const { return _alphaToOneEnable; }

	/// <summary>Return true if multisampling state is enabled</summary>
	/// <returns>true if multisampling state is enabled</returns>
	bool isStateEnabled()const { return _stateEnabled; }
};

/// <summary>Create params for Pipeline Dynamic states. Enable each state that you want to be able to dynamically
/// set.</summary>
struct DynamicStatesCreateParam
{
private:
	bool _dynamicStates[(uint32)types::DynamicState::Count];
public:
	/// <summary>Constructor</summary>
	DynamicStatesCreateParam() { memset(_dynamicStates, 0, sizeof(_dynamicStates)); }

	/// <summary>Check if a specific dynamic state is enabled.</summary>
	/// <param name="state">The state to check</param>
	/// <returns>true if <paramref name="state"/>is enabled, otherwise false</returns>
	bool isDynamicStateEnabled(types::DynamicState state)const { return (_dynamicStates[(uint32)state]); }

	/// <summary>Enable/disable a dynamic state</summary>
	/// <param name="state">The state to enable or disable</param>
	/// <param name="enable">True to enable, false to disable</param>
	/// <returns>Return this object(allows chained calls)</returns>
	DynamicStatesCreateParam& setDynamicState(types::DynamicState state, bool enable)
	{
		_dynamicStates[(uint32)state] = enable;
		return *this;
	}
};

/// <summary>A representation of a Shader constant</summary>
struct ShaderConstantInfo
{
	uint32 constantId;
	byte   data[64];// max can hold 4x4 matrix
	types::GpuDatatypes::Enum gpuDataType;
	uint32 sizeInBytes;
	ShaderConstantInfo() : constantId(0), gpuDataType(types::GpuDatatypes::none), sizeInBytes(0)
	{
		memset(data, 0, sizeof(data));
	}

	bool isValid()const
	{
		return (sizeInBytes != 0 && gpuDataType != types::GpuDatatypes::none);
	}

	ShaderConstantInfo(uint32 constantId, uint32 data) : constantId(constantId),
		gpuDataType(types::GpuDatatypes::uinteger), sizeInBytes(sizeof(data))
	{
		memcpy(this->data, &data, sizeof(data));
	}

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

	ShaderConstantInfo(uint32 constantId, const glm::mat3x3& data) : constantId(constantId),
		gpuDataType(types::GpuDatatypes::mat3x3), sizeInBytes(sizeof(data))
	{
		memcpy(this->data, &data, sizeof(data));
	}

	ShaderConstantInfo(uint32 constantId, const glm::mat4x4& data) : constantId(constantId),
		gpuDataType(types::GpuDatatypes::mat4x4), sizeInBytes(sizeof(data))
	{
		memcpy(this->data, &data, sizeof(data));
	}
};

/// <summary>Pipeline vertex Shader stage create param.</summary>
struct ShaderStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
private:
	api::Shader _shader;
	ShaderConstantInfo _shaderConsts[types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos];
	pvr::uint32 _shaderConstsCount;
	std::string _entryPoint;
public:

	/// <summary>Constructor.</summary>
	ShaderStageCreateParam() : _entryPoint("main"), _shaderConstsCount(0) {}

	/// <summary>Construct from a api::Shader object</summary>
	/// <param name="shader">A vertex shader</param>
	ShaderStageCreateParam(const api::Shader& shader) : _shader(shader), _entryPoint("main"), _shaderConstsCount(0) {}

	/// <summary>Get the shader of this shader stage object</summary>
	/// <returns>The shader</returns>
	const api::Shader& getShader() const { return _shader; }

	/// <summary>Return true if this state is active (contains a shader)</summary>
	/// <returns>True if valid, otherwise false</returns>
	bool isActive()const { return _shader.isValid(); }


	/// <summary>Set the shader.</summary>
	/// <param name="shader">A shader</param>
	void setShader(const api::Shader& shader) { _shader = shader; }

	/// <summary>Set the shader entry point function (default: "main"). Only supported for specific APIs</summary>
	/// <param name="entryPoint">A name of a function that will be used as an entry point in the shader</param>
	void setEntryPoint(const char* entryPoint) { _entryPoint = entryPoint; }

	/// <summary>Get the entry point of the shader</summary>
	/// <returns>The entry point of the shader</returns>
	const char* getEntryPoint()const { return _entryPoint.c_str(); }

	ShaderStageCreateParam& operator=(const api::Shader& shader) { setShader(shader); return *this; }

	/// <summary>Set a shader constants to the shader</summary>
	/// <param name="index">The index of the shader constant to set (does not have to be in order)</param>
	/// <param name="shaderConst">The shader constant to set to index <paramref name="index"/></param>
	/// <returns>Return this (allow chaining)</returns>
	ShaderStageCreateParam& setShaderConstant(uint32 index, const ShaderConstantInfo& shaderConst)
	{
		debug_assertion(index < pvr::types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Specialisation index is invalid.");
		if (!_shaderConsts[index].isValid())
		{
			_shaderConsts[index] = shaderConst;
			_shaderConstsCount++;
		}
		return *this;
	}

	/// <summary>Set all shader constants.</summary>
	/// <param name="shaderConsts">A c-style array containing the shader constants</param>
	/// <param name="constantsCount">The number of shader constants in <paramref name="shaderConsts"/></param>
	/// <returns>Return this (allow chaining)</returns>
	/// <remarks>Uses better memory reservation than the setShaderConstant counterpart.</remarks>
	ShaderStageCreateParam& setShaderConstants(const ShaderConstantInfo* shaderConsts, uint32 constantsCount)
	{
		debug_assertion(constantsCount < pvr::types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Specialisation index is invalid.");

		_shaderConstsCount = 0;
		for (pvr::uint32 i = 0; i < constantsCount; i++)
		{
			_shaderConsts[i] = shaderConsts[i];
			_shaderConstsCount++;
		}
		return *this;
	}

	/// <summary>Retrieve a ShaderConstant by index</summary>
	/// <param name="index">The index of the ShaderConstant to retrieve</param>
	/// <returns>The shader constant</returns>
	const ShaderConstantInfo& getShaderConstant(uint32 index)const
	{
		debug_assertion(index < pvr::types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Specialisation index is invalid.");
		return _shaderConsts[index];
	}

	/// <summary>Get all shader constants</summary>
	/// <returns>Return an array of all defined shader constants</returns>
	const ShaderConstantInfo* getAllShaderConstants()const
	{
		return &_shaderConsts[0];
	}

	/// <summary>Get the number of shader constants</summary>
	/// <returns>The number of shader constants</returns>
	uint32 getNumShaderConsts()const
	{
		return (uint32)_shaderConstsCount;
	}
};

/// <summary>Creation parameters for a Vertex shader</summary>
struct VertexShaderStageCreateParam : public ShaderStageCreateParam
{
	ShaderStageCreateParam& operator=(const api::Shader& shader)
	{
		setShader(shader);
		return *this;
	}
};

/// <summary>Creation parameters for a Fragment Shader</summary>
struct FragmentShaderStageCreateParam : public ShaderStageCreateParam
{
	ShaderStageCreateParam& operator=(const api::Shader& shader)
	{
		setShader(shader);
		return *this;
	}
};

/// <summary>Creation parameters for a Geometry Shader</summary>
struct GeometryShaderStageCreateParam : public ShaderStageCreateParam
{
	ShaderStageCreateParam& operator=(const api::Shader& shader)
	{
		setShader(shader);
		return *this;
	}
};

/// <summary>Creation parameters for a Compute Shader</summary>
struct ComputeShaderStageCreateParam : public ShaderStageCreateParam
{
	ShaderStageCreateParam& operator=(const api::Shader& shader)
	{
		setShader(shader);
		return *this;
	}
};

/// <summary>Creation parameters for all Tesselation shaders</summary>
struct TesselationStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipeline_;
private:
	api::Shader _controlShader, _evalShader;
	uint32 _patchControlPoints;
	ShaderConstantInfo _shaderConstsTessCtrl[types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos];
	uint32 _shaderConstsTessCtrlCount;

	ShaderConstantInfo _shaderConstTessEval[types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos];
	uint32 _shaderConstTessEvalCount;

	std::string _controlShaderEntryPoint;
	std::string _evalShaderEntryPoint;
public:

	/// <summary>Constructor</summary>
	TesselationStageCreateParam() : _patchControlPoints(3), _shaderConstsTessCtrlCount(0),
		_shaderConstTessEvalCount(0), _controlShaderEntryPoint("main"), _evalShaderEntryPoint("main") {}

	/// <summary>Get the Tessellation Control shader</summary>
	/// <returns>The Tessellation Control shader</returns>
	const api::Shader& getControlShader() const { return _controlShader; }

	/// <summary>Get the Tessellation Evaluation shader</summary>
	/// <returns>The Tessellation Evaluation shader</returns>
	const api::Shader& getEvaluationShader()const { return _evalShader; }

	/// <summary>Check if the Tessellation Control shader has been set</summary>
	/// <returns>true if the Tessellation Control shader has been set</returns>
	bool isControlShaderActive()const { return _controlShader.isValid(); }

	/// <summary>Check if the Tessellation Evaluation shader has been set</summary>
	/// <returns>true if the Tessellation Evaluation shader has been set</returns>
	bool isEvaluationShaderActive()const { return _evalShader.isValid();}



	/// <summary>Set the control shader.</summary>
	/// <param name="shader">A shader to set to the Tessellation Control stage</param>
	/// <returns>this (allow chaining)</returns>
	TesselationStageCreateParam& setControlShader(const api::Shader& shader)
	{
		_controlShader = shader;
		return *this;
	}

	TesselationStageCreateParam& setControlShaderEntryPoint(const char* entryPoint)
	{
		_controlShaderEntryPoint.assign(entryPoint);
		return *this;
	}

	TesselationStageCreateParam& setEvaluationShaderEntryPoint(const char* entryPoint)
	{
		_evalShaderEntryPoint.assign(entryPoint);
		return *this;
	}


	/// <summary>Set the control shader.</summary>
	/// <param name="shader">A shader to set to the Tessellation Control stage</param>
	/// <returns>this (allow chaining)</returns>
	TesselationStageCreateParam& setEvaluationShader(const api::Shader& shader)
	{
		_evalShader = shader;
		return *this;
	}

	/// <summary>Set number of control points</summary>
	/// <param name="controlPoints">The number of control points per patch</param>
	/// <returns>this (allow chaining)</returns>
	TesselationStageCreateParam& setNumPatchControlPoints(uint32 controlPoints)
	{
		_patchControlPoints = controlPoints;
		return *this;
	}

	/// <summary>Get number of control points</summary>
	/// <returns>The number of patch control points</returns>
	uint32 getNumPatchControlPoints()const { return _patchControlPoints; }

	/// <summary>Set a shader constant for the Tessellation Control shader</summary>
	/// <param name="index">Index of the constant to set</param>
	/// <param name="shaderConst">Value of the constant to set</param>
	/// <returns>Return this for chaining</returns>
	TesselationStageCreateParam& setControlShaderConstant(uint32 index, const ShaderConstantInfo& shaderConst)
	{
		debug_assertion(index < types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Control Shader constants out of range.");
		_shaderConstsTessCtrl[index] = shaderConst;
		_shaderConstsTessCtrlCount++;
		return *this;
	}

	/// <summary>Set all Tessellation Control shader constants.</summary>
	/// <param name="shaderConsts">A c-style array containing the shader constants</param>
	/// <param name="constantsCount">The number of shader constants in <paramref name="shaderConsts"/></param>
	/// <returns>Return this (allow chaining)</returns>
	/// <remarks>Uses better memory reservation than the setShaderConstant counterpart.</remarks>
	TesselationStageCreateParam& setControlShaderConstants(const ShaderConstantInfo* shaderConsts, uint32 constantsCount)
	{
		debug_assertion(constantsCount < types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Control Shader constants out of range.");
		for (pvr::uint32 i = 0; i < constantsCount; i++)
		{
			_shaderConstsTessCtrl[i] = shaderConsts[constantsCount];
		}
		_shaderConstsTessCtrlCount = constantsCount;
		return *this;
	}

	/// <summary>Get a Control shader constant</summary>
	/// <param name="index">The index of the constant to get. It is undefined to retrieve a constant that does not
	/// exist.</param>
	/// <returns>The Constant to get</returns>
	const ShaderConstantInfo& getControlShaderConstant(uint32 index)const
	{
		debug_assertion(index < types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Control Shader constants out of range.");
		return _shaderConstsTessCtrl[index];
	}

	/// <summary>Return all control shader constants as a c-style array</summary>
	/// <returns>C-style array of all shader constants</returns>
	const ShaderConstantInfo* getAllControlShaderConstants()const { return &_shaderConstsTessCtrl[0]; }

	/// <summary>Return number of control shader constants</summary>
	uint32 getNumControlShaderConstants()const { return _shaderConstsTessCtrlCount; }

	/// <summary>Set a shader constant for the Tessellation Evaluation shader</summary>
	/// <param name="index">Index of the constant to set</param>
	/// <param name="shaderConst">Value of the constant to set</param>
	/// <returns>Return this for chaining</returns>
	void setEvaluationShaderConstant(uint32 index, const ShaderConstantInfo& shaderConst)
	{
		debug_assertion(index < types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Evaluation Shader constants out of range.");
		_shaderConstTessEval[index] = shaderConst;
		_shaderConstTessEvalCount++;
	}

	/// <summary>Set all Tessellation Evaluation shader constants.</summary>
	/// <param name="shaderConsts">A c-style array containing the shader constants</param>
	/// <param name="constantsCount">The number of shader constants in <paramref name="shaderConsts"/></param>
	/// <returns>Return this (allow chaining)</returns>
	/// <remarks>Uses better memory reservation than the setShaderConstant counterpart.</remarks>
	TesselationStageCreateParam& setEvaluationShaderConstants(const ShaderConstantInfo* shaderConsts, uint32 constantsCount)
	{
		debug_assertion(constantsCount < types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Evaluation Shader constants out of range.");
		for (pvr::uint32 i = 0; i < constantsCount; i++)
		{
			_shaderConstTessEval[i] = shaderConsts[constantsCount];
		}
		_shaderConstTessEvalCount = constantsCount;
		return *this;
	}

	/// <summary>Get Evaluation shader constants</summary>
	/// <param name="index">The index of the constant to retrieve. It is undefined to retrieve a constant that does
	/// not exist.</param>
	/// <returns>The ShaderConstantInfo at index <paramref name="index"/></returns>
	const ShaderConstantInfo& getEvaluationlShaderConstant(uint32 index)const
	{
		return _shaderConstTessEval[index];
	}

	/// <summary>Return all evaluationshader constants</summary>
	const ShaderConstantInfo* getAllEvaluationShaderConstants()const { return &_shaderConstTessEval[0]; }

	/// <summary>Return number of evaluatinon shader constants</summary>
	/// <returns>The number of evaluatinon shader constants</returns>
	uint32 getNumEvaluatinonShaderConstants()const { return _shaderConstTessEvalCount; }

	const char* getEvaluationShaderEntryPoint() const { return  _evalShaderEntryPoint.c_str(); }

	const char* getControlShaderEntryPoint() const { return  _controlShaderEntryPoint.c_str(); }

};

/// <summary>OGLES2TextureUnitBindings struct. This struct does the shader's texture unit reflection as the shader does not support
/// layout qualifiers. ONLY takes effect for OPENGLES.</summary>
struct OGLES2TextureUnitBindings
{
private:
	std::string _texUnit[types::PipelineDefaults::TextureUnitBindings::MaxOGLES2TextureUnitBindings];
	pvr::uint32 _texUnitsCount;
public:
	OGLES2TextureUnitBindings() : _texUnitsCount(0) {}

	/// <summary>Set texture unit.</summary>
	/// <param name="unit">Texture binding unit. Unit must be consecutive</param>
	/// <param name="name">Texture binding name</param>
	/// <returns>This object</returns>
	OGLES2TextureUnitBindings& setTextureUnit(uint32 unit, const char* name)
	{
		debug_assertion(unit < types::PipelineDefaults::TextureUnitBindings::MaxOGLES2TextureUnitBindings, "Texture unit out of range.");
		_texUnit[unit] = name;
		_texUnitsCount++;
		return *this;
	}

	/// <summary>Return texture unit binding name</summary>
	/// <param name="unit">Texture unit</param>
	const char* getTextureUnitName(uint32 unit)const
	{
		debug_assertion(unit < types::PipelineDefaults::TextureUnitBindings::MaxOGLES2TextureUnitBindings, "Texture unit out of range.");
		return _texUnit[unit].c_str();
	}

	/// <summary>Return texture unit binding id</summary>
	/// <param name="name">Texture binding name</param>
	int32 getTextureUnitId(const char* name)const
	{
		for (pvr::uint32 i = 0; i < _texUnitsCount; i++)
		{
			if (_texUnit[i].compare(name) != 0)
			{
				return i;
			}
		}
		return -1;
	}

	/// <summary>Return number of bindings</summary>
	/// <returns>The number of bindings</returns>
	uint32 getBindingCount()const
	{
		return _texUnitsCount;
	}
};

/// <summary>Create params for Ray shader stages. Set ray shader stages and entry points.</summary>
struct RayShaderStageCreateParam
{
public:

	/// <summary>Constructor</summary>
	RayShaderStageCreateParam() : _rayShadersCount(0)
	{
		for (pvr::uint32 i = 0; i < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders; i++)
		{
			_entryPointsPerRayShaderCount[i] = 0;

			for (pvr::uint32 j = 0; j < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctEntryPointsPerRayShader; j++)
			{
				_entryPointsPerRayShader[i][j] = "";
				_shaderConstsCount[i][j] = 0;
			}
		}
	}

	/// <summary>Set ray shader at index.</summary>
	/// <param name="rayShaderIndex">The index at which to add a ray shader. Unit must be consecutive</param>
	/// <param name="shader">Ray shader</param>
	/// <returns>This object</returns>
	RayShaderStageCreateParam& setRayShader(pvr::uint32 rayShaderIndex, const pvr::api::Shader& shader)
	{
		debug_assertion(rayShaderIndex < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders, "Too many shaders specified");
		if (!_rayShaders[rayShaderIndex].isValid())
		{
			_rayShadersCount++;
		}
		_rayShaders[rayShaderIndex] = shader;
		return *this;
	}

	/// <summary>Set ray shader at index.</summary>
	/// <param name="rayShaderIndex">The index at which to add a ray shader. Unit must be consecutive</param>
	/// <param name="shader">Ray shader</param>
	/// <param name="entryPointsCount">The number of entry points for the ray shader</param>
	/// <returns>This object</returns>
	RayShaderStageCreateParam& setRayShaderWithDefaultEntryPoints(pvr::uint32 rayShaderIndex, const pvr::api::Shader& shader, pvr::uint32 entryPointsCount)
	{
		setRayShader(rayShaderIndex, shader);
		for (pvr::uint32 i = 0; i < entryPointsCount; i++)
		{
			setDefaultRayShaderEntrypoint(rayShaderIndex, i);
		}

		return *this;
	}

	/// <summary>Set a number of ray shaders.</summary>
	/// <param name="offset">The starting index at which to add a ray shader.</param>
	/// <param name="count">The number of ray shaders</param>
	/// <param name="shaders">Count number of ray shaders to add</param>
	/// <returns>This object</returns>
	RayShaderStageCreateParam& setRayShaders(pvr::uint32 offset, pvr::uint32 count, pvr::api::Shader const* shaders)
	{
		debug_assertion(offset + count < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders, "Too many shaders specified");
		debug_assertion(offset < count, "Offset specified must be larger than the count");

		pvr::uint32 currentShaderIndex = 0;
		for (pvr::uint32 i = offset; i < count; i++)
		{
			setRayShader(i, shaders[currentShaderIndex]);
			currentShaderIndex++;
		}

		return *this;
	}

	/// <summary>Determines whether this RayShaderStageCreateParam is active.</summary>
	/// <returns>Whether this RayShaderStageCreateParam is active</returns>
	bool isActive() const
	{
		return _rayShadersCount != 0;
	}

	/// <summary>Gets the number of ray shaders in use.</summary>
	/// <returns>The number of ray shaders</returns>
	pvr::uint32 getNumberOfRayShaders() const
	{
		return _rayShadersCount;
	}

	/// <summary>Gets the ray shader at a particular index.</summary>
	/// <param name="rayShaderIndex">The index of a ray shader to get.</param>
	/// <returns>This shader at rayShaderIndex</returns>
	const pvr::api::Shader& getRayShader(pvr::uint32 rayShaderIndex) const
	{
		debug_assertion(rayShaderIndex < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders, "Invalid ray shader specified");
		return _rayShaders[rayShaderIndex];
	}

	/// <summary>Sets a ray shader at a particular index and an entry point for it.</summary>
	/// <param name="shader">Ray shader</param>
	/// <param name="rayShaderIndex">The index of a ray shader to set.</param>
	/// <param name="entryPointIndex">The index of an entry point to add.</param>
	/// <param name="entryPoint">The entry point to add.</param>
	void setRayShaderAndEntrypoint(const pvr::api::Shader& shader, pvr::uint32 rayShaderIndex, pvr::uint32 entryPointIndex, const char* entryPoint)
	{
		debug_assertion(rayShaderIndex < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders, "Invalid ray shader specified");
		setRayShader(rayShaderIndex, shader);
		setRayShaderEntrypoint(rayShaderIndex, entryPointIndex, entryPoint);
	}

	/// <summary>Sets an entry point for a particular ray shader at a particular index.</summary>
	/// <param name="rayShaderIndex">The index of a ray shader to set.</param>
	/// <param name="entryPointIndex">The index of an entry point to add.</param>
	/// <param name="entryPoint">The entry point to add.</param>
	void setRayShaderEntrypoint(pvr::uint32 rayShaderIndex, pvr::uint32 entryPointIndex, const char* entryPoint)
	{
		debug_assertion(rayShaderIndex < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders, "Invalid ray shader specified");
		pvr::uint32 newEntryPointIndex = std::min(entryPointIndex, pvr::types::PipelineDefaults::ShaderStage::MaxDistinctEntryPointsPerRayShader);
		if (_entryPointsPerRayShader[rayShaderIndex][newEntryPointIndex].compare("") == 0)
		{
			_entryPointsPerRayShaderCount[rayShaderIndex]++;
		}
		_entryPointsPerRayShader[rayShaderIndex][newEntryPointIndex] = entryPoint;
	}

	/// <summary>Sets a default entry point for a particular ray shader at a particular index.</summary>
	/// <param name="rayShaderIndex">The index of a ray shader to set.</param>
	/// <param name="entryPointIndex">The index of an entry point to add.</param>
	void setDefaultRayShaderEntrypoint(pvr::uint32 rayShaderIndex, pvr::uint32 entryPointIndex)
	{
		pvr::uint32 newEntryPointIndex = std::min(entryPointIndex, pvr::types::PipelineDefaults::ShaderStage::MaxDistinctEntryPointsPerRayShader);
		if (newEntryPointIndex > 0)
		{
			setRayShaderEntrypoint(rayShaderIndex, newEntryPointIndex, pvr::strings::createFormatted("main%d", newEntryPointIndex).c_str());
		}
		else
		{
			setRayShaderEntrypoint(rayShaderIndex, newEntryPointIndex, pvr::string("main").c_str());
		}
	}

	/// <summary>Gets a particular ray shader entry point for a ray shader at a particular index.</summary>
	/// <param name="rayShaderIndex">The index of a ray shader.</param>
	/// <param name="entryPointIndex">The entry point index.</param>
	/// <returns>This entry point for the particular ray shader at entryPointIndex</returns>
	const char* getRayShaderEntrypoint(pvr::uint32 rayShaderIndex, pvr::uint32 entryPointIndex) const
	{
		debug_assertion(rayShaderIndex < _rayShadersCount, "Invalid ray shader specified");
		debug_assertion(entryPointIndex < _entryPointsPerRayShaderCount[rayShaderIndex], "Invalid ray shader entry point specified");
		return _entryPointsPerRayShader[rayShaderIndex][entryPointIndex].c_str();
	}

	/// <summary>Gets the number of entry points for a ray shader.</summary>
	/// <param name="rayShaderIndex">The index of a ray shader.</param>
	/// <returns>The number of entry points for the particular ray shader</returns>
	const pvr::uint32 getNumberOfEntryPointsForRayShader(pvr::uint32 rayShaderIndex) const
	{
		return _entryPointsPerRayShaderCount[rayShaderIndex];
	}

	/// <summary>Set a shader constants to the shader</summary>
	/// <param name="rayShaderIndex">The index of the shader constant to set (does not have to be in order)</param>
	/// <param name="entryPointIndex">The index of the entry point of the constant to retrieve</param>
	/// <param name="specialisationIndex">The index of the specializationIndex of the constant to retrieve</param>
	/// <param name="shaderConst">The shader constant to set to index <paramref name="index"/></param>
	/// <returns>Return this (allow chaining)</returns>
	RayShaderStageCreateParam& setShaderConstant(pvr::uint32 rayShaderIndex, uint32 entryPointIndex, uint32 specialisationIndex, const ShaderConstantInfo& shaderConst)
	{
		debug_assertion(rayShaderIndex < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders, "Invalid ray shader specified");
		debug_assertion(entryPointIndex < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctEntryPointsPerRayShader, "Invalid ray shader entry point specified");
		debug_assertion(specialisationIndex < pvr::types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Specialisation index is invalid.");
		if (!_shaderConsts[rayShaderIndex][entryPointIndex][specialisationIndex].isValid())
		{
			_shaderConsts[rayShaderIndex][entryPointIndex][specialisationIndex] = shaderConst;
			_shaderConstsCount[rayShaderIndex][entryPointIndex]++;
		}
		return *this;
	}

	/// <summary>Set all shader constants.</summary>
	/// <param name="shaderConsts">A c-style array containing the shader constants</param>
	/// <param name="constantsCount">The number of shader constants in <paramref name="shaderConsts"/></param>
	/// <returns>Return this (allow chaining)</returns>
	/// <remarks>Uses better memory reservation than the setShaderConstant counterpart.</remarks>
	RayShaderStageCreateParam& setShaderConstants(pvr::uint32 rayShaderIndex, uint32 entryPointIndex, const ShaderConstantInfo* shaderConsts, uint32 constantsCount)
	{
		debug_assertion(rayShaderIndex < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders, "Invalid ray shader specified");
		debug_assertion(entryPointIndex < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctEntryPointsPerRayShader, "Invalid ray shader entry point specified");
		debug_assertion(constantsCount < pvr::types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Specialisation index is invalid.");

		_shaderConstsCount[rayShaderIndex][entryPointIndex] = 0;
		for (pvr::uint32 i = 0; i < constantsCount; i++)
		{
			setShaderConstant(rayShaderIndex, entryPointIndex, i, shaderConsts[i]);
		}
		return *this;
	}

	/// <summary>Retrieve a ShaderConstant by index</summary>
	/// <param name="rayShaderIndex">The index of the ShaderConstant to retrieve</param>
	/// <param name="entryPointIndex">The index of the entry point of the constant to retrieve</param>
	/// <param name="specialisationIndex">The index of the specializationIndex of the constant to retrieve</param>
	/// <returns>The shader constant</returns>
	const ShaderConstantInfo& getShaderConstant(pvr::uint32 rayShaderIndex, uint32 entryPointIndex, uint32 specialisationIndex)const
	{
		debug_assertion(rayShaderIndex < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders, "Invalid ray shader specified");
		debug_assertion(entryPointIndex < pvr::types::PipelineDefaults::ShaderStage::MaxDistinctEntryPointsPerRayShader, "Invalid ray shader entry point specified");
		debug_assertion(specialisationIndex < pvr::types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos, "Specialisation index is invalid.");
		return _shaderConsts[rayShaderIndex][entryPointIndex][specialisationIndex];
	}

	/// <summary>Get all shader constants</summary>
	/// <returns>Return an array of all defined shader constants</returns>
	const ShaderConstantInfo* getAllShaderConstants()const
	{
		return &_shaderConsts[0][0][0];
	}

	/// <summary>Get the number of shader constants</summary>
	/// <returns>The number of shader constants</returns>
	uint32 getNumShaderConsts(pvr::uint32 rayShaderIndex, uint32 entryPointIndex)const
	{
		return _shaderConstsCount[rayShaderIndex][entryPointIndex];
	}

private:
	pvr::api::Shader _rayShaders[pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders];
	std::string _entryPointsPerRayShader[pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders][pvr::types::PipelineDefaults::ShaderStage::MaxDistinctEntryPointsPerRayShader];

	pvr::uint32 _rayShadersCount;
	pvr::uint32 _entryPointsPerRayShaderCount[pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders];

	ShaderConstantInfo _shaderConsts[pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders][pvr::types::PipelineDefaults::ShaderStage::MaxDistinctEntryPointsPerRayShader][types::PipelineDefaults::SpecialisationStates::MaxSpecialisationInfos];
	pvr::uint32 _shaderConstsCount[pvr::types::PipelineDefaults::ShaderStage::MaxDistinctRayShaders][pvr::types::PipelineDefaults::ShaderStage::MaxDistinctEntryPointsPerRayShader];
};

/// <summary>Create params for the scene traversal stage. Set scene traversal shader stages and entry points.</summary>
struct SceneTraversalShaderStageCreateParam : public ShaderStageCreateParam
{
	ShaderStageCreateParam& operator=(const api::Shader& shader)
	{
		setShader(shader);
		return *this;
	}
};

/// <summary>Create params for the ray intersection states used when a ray intersects a triangle.</summary>
struct RayIntersectionStateCreateParam
{
public:

	/// <summary>Constructor.</summary>
	RayIntersectionStateCreateParam(bool occluder = true, pvr::types::VisibleFace visibleFace = pvr::types::VisibleFace::FrontBack,
	                                pvr::types::PolygonWindingOrder windingOrder = pvr::types::PolygonWindingOrder::Default, bool hasDecal = false,
	                                pvr::uint32 rayTypeVisibilityCount = 0)
		:
		_isOccluder(occluder), _visibleFace(visibleFace), _windingOrder(windingOrder), _decal(hasDecal)
	{
		_rayTypeVisibilityCount = std::min(rayTypeVisibilityCount, pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes);

		// set min(_rayTypeVisibilityCount, MaxRayTypes) to true
		for (pvr::uint32 i = 0; i < _rayTypeVisibilityCount; i++)
		{
			_rayTypeVisibilities[i] = { true, true };
		}

		// set the rest to false
		for (pvr::uint32 i = pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes - _rayTypeVisibilityCount; i < pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes; i++)
		{
			_rayTypeVisibilities[i] = { false, false };
		}
	}

	/// <summary>Constructor.</summary>
	RayIntersectionStateCreateParam(bool* rayTypeVisibilities, pvr::uint32 rayTypeVisibilityCount = 0,
	                                bool occluder = true, pvr::types::VisibleFace visibleFace = pvr::types::VisibleFace::FrontBack,
	                                pvr::types::PolygonWindingOrder windingOrder = pvr::types::PolygonWindingOrder::Default, bool hasDecal = false)
		:
		_isOccluder(occluder), _visibleFace(visibleFace), _windingOrder(windingOrder), _decal(hasDecal)
	{
		_rayTypeVisibilityCount = std::min(rayTypeVisibilityCount, pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes);

		// set min(_rayTypeVisibilityCount, MaxRayTypes) to rayTypeVisibilities
		for (pvr::uint32 i = 0; i < _rayTypeVisibilityCount; i++)
		{
			_rayTypeVisibilities[i] = { rayTypeVisibilities[i], true };
		}

		// set the rest to false
		for (pvr::uint32 i = pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes - _rayTypeVisibilityCount; i < pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes; i++)
		{
			_rayTypeVisibilities[i] = { false, false };
		}
	}

	/// <summary>Gets whether the ray intersection occludes.</summary>
	/// <returns>Occluder true or non-occluder false</returns>
	bool doesOcclude() const
	{
		return _isOccluder;
	}

	/// <summary>Enable/disables the occluder intersection state.</summary>
	/// <param name="enable">true for enable, false for disable.</param>
	/// <returns>This object</returns>
	RayIntersectionStateCreateParam& setOccluder(bool enable)
	{
		_isOccluder = enable;
		return *this;
	}

	/// <summary>Getter for whether the ray intersection uses a decal.</summary>
	/// <returns>True if a decal is used, false if not.</returns>
	bool hasDecal() const
	{
		return _decal;
	}

	/// <summary>Enable/disables the decal intersection state.</summary>
	/// <param name="enable">true for enable, false for disable.</param>
	/// <returns>This object</returns>
	RayIntersectionStateCreateParam& setDecal(bool enable)
	{
		_decal = enable;
		return *this;
	}

	/// <summary>Getter for the visible faces used in the intersection state.</summary>
	/// <returns>This visible faces.</returns>
	pvr::types::VisibleFace getVisibleFace() const
	{
		return _visibleFace;
	}

	/// <summary>Sets the visible face for the intersection state.</summary>
	/// <param name="visibleFace">The visible face to use in the intersection.</param>
	/// <returns>This object</returns>
	RayIntersectionStateCreateParam& setVisibleFace(pvr::types::VisibleFace visibleFace)
	{
		_visibleFace = visibleFace;
		return *this;
	}

	/// <summary>Getter for the polygon winding order.</summary>
	/// <returns>The polygon winding order used</returns>
	pvr::types::PolygonWindingOrder getPolygonWindingOrder() const
	{
		return _windingOrder;
	}

	/// <summary>Sets winding order used in the intersection state.</summary>
	/// <param name="windingOrder">The winding order to use in the intersection.</param>
	/// <returns>This object</returns>
	RayIntersectionStateCreateParam& setWindingOrder(pvr::types::PolygonWindingOrder windingOrder)
	{
		_windingOrder = windingOrder;
		return *this;
	}

	/// <summary>Gets the number of ray type visibilities.</summary>
	/// <returns>The number of active ray type visibilities</returns>
	size_t getNumRayTypeVisibilities() const
	{
		return _rayTypeVisibilityCount;
	}

	/// <summary>Gets the ray type visibility for the ray type at index.</summary>
	/// <param name="index">The index of the ray type.</param>
	/// <returns>visibility for a certain ray type</returns>
	bool getRayTypeVisibility(pvr::uint32 index) const
	{
		debug_assertion(index < pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes, "Ray type index is invalid.");
		return _rayTypeVisibilities[index].visibility;
	}

	/// <summary>Sets the ray type visibility for the ray type at index.</summary>
	/// <param name="index">The index of the ray type.</param>
	/// <param name="visiblity">The visibility for the ray type.</param>
	/// <returns>This object</returns>
	RayIntersectionStateCreateParam& setRayTypeVisibility(pvr::uint32 index, const bool& visiblity)
	{
		debug_assertion(index < pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes, "Ray type index is invalid.");
		_rayTypeVisibilities[index].visibility = visiblity;

		// only increment if a visibility does not previously exist
		if (!_rayTypeVisibilities[index].isValid)
		{
			_rayTypeVisibilityCount++;
		}

		return *this;
	}

	/// <summary>Sets a number of ray type visibilities.</summary>
	/// <param name="offset">The offset of the ray.</param>
	/// <param name="count">The number of visibilities for the ray type.</param>
	/// <param name="visiblities">A pointer to at least count booleans for visibility.</param>
	/// <returns>This object</returns>
	RayIntersectionStateCreateParam& setRayTypeVisibilities(pvr::uint32 offset, pvr::uint32 count, bool const* visiblities)
	{
		debug_assertion(offset + count < pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes, "Ray type index is invalid.");

		pvr::uint32 currentIndex = 0;
		for (pvr::uint32 i = offset; i < count; i++)
		{
			_rayTypeVisibilities[i].visibility = visiblities[currentIndex];
			_rayTypeVisibilities[i].isValid = true;
			currentIndex++;
		}

		// after adding count visibilities check the new total number of valid visibilities added
		for (pvr::uint32 i = 0; i < pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes; i++)
		{
			if (_rayTypeVisibilities[i].isValid)
			{
				_rayTypeVisibilityCount++;
			}
		}

		return *this;
	}

private:
	bool _isOccluder;
	bool _decal;
	struct RayVisibility
	{
		bool visibility;
		bool isValid;
	};
	RayVisibility _rayTypeVisibilities[pvr::types::PipelineDefaults::ShaderStage::MaxRayTypes];
	pvr::uint32 _rayTypeVisibilityCount;
	pvr::types::VisibleFace _visibleFace;
	pvr::types::PolygonWindingOrder _windingOrder;
};













}
}
}
