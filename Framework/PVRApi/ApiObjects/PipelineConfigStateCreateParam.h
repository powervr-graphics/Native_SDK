/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\PipelineConfigStateCreateParam.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the CreateParameters for the Pipeline Config States, used to set states to the PipelineCreateParam objects.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/GraphicsStateCreateParam.h"
#include "PVRApi/ApiObjects/Texture.h"
#include <map>
#include <vector>
namespace pvr {
namespace api {
namespace impl {
struct GraphicsStateContainer;
class GraphicsPipelineImpl;
}
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
private:
	friend class ::pvr::api::impl::GraphicsPipelineImpl;
	// depth



	// stencil
	bool depthTest;  //!< Enable/disable depth test. Default false
	bool depthWrite;
	bool stencilTestEnable; //!< Enable/disable stencil test. Default false
	StencilOp::Enum opDepthPassFront;
	StencilOp::Enum opDepthFailFront;
	StencilOp::Enum opStencilFailFront;

	StencilOp::Enum opDepthPassBack;
	StencilOp::Enum opDepthFailBack;
	StencilOp::Enum opStencilFailBack;

	ComparisonMode::Enum depthCmpOp; //!< Depth compare operation. Default LESS
	ComparisonMode::Enum cmpOpStencilFront;
	ComparisonMode::Enum cmpOpStencilBack;
	void createStateObjects(impl::GraphicsStateContainer& state, DepthStencilStateCreateParam* parent = NULL)const;
public:

