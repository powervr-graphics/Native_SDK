/*!*********************************************************************************************************************
\file         PVRApi\OGLES\PipelineConfigStatesGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Internal use. Contains objects required by the OpenGL ES versions of GraphicsPipeline and ComputePipeline.
				These are the objects that  the actual work and  execute the underlying API commands in their "set",
				"unset", "reset" functions.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRApi/ApiObjects/PipelineConfig.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/Bindables.h"

namespace pvr {
namespace api {
namespace gles { class GraphicsPipelineImplGles; }
namespace impl {

//!\cond NO_DOXYGEN
enum class GraphicsStateType
{
	ShaderProgram, VertexShader, FragmentShader, GeometryShader, TessellationControlShader,
	TessellationEvaluationShader, DepthTest, DepthClear, DepthWrite,
	PolygonCulling, PolygonWindingOrder, BlendRgba, BlendTest, PolygonFill,
	ScissorTest, StencilOpFront, StencilOpBack, FrameBufferClear,
	FrameBufferWrite, DepthFunc, BlendEq, StencilTest, StencilClear,
	VertexAttributeFormatState, VertexAttributeLocation, Count
};

//!\endcond
class GraphicsPipeline_;
class GraphicsPipelineImplState;

/*!**********************************************************************************
\brief Base interface for a pipeline state object.
************************************************************************************/
class PipelineState
{
	friend class ::pvr::api::gles::GraphicsPipelineImplGles;
	friend class ::pvr::api::impl::ComputePipeline_;
	friend struct ::pvr::api::ComputePipelineCreateParam;
	friend struct ::pvr::api::GraphicsPipelineCreateParam;
public:
	typedef PipelineState* ptr_type;
	virtual ptr_type createClone() const = 0;
	virtual ptr_type createDefault() const = 0;
	static void destroyClone(ptr_type clone)
	{
		delete clone;
	}
	PipelineState() : m_parent(NULL) { }

	/*!**********************************************************************************
	\brief Set default state.
	************************************************************************************/
	virtual void setDefault(IGraphicsContext& device) = 0;

	/*!**********************************************************************************
	\brief Destructor.
	************************************************************************************/
	virtual ~PipelineState() {}

	/*!**********************************************************************************
	\brief Set this state.
	************************************************************************************/
	virtual void set(IGraphicsContext& device) = 0;

	/*!**********************************************************************************
	\brief Unset this state.
	************************************************************************************/
	virtual void unset(IGraphicsContext& device) = 0;

	/*!**********************************************************************************
	\brief Reset this state.
	************************************************************************************/
	virtual void reset(IGraphicsContext& device) = 0;

	/*!******************************************************************************
	\brief	Type of this state.
	\return	void
	********************************************************************************/
	virtual GraphicsStateType getStateType() const = 0;

	/*!******************************************************************************
	\brief	Check is a valid state.
	\return	bool
	********************************************************************************/
	virtual bool isValid() { return m_isValid; }
protected:
	PipelineState*  m_parent;
	bool			m_isValid;
};

/*!******************************************************************************
\brief Base class for graphics pipeline state.
********************************************************************************/
class GraphicsPipelineImplState : public PipelineState
{
public:
	virtual ~GraphicsPipelineImplState() {}
protected:
};

/*!******************************************************************************
\brief Base class for compute pipeline states.
********************************************************************************/
class ComputePipelineImplState : public PipelineState
{
public:
	virtual ~ComputePipelineImplState() {}
};
}


namespace pipelineCreation {
//	Forward Declarations
struct DepthStencilStateCreateParam;
struct RasterStateCreateParam;
struct ColorBlendStateCreateParam;
struct ViewportStateCreateParam;
struct StencilStateCreateParam;
}

namespace gles {


class DepthTestState : public impl::GraphicsPipelineImplState
{
public:

	/*!
	   \brief Constructor
	   \param enable Enable/ disable depthtest (Default: false)
	 */
	DepthTestState(bool enable = types::PipelineDefaults::DepthStencilStates::DepthTestEnabled) { m_depthTestEnabled = enable; }

	/*!
	   \brief Create a new defualt DepthTestState
	 */
	impl::PipelineState::ptr_type createDefault() const { return new DepthTestState(); }

	/*!
	   \brief Create a new clone of this state
	 */
	impl::PipelineState::ptr_type createClone() const
	{
		return new DepthTestState(m_depthTestEnabled);
	}

