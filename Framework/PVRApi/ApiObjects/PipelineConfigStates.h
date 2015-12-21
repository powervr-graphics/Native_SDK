/*!*********************************************************************************************************************
\file         PVRApi\ApiObjects\PipelineConfigStates.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Internal use. Contains objects required by the GraphicsPipeline and ComputePipeline. These are the objects that
              do the actual work and  execute the underlying API commands in their "set", "unset", "reset" functions.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRApi/ApiObjects/PipelineState.h"
#include "PVRApi/Bindables.h"

namespace pvr {
namespace api {
namespace pipelineCreation {
//	Forward Declarations
struct DepthStencilStateCreateParam;
struct RasterStateCreateParam;
struct ColorBlendStateCreateParam;
struct ViewportStateCreateParam;
struct StencilStateCreateParam;
}

namespace impl {


class DepthTestState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::DepthStencilStateCreateParam;
	friend class api::impl::CommandBufferImpl;
public:
	PipelineState::ptr_type createDefault()const { return new DepthTestState(); }
	PipelineState::ptr_type createClone()const
	{
		return new DepthTestState(m_depthTestEnabled);
	}
	GraphicsStateType::Enum getStateType() const { return GraphicsStateType::DepthTest; }
	bool operator==(const DepthTestState& rhs)const	{ return (m_depthTestEnabled == rhs.m_depthTestEnabled); }
	bool operator!=(const DepthTestState& rhs)const { return !(*this == rhs); }
private:
	void set(pvr::IGraphicsContext& device) { commitState(device, m_depthTestEnabled); }
	void reset(pvr::IGraphicsContext& device) { m_depthTestEnabled = false; }
	void unset(pvr::IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	DepthTestState(bool enable = false) { m_depthTestEnabled = enable; }
	void commitState(pvr::IGraphicsContext& device, bool depthTest);
	void setDefault(pvr::IGraphicsContext& device) {	commitState(device, false);	}
	bool m_depthTestEnabled;
};

class DepthFuncState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::DepthStencilStateCreateParam;
public:
	bool operator==(const DepthFuncState& rhs)const { return m_depthFunc == rhs.m_depthFunc; }
	bool operator!=(const DepthFuncState& rhs)const { return !(*this == rhs); }

	void set(pvr::IGraphicsContext& device) { commitState(device, m_depthFunc); }
	void unset(pvr::IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); }
		else { setDefault(device); }
	}
	void reset(pvr::IGraphicsContext& device) { m_depthFunc = ComparisonMode::Less; }
	PipelineState::ptr_type createClone()const { return new DepthFuncState(m_depthFunc); }
	PipelineState::ptr_type createDefault()const { return new DepthFuncState(); }
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::DepthFunc; }
private:
	DepthFuncState() { m_depthFunc = ComparisonMode::Less; }
	DepthFuncState(ComparisonMode::Enum depthFunc) { m_depthFunc = depthFunc; }
	void setDefault(pvr::IGraphicsContext& device);
	void commitState(pvr::IGraphicsContext& device, ComparisonMode::Enum func);
	ComparisonMode::Enum m_depthFunc;
};

class DepthWriteState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::DepthStencilStateCreateParam;
public:
	void set(pvr::IGraphicsContext& device) { commitState(device, m_depthWriteEnabled); }
	void unset(pvr::IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); }
		else { setDefault(device); }
	}
	void reset(pvr::IGraphicsContext& device) { m_depthWriteEnabled = true; }
	bool operator==(const DepthWriteState& rhs)const { return m_depthWriteEnabled == rhs.m_depthWriteEnabled; }
	GraphicsStateType::Enum getStateType() const { return GraphicsStateType::DepthWrite; }
	bool operator!=(const DepthWriteState& rhs)const { return m_depthWriteEnabled != rhs.m_depthWriteEnabled; }
	PipelineState::ptr_type createClone()const { return new DepthWriteState(m_depthWriteEnabled); }
	PipelineState::ptr_type createDefault()const { return new DepthWriteState(); }
private:
	DepthWriteState() { m_depthWriteEnabled = true; }
	DepthWriteState(bool enabled) { m_depthWriteEnabled = enabled; }
	void setDefault(pvr::IGraphicsContext& device);
	void commitState(pvr::IGraphicsContext& device, bool depthWrite);
	bool m_depthWriteEnabled;
};

/*!****************************************************************************************************************
\brief Pipeline. Controls the polygon culling state.
*******************************************************************************************************************/
class PolygonFrontFaceState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::RasterStateCreateParam;
public:
	void set(pvr::IGraphicsContext& device) { commitState(device, m_cullFace); }
	void reset(pvr::IGraphicsContext& device) { m_cullFace = Face::Back; }
	void unset(pvr::IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); }
		else { setDefault(device); }
	}
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::PolygonCulling; }
	PipelineState::ptr_type createClone()const { return new PolygonFrontFaceState(*this); }
	PipelineState::ptr_type createDefault()const { return new PolygonFrontFaceState(); }
	bool operator==(const PolygonFrontFaceState& rhs)const { return (m_cullFace == rhs.m_cullFace); }
	bool operator!=(const PolygonFrontFaceState& rhs)const { return !(*this == rhs); }