	/*!*********************************************************************************************************************
	\brief  Set all Depth and Stencil parameters.
	***********************************************************************************************************************/
	DepthStencilStateCreateParam(bool depthWrite = true, bool depthTest = true,
	                             ComparisonMode::Enum depthCompareFunc = ComparisonMode::Less,
	                             bool stencilTest = false,
	                             StencilOp::Enum opFrontStencilFail = StencilOp::Keep,
	                             StencilOp::Enum opFrontDepthFail = StencilOp::Keep,
	                             StencilOp::Enum opFrontDepthPass = StencilOp::Keep,
	                             StencilOp::Enum opBackStencilFail = StencilOp::Keep,
	                             StencilOp::Enum opBackDepthFail = StencilOp::Keep,
	                             StencilOp::Enum opBackDepthPass = StencilOp::Keep) :
		depthTest(depthTest), depthWrite(depthWrite), stencilTestEnable(stencilTest), 
		opDepthPassFront(opFrontDepthPass), opDepthFailFront(opFrontDepthFail), opStencilFailFront(opFrontStencilFail),
		opDepthPassBack(opBackDepthPass), opDepthFailBack(opBackDepthFail), opStencilFailBack(opBackStencilFail),
		depthCmpOp(depthCompareFunc), cmpOpStencilFront(ComparisonMode::Default), 	cmpOpStencilBack(ComparisonMode::Default) {}

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
	DepthStencilStateCreateParam& setDepthCompareFunc(ComparisonMode::Enum compareFunc)
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
	\brief	Set the operation to perform on the Stencil buffer when the stencil test fails.
	\return this object (allows chained calls)
	\param	face The face for which the operation will be updated.
	\param  stencilOp The operation to perform on (face) on stencil fail.
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilOpStencilFail(pvr::api::Face::Enum face, StencilOp::Enum stencilOp)
	{
		switch (face)
		{
		case pvr::api::Face::Front: opStencilFailFront = stencilOp; break;
		case pvr::api::Face::Back: opStencilFailBack = stencilOp; break;
		case pvr::api::Face::FrontBack: opStencilFailFront = opStencilFailBack = stencilOp; break;
		case pvr::api::Face::None: break;
		}
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Set the operation to perform on the Stencil buffer when the Stencil test passes but the Depth test fails.
	\return this object (allows chained calls)
	\param	face The face for which the operation will be updated.
	\param  stencilOp The operation to perform on (face) on stencil pass & depth fail.
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilOpDepthFail(pvr::api::Face::Enum face, StencilOp::Enum stencilOp)
	{
		switch (face)
		{
		case pvr::api::Face::Front: opDepthFailFront = stencilOp; break;
		case pvr::api::Face::Back: opDepthFailBack = stencilOp; break;
		case pvr::api::Face::FrontBack: opDepthFailFront = opDepthFailBack = stencilOp; break;
		case pvr::api::Face::None: break;
		}
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Set the operation  to perform on the Stencil buffer when both the Stencil and Depth tests pass.
	\return this object (allows chained calls)
	\param	face The face for which the operation will be updated.
	\param  stencilOp The operation to perform on (face) on stencil pass & depth fail.
	\description For purposes of this function, the depth test is also considered passed if the Depth testing is
	        disabled or there is no depth buffer.
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilOpDepthPass(pvr::api::Face::Enum face, StencilOp::Enum stencilOp)
	{
		switch (face)
		{
		case pvr::api::Face::Front: opDepthPassFront = stencilOp; break;
		case pvr::api::Face::Back: opDepthPassBack = stencilOp; break;
		case pvr::api::Face::FrontBack: opDepthPassFront = opDepthPassBack = stencilOp; break;
		case pvr::api::Face::None: break;
		}
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Set all stencil ops at the same time.
	\return this object (allows chained calls)
	\param	face The face for which the operations will be updated.
	\param	stencilFail Specifies the stencil action to perform when the stencil test fails.
	\param	depthFail Specifies the stencil action to perform when the stencil test passes, but the depth
	        test fails.
	\param	depthPass Specifies the stencil action  to perform when both the stencil test and the depth
	        test pass, or when the stencil test passes and either there is no depth buffer or depth testing is not
			enabled.
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilOp(pvr::api::Face::Enum face, StencilOp::Enum stencilFail,
	        StencilOp::Enum depthFail, StencilOp::Enum depthPass)
	{
		switch (face)
		{
		case pvr::api::Face::Front:
			opStencilFailFront = stencilFail;
			opDepthFailFront = depthFail;
			opDepthPassFront = depthPass;
			break;
		case pvr::api::Face::Back:
			opStencilFailBack = stencilFail;
			opDepthFailBack = depthFail;
			opDepthPassBack = depthPass;
			break;
		case pvr::api::Face::FrontBack:
			opStencilFailFront = stencilFail;
			opDepthFailFront = depthFail;
			opDepthPassFront = depthPass;
			opStencilFailBack = stencilFail;
			opDepthFailBack = depthFail;
			opDepthPassBack = depthPass;
			break;
		case pvr::api::Face::None: break;
		}
		return *this;
	}

	/*!****************************************************************************************************************
	\brief	Set the stencil compare function.
	\return this object (allows chained calls)
	\param	face The face for which the compare function will be updated.
	\param	cmpMode The compare mode to set the face to.
	*******************************************************************************************************************/
	DepthStencilStateCreateParam& setStencilCompareFunc(pvr::api::Face::Enum face, ComparisonMode::Enum cmpMode)
	{
		switch (face)
		{
		case Face::Front:
			cmpOpStencilFront = cmpMode;
			break;
		case Face::Back:
			cmpOpStencilBack = cmpMode;
			break;
		case Face::FrontBack:
			cmpOpStencilFront = cmpOpStencilBack = cmpMode;
			break;
		case Face::None: break;
		}
		return *this;
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
	friend class ::pvr::api::impl::GraphicsPipelineImpl;
	void createStateObjects(impl::GraphicsStateContainer& state, VertexInputCreateParam* parent = NULL)const;
	std::map<pvr::uint16, VertexInputBindingInfo> inputBindings;
	std::map<pvr::uint16, std::vector<VertexAttributeInfo> > attributes;
public:

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
	VertexInputCreateParam& setInputBinding(pvr::uint16 bufferBinding, pvr::uint16 strideInBytes = 0,
	                                        StepRate::Enum stepRate = StepRate::Vertex)
	{
		inputBindings[bufferBinding] = VertexInputBindingInfo(bufferBinding, strideInBytes, stepRate);
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
		attributes[bufferBinding].push_back(attrib);
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
	        const VertexAttributeLayout& layout, const char* attributeName = "")
	{
		attributes[bufferBinding].push_back(
		    VertexAttributeInfo(index, layout.dataType, layout.width, layout.offset, attributeName));
		return *this;
	};
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
	/*!*********************************************************************************************************************
	\brief Create and configure an InputAssembler configuration.
	\param topology Primitive Topology (default: TriangleList)
 	\param disableVertexReuse   Disable Vertex Reuse (true:disabled false:enabled). Default:true
	\param primitiveRestartEnable (true: enabled, false: disabled) Default:false
	\param primitiveRestartIndex Primitive Restart Index. Default  0xFFFFFFFF
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	InputAssemblerStateCreateParam(PrimitiveTopology::Enum topology = PrimitiveTopology::None,
	                               bool disableVertexReuse = true, bool primitiveRestartEnable = false, pvr::uint32 primitiveRestartIndex = 0xFFFFFFFF) :
		topology(topology), disableVertexReuse(disableVertexReuse), primitiveRestartEnable(primitiveRestartEnable),
		primitiveRestartIndex(primitiveRestartIndex) {}

	/*!*********************************************************************************************************************
	\brief Set a primitive restart index (an index that, if found in the vertex stream, will stop and restart the rendering as if
	       another draw command had been issued).
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	InputAssemblerStateCreateParam& setPrimitiveRestartIndex(pvr::uint32 restartIndex)
	{
		primitiveRestartIndex = restartIndex; return *this;
	}

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
	InputAssemblerStateCreateParam& setPrimitiveTopology(PrimitiveTopology::Enum topology)
	{
		this->topology = topology; return *this;
	}
private:
	friend class ::pvr::api::impl::GraphicsPipelineImpl;
	mutable PrimitiveTopology::Enum topology;
	bool							  disableVertexReuse;
	bool							  primitiveRestartEnable;
	pvr::uint32						  primitiveRestartIndex;
	void createStateObjects(impl::GraphicsStateContainer& state, InputAssemblerStateCreateParam* parent = NULL)const;

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
	BlendFactor::Enum	srcBlendColor;	//!< Source Blending color factor
	BlendFactor::Enum	destBlendColor;	//!< Destination blending color factor
	BlendFactor::Enum	srcBlendAlpha;	//!< Source blending alpha factor
	BlendFactor::Enum	destBlendAlpha;	//!< Destination blending alpha factor
	BlendOp::Enum		blendOpColor;	//!< Blending operation color
	BlendOp::Enum		blendOpAlpha;	//!< Blending operation alpha
	ColorChannel::Bits	channelWriteMask;//!<Channel writing mask

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
	ColorBlendAttachmentState(bool blendEnable = false, BlendFactor::Enum srcBlendColor = BlendFactor::One,
	                          BlendFactor::Enum destBlendColor = BlendFactor::Zero, BlendFactor::Enum srcBlendAlpha = BlendFactor::One,
	                          BlendFactor::Enum destBlendAlpha = BlendFactor::Zero, BlendOp::Enum blendOpColor = BlendOp::Add,
	                          BlendOp::Enum blendOpAlpha = BlendOp::Add, pvr::uint32 channelWriteMask = ColorChannel::All) :
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
	ColorBlendAttachmentState(bool blendEnable, BlendFactor::Enum srcBlendColorAlpha, BlendFactor::Enum dstBlendColorAlpha,
	                          BlendOp::Enum blendOpColorAlpha, pvr::uint32 channelWriteMask = ColorChannel::All) :
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
private:
	bool alphaToCoverageEnable;
	bool logicOpEnable;
	LogicOp::Enum logicOp;
	std::vector<ColorBlendAttachmentState> attachmentStates;
	void createStateObjects(impl::GraphicsStateContainer& state, ColorBlendStateCreateParam* parent = NULL)const;
	friend class ::pvr::api::impl::GraphicsPipelineImpl;
public:

	/*!*********************************************************************************************************************
	\brief Create a Color Blend state object.
	\param alphaToCoverageEnable enable/ disable alpa to coverage (default:disable)
	\param logicOpEnable enable/disable logicOp (default:disable)
	\param logicOp Select logic operation (default:Set)
	\param attachmentStates An array of color blend attachment states (default: NULL)
	\param numAttachmentStates Number of color attachment states in array (default: 0)
	***********************************************************************************************************************/
	ColorBlendStateCreateParam(bool alphaToCoverageEnable = false,
	                           bool logicOpEnable = false,
	                           LogicOp::Enum logicOp = LogicOp::Set,
	                           ColorBlendAttachmentState* attachmentStates = NULL,
	                           pvr::uint32 numAttachmentStates = 0) :
		alphaToCoverageEnable(alphaToCoverageEnable), logicOpEnable(logicOpEnable), logicOp(logicOp)
	{
		this->attachmentStates.assign(attachmentStates, attachmentStates + numAttachmentStates);
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
	ColorBlendStateCreateParam& setLogicOp(LogicOp::Enum logicOp)
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
	ColorBlendStateCreateParam& addAttachmentState(pvr::uint32 index, const ColorBlendAttachmentState& state)
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
private:
	friend class ::pvr::api::impl::GraphicsPipelineImpl;
	pvr::uint32 viewportCount;
	CoordinateOrigin::Enum clipOrigin;
	DepthMode::Enum depthMode;
	void createStateObjects(impl::GraphicsStateContainer& state, ViewportStateCreateParam* parent = NULL)const;
public:

	/*!*********************************************************************************************************************
	\brief Constructs a Viewport State object.
	\param numViewport The total number of viewports (default 1)
	\param clipOrigin The Clip Origin of all viewports (default LowerLeft)
	\param depthMode  The depth mode of all viewports (default 0..1)
	***********************************************************************************************************************/
	ViewportStateCreateParam(pvr::uint32 numViewport = 1,
	                         CoordinateOrigin::Enum clipOrigin = CoordinateOrigin::LowerLeft,
	                         DepthMode::Enum depthMode = DepthMode::ZeroToOne) :
		viewportCount(numViewport),
		clipOrigin(clipOrigin),
		depthMode(depthMode) {}

	/*!*********************************************************************************************************************
	\brief Set the total number of viewports.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ViewportStateCreateParam& setNumViewport(pvr::uint32 numViewport)
	{
		viewportCount = numViewport; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the clip origin.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ViewportStateCreateParam& setClipOrigin(CoordinateOrigin::Enum clipOrigin)
	{
		this->clipOrigin = clipOrigin;	return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the depth mode.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	ViewportStateCreateParam& setDepthMode(DepthMode::Enum depthMode)
	{
		this->depthMode = depthMode; return *this;
	}
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
private:
	Face::Enum cullFace;
	PolygonWindingOrder::Enum cullMode;
	bool enableDepthClip;
	bool enableRasterizerDiscard;
	bool enableProgramPointSize;
	CoordinateOrigin::Enum pointOrigin;
	FillMode::Enum fillMode;
	ProvokingVertex::Enum provokingVertex;
public:
	friend class ::pvr::api::impl::GraphicsPipelineImpl;

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
	RasterStateCreateParam(Face::Enum cullFace = Face::Back,
	                       PolygonWindingOrder::Enum windingOrder = PolygonWindingOrder::FrontFaceCCW,
	                       bool enableDepthClip = true,
	                       bool enableRasterizerDiscard = false,
	                       bool enableProgramPointSize = false,
	                       CoordinateOrigin::Enum pointOrigin = CoordinateOrigin::LowerLeft,
	                       FillMode::Enum fillMode = FillMode::FrontBackFill,
	                       ProvokingVertex::Enum provokingVertex = ProvokingVertex::First) :
		cullFace(cullFace),
		cullMode(windingOrder),
		enableDepthClip(enableDepthClip),
		enableRasterizerDiscard(enableRasterizerDiscard),
		enableProgramPointSize(enableProgramPointSize),
		pointOrigin(pointOrigin),
		fillMode(fillMode),
		provokingVertex(provokingVertex)	{}

	/*!*********************************************************************************************************************
	\brief Set the face that will be culled (front/back/both/none).
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setCullFace(Face::Enum face)
	{
		this->cullFace = face; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set polygon winding order.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setCullMode(PolygonWindingOrder::Enum cullMode)
	{
		this->cullMode = cullMode; return *this;
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
	\brief Set point origin.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setPointOrigin(CoordinateOrigin::Enum coordinateOrigin)
	{
		pointOrigin = coordinateOrigin; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set polygon fill mode.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setFillMode(FillMode::Enum mode)
	{
		fillMode = mode; return *this;
	}

	/*!*********************************************************************************************************************
	\brief Set the Provoking Vertex.
	\return this object (allows chained calls)
	***********************************************************************************************************************/
	RasterStateCreateParam& setProvokingVertex(ProvokingVertex::Enum provokingVertex)
	{
		this->provokingVertex = provokingVertex; return *this;
	}
private:
	void createStateObjects(impl::GraphicsStateContainer& storage, RasterStateCreateParam* parent = NULL)const;
};

//!\cond NO_DOXYGEN
struct MultiSampleStateCreateParam
{
private:
	friend class ::pvr::api::impl::GraphicsPipelineImpl;
	bool multisampleEnable;
	bool sampleShadingEnable;
	pvr::uint32 numSamples;
	pvr::float32 minSampleShading;
	pvr::uint32 sampleMask;
	void createStateObjects(impl::GraphicsStateContainer& state, MultiSampleStateCreateParam* parent = NULL)const;
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
	                             pvr::uint32 numSamples = 1,  pvr::float32 minSampleShading = 0.0f,  pvr::uint32 sampleMask = 0) :
		multisampleEnable(multisampleEnable), sampleShadingEnable(sampleShadingEnable),
		numSamples(numSamples),	minSampleShading(minSampleShading),	sampleMask(sampleMask) {}

	/*!*********************************************************************************************************************
	\brief Enable/disable multisampling
	\param enable true enable, false disable
	\return this (allow chaining)
	***********************************************************************************************************************/
	MultiSampleStateCreateParam& setMultiSampleEnable(bool enable)
	{
		multisampleEnable = enable; return *this;
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
	MultiSampleStateCreateParam& setNumSamples(pvr::uint32 numSamples)
	{
		this->numSamples = numSamples; return *this;
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
};
//!\endcond

/*!*********************************************************************************************************************
\brief Pipeline vertex Shader stage create param.
***********************************************************************************************************************/
struct VertexShaderStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipelineImpl;
private:
	api::Shader shader;
	void createStateObjects(impl::GraphicsStateContainer& state, VertexShaderStageCreateParam* parent = NULL)const;
public:

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
};

/*!*********************************************************************************************************************
\brief Pipeline Fragement shader stage create param.
***********************************************************************************************************************/
struct FragmentShaderStageCreateParam
{
	friend class ::pvr::api::impl::GraphicsPipelineImpl;
private:
	api::Shader shader;

	/*!*********************************************************************************************************************
	\brief Create pipeline state object.
	***********************************************************************************************************************/
	void createStateObjects(impl::GraphicsStateContainer& state, FragmentShaderStageCreateParam* parent = NULL)const;
public:

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
};
}
}
}