	/*!
	   \brief Get this state type
	 */
	impl::GraphicsStateType getStateType() const { return impl::GraphicsStateType::DepthTest; }

	/*!
	   \brief operator ==
	   \param rhs
	   \return Return true if equal
	 */
	bool operator==(const DepthTestState& rhs)const	{ return (m_depthTestEnabled == rhs.m_depthTestEnabled); }

	/*!
	   \brief operator !=
	   \param rhs
	   \return Return true if not equal
	 */
	bool operator!=(const DepthTestState& rhs)const { return !(*this == rhs); }

	/*!
	   \brief commit this state to the gpu
	   \param device
	 */
	void set(IGraphicsContext& device) { commitState(device, m_depthTestEnabled); }

	/*!
	   \brief Reset this state to default
	   \param device
	 */
	void reset(IGraphicsContext& device) { m_depthTestEnabled = types::PipelineDefaults::DepthStencilStates::DepthTestEnabled; }

	/*!
	   \brief Unset this state. Roll back to the parent state, or to the default state.
	   \param device
	 */
	void unset(IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }

	/*!
	   \brief Commit this state to the gpu
	   \param device
	   \param depthTest Enable/disable depth test
	 */
	void commitState(IGraphicsContext& device, bool depthTest);

	/*!
	   \brief Commit default state to the gpu.
	   \param device
	 */
	void setDefault(IGraphicsContext& device) { commitState(device, types::PipelineDefaults::DepthStencilStates::DepthTestEnabled); }

	bool m_depthTestEnabled;
};

class DepthFuncState : public impl::GraphicsPipelineImplState
{
public:
	bool operator==(const DepthFuncState& rhs)const { return m_depthFunc == rhs.m_depthFunc; }
	bool operator!=(const DepthFuncState& rhs)const { return !(*this == rhs); }

	void set(IGraphicsContext& device) { commitState(device, m_depthFunc); }
	void unset(IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); }
		else { setDefault(device); }
	}
	void reset(IGraphicsContext& device) { m_depthFunc = types::ComparisonMode::DefaultDepthFunc; }
	impl::PipelineState::ptr_type createClone()const { return new DepthFuncState(m_depthFunc); }
	impl::PipelineState::ptr_type createDefault()const { return new DepthFuncState(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::DepthFunc; }
	DepthFuncState() { m_depthFunc = types::ComparisonMode::DefaultDepthFunc; }
	DepthFuncState(types::ComparisonMode depthFunc) { m_depthFunc = depthFunc; }
	void setDefault(IGraphicsContext& device) { commitState(device, types::ComparisonMode::DefaultDepthFunc); }
	void commitState(IGraphicsContext& device, types::ComparisonMode func);
	types::ComparisonMode m_depthFunc;
};

class DepthWriteState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, m_depthWriteEnabled); }
	void unset(IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); }
		else { setDefault(device); }
	}
	void reset(IGraphicsContext& device) { m_depthWriteEnabled = types::PipelineDefaults::DepthStencilStates::DepthWriteEnabled; }
	bool operator==(const DepthWriteState& rhs)const { return m_depthWriteEnabled == rhs.m_depthWriteEnabled; }
	impl::GraphicsStateType getStateType() const { return impl::GraphicsStateType::DepthWrite; }
	bool operator!=(const DepthWriteState& rhs)const { return m_depthWriteEnabled != rhs.m_depthWriteEnabled; }
	impl::PipelineState::ptr_type createClone()const { return new DepthWriteState(m_depthWriteEnabled); }
	impl::PipelineState::ptr_type createDefault()const { return new DepthWriteState(); }
	DepthWriteState() { m_depthWriteEnabled = types::PipelineDefaults::DepthStencilStates::DepthWriteEnabled; }
	DepthWriteState(bool enabled) { m_depthWriteEnabled = enabled; }
	void setDefault(IGraphicsContext& device) { commitState(device, types::PipelineDefaults::DepthStencilStates::DepthWriteEnabled); };
	void commitState(IGraphicsContext& device, bool depthWrite);
	bool m_depthWriteEnabled;
};

