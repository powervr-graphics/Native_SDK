/*!*********************************************************************************************************************
\file         PVRApi\OGLES\PipelineConfigStates.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Definitions of the OpenGL ES implementation of several Pipeline State objects (see GraphicsPipeline).
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/PipelineConfigStatesGles.h"
#include "PVRApi/ApiIncludes.h"
#include "PVRNativeApi/ApiErrors.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"

namespace pvr {
using namespace types;
namespace api {
namespace gles {
void DepthTestState::commitState(pvr::IGraphicsContext& device, bool depthTest)
{
	debugLogApiError("DepthTestState::setDepthTest enter");
	if (static_cast<pvr::platform::ContextGles&>(device).getCurrentRenderStates().depthStencil.depthTest == depthTest)
	{
		return;
	}
	static_cast<pvr::platform::ContextGles&>(device).getCurrentRenderStates().depthStencil.depthTest = depthTest;
	depthTest ? gl::Enable(GL_DEPTH_TEST) : gl::Disable(GL_DEPTH_TEST);
	debugLogApiError("DepthTestState::setDepthTest exit");
}

void DepthWriteState::commitState(pvr::IGraphicsContext& device, bool depthWrite)
{
	pvr::platform::ContextGles& deviceES = static_cast<pvr::platform::ContextGles&>(device);
	if (deviceES.getCurrentRenderStates().depthStencil.depthWrite == depthWrite)
	{
		return;
	}
	deviceES.getCurrentRenderStates().depthStencil.depthWrite = depthWrite;
	debugLogApiError("DepthWriteState::setDepthWrite enter");
	gl::DepthMask(depthWrite ? GL_TRUE : GL_FALSE);
	debugLogApiError("DepthWriteState::setDepthWrite exit");
}

void PolygonFrontFaceState::commitState(pvr::IGraphicsContext& device, types::Face cullFace)
{
	debugLogApiError("PolygonFrontFaceState::commitState enter");
	platform::ContextGles& deviceEs = static_cast<platform::ContextGles&>(device);
	if (deviceEs.getCurrentRenderStates().cullFace == cullFace) { return; }
	deviceEs.getCurrentRenderStates().cullFace = cullFace;
    if (cullFace == types::Face::None)
	{
		gl::Disable(GL_CULL_FACE);
	}
	else
	{
		gl::Enable(GL_CULL_FACE);
		gl::CullFace(pvr::api::ConvertToGles::face(cullFace));
	}

	debugLogApiError("PolygonFrontFaceState::commitState exit");
}

void TessPatchControlPoints::commitState(IGraphicsContext &device, uint32 controlPoints)
{
	debugLogApiError("TessPatchControlPoints::commitState begin");
	if(glext::PatchParameteriEXT == NULL)
	{
		Log(Logger::Debug, "Tesselation patch control points is not supported");
		return ;
	}
#if defined(GL_PATCH_VERTICES_EXT)
	glext::PatchParameteriEXT(GL_PATCH_VERTICES_EXT, controlPoints);
#endif
	debugLogApiError("TessPatchControlPoints::commitState exit");
}

void PolygonWindingOrderState::commitState(pvr::IGraphicsContext& device, types::PolygonWindingOrder windingOrder)
{
	debugLogApiError("PolygonWindingOrderState::commitState enter");
	platform::ContextGles& deviceEs = static_cast<platform::ContextGles&>(device);
	if (deviceEs.getCurrentRenderStates().polyWindingOrder == windingOrder) { return; }
	deviceEs.getCurrentRenderStates().polyWindingOrder = windingOrder;
	gl::FrontFace(windingOrder == PolygonWindingOrder::FrontFaceCCW ? GL_CCW : GL_CW);
	debugLogApiError("PolygonWindingOrderState::commitState exit");
}

void BlendOpState::commitState(pvr::IGraphicsContext& device, BlendOp rgbBlendOp, BlendOp alphaBlendOp)
{
	debugLogApiError("BlendOpState::setBlendEq enter");
	platform::ContextGles& deviceEs = static_cast<platform::ContextGles&>(device);
	if ((deviceEs.getCurrentRenderStates().rgbBlendOp == rgbBlendOp) &&
	        (deviceEs.getCurrentRenderStates().alphaBlendOp == alphaBlendOp)) {	return;	}
	deviceEs.getCurrentRenderStates().rgbBlendOp = rgbBlendOp;
	deviceEs.getCurrentRenderStates().alphaBlendOp = alphaBlendOp;

	gl::BlendEquationSeparate(ConvertToGles::blendEq(rgbBlendOp), ConvertToGles::blendEq(alphaBlendOp));
	debugLogApiError("BlendOpState::setBlendEq exit");
}


void BlendFactorState::commitState(pvr::IGraphicsContext& device, types::BlendFactor srcRgbFactor,
                                   types::BlendFactor dstRgbFactor, types::BlendFactor srcAlphaFactor, types::BlendFactor dstAlphaFactor)
{
	debugLogApiError("BlendOpState::setBlendFactor enter");
	platform::ContextGles::RenderStatesTracker& currentStates = static_cast<platform::ContextGles&>(device).getCurrentRenderStates();
	if ((currentStates.srcRgbFactor == srcRgbFactor) &&
	        (currentStates.srcAlphaFactor == srcAlphaFactor) &&
	        (currentStates.destRgbFactor == dstRgbFactor) &&
	        (currentStates.destAlphaFactor == dstAlphaFactor))
	{
		return;
	}
	currentStates.srcRgbFactor = (BlendFactor)srcRgbFactor;
	currentStates.srcAlphaFactor = (BlendFactor)srcAlphaFactor;
	currentStates.destRgbFactor = (BlendFactor)dstRgbFactor;
	currentStates.destAlphaFactor = (BlendFactor)dstAlphaFactor;
	gl::BlendFuncSeparate(ConvertToGles::blendFactor((BlendFactor)srcRgbFactor),
	                      ConvertToGles::blendFactor((BlendFactor)dstRgbFactor),
	                      ConvertToGles::blendFactor((BlendFactor)srcAlphaFactor),
	                      ConvertToGles::blendFactor((BlendFactor)dstAlphaFactor));
	debugLogApiError("BlendOpState::setBlendFactor exit");
}

BlendFactorState::BlendFactorState(BlendFactor srcRgbFactor, BlendFactor dstRgbFactor,
                                   BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor)
{
	packData(srcRgbFactor, dstRgbFactor, srcAlphaFactor, dstAlphaFactor);
}

void BlendingEnableState::commitState(pvr::IGraphicsContext& device, bool blendTest)
{
	debugLogApiError("BlendingEnableState::setBlendTest enter");
	if (static_cast<pvr::platform::ContextGles&>(device).getCurrentRenderStates().enabledBlend == blendTest)
	{
		return;
	}
	static_cast<pvr::platform::ContextGles&>(device).getCurrentRenderStates().enabledBlend = blendTest;
	(blendTest ? gl::Enable(GL_BLEND) : gl::Disable(GL_BLEND));
	debugLogApiError("BlendingEnableState::setBlendTest exit");
}

void DepthClearState::commitState(float32 depth)
{
	debugLogApiError("DepthClearState::commitState enter");
	gl::ClearDepthf(depth);
	gl::Clear(GL_DEPTH_BUFFER_BIT);
	debugLogApiError("DepthClearState::commitState exit");
}

void ColorWriteMask::commitState(pvr::IGraphicsContext& device, const glm::bvec4 mask)
{
	debugLogApiError("SetColorWriteMask::execute enter");
	if (static_cast<platform::ContextGles&>(device).getCurrentRenderStates().colorWriteMask == mask)
	{
		return;
	}
	gl::ColorMask(mask[0], mask[1], mask[2], mask[3]);
	static_cast<platform::ContextGles&>(device).getCurrentRenderStates().colorWriteMask = mask;
	debugLogApiError("SetColorWriteMask::execute exit");
}

void DepthFuncState::commitState(pvr::IGraphicsContext& device, ComparisonMode func)
{
	debugLogApiError("FrameBufferWriteState::setDepthFunc enter");
	if (static_cast<platform::ContextGles&>(device).getCurrentRenderStates().depthStencil.depthOp == func)
	{
		return;
	}
	static_cast<platform::ContextGles&>(device).getCurrentRenderStates().depthStencil.depthOp = func;
	gl::DepthFunc(ConvertToGles::comparisonMode(func));
	debugLogApiError("FrameBufferWriteState::setDepthFunc exit");
}

void StencilClearState::commitState(pvr::IGraphicsContext& device, int32 clearStencil)
{
	debugLogApiError("StencilClearState::clearStencil enter");
	if (static_cast<platform::ContextGles&>(device).getCurrentRenderStates().depthStencil.clearStencilValue == clearStencil)
	{
		return;
	}
	static_cast<platform::ContextGles&>(device).getCurrentRenderStates().depthStencil.clearStencilValue = clearStencil;
	gl::ClearStencil(clearStencil);
	debugLogApiError("StencilClearState::clearStencil exit");
}

void StencilTestState::commitState(pvr::IGraphicsContext& device, bool flag)
{
	debugLogApiError("StencilClearState::setStencilTest enter");
	if (static_cast<platform::ContextGles&>(device).getCurrentRenderStates().depthStencil.enableStencilTest == flag)
	{
		return;
	}
	static_cast<platform::ContextGles&>(device).getCurrentRenderStates().depthStencil.enableStencilTest = flag;
	(flag ? gl::Enable(GL_STENCIL_TEST) : gl::Disable(GL_STENCIL_TEST));
	debugLogApiError("StencilClearState::setStencilTest exit");
}

void StencilOpFrontState::commitState(pvr::IGraphicsContext& device, StencilOp opStencilFail, StencilOp opDepthFail,
                                      StencilOp opDepthStencilPass)
{
	debugLogApiError("StencilOpFrontState::commitState enter");
	platform::ContextGles::RenderStatesTracker& currentStates = static_cast<platform::ContextGles&>(device).getCurrentRenderStates();
	if ((currentStates.depthStencil.stencilFailOpFront == opStencilFail) &&
	        (currentStates.depthStencil.depthFailOpFront == opDepthFail) &&
	        (currentStates.depthStencil.depthStencilPassOpFront == opDepthStencilPass))
	{
		return;
	}
	currentStates.depthStencil.stencilFailOpFront = opStencilFail;
	currentStates.depthStencil.depthFailOpFront = opDepthFail;
	currentStates.depthStencil.depthStencilPassOpFront = opDepthStencilPass;

	gl::StencilOpSeparate(GL_FRONT,  pvr::api::ConvertToGles::stencilOp(opStencilFail),
	                      pvr::api::ConvertToGles::stencilOp(opDepthFail),
	                      pvr::api::ConvertToGles::stencilOp(opDepthStencilPass));

	debugLogApiError("StencilOpFrontState::commitState exit");
}

void StencilOpBackState::commitState(pvr::IGraphicsContext& device, StencilOp opStencilFail,
                                     StencilOp opDepthFail, StencilOp opDepthStencilPass)
{
	debugLogApiError("StencilOpBackState::commitState enter");
	platform::ContextGles::RenderStatesTracker& currentStates =
	  static_cast<platform::ContextGles&>(device).getCurrentRenderStates();
	if ((currentStates.depthStencil.stencilFailOpBack == opStencilFail) &&
	        (currentStates.depthStencil.depthFailOpBack == opDepthFail) &&
	        (currentStates.depthStencil.depthStencilPassOpBack == opDepthStencilPass))
	{
		return;
	}
	currentStates.depthStencil.stencilFailOpBack = opStencilFail;
	currentStates.depthStencil.depthFailOpBack = opDepthFail;
	currentStates.depthStencil.depthStencilPassOpBack = opDepthStencilPass;
	gl::StencilOpSeparate(GL_BACK, pvr::api::ConvertToGles::stencilOp(opStencilFail),
	                      pvr::api::ConvertToGles::stencilOp(opDepthFail),
	                      pvr::api::ConvertToGles::stencilOp(opDepthStencilPass));
	debugLogApiError("StencilOpBackState::commitState exit");
}

void ScissorTestState::commitState(pvr::IGraphicsContext& device, bool enable)
{
	debugLogApiError("StencilOpBackState::commitState enter");
	if (static_cast<platform::ContextGles&>(device).getCurrentRenderStates().enabledScissorTest ==
	        enable) { return; }

	static_cast<platform::ContextGles&>(device).getCurrentRenderStates().enabledScissorTest = enable;
	(enable ? gl::Enable(GL_SCISSOR_TEST) : gl::Disable(GL_SCISSOR_TEST));
	debugLogApiError("StencilOpBackState::commitState exit");
}

void StencilCompareOpFront::commitState(pvr::IGraphicsContext& device, pvr::ComparisonMode cmp)
{
	debugLogApiError("StencilComapreOpFront::commitState enter");
	platform::ContextGles::RenderStatesTracker& recordedStates =
	  static_cast<platform::ContextGles&>(device).getCurrentRenderStates();
	if (cmp != recordedStates.depthStencil.stencilOpFront)
	{
		gl::StencilFuncSeparate(GL_FRONT, ConvertToGles::comparisonMode(cmp), recordedStates.depthStencil.refFront,
		                        recordedStates.depthStencil.readMaskFront);
		recordedStates.depthStencil.stencilOpFront =  cmp;
	}
	debugLogApiError("StencilComapreOpFront::commitState exit");
}

void StencilCompareOpBack::commitState(pvr::IGraphicsContext& device, pvr::ComparisonMode cmp)
{
	debugLogApiError("StencilCompareOpBack::commitState enter");
	platform::ContextGles::RenderStatesTracker& recordedStates =
	  static_cast<platform::ContextGles&>(device).getCurrentRenderStates();
	if (cmp != recordedStates.depthStencil.stencilOpBack)
	{
		gl::StencilFuncSeparate(GL_BACK, ConvertToGles::comparisonMode(cmp), recordedStates.depthStencil.refBack,
		                        recordedStates.depthStencil.readMaskBack);
		recordedStates.depthStencil.stencilOpBack = cmp;
	}
	debugLogApiError("StencilCompareOpBack::commitState exit");
}

GraphicsShaderProgramState::GraphicsShaderProgramState()
{
	m_isValid = false;
	m_shaderProgram.construct();
}
GraphicsShaderProgramState::GraphicsShaderProgramState(const GraphicsShaderProgramState& shaderState)
{
	m_isValid = true;
	m_shaderProgram = shaderState.m_shaderProgram;
}
void GraphicsShaderProgramState::bind() const
{
	gl::UseProgram(m_shaderProgram->handle);
	debugLogApiError("GraphicsShaderProgramState::bind exit");
}
void GraphicsShaderProgramState::reset(pvr::IGraphicsContext& device)
{
	gl::UseProgram(0);
	debugLogApiError("GraphicsShaderProgramState::reset exit");
}
void GraphicsShaderProgramState::destroy()
{
	gl::DeleteProgram(m_shaderProgram->handle); m_shaderProgram.reset(); m_isValid = false;
	debugLogApiError("GraphicsShaderProgramState::destoy exit");
}
void GraphicsShaderProgramState::generate()
{
	if (!m_shaderProgram.isValid()) { m_shaderProgram.construct(0); }
	m_shaderProgram->handle = gl::CreateProgram(); m_isValid = true;
	debugLogApiError("GraphicsShaderProgramState::generate exit");
}

bool GraphicsShaderProgramState::saveProgramBinary(Stream& outFile)
{
#if (!defined(BUILD_API_MAX)||(BUILD_API_MAX>=30))
	// validate the program
	GLint linked;
	gl::GetProgramiv(m_shaderProgram->handle, GL_LINK_STATUS, &linked);
	if (!linked) { return false; }

	// get the length of the shader binary program in memory.
	GLsizei length = 0;
	gl::GetProgramiv(m_shaderProgram->handle, GL_PROGRAM_BINARY_LENGTH, &length);

	// No binary?
	if (length == 0) { return false; }

	std::vector<byte> shaderBinary;
	shaderBinary.resize(length);

	GLenum binaryFmt = 0;
	GLsizei lengthWritten = 0;
	gl::GetProgramBinary(m_shaderProgram->handle, length, &lengthWritten, &binaryFmt, &shaderBinary[0]);

	// save failed?
	if (!lengthWritten) { return false; }

	// save the binary format
	size_t fileWrittenLen = 0;
	bool rslt = outFile.write(sizeof(GLenum), 1, (void*)&binaryFmt, fileWrittenLen);

	// File failed
	if (!rslt)  { return false; }

	// save the program
	rslt = outFile.write(length, 1, &shaderBinary[0], fileWrittenLen);

	return rslt;
#else
	assertion(0, "ShaderUtils::saveProgramBinary Underlying API OpenGL ES 2 does not support Program Binaries");
	Log(Log.Error,
	    "ShaderUtils::saveProgramBinary Underlying API OpenGL ES 2 does not support Program Binaries");
	return Result::UnsupportedRequest;
#endif
}


/////////////////////////////// COMPUTE SHADER ///////////////////////////////
void ComputeShaderProgramState::generate()
{
	if (!m_shaderProgram.isValid()) { m_shaderProgram.construct(0); }
	m_shaderProgram->handle = gl::CreateProgram(); m_isValid = true;
	debugLogApiError("ComputeShaderProgramState::generate exit");
}
void ComputeShaderProgramState::bind()
{
	gl::UseProgram(m_shaderProgram->handle);
	debugLogApiError("ComputeShaderProgramState::bind exit");
}
void ComputeShaderProgramState::reset(pvr::IGraphicsContext& device)
{
	gl::UseProgram(0);
	debugLogApiError("ComputeShaderProgramState::reset exit");
}
}// namespace impl
}// namespace api
}// namespace pvr
//!\endcond