private:
	PolygonFrontFaceState() : m_cullFace(pvr::api::Face::Back) {}
	PolygonFrontFaceState(Face::Enum cullFace) : m_cullFace(cullFace) {}
	void commitState(pvr::IGraphicsContext& device, Face::Enum cullFace);
	void setDefault(pvr::IGraphicsContext& device);
	Face::Enum m_cullFace;
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the polygon winding-order.
*******************************************************************************************************************/
class PolygonWindingOrderState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::RasterStateCreateParam;
public:
	void set(pvr::IGraphicsContext& device) { commitState(device, m_windingOrder); }
	void unset(pvr::IGraphicsContext& device)
	{
		if (m_parent) {	m_parent->set(device); }
		else { setDefault(device); }
	}
	void reset(pvr::IGraphicsContext& device) { m_windingOrder = PolygonWindingOrder::FrontFaceCCW; }
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::PolygonWindingOrder; }
	ptr_type createDefault()const { return new PolygonWindingOrderState(); }
	ptr_type createClone()const { return new PolygonWindingOrderState(*this); }
private:
	PolygonWindingOrderState() {}
	PolygonWindingOrderState(PolygonWindingOrder::Enum windingOrder) : m_windingOrder(windingOrder) {}
	void setDefault(pvr::IGraphicsContext& device);
	void commitState(pvr::IGraphicsContext& device, PolygonWindingOrder::Enum windingOrderCCW);
	PolygonWindingOrder::Enum m_windingOrder;
};



//
//class PolygonFillState : public GraphicsPipelineImplState
//{
//public:
//
//private:
//
//};

/*!*********************************************************************************************************************
\brief  Sets the Color write mask.
***********************************************************************************************************************/
class ColorWriteMask : public GraphicsPipelineImplState
{
	glm::bvec4 writeMask;
	void execute(impl::CommandBufferImpl& cmdBuff);
public:
	bool operator==(const ColorWriteMask& rhs)const { return writeMask == rhs.writeMask; }
	bool operator!=(const ColorWriteMask& rhs)const { return !(*this == rhs); }
	ptr_type createClone()const { return new ColorWriteMask(*this); }
	ptr_type createDefault()const { return new ColorWriteMask(); }
	void set(pvr::IGraphicsContext& device) { commitState(device, writeMask); }
	void unset(pvr::IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); return; }
		setDefault(device);
	}
	void setDefault(pvr::IGraphicsContext& device) { commitState(device, glm::bvec4(true)); }
	void reset(pvr::IGraphicsContext& device) {}
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::BlendTest; }
	void commitState(pvr::IGraphicsContext& device, const glm::bvec4 mask);
	/*!******************************************************************************
	\brief	Mask rgba channels, enable write if true.
	\param	r Red channel
	\param	g Green channel
	\param	b Blue channel
	\param	a Alpha channel
	********************************************************************************/
	ColorWriteMask(bool r, bool g, bool b, bool a)
	{
		writeMask[0] = r, writeMask[1] = g, writeMask[2] = b, writeMask[3] = a;
	}

	/*!******************************************************************************
	\brief	Mask rgba channels, enable write if true.
	\param	maskRGBA Mask all channels to the same value.
	********************************************************************************/
	ColorWriteMask(bool maskRGBA = true)
	{
		writeMask = glm::bvec4(maskRGBA);
	}

	ColorWriteMask(ColorChannel::Bits channelBits) :
		writeMask(!!(channelBits & ColorChannel::R),
		          !!(channelBits & ColorChannel::G),
		          !!(channelBits & ColorChannel::B),
		          !!(channelBits & ColorChannel::A))
	{
	}
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the Blending enable/disable.
*******************************************************************************************************************/
class BlendingEnableState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::ColorBlendStateCreateParam;
public:
	bool operator==(const BlendingEnableState& rhs)const { return m_blendTestEnabled == rhs.m_blendTestEnabled; }
	bool operator!=(const BlendingEnableState& rhs)const { return !(*this == rhs); }
	ptr_type createClone()const { return new BlendingEnableState(*this); }
	ptr_type createDefault()const { return new BlendingEnableState(); }
	void set(pvr::IGraphicsContext& device) { commitState(device, m_blendTestEnabled); }
	void unset(pvr::IGraphicsContext& device)
	{
		if (m_parent) { m_parent->set(device); return; }
		commitState(device, !m_blendTestEnabled);
	}
	void reset(pvr::IGraphicsContext& device) { m_blendTestEnabled = false; }
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::BlendTest; }
private:
	BlendingEnableState() { }
	BlendingEnableState(bool enable) : m_blendTestEnabled(enable) { }
	bool m_blendTestEnabled;
	void setDefault(pvr::IGraphicsContext& device);
	void commitState(pvr::IGraphicsContext&, bool blendTest);
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the Blend Operation.
*******************************************************************************************************************/
class BlendFactorState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::ColorBlendStateCreateParam;
public:
	void set(pvr::IGraphicsContext& device)
	{
		commitState(device, (m_pack & 0xF000) >> 12, (m_pack & 0x0F00) >> 8, (m_pack & 0x00F0) >> 4, m_pack & 0x000F);
	}
	void reset(pvr::IGraphicsContext& device)
	{
		packData(BlendFactor::One, BlendFactor::Zero, BlendFactor::One, BlendFactor::Zero);
		setDefault(device);
	}
	void unset(pvr::IGraphicsContext& device)
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
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::BlendRgba; }
	PipelineState::ptr_type createClone()const { return new BlendFactorState(m_pack); }
	PipelineState::ptr_type createDefault()const { return new BlendFactorState(); }
	bool operator==(const BlendFactorState& rhs)const { return m_pack == rhs.m_pack; }
	bool operator!=(const BlendFactorState& rhs)const { return m_pack != rhs.m_pack; }