/*!****************************************************************************************************************
\brief Pipeline. Controls the polygon culling state.
*******************************************************************************************************************/
class PolygonFrontFaceState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, m_cullFace); }
	void reset(IGraphicsContext& device) { m_cullFace = types::Face::DefaultCullFace; }
	void unset(IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); }
		else { setDefault(device); }
	}
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::PolygonCulling; }
	impl::PipelineState::ptr_type createClone()const { return new PolygonFrontFaceState(*this); }
	impl::PipelineState::ptr_type createDefault()const { return new PolygonFrontFaceState(); }
	bool operator==(const PolygonFrontFaceState& rhs)const { return (m_cullFace == rhs.m_cullFace); }
	bool operator!=(const PolygonFrontFaceState& rhs)const { return !(*this == rhs); }
	PolygonFrontFaceState() : m_cullFace(types::Face::DefaultCullFace) {}
	PolygonFrontFaceState(types::Face cullFace) : m_cullFace(cullFace) {}

	void commitState(IGraphicsContext& device, types::Face cullFace);
	void setDefault(IGraphicsContext& device) { commitState(device, types::Face::DefaultCullFace); }
	types::Face m_cullFace;
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the polygon winding-order.
*******************************************************************************************************************/
class PolygonWindingOrderState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, m_windingOrder); }
	void unset(IGraphicsContext& device)
	{
		if (m_parent) {	m_parent->set(device); }
		else { setDefault(device); }
	}
	void reset(IGraphicsContext& device) { m_windingOrder = types::PolygonWindingOrder::Default; }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::PolygonWindingOrder; }
	ptr_type createDefault()const { return new PolygonWindingOrderState(); }
	ptr_type createClone()const { return new PolygonWindingOrderState(*this); }
	PolygonWindingOrderState() {}
	PolygonWindingOrderState(types::PolygonWindingOrder windingOrder) : m_windingOrder(windingOrder) {}
	void setDefault(IGraphicsContext& device) { commitState(device, types::PolygonWindingOrder::Default); };
	void commitState(IGraphicsContext& device, types::PolygonWindingOrder windingOrderCCW);
	types::PolygonWindingOrder m_windingOrder;
};

class TessPatchControlPoints : public impl::GraphicsPipelineImplState
{
public:
	uint32 patchControlPoints;
	TessPatchControlPoints(uint32 patchControlPoints =
	                         types::PipelineDefaults::Tesselation::NumControlPoints) :
		patchControlPoints(patchControlPoints) {}
	void execute(impl::CommandBuffer_& cmdBuff);
	bool operator==(const TessPatchControlPoints& rhs)const { return patchControlPoints == rhs.patchControlPoints; }
	bool operator!=(const TessPatchControlPoints& rhs)const { return !(*this == rhs); }
	ptr_type createClone()const { return new TessPatchControlPoints(*this); }
	ptr_type createDefault()const
	{
		return new TessPatchControlPoints(
		         types::PipelineDefaults::Tesselation::NumControlPoints);
	}
	void set(IGraphicsContext& device) { commitState(device, patchControlPoints); }
	void unset(IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); return; }
		setDefault(device);
	}
	void setDefault(IGraphicsContext& device)
	{
		commitState(device, types::PipelineDefaults::Tesselation::NumControlPoints);
	}
	void reset(IGraphicsContext& device)
	{
		patchControlPoints = types::PipelineDefaults::Tesselation::NumControlPoints;
	}
	impl::GraphicsStateType getStateType()const
	{
		return impl::GraphicsStateType::TessellationControlShader;
	}
	void commitState(IGraphicsContext& device, uint32 patchControlPoints);
};


