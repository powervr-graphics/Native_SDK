/*!
\brief Internal use. Contains objects required by the OpenGL ES versions of GraphicsPipeline and ComputePipeline. These
are the objects that the actual work and execute the underlying API commands in their "set", "unset", "reset"
functions.
\file PVRApi/OGLES/PipelineConfigStatesGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/PipelineConfig.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/Bindables.h"

namespace pvr {
namespace api {
namespace gles { class GraphicsPipelineImplGles; }
namespace impl {

enum class GraphicsStateType
{
	ShaderProgram, VertexShader, FragmentShader, GeometryShader, TessellationControlShader,
	TessellationEvaluationShader, DepthTest, DepthClear, DepthWrite, DepthBias,
	PolygonCulling, PolygonWindingOrder, BlendRgba, BlendTest, PolygonFill,
	ScissorTest, StencilOpFront, StencilOpBack, FrameBufferClear,
	FrameBufferWrite, DepthFunc, BlendEq, StencilTest, StencilClear,
	VertexAttributeFormatState, VertexAttributeLocation, Count
};

class GraphicsPipeline_;
class GraphicsPipelineImplState;

/// <summary>Base interface for a pipeline state object.</summary>
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
	PipelineState() : _parent(NULL) { }

	/// <summary>Destructor.</summary>
	virtual ~PipelineState() {}

	/// <summary>Set this state.</summary>
	virtual void set(IGraphicsContext& device) = 0;

	/// <summary>Type of this state.</summary>
	/// <returns>void</returns>
	virtual GraphicsStateType getStateType() const = 0;

	/// <summary>Check is a valid state.</summary>
	/// <returns>bool</returns>
	virtual bool isValid() { return _isValid; }
protected:
	PipelineState*  _parent;
	bool      _isValid;
};

/// <summary>Base class for graphics pipeline state.</summary>
class GraphicsPipelineImplState : public PipelineState
{
public:
	virtual ~GraphicsPipelineImplState() {}
protected:
};

/// <summary>Base class for compute pipeline states.</summary>
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

	/// <summary>Constructor</summary>
	/// <param name="enable">Enable/ disable depthtest (Default: false)</param>
	DepthTestState(bool enable = types::PipelineDefaults::DepthStencilStates::DepthTestEnabled) { _depthTestEnabled = enable; }

	/// <summary>Create a new defualt DepthTestState</summary>
	impl::PipelineState::ptr_type createDefault() const { return new DepthTestState(); }

	/// <summary>Create a new clone of this state</summary>
	impl::PipelineState::ptr_type createClone() const
	{
		return new DepthTestState(_depthTestEnabled);
	}

	/// <summary>Get this state type</summary>
	impl::GraphicsStateType getStateType() const { return impl::GraphicsStateType::DepthTest; }

	/// <summary>operator ==</summary>
	/// <param name="rhs"></param>
	/// <returns>Return true if equal</returns>
	bool operator==(const DepthTestState& rhs)const { return (_depthTestEnabled == rhs._depthTestEnabled); }

	/// <summary>operator !=</summary>
	/// <param name="rhs"></param>
	/// <returns>Return true if not equal</returns>
	bool operator!=(const DepthTestState& rhs)const { return !(*this == rhs); }

	/// <summary>commit this state to the gpu</summary>
	/// <param name="device"></param>
	void set(IGraphicsContext& device) { commitState(device, _depthTestEnabled); }

	/// <summary>Commit this state to the gpu</summary>
	/// <param name="device"></param>
	/// <param name="depthTest">Enable/disable depth test</param>
	void commitState(IGraphicsContext& device, bool depthTest);

	bool _depthTestEnabled;
};

class DepthFuncState : public impl::GraphicsPipelineImplState
{
public:
	bool operator==(const DepthFuncState& rhs)const { return _depthFunc == rhs._depthFunc; }
	bool operator!=(const DepthFuncState& rhs)const { return !(*this == rhs); }

	void set(IGraphicsContext& device) { commitState(device, _depthFunc); }
	impl::PipelineState::ptr_type createClone()const { return new DepthFuncState(_depthFunc); }
	impl::PipelineState::ptr_type createDefault()const { return new DepthFuncState(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::DepthFunc; }
	DepthFuncState() { _depthFunc = types::ComparisonMode::DefaultDepthFunc; }
	DepthFuncState(types::ComparisonMode depthFunc) { _depthFunc = depthFunc; }
	void commitState(IGraphicsContext& device, types::ComparisonMode func);
	types::ComparisonMode _depthFunc;
};

class DepthWriteState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, _depthWriteEnabled); }
	bool operator==(const DepthWriteState& rhs)const { return _depthWriteEnabled == rhs._depthWriteEnabled; }
	impl::GraphicsStateType getStateType() const { return impl::GraphicsStateType::DepthWrite; }
	bool operator!=(const DepthWriteState& rhs)const { return _depthWriteEnabled != rhs._depthWriteEnabled; }
	impl::PipelineState::ptr_type createClone()const { return new DepthWriteState(_depthWriteEnabled); }
	impl::PipelineState::ptr_type createDefault()const { return new DepthWriteState(); }
	DepthWriteState() { _depthWriteEnabled = types::PipelineDefaults::DepthStencilStates::DepthWriteEnabled; }
	DepthWriteState(bool enabled) { _depthWriteEnabled = enabled; }
	void commitState(IGraphicsContext& device, bool depthWrite);
	bool _depthWriteEnabled;
};

/// <summary>Pipeline. Controls the polygon culling state.</summary>
class PolygonFrontFaceState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, _cullFace); }

	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::PolygonCulling; }
	impl::PipelineState::ptr_type createClone()const { return new PolygonFrontFaceState(*this); }
	impl::PipelineState::ptr_type createDefault()const { return new PolygonFrontFaceState(); }
	bool operator==(const PolygonFrontFaceState& rhs)const { return (_cullFace == rhs._cullFace); }
	bool operator!=(const PolygonFrontFaceState& rhs)const { return !(*this == rhs); }
	PolygonFrontFaceState() : _cullFace(types::Face::Default) {}
	PolygonFrontFaceState(types::Face cullFace) : _cullFace(cullFace) {}

	void commitState(IGraphicsContext& device, types::Face cullFace);
	types::Face _cullFace;
};

/// <summary>Pipeline. Controls the polygon winding-order.</summary>
class PolygonWindingOrderState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, _windingOrder); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::PolygonWindingOrder; }
	ptr_type createDefault()const { return new PolygonWindingOrderState(); }
	ptr_type createClone()const { return new PolygonWindingOrderState(*this); }
	PolygonWindingOrderState() {}
	PolygonWindingOrderState(types::PolygonWindingOrder windingOrder) : _windingOrder(windingOrder) {}
	void commitState(IGraphicsContext& device, types::PolygonWindingOrder windingOrderCCW);
	types::PolygonWindingOrder _windingOrder;
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
		return new TessPatchControlPoints(types::PipelineDefaults::Tesselation::NumControlPoints);
	}
	void set(IGraphicsContext& device) { commitState(device, patchControlPoints); }
	impl::GraphicsStateType getStateType()const
	{
		return impl::GraphicsStateType::TessellationControlShader;
	}
	void commitState(IGraphicsContext& device, uint32 patchControlPoints);
};


/// <summary>Sets the Color write mask.</summary>
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

/// <summary>Pipeline. Controls the Blending enable/disable.</summary>
class BlendingEnableState : public impl::GraphicsPipelineImplState
{
public:
	bool operator==(const BlendingEnableState& rhs)const { return _blendTestEnabled == rhs._blendTestEnabled; }
	bool operator!=(const BlendingEnableState& rhs)const { return !(*this == rhs); }
	ptr_type createClone()const { return new BlendingEnableState(*this); }
	ptr_type createDefault()const { return new BlendingEnableState(); }
	void set(IGraphicsContext& device) { commitState(device, _blendTestEnabled); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::BlendTest; }
	BlendingEnableState() { }
	BlendingEnableState(bool enable) : _blendTestEnabled(enable) { }
	bool _blendTestEnabled;
	void commitState(IGraphicsContext&, bool blendTest);
};

/// <summary>Pipeline. Controls the Blend Operation.</summary>
class BlendFactorState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device)
	{
		commitState(device, types::BlendFactor((_pack & 0xF000) >> 12), types::BlendFactor((_pack & 0x0F00) >> 8), types::BlendFactor((_pack & 0x00F0) >> 4), types::BlendFactor(_pack & 0x000F));
	}


	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::BlendRgba; }
	impl::PipelineState::ptr_type createClone()const { return new BlendFactorState(_pack); }
	impl::PipelineState::ptr_type createDefault()const { return new BlendFactorState(); }
	bool operator==(const BlendFactorState& rhs)const { return _pack == rhs._pack; }
	bool operator!=(const BlendFactorState& rhs)const { return _pack != rhs._pack; }

	BlendFactorState() {}
	BlendFactorState(types::BlendFactor srcRgbFactor, types::BlendFactor dstRgbFactor,
	                 types::BlendFactor srcAlphaFactor,
	                 types::BlendFactor dstAlphaFactor);
	BlendFactorState(uint32 data) : _pack((uint16)data) {}
	void commitState(IGraphicsContext& device, types::BlendFactor srcRgbFactor, types::BlendFactor dstRgbFactor, types::BlendFactor srcAlphaFactor,
	                 types::BlendFactor dstAlphaFactor);

	void packData(types::BlendFactor srcRgbFactor, types::BlendFactor dstRgbFactor, types::BlendFactor srcAlphaFactor, types::BlendFactor dstAlphaFactor)
	{
		_pack = ((uint16)srcRgbFactor << 12);
		_pack |= ((uint16)dstRgbFactor << 8);
		_pack |= ((uint16)srcAlphaFactor << 4);
		_pack |= (uint16)dstAlphaFactor;
	}
	uint16 _pack;
};

/// <summary>Pipeline. Controls the Blend equation.</summary>
class BlendOpState : public impl::GraphicsPipelineImplState
{
public:
	bool operator==(const BlendOpState& rhs)const
	{
		return (_rgbBlendEq == rhs._rgbBlendEq) && (_alphaBlendEq == rhs._alphaBlendEq);
	}
	bool operator!=(const BlendOpState& rhs)const { return !(*this == rhs); }
	void set(IGraphicsContext& device) { commitState(device, _rgbBlendEq, _alphaBlendEq); }
	impl::PipelineState::ptr_type createClone()const { return new BlendOpState(_rgbBlendEq, _alphaBlendEq); }
	impl::PipelineState::ptr_type createDefault()const { return new BlendOpState(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::BlendEq; }

	BlendOpState() { }
	BlendOpState(types::BlendOp rgbBlendEquation, types::BlendOp alphaBlendEquation) :
		_rgbBlendEq(rgbBlendEquation), _alphaBlendEq(alphaBlendEquation) {}

	void commitState(IGraphicsContext& device, types::BlendOp rgbBlendEquation, types::BlendOp alphaBlendEquation);

	types::BlendOp _rgbBlendEq;
	types::BlendOp _alphaBlendEq;
};

/// <summary>Pipeline state. Controls the depth clear value.</summary>
class DepthClearState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(_clearDepth); }

	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::DepthClear; }
	impl::PipelineState::ptr_type createClone()const  { return new DepthClearState(*this); }
	impl::PipelineState::ptr_type createDefault()const { return new DepthClearState(); }
	bool operator==(const DepthClearState& rhs) { return (_clearDepth == rhs._clearDepth); }
	bool operator!=(const DepthClearState& rhs) { return !(*this == rhs); }

	DepthClearState() { }
	DepthClearState(float32 depth) { _clearDepth = depth; }
	void commitState(float32 depth);

	float32 _clearDepth;
};

/// <summary>Pipeline State. Controls the stencil clear value.</summary>
class StencilClearState : public impl::GraphicsPipelineImplState
{
public:
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilClear; }
	void set(IGraphicsContext& device) { commitState(device, _clearStencil); }

	ptr_type createClone()const { return new StencilClearState(*this); }
	ptr_type createDefault()const { return new StencilClearState(); }

	StencilClearState() { }
	StencilClearState(int32 clearStencil) : _clearStencil(clearStencil) {}

	void commitState(IGraphicsContext& device, int32 clearStencil);
	int32 _clearStencil;
};

/// <summary>Pipeline. Controls the stencil test enable/disable.</summary>
class StencilTestState : public impl::GraphicsPipelineImplState
{
public:
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilTest; }
	void set(IGraphicsContext& device) { commitState(device, _stencilTest); }
	ptr_type createClone()const { return new StencilTestState(*this); }
	ptr_type createDefault()const { return new StencilTestState(); }

	StencilTestState() {}
	StencilTestState(bool enableTest) : _stencilTest(enableTest) {}
	void commitState(IGraphicsContext& device, bool flag);
	bool _stencilTest;
};

class StencilCompareOpFront : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, _cmpOp); }
	ptr_type createClone()const { return new StencilCompareOpFront(*this); }
	ptr_type createDefault()const { return new StencilCompareOpFront(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilOpFront; }

	StencilCompareOpFront(types::ComparisonMode cmp = types::ComparisonMode::DefaultStencilFunc) : _cmpOp(cmp) {}
	types::ComparisonMode _cmpOp;
	void commitState(IGraphicsContext& device, types::ComparisonMode cmp);
};

class StencilCompareOpBack : public impl::GraphicsPipelineImplState
{
public:
	types::ComparisonMode _cmpOp;

	void set(IGraphicsContext& device) { commitState(device, _cmpOp); }
	ptr_type createClone()const { return new StencilCompareOpBack(*this); }
	ptr_type createDefault()const { return new StencilCompareOpBack(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilOpBack; }

	StencilCompareOpBack(types::ComparisonMode cmp = types::ComparisonMode::DefaultStencilFunc) : _cmpOp(cmp) {}
	void commitState(IGraphicsContext& device, types::ComparisonMode cmp);
};

/// <summary>Pipeline. Controls the front stencil op.</summary>
class StencilOpFrontState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, _opStencilFail, _opDepthFail, _opDepthPass); }

	ptr_type createClone()const { return new StencilOpFrontState(*this); }
	ptr_type createDefault()const { return new StencilOpFrontState(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilOpFront; }

	StencilOpFrontState() { }
	StencilOpFrontState(types::StencilOp opStencilFail, types::StencilOp opDepthFail, types::StencilOp opDepthPass) :
		_opStencilFail(opStencilFail), _opDepthPass(opDepthPass), _opDepthFail(opDepthFail) {}

	void commitState(IGraphicsContext& device, types::StencilOp opStencilFail, types::StencilOp opDepthFail,
	                 types::StencilOp opDepthStencilPass);

	types::StencilOp _opStencilFail;
	types::StencilOp _opDepthPass;
	types::StencilOp _opDepthFail;
};

/// <summary>Pipeline. Controls the back stencil op.</summary>
class StencilOpBackState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, _opStencilFail, _opDepthFail, _opDepthPass); }
	ptr_type createClone()const { return new StencilOpBackState(*this); }
	ptr_type createDefault()const { return new StencilOpBackState(); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::StencilOpBack; }

	StencilOpBackState() { }
	StencilOpBackState(types::StencilOp opStencilFail, types::StencilOp opDepthFail,
	                   types::StencilOp opDepthPass) :
		_opStencilFail(opStencilFail), _opDepthPass(opDepthPass), _opDepthFail(opDepthFail) {}
	types::StencilOp _opStencilFail;
	types::StencilOp _opDepthPass;
	types::StencilOp _opDepthFail;

	void commitState(IGraphicsContext& device, types::StencilOp opStencilFail, types::StencilOp opDepthFail,
	                 types::StencilOp opDepthPass);
};

/// <summary>Pipeline. Controls the scissor test. Enable/ disable.</summary>
class ScissorTestState : public impl::GraphicsPipelineImplState
{
public:
	void set(IGraphicsContext& device) { commitState(device, _scissorTest); }
	impl::PipelineState::ptr_type createClone()const  { return new ScissorTestState(*this); }
	impl::PipelineState::ptr_type createDefault()const
	{
		return new ScissorTestState(types::PipelineDefaults::ViewportScissor::ScissorTestEnabled);
	}
	bool operator==(const ScissorTestState& rhs) { return ((_scissorTest == rhs._scissorTest)); }
	bool operator!=(const ScissorTestState& rhs) { return !(*this == rhs); }
	impl::GraphicsStateType getStateType()const { return impl::GraphicsStateType::ScissorTest; }

	ScissorTestState() : _scissorTest(types::PipelineDefaults::ViewportScissor::ScissorTestEnabled) { }
	ScissorTestState(bool enable) : _scissorTest(enable) {}
	void commitState(IGraphicsContext& device, bool enable);
	bool _scissorTest;
};

/// <summary>Pipeline graphics shader program state.</summary>
class GraphicsShaderProgramState : public impl::GraphicsPipelineImplState
{
public:
	GraphicsShaderProgramState(const GraphicsShaderProgramState& shaderProgram);
	GraphicsShaderProgramState();

	/// <summary>Bind this program state.</summary>
	void bind(IGraphicsContext& device) const;

	/// <summary>Set this program state.</summary>
	void set(IGraphicsContext& device) { bind(device); }

	/// <summary>Return default program state.</summary>
	PipelineState::ptr_type createDefault()const { return new GraphicsShaderProgramState(); }

	/// <summary>Return clone program state.</summary>
	PipelineState::ptr_type createClone()const { return new GraphicsShaderProgramState(*this); }

	/// <summary>Get thi state type.</summary>
	impl::GraphicsStateType getStateType() const { return impl::GraphicsStateType::ShaderProgram; }

	bool operator==(const GraphicsShaderProgramState& rhs)const { return (_shaderProgram == rhs._shaderProgram); }
	bool operator!=(const GraphicsShaderProgramState& rhs)const { return !(*this == rhs); }
	void generate();

	/// <summary>Destroy this.</summary>
	void destroy();

	/// <summary>Save the program binary to a file or other stream.</summary>
	/// <param name="outputStream">Output stream. Must be writable</param>
	/// <returns>True if it is successful</returns>
	bool saveProgramBinary(Stream& outputStream);

	mutable native::HPipeline _shaderProgram;
};

/// <summary>ComputePipeline shader program state.</summary>
class ComputeShaderProgramState : public impl::ComputePipelineImplState
{
public:

	/// <summary>Contstructor. Initializes a new instance of this class.</summary>
	ComputeShaderProgramState() { _isValid = false; }

	/// <summary>Copy Contstructor. Initializes a new instance of this class by copying another.</summary>
	ComputeShaderProgramState(const ComputeShaderProgramState& shaderProgram)
		: _shaderProgram(shaderProgram._shaderProgram)
	{
		_isValid = true;
	}

	/// <summary>Bind this program state.</summary>
	void bind(IGraphicsContext& device);

	/// <summary>Set this program state.</summary>
	void set(IGraphicsContext& device) { bind(device); }

	/// <summary>Return a default prgram state.</summary>
	PipelineState::ptr_type createDefault()const { return new ComputeShaderProgramState(); }

	/// <summary>Return clone of this program state.</summary>
	PipelineState::ptr_type createClone()const { return new ComputeShaderProgramState(*this); }

	/// <summary>Return this state type.</summary>
	impl::GraphicsStateType getStateType() const { return impl::GraphicsStateType::ShaderProgram; }

	bool operator==(const ComputeShaderProgramState& rhs)const { return (_shaderProgram == rhs._shaderProgram); }
	bool operator!=(const ComputeShaderProgramState& rhs)const { return !(*this == rhs); }
	void generate();

	/// <summary>Return the api program object.</summary>
	const native::HPipeline& get() const { return _shaderProgram; }

	/// <summary>Return the api program object.</summary>
	native::HPipeline& get() { return _shaderProgram; }

	native::HPipeline _shaderProgram;
};


class DepthBiasState : public impl::GraphicsPipelineImplState
{
public:
	DepthBiasState(bool enableDepthBias = false, float32 depthBiasClamp = 0.f, float32 depthBiasConstantFactor = 0.f,
	               float32 depthBiasSlopeFactor = 0.f): _enableDepthBias(enableDepthBias), _depthBiasClamp(depthBiasClamp),
		_depthBiasConstantFactor(depthBiasConstantFactor), _depthBiasSlopeFactor(depthBiasSlopeFactor) {}

	DepthBiasState(const pipelineCreation::RasterStateCreateParam& state) :
		_enableDepthBias(state.isDepthBiasEnabled()), _depthBiasClamp(state.getDepthBiasClamp()),
		_depthBiasConstantFactor(state.getDepthBiasConstantFactor()), _depthBiasSlopeFactor(state.getDepthBiasSlopeFactor()) {}


	impl::PipelineState::ptr_type createDefault() const { return new DepthBiasState(); }
	impl::PipelineState::ptr_type createClone() const
	{
		return new DepthBiasState(_enableDepthBias, _depthBiasClamp, _depthBiasConstantFactor, _depthBiasSlopeFactor);
	}
	impl::GraphicsStateType getStateType() const { return impl::GraphicsStateType::DepthBias; }
	bool operator==(const DepthBiasState& rhs)const
	{
		return (!_enableDepthBias && !rhs._enableDepthBias) || (
		         _enableDepthBias == rhs._enableDepthBias &&
		         _depthBiasClamp == rhs._depthBiasClamp &&
		         _depthBiasConstantFactor == rhs._depthBiasConstantFactor &&
		         _depthBiasSlopeFactor == rhs._depthBiasSlopeFactor);
	}
	bool operator!=(const DepthBiasState& rhs)const { return !(*this == rhs); }
	void set(IGraphicsContext& device)
	{
		commitState(device, _enableDepthBias, _depthBiasClamp, _depthBiasConstantFactor, _depthBiasSlopeFactor);
	}
	void reset(IGraphicsContext& device)
	{
		_enableDepthBias = false;
		_depthBiasClamp = 0.f;
		_depthBiasConstantFactor = 0.f;
		_depthBiasSlopeFactor = 0.f;
	}
	void unset(IGraphicsContext& device) { if (_parent) { _parent->set(device); } else { setDefault(device); } }
	void commitState(IGraphicsContext& device, bool enable, float32 clamp, float32 constantFactor, float32 slopeFactor);
	void setDefault(IGraphicsContext& device) { commitState(device, false, 0.f, 0.f, 0.f); }

	bool _enableDepthBias;
	float32 _depthBiasClamp;
	float32 _depthBiasConstantFactor;
	float32 _depthBiasSlopeFactor;
};


}

inline const native::HPipeline_& native_cast(const gles::GraphicsShaderProgramState& object)
{
	return *object._shaderProgram;
}
inline native::HPipeline_& native_cast(gles::GraphicsShaderProgramState& object)
{
	return *object._shaderProgram;
}
inline const native::HPipeline_& native_cast(const gles::ComputeShaderProgramState& object)
{
	return *object._shaderProgram;
}
inline native::HPipeline_& native_cast(gles::ComputeShaderProgramState& object)
{
	return *object._shaderProgram;
}
}
}