private:
	BlendFactorState() {}
	BlendFactorState(BlendFactor::Enum srcRgbFactor, BlendFactor::Enum dstRgbFactor, BlendFactor::Enum srcAlphaFactor,
	                 BlendFactor::Enum dstAlphaFactor);
	BlendFactorState(uint32 data) : m_pack((uint16)data) {}
	void commitState(pvr::IGraphicsContext& device, uint8 srcRgbFactor, uint8 dstRgbFactor, uint8 srcAlphaFactor,
	                 uint8 dstAlphaFactor);
	void setDefault(pvr::IGraphicsContext& device);
	void packData(uint8 srcRgbFactor, uint8 dstRgbFactor, uint8 srcAlphaFactor, uint8 dstAlphaFactor)
	{
		m_pack = (srcRgbFactor << 12);
		m_pack |= (dstRgbFactor << 8);
		m_pack |= (srcAlphaFactor << 4);
		m_pack |= dstAlphaFactor;
	}
	uint16 m_pack;
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the Blend equation.
*******************************************************************************************************************/
class BlendOpState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::ColorBlendStateCreateParam;
public:
	bool operator==(const BlendOpState& rhs)const
	{
		return (m_rgbBlendEq == rhs.m_rgbBlendEq) && (m_alphaBlendEq == rhs.m_alphaBlendEq);
	}
	bool operator!=(const BlendOpState& rhs)const { return !(*this == rhs); }
	void set(pvr::IGraphicsContext& device) { commitState(device, m_rgbBlendEq, m_alphaBlendEq); };
	void reset(pvr::IGraphicsContext& device) { m_rgbBlendEq = m_alphaBlendEq = BlendOp::Add; }
	void unset(pvr::IGraphicsContext& device)
	{
		if (m_parent)
		{
			m_parent->set(device);
			return;
		}
		setDefault(device);
	}
	PipelineState::ptr_type createClone()const { return new BlendOpState(m_rgbBlendEq, m_alphaBlendEq); }
	PipelineState::ptr_type createDefault()const { return new BlendOpState(); }
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::BlendEq; }
private:
	BlendOpState() { }
	BlendOpState(BlendOp::Enum rgbBlendEquation, BlendOp::Enum alphaBlendEquation) : m_rgbBlendEq(rgbBlendEquation),
		m_alphaBlendEq(alphaBlendEquation) {}

	void commitState(pvr::IGraphicsContext& device, BlendOp::Enum rgbBlendEquation, BlendOp::Enum alphaBlendEquation);
	void setDefault(pvr::IGraphicsContext& device)
	{
		commitState(device, pvr::api::BlendOp::Add, pvr::api::BlendOp::Add);
	}

	BlendOp::Enum m_rgbBlendEq;
	BlendOp::Enum m_alphaBlendEq;
};