/*!*********************************************************************************************************************
\brief  Sets the Color write mask.
***********************************************************************************************************************/
class ColorWriteMask : public impl::GraphicsPipelineImplState
{
public:
	glm::bvec4 writeMask;
	void execute(impl::CommandBuffer_& cmdBuff);
	bool operator==(const ColorWriteMask& rhs)const { return writeMask == rhs.writeMask; }
	bool operator!=(const ColorWriteMask& rhs)const { return !(*this == rhs); }
	ptr_type createClone()const { return new ColorWriteMask(*this); }
	ptr_type createDefault()const { return new ColorWriteMask(); }
	void set(IGraphicsContext& device) { commitState(device, writeMask); }
	void unset(IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); return; }
		setDefault(device);
	}
	void setDefault(IGraphicsContext& device)
	{
		commitState(device,
		            glm::bvec4(types::PipelineDefaults::ColorWrite::ColorMaskR,
		                       types::PipelineDefaults::ColorWrite::ColorMaskG,
		                       types::PipelineDefaults::ColorWrite::ColorMaskB,
		                       types::PipelineDefaults::ColorWrite::ColorMaskA));
	}
	void reset(IGraphicsContext& device) {}
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::BlendTest; }
	void commitState(IGraphicsContext& device, const glm::bvec4 mask);

	ColorWriteMask(bool r, bool g, bool b, bool a)
	{
		writeMask[0] = r, writeMask[1] = g, writeMask[2] = b, writeMask[3] = a;
	}

	ColorWriteMask(bool maskRGBA = types::PipelineDefaults::ColorWrite::ColorMaskR)
	{
		writeMask = glm::bvec4(maskRGBA);
	}

	ColorWriteMask(types::ColorChannel channelBits) :
		writeMask(static_cast<pvr::uint32>(channelBits & types::ColorChannel::R) != 0,
		          static_cast<pvr::uint32>(channelBits & types::ColorChannel::G) != 0,
		          static_cast<pvr::uint32>(channelBits & types::ColorChannel::B) != 0,
		          static_cast<pvr::uint32>(channelBits & types::ColorChannel::A) != 0)
	{
	}
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the Blending enable/disable.
*******************************************************************************************************************/
class BlendingEnableState : public impl::GraphicsPipelineImplState
{
public:
	bool operator==(const BlendingEnableState& rhs)const { return m_blendTestEnabled == rhs.m_blendTestEnabled; }
	bool operator!=(const BlendingEnableState& rhs)const { return !(*this == rhs); }
	ptr_type createClone()const { return new BlendingEnableState(*this); }
	ptr_type createDefault()const { return new BlendingEnableState(); }
	void set(IGraphicsContext& device) { commitState(device, m_blendTestEnabled); }
	void unset(IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); return; }
		commitState(device, !m_blendTestEnabled);
	}
	void reset(IGraphicsContext& device) { m_blendTestEnabled = types::PipelineDefaults::ColorBlend::BlendEnabled; }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::BlendTest; }
	BlendingEnableState() { }
	BlendingEnableState(bool enable) : m_blendTestEnabled(enable) { }
	bool m_blendTestEnabled;
	void setDefault(IGraphicsContext& device) { commitState(device, types::PipelineDefaults::ColorBlend::BlendEnabled); }
	void commitState(IGraphicsContext&, bool blendTest);
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the Blend Operation.
*******************************************************************************************************************/
class BlendFactorState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device)
	{
		commitState(device, types::BlendFactor((m_pack & 0xF000) >> 12), types::BlendFactor((m_pack & 0x0F00) >> 8), types::BlendFactor((m_pack & 0x00F0) >> 4), types::BlendFactor(m_pack & 0x000F));
	}
	void reset(IGraphicsContext& device)
	{
		packData(types::BlendFactor::DefaultSrcRgba, types::BlendFactor::DefaultDestRgba,
		         types::BlendFactor::DefaultSrcRgba, types::BlendFactor::DefaultDestRgba);
		setDefault(device);
	}
	void unset(IGraphicsContext& device)
	{
		if (m_parent)
		{
			m_parent->set(device);
		}
		else
		{
			setDefault(device);
		}
	}
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::BlendRgba; }
	impl::PipelineState::ptr_type createClone()const { return new BlendFactorState(m_pack); }
	impl::PipelineState::ptr_type createDefault()const { return new BlendFactorState(); }
	bool operator==(const BlendFactorState& rhs)const { return m_pack == rhs.m_pack; }
	bool operator!=(const BlendFactorState& rhs)const { return m_pack != rhs.m_pack; }

	BlendFactorState() {}
	BlendFactorState(types::BlendFactor srcRgbFactor, types::BlendFactor dstRgbFactor,
	                 types::BlendFactor srcAlphaFactor,
	                 types::BlendFactor dstAlphaFactor);
	BlendFactorState(uint32 data) : m_pack((uint16)data) {}
	void commitState(IGraphicsContext& device, types::BlendFactor srcRgbFactor, types::BlendFactor dstRgbFactor, types::BlendFactor srcAlphaFactor,
	                 types::BlendFactor dstAlphaFactor);
	void setDefault(IGraphicsContext& device)
	{
		commitState(device, types::BlendFactor::DefaultSrcRgba, types::BlendFactor::DefaultDestRgba,
		            types::BlendFactor::DefaultSrcRgba, types::BlendFactor::DefaultDestRgba);
	}
	void packData(types::BlendFactor srcRgbFactor, types::BlendFactor dstRgbFactor, types::BlendFactor srcAlphaFactor, types::BlendFactor dstAlphaFactor)
	{
		m_pack = ((uint16)srcRgbFactor << 12);
		m_pack |= ((uint16)dstRgbFactor << 8);
		m_pack |= ((uint16)srcAlphaFactor << 4);
		m_pack |= (uint16)dstAlphaFactor;
	}
	uint16 m_pack;
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the Blend equation.
*******************************************************************************************************************/
class BlendOpState : public impl::GraphicsPipelineImplState
{
public:
	bool operator==(const BlendOpState& rhs)const
	{
		return (m_rgbBlendEq == rhs.m_rgbBlendEq) && (m_alphaBlendEq == rhs.m_alphaBlendEq);
	}
	bool operator!=(const BlendOpState& rhs)const { return !(*this == rhs); }
	void set(IGraphicsContext& device) { commitState(device, m_rgbBlendEq, m_alphaBlendEq); }
	void reset(IGraphicsContext& device) { m_rgbBlendEq = m_alphaBlendEq = types::BlendOp::Default; }
	void unset(IGraphicsContext& device)
	{
		if (m_parent)
		{
			m_parent->set(device);
			return;
		}
		setDefault(device);
	}
	impl::PipelineState::ptr_type createClone()const { return new BlendOpState(m_rgbBlendEq, m_alphaBlendEq); }
	impl::PipelineState::ptr_type createDefault()const { return new BlendOpState(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::BlendEq; }