/*!****************************************************************************************************************
\brief  Pipeline state. Controls the depth clear value.
*******************************************************************************************************************/
class DepthClearState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::DepthStencilStateCreateParam;
public:
	void set(pvr::IGraphicsContext& device) { commitState(m_clearDepth); }
	void reset(pvr::IGraphicsContext& device) { m_clearDepth = 1.f; }
	void unset(pvr::IGraphicsContext& device)
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
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::DepthClear; }
	PipelineState::ptr_type createClone()const	{ return new DepthClearState(*this); }
	PipelineState::ptr_type createDefault()const { return new DepthClearState(); }
	bool operator==(const DepthClearState& rhs) { return (m_clearDepth == rhs.m_clearDepth); }
	bool operator!=(const DepthClearState& rhs) { return !(*this == rhs); }

private:
	DepthClearState() { }

	DepthClearState(float32 depth) { m_clearDepth = depth; }
	void commitState(float32 depth);
	void setDefault(pvr::IGraphicsContext& device);
	float32 m_clearDepth;
};

/*!****************************************************************************************************************
\brief  Pipeline State. Controls the stencil clear value.
*******************************************************************************************************************/
class StencilClearState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::DepthStencilStateCreateParam;
public:
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::StencilClear; }
	void set(pvr::IGraphicsContext& device) { commitState(device, m_clearStencil); }
	void unset(pvr::IGraphicsContext& device) { if (m_parent) { m_parent->set(device); return; } setDefault(device); }
	void reset(pvr::IGraphicsContext& device) { m_clearStencil = 0; }
	ptr_type createClone()const { return new StencilClearState(*this); }
	ptr_type createDefault()const { return new StencilClearState(); }
private:
	StencilClearState() { }
	StencilClearState(int32 clearStencil) : m_clearStencil(clearStencil) {}
	void setDefault(pvr::IGraphicsContext& device);
	void commitState(pvr::IGraphicsContext& device, int32 clearStencil);
	int32 m_clearStencil;
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the stencil test enable/disable.
*******************************************************************************************************************/
class StencilTestState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::DepthStencilStateCreateParam;
public:
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::StencilTest; }
	void set(pvr::IGraphicsContext& device) { commitState(device, m_stencilTest); }
	void unset(pvr::IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(pvr::IGraphicsContext& device) { m_stencilTest = false; }

	ptr_type createClone()const { return new StencilTestState(*this); }
	ptr_type createDefault()const { return new StencilTestState(); }
private:
	StencilTestState() {}
	StencilTestState(bool enableTest) : m_stencilTest(enableTest) {}
	void setDefault(pvr::IGraphicsContext& device);
	void commitState(pvr::IGraphicsContext& device, bool flag);
	bool m_stencilTest;
};


class StencilCompareOpFront : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::DepthStencilStateCreateParam;
public:
	void set(pvr::IGraphicsContext& device) { commitState(device, m_cmpOp); }
	void unset(pvr::IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(pvr::IGraphicsContext& device) { m_cmpOp = ComparisonMode::Always; }
	ptr_type createClone()const { return new StencilCompareOpFront(*this); }
	ptr_type createDefault()const { return new StencilCompareOpFront(); }
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::StencilOpBack; }
private:
	StencilCompareOpFront(pvr::ComparisonMode::Enum cmp = ComparisonMode::DefaultStencilOpFront) : m_cmpOp(cmp) {}
	ComparisonMode::Enum m_cmpOp;

	void setDefault(pvr::IGraphicsContext& device) { commitState(device, ComparisonMode::DefaultStencilOpFront); }
	void commitState(pvr::IGraphicsContext& device, pvr::ComparisonMode::Enum cmp);
};