	BlendOpState() { }
	BlendOpState(types::BlendOp rgbBlendEquation, types::BlendOp alphaBlendEquation) :
		m_rgbBlendEq(rgbBlendEquation), m_alphaBlendEq(alphaBlendEquation) {}

	void commitState(IGraphicsContext& device, types::BlendOp rgbBlendEquation, types::BlendOp alphaBlendEquation);
	void setDefault(IGraphicsContext& device)
	{
		commitState(device, types::BlendOp::Default, types::BlendOp::Default);
	}

	types::BlendOp m_rgbBlendEq;
	types::BlendOp m_alphaBlendEq;
};

/*!****************************************************************************************************************
\brief  Pipeline state. Controls the depth clear value.
*******************************************************************************************************************/
class DepthClearState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(m_clearDepth); }
	void reset(IGraphicsContext& device)
	{
		m_clearDepth = types::PipelineDefaults::DepthStencilStates::DepthClearValue;
	}
	void unset(IGraphicsContext& device)
	{
		if (m_parent)
		{
			m_parent->set(device);
		}
		else
		{
			setDefault(device);
		}
	}
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::DepthClear; }
	impl::PipelineState::ptr_type createClone()const	{ return new DepthClearState(*this); }
	impl::PipelineState::ptr_type createDefault()const { return new DepthClearState(); }
	bool operator==(const DepthClearState& rhs) { return (m_clearDepth == rhs.m_clearDepth); }
	bool operator!=(const DepthClearState& rhs) { return !(*this == rhs); }

	DepthClearState() { }
	DepthClearState(float32 depth) { m_clearDepth = depth; }
	void commitState(float32 depth);
	void setDefault(IGraphicsContext& device)
	{
		commitState(types::PipelineDefaults::DepthStencilStates::DepthClearValue);
	}
	float32 m_clearDepth;
};

/*!****************************************************************************************************************
\brief  Pipeline State. Controls the stencil clear value.
*******************************************************************************************************************/
class StencilClearState : public impl::GraphicsPipelineImplState
{
public:
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilClear; }
	void set(IGraphicsContext& device) { commitState(device, m_clearStencil); }
	void unset(IGraphicsContext& device) { if (m_parent) { m_parent->set(device); return; } setDefault(device); }
	void reset(IGraphicsContext& device)
	{
		m_clearStencil = types::PipelineDefaults::DepthStencilStates::StencilClearValue;
	}
	ptr_type createClone()const { return new StencilClearState(*this); }
	ptr_type createDefault()const { return new StencilClearState(); }

	StencilClearState() { }
	StencilClearState(int32 clearStencil) : m_clearStencil(clearStencil) {}
	void setDefault(IGraphicsContext& device)
	{
		commitState(device, types::PipelineDefaults::DepthStencilStates::StencilClearValue);
	}
	void commitState(IGraphicsContext& device, int32 clearStencil);
	int32 m_clearStencil;
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the stencil test enable/disable.
*******************************************************************************************************************/
class StencilTestState : public impl::GraphicsPipelineImplState
{
public:
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilTest; }
	void set(IGraphicsContext& device) { commitState(device, m_stencilTest); }
	void unset(IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(IGraphicsContext& device) { m_stencilTest = types::PipelineDefaults::DepthStencilStates::StencilTestEnabled; }

	ptr_type createClone()const { return new StencilTestState(*this); }
	ptr_type createDefault()const { return new StencilTestState(); }

	StencilTestState() {}
	StencilTestState(bool enableTest) : m_stencilTest(enableTest) {}
	void setDefault(IGraphicsContext& device)
	{
		commitState(device, types::PipelineDefaults::DepthStencilStates::StencilTestEnabled);
	}
	void commitState(IGraphicsContext& device, bool flag);
	bool m_stencilTest;
};

class StencilCompareOpFront : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, m_cmpOp); }
	void unset(IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(IGraphicsContext& device) { m_cmpOp = types::ComparisonMode::DefaultStencilOpFront; }
	ptr_type createClone()const { return new StencilCompareOpFront(*this); }
	ptr_type createDefault()const { return new StencilCompareOpFront(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilOpFront; }

	StencilCompareOpFront(types::ComparisonMode cmp = types::ComparisonMode::DefaultStencilOpFront) : m_cmpOp(cmp) {}
	types::ComparisonMode m_cmpOp;

	void setDefault(IGraphicsContext& device) { commitState(device, types::ComparisonMode::DefaultStencilOpFront); }
	void commitState(IGraphicsContext& device, types::ComparisonMode cmp);
};

class StencilCompareOpBack : public impl::GraphicsPipelineImplState
{
public:
	types::ComparisonMode m_cmpOp;

	void set(IGraphicsContext& device) { commitState(device, m_cmpOp); }
	void unset(IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(IGraphicsContext& device) { m_cmpOp = types::ComparisonMode::DefaultStencilOpBack; }
	ptr_type createClone()const { return new StencilCompareOpBack(*this); }
	ptr_type createDefault()const { return new StencilCompareOpBack(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilOpBack; }

	StencilCompareOpBack(types::ComparisonMode cmp = types::ComparisonMode::DefaultStencilOpBack) : m_cmpOp(cmp) {}

	void setDefault(IGraphicsContext& device) { commitState(device, types::ComparisonMode::DefaultStencilOpBack); }
	void commitState(IGraphicsContext& device, types::ComparisonMode cmp);
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the front stencil op.
*******************************************************************************************************************/
class StencilOpFrontState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, m_opStencilFail, m_opDepthFail, m_opDepthPass); }
	void unset(IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(IGraphicsContext& device)
	{
		m_opDepthFail = types::StencilOp::DefaultDepthFailFront;
		m_opStencilFail = types::StencilOp::DefaultStencilFailFront;
		m_opDepthPass = types::StencilOp::DefaultDepthStencilPassFront;
	}
	ptr_type createClone()const { return new StencilOpFrontState(*this); }
	ptr_type createDefault()const { return new StencilOpFrontState(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilOpFront; }

	StencilOpFrontState() { }
	StencilOpFrontState(types::StencilOp opStencilFail, types::StencilOp opDepthFail, types::StencilOp opDepthPass) :
		m_opStencilFail(opStencilFail), m_opDepthPass(opDepthPass), m_opDepthFail(opDepthFail) {}
	types::StencilOp m_opStencilFail;
	types::StencilOp m_opDepthPass;
	types::StencilOp m_opDepthFail;
	void setDefault(IGraphicsContext& device)
	{
		commitState(device, types::StencilOp::DefaultDepthFailFront,
		            types::StencilOp::DefaultStencilFailFront,
		            types::StencilOp::DefaultDepthStencilPassFront);
	}
	void commitState(IGraphicsContext& device, types::StencilOp opStencilFail, types::StencilOp opDepthFail,
	                 types::StencilOp opDepthStencilPass);
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the back stencil op.
*******************************************************************************************************************/
class StencilOpBackState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, m_opStencilFail, m_opDepthFail, m_opDepthPass); }
	void unset(IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(IGraphicsContext& device)
	{
		m_opDepthFail = types::StencilOp::DefaultDepthFailBack;
		m_opStencilFail = types::StencilOp::DefaultStencilFailBack;
		m_opDepthPass = types::StencilOp::DefaultDepthStencilPassBack;
	}
	ptr_type createClone()const { return new StencilOpBackState(*this); }
	ptr_type createDefault()const { return new StencilOpBackState(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilOpBack; }

	StencilOpBackState() { }
	StencilOpBackState(types::StencilOp opStencilFail, types::StencilOp opDepthFail,
	                   types::StencilOp opDepthPass) :
		m_opStencilFail(opStencilFail), m_opDepthPass(opDepthPass), m_opDepthFail(opDepthFail) {}
	types::StencilOp m_opStencilFail;
	types::StencilOp m_opDepthPass;
	types::StencilOp m_opDepthFail;
	void setDefault(IGraphicsContext& device)
	{
		commitState(device, types::StencilOp::DefaultStencilFailBack, types::StencilOp::DefaultDepthFailBack,
		            types::StencilOp::DefaultDepthStencilPassBack);
	}

	void commitState(IGraphicsContext& device, types::StencilOp opStencilFail, types::StencilOp opDepthFail,
	                 types::StencilOp opDepthPass);
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the scissor test. Enable/ disable.
*******************************************************************************************************************/
class ScissorTestState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, m_scissorTest); }
	void unset(IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(IGraphicsContext& device) { setDefault(device); }
	impl::PipelineState::ptr_type createClone()const	{ return new ScissorTestState(*this); }
	impl::PipelineState::ptr_type createDefault()const
	{
		return new ScissorTestState(types::PipelineDefaults::ViewportScissor::ScissorTestEnabled);
	}
	bool operator==(const ScissorTestState& rhs) { return ((m_scissorTest == rhs.m_scissorTest)); }
	bool operator!=(const ScissorTestState& rhs) { return !(*this == rhs); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::ScissorTest; }

	ScissorTestState() : m_scissorTest(types::PipelineDefaults::ViewportScissor::ScissorTestEnabled) { }
	ScissorTestState(bool enable) : m_scissorTest(enable) {}
	void commitState(IGraphicsContext& device, bool enable);
	void setDefault(IGraphicsContext& device) { commitState(device, types::PipelineDefaults::ViewportScissor::ScissorTestEnabled); }
	bool m_scissorTest;
};

/*!*********************************************************************************************************************
\brief Pipeline graphics shader program state.
***********************************************************************************************************************/
class GraphicsShaderProgramState : public impl::GraphicsPipelineImplState
{
public:
	GraphicsShaderProgramState(const GraphicsShaderProgramState& shaderProgram);
	GraphicsShaderProgramState();

	/*!*********************************************************************************************************************
	\brief Bind this program state.
	***********************************************************************************************************************/
	void bind() const;

	/*!*********************************************************************************************************************
	\brief Set this program state.
	***********************************************************************************************************************/
	void set(IGraphicsContext& device) { bind(); }

	/*!*********************************************************************************************************************
	\brief Reset this program state.
	***********************************************************************************************************************/
	void reset(IGraphicsContext& device);

	/*!*********************************************************************************************************************
	\brief Unset this program state.
	***********************************************************************************************************************/
	void unset(IGraphicsContext& device)
	{
		if (m_parent)
		{
			m_parent->set(device);
		}
		else { reset(device); }
	}

	/*!*********************************************************************************************************************
	\brief Return default program state.
	***********************************************************************************************************************/
	PipelineState::ptr_type createDefault()const { return new GraphicsShaderProgramState(); }

	/*!*********************************************************************************************************************
	\brief Return clone program state.
	***********************************************************************************************************************/
	PipelineState::ptr_type createClone()const { return new GraphicsShaderProgramState(*this); }

	/*!*********************************************************************************************************************
	\brief Get thi state type.
	***********************************************************************************************************************/
	impl::GraphicsStateType getStateType() const { return impl::GraphicsStateType::ShaderProgram; }

	bool operator==(const GraphicsShaderProgramState& rhs)const { return (m_shaderProgram == rhs.m_shaderProgram); }
	bool operator!=(const GraphicsShaderProgramState& rhs)const { return !(*this == rhs); }
	void generate();

	/*!*********************************************************************************************************************
	\brief Destroy this.
	***********************************************************************************************************************/
	void destroy();

	/*!************************************************************************************************************
	\brief	Return the api program object.
	***************************************************************************************************************/
	native::HPipeline_& getNativeObject() const { return *m_shaderProgram; }

	void getUniformsLocation(const char8** uniforms, uint32 numUniforms, std::vector<int32>& outLocation);
	int32 getUniformLocation(const char8* uniform);

	/*!************************************************************************
	\brief Save the program binary to a file or other stream.
	\param outputStream Output stream. Must be writable
	\return True if it is successful
	**************************************************************************/
	bool saveProgramBinary(Stream& outputStream);

private:
	mutable native::HPipeline m_shaderProgram;
	void setDefault(IGraphicsContext& device) { }
};

/*!*********************************************************************************************************************
\brief ComputePipeline shader program state.
***********************************************************************************************************************/
class ComputeShaderProgramState : public impl::ComputePipelineImplState
{
public:

	/*!*********************************************************************************************************************
	\brief
	***********************************************************************************************************************/
	ComputeShaderProgramState() { m_isValid = false; }

	/*!*********************************************************************************************************************
	\brief
	***********************************************************************************************************************/
	ComputeShaderProgramState(const ComputeShaderProgramState& shaderProgram)
		: m_shaderProgram(shaderProgram.m_shaderProgram)
	{
		m_isValid = true;
	}

	/*!*********************************************************************************************************************
	\brief Bind this program state.
	***********************************************************************************************************************/
	void bind();

	/*!*********************************************************************************************************************
	\brief Set this program state.
	***********************************************************************************************************************/
	void set(IGraphicsContext& device) { bind(); }

	/*!*********************************************************************************************************************
	\brief Reset this program state.
	***********************************************************************************************************************/
	void reset(IGraphicsContext& device);

	/*!*********************************************************************************************************************
	\brief Unset this program state.
	***********************************************************************************************************************/
	void unset(IGraphicsContext& device)
	{
		if (m_parent)
		{
			m_parent->set(device);
		}
		else { reset(device); }
	}

	/*!*********************************************************************************************************************
	\brief Return native handle.
	***********************************************************************************************************************/
	native::HPipeline_& getNativeObject() { return *m_shaderProgram; }

	/*!*********************************************************************************************************************
	\brief Return a default prgram state.
	***********************************************************************************************************************/
	PipelineState::ptr_type createDefault()const { return new ComputeShaderProgramState(); }

	/*!*********************************************************************************************************************
	\brief Return clone of this program state.
	***********************************************************************************************************************/
	PipelineState::ptr_type createClone()const { return new ComputeShaderProgramState(*this); }

	/*!*********************************************************************************************************************
	\brief Return this state type.
	***********************************************************************************************************************/
	impl::GraphicsStateType getStateType() const { return impl::GraphicsStateType::ShaderProgram; }

	bool operator==(const ComputeShaderProgramState& rhs)const { return (m_shaderProgram == rhs.m_shaderProgram); }
	bool operator!=(const ComputeShaderProgramState& rhs)const { return !(*this == rhs); }
	void generate();
	void destroy();

	/*!************************************************************************************************************
	\brief	Return the api program object.
	***************************************************************************************************************/
	const native::HPipeline& get() const { return m_shaderProgram; }

	/*!************************************************************************************************************
	\brief	Return the api program object.
	***************************************************************************************************************/
	native::HPipeline& get() { return m_shaderProgram; }

	/*!************************************************************************************************************
	\brief If free standing uniforms are supported by the underlying API (ApiCapability::Uniforms), get an array
	of the locations of several shader uniform variables for use with setUniform/setUniformPtr.
	If a uniform is inactive, its location is set to -1.
	\param uniforms An array of uniform variable names
	\param numUniforms The number of uniforms in the array
	\param outLocation A vector where the locations will be saved on a 1-1 mapping with uniforms.
	***************************************************************************************************************/
	void getUniformsLocation(const char8** uniforms, uint32 numUniforms, std::vector<int32>& outLocation);

	/*!************************************************************************************************************
	\brief If free standing uniforms are supported by the underlying API (ApiCapability::Uniforms), get the location
	of a shader uniform variable for use with setUniform/setUniformPtr.
	If a uniform is inactive, -1 is returned.
	\param uniform The name of a uniform variable
	\return outLocation
	***************************************************************************************************************/
	int32 getUniformLocation(const char8* uniform);
private:
	native::HPipeline m_shaderProgram;
	void setDefault(IGraphicsContext& device) { }
};

}
}
}
//!\endcond