class StencilCompareOpBack : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::DepthStencilStateCreateParam;
public:
	void set(pvr::IGraphicsContext& device) { commitState(device, m_cmpOp); }
	void unset(pvr::IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(pvr::IGraphicsContext& device) { m_cmpOp = ComparisonMode::Always; }
	ptr_type createClone()const { return new StencilCompareOpBack(*this); }
	ptr_type createDefault()const { return new StencilCompareOpBack(); }
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::StencilOpBack; }
private:
	StencilCompareOpBack(pvr::ComparisonMode::Enum cmp = ComparisonMode::DefaultStencilOpBack) : m_cmpOp(cmp) {}
	ComparisonMode::Enum m_cmpOp;

	void setDefault(pvr::IGraphicsContext& device) { commitState(device, ComparisonMode::DefaultStencilOpBack); }
	void commitState(pvr::IGraphicsContext& device, pvr::ComparisonMode::Enum cmp);
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the front stencil op.
*******************************************************************************************************************/
class StencilOpFrontState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::DepthStencilStateCreateParam;
public:
	void set(pvr::IGraphicsContext& device) { commitState(device, m_opStencilFail, m_opDepthFail, m_opDepthPass); }
	void unset(pvr::IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(pvr::IGraphicsContext& device) { m_opDepthFail = m_opStencilFail = m_opDepthPass = StencilOp::Keep; }
	ptr_type createClone()const { return new StencilOpFrontState(*this); }
	ptr_type createDefault()const { return new StencilOpFrontState(); }
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::StencilOpFront; }
private:
	StencilOpFrontState() { }
	StencilOpFrontState(StencilOp::Enum opStencilFail, StencilOp::Enum opDepthFail, StencilOp::Enum opDepthPass) :
		m_opStencilFail(opStencilFail), m_opDepthPass(opDepthPass), m_opDepthFail(opDepthFail) {}
	StencilOp::Enum m_opStencilFail;
	StencilOp::Enum m_opDepthPass;
	StencilOp::Enum m_opDepthFail;
	void setDefault(pvr::IGraphicsContext& device);
	void commitState(pvr::IGraphicsContext& device, StencilOp::Enum opStencilFail, StencilOp::Enum opDepthFail,
	                 StencilOp::Enum opDepthStencilPass);
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the back stencil op.
*******************************************************************************************************************/
class StencilOpBackState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::DepthStencilStateCreateParam;
public:
	void set(pvr::IGraphicsContext& device) { commitState(device, m_opStencilFail, m_opDepthFail, m_opDepthPass); }
	void unset(pvr::IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(pvr::IGraphicsContext& device) { m_opDepthFail = m_opStencilFail = m_opDepthPass = StencilOp::Keep; }
	ptr_type createClone()const { return new StencilOpBackState(*this); }
	ptr_type createDefault()const { return new StencilOpBackState(); }
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::StencilOpBack; }
private:
	StencilOpBackState() { }
	StencilOpBackState(StencilOp::Enum opStencilFail, StencilOp::Enum opDepthFail, StencilOp::Enum opDepthPass) :
		m_opStencilFail(opStencilFail), m_opDepthPass(opDepthPass), m_opDepthFail(opDepthFail) {}
	StencilOp::Enum m_opStencilFail;
	StencilOp::Enum m_opDepthPass;
	StencilOp::Enum m_opDepthFail;
	void setDefault(pvr::IGraphicsContext& device);
	void commitState(pvr::IGraphicsContext& device, StencilOp::Enum opStencilFail, StencilOp::Enum opDepthFail,
	                 StencilOp::Enum opDepthPass);
};

/*!****************************************************************************************************************
\brief  Pipeline. Controls the scissor test. Enable/ disable.
*******************************************************************************************************************/
class ScissorTestState : public GraphicsPipelineImplState
{
	friend struct api::pipelineCreation::ViewportStateCreateParam;
public:
	void set(pvr::IGraphicsContext& device) { commitState(device, m_scissorTest); }
	void unset(pvr::IGraphicsContext& device) { if (m_parent) { m_parent->set(device); } else { setDefault(device); } }
	void reset(pvr::IGraphicsContext& device) { setDefault(device); }
	PipelineState::ptr_type createClone()const	{ return new ScissorTestState(*this); }
	PipelineState::ptr_type createDefault()const { return new ScissorTestState(false); }
	bool operator==(const ScissorTestState& rhs) { return ((m_scissorTest == rhs.m_scissorTest)); }
	bool operator!=(const ScissorTestState& rhs) { return !(*this == rhs); }
	GraphicsStateType::Enum getStateType()const { return GraphicsStateType::ScissorTest; }
private:
	ScissorTestState() : m_scissorTest(false) { }
	ScissorTestState(bool enable) : m_scissorTest(enable) {}
	void commitState(pvr::IGraphicsContext& device, bool enable);
	void setDefault(pvr::IGraphicsContext& device);
	bool m_scissorTest;
};

class MultisampleState : public GraphicsPipelineImplState
{
public:
	/*MultisampleState() { setDefault(true); }
	enum { MultiSample = 0x01, AlphaToCoverage = 0x02, AlphaToOne = 0x04 };*/

};

}
}
}
//!\endcond