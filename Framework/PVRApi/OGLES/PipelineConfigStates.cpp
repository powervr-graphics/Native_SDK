/*!
\brief Definitions of the OpenGL ES implementation of several Pipeline State objects (see GraphicsPipeline).
\file PVRApi/OGLES/PipelineConfigStates.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/OGLES/PipelineConfigStatesGles.h"
#include "PVRApi/ApiIncludes.h"
#include "PVRNativeApi/OGLES/ApiErrorsGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"

namespace pvr {
using namespace types;
namespace api {
namespace gles {
void DepthTestState::commitState(IGraphicsContext& device, bool depthTest)
{
	debugLogApiError("DepthTestState::setDepthTest enter");
	auto& renderstates = native_cast(device).getCurrentRenderStates();
	if (renderstates.depthStencil.depthTest == depthTest)
	{
		return;
	}
	native_cast(device).getCurrentRenderStates().depthStencil.depthTest = depthTest;
	depthTest ? gl::Enable(GL_DEPTH_TEST) : gl::Disable(GL_DEPTH_TEST);
	debugLogApiError("DepthTestState::setDepthTest exit");
}

void DepthBiasState::commitState(IGraphicsContext& device, bool enable, float32 clamp, float32 constantFactor, float32 slopeFactor)
{
	debugLogApiError("DepthBiasState::commitState enter");
	auto& renderstates = native_cast(device).getCurrentRenderStates().depthStencil;

	if (_enableDepthBias != renderstates.depthBias)
	{
		if (_enableDepthBias) { gl::Enable(GL_POLYGON_OFFSET_FILL); }
		else { gl::Disable(GL_POLYGON_OFFSET_FILL); }
		renderstates.depthBias = _enableDepthBias;
	}
	if (_enableDepthBias && (_depthBiasConstantFactor != renderstates.depthBiasConstantFactor ||
	                         _depthBiasSlopeFactor != renderstates.depthBiasSlopeFactor))
	{
		gl::PolygonOffset(_depthBiasSlopeFactor, _depthBiasConstantFactor);
		renderstates.depthBiasConstantFactor = _depthBiasConstantFactor;
		renderstates.depthBiasSlopeFactor = _depthBiasSlopeFactor;
	}

	debugLogApiError("DepthBiasState::commitState exit");
}


void DepthWriteState::commitState(IGraphicsContext& device, bool depthWrite)
{
	platform::ContextGles& deviceES = native_cast(device);
	if (deviceES.getCurrentRenderStates().depthStencil.depthWrite == depthWrite)
	{
		return;
	}
	deviceES.getCurrentRenderStates().depthStencil.depthWrite = depthWrite;
	debugLogApiError("DepthWriteState::setDepthWrite enter");
	gl::DepthMask(depthWrite ? GL_TRUE : GL_FALSE);
	debugLogApiError("DepthWriteState::setDepthWrite exit");
}

<<<<<<< HEAD
void PolygonFrontFaceState::commitState(pvr::IGraphicsContext& device, types::Face cullFace)
=======
void PolygonFrontFaceState::commitState(IGraphicsContext& device, types::Face cullFace)
>>>>>>> 1776432f... 4.3
{
	debugLogApiError("PolygonFrontFaceState::commitState enter");
	platform::ContextGles& deviceEs = native_cast(device);
	if (deviceEs.getCurrentRenderStates().cullFace == cullFace) { return; }
	deviceEs.getCurrentRenderStates().cullFace = cullFace;
	if (cullFace == types::Face::None)
	{
		gl::Disable(GL_CULL_FACE);
	}
	else
	{
		gl::Enable(GL_CULL_FACE);
		gl::CullFace(nativeGles::ConvertToGles::face(cullFace));
	}
	deviceEs.getCurrentRenderStates().cullFace = cullFace;
	debugLogApiError("PolygonFrontFaceState::commitState exit");
}

<<<<<<< HEAD
void TessPatchControlPoints::commitState(IGraphicsContext &device, uint32 controlPoints)
{
	debugLogApiError("TessPatchControlPoints::commitState begin");
	if(glext::PatchParameteriEXT == NULL)
=======
void TessPatchControlPoints::commitState(IGraphicsContext& device, uint32 controlPoints)
{
	debugLogApiError("TessPatchControlPoints::commitState begin");
	if (glext::PatchParameteriEXT == NULL)
>>>>>>> 1776432f... 4.3
	{
		Log(Logger::Debug, "Tesselation patch control points is not supported");
		return ;
	}
#if defined(GL_PATCH_VERTICES_EXT)
	glext::PatchParameteriEXT(GL_PATCH_VERTICES_EXT, controlPoints);
#endif
	debugLogApiError("TessPatchControlPoints::commitState exit");
}

<<<<<<< HEAD
void PolygonWindingOrderState::commitState(pvr::IGraphicsContext& device, types::PolygonWindingOrder windingOrder)
=======
void PolygonWindingOrderState::commitState(IGraphicsContext& device, types::PolygonWindingOrder windingOrder)
>>>>>>> 1776432f... 4.3
{
	debugLogApiError("PolygonWindingOrderState::commitState enter");
	platform::ContextGles& deviceEs = native_cast(device);
	if (deviceEs.getCurrentRenderStates().polyWindingOrder == windingOrder) { return; }
	deviceEs.getCurrentRenderStates().polyWindingOrder = windingOrder;
	gl::FrontFace(windingOrder == PolygonWindingOrder::FrontFaceCCW ? GL_CCW : GL_CW);
	debugLogApiError("PolygonWindingOrderState::commitState exit");
}

<<<<<<< HEAD
void BlendOpState::commitState(pvr::IGraphicsContext& device, BlendOp rgbBlendOp, BlendOp alphaBlendOp)
=======
void BlendOpState::commitState(IGraphicsContext& device, BlendOp rgbBlendOp, BlendOp alphaBlendOp)
>>>>>>> 1776432f... 4.3
{
	debugLogApiError("BlendOpState::setBlendEq enter");
	platform::ContextGles& deviceEs = native_cast(device);
	if ((deviceEs.getCurrentRenderStates().rgbBlendOp == rgbBlendOp) &&
	    (deviceEs.getCurrentRenderStates().alphaBlendOp == alphaBlendOp)) { return; }
	deviceEs.getCurrentRenderStates().rgbBlendOp = rgbBlendOp;
	deviceEs.getCurrentRenderStates().alphaBlendOp = alphaBlendOp;

	gl::BlendEquationSeparate(nativeGles::ConvertToGles::blendEq(rgbBlendOp), nativeGles::ConvertToGles::blendEq(alphaBlendOp));
	debugLogApiError("BlendOpState::setBlendEq exit");
}


<<<<<<< HEAD
void BlendFactorState::commitState(pvr::IGraphicsContext& device, types::BlendFactor srcRgbFactor,
=======
void BlendFactorState::commitState(IGraphicsContext& device, types::BlendFactor srcRgbFactor,
>>>>>>> 1776432f... 4.3
                                   types::BlendFactor dstRgbFactor, types::BlendFactor srcAlphaFactor, types::BlendFactor dstAlphaFactor)
{
	debugLogApiError("BlendOpState::setBlendFactor enter");
	platform::ContextGles::RenderStatesTracker& currentStates = native_cast(device).getCurrentRenderStates();
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
<<<<<<< HEAD
	gl::BlendFuncSeparate(ConvertToGles::blendFactor((BlendFactor)srcRgbFactor),
	                      ConvertToGles::blendFactor((BlendFactor)dstRgbFactor),
	                      ConvertToGles::blendFactor((BlendFactor)srcAlphaFactor),
	                      ConvertToGles::blendFactor((BlendFactor)dstAlphaFactor));
=======
	gl::BlendFuncSeparate(nativeGles::ConvertToGles::blendFactor((BlendFactor)srcRgbFactor),
	                      nativeGles::ConvertToGles::blendFactor((BlendFactor)dstRgbFactor),
	                      nativeGles::ConvertToGles::blendFactor((BlendFactor)srcAlphaFactor),
	                      nativeGles::ConvertToGles::blendFactor((BlendFactor)dstAlphaFactor));
>>>>>>> 1776432f... 4.3
	debugLogApiError("BlendOpState::setBlendFactor exit");
}

BlendFactorState::BlendFactorState(BlendFactor srcRgbFactor, BlendFactor dstRgbFactor,
                                   BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor)
{
	packData(srcRgbFactor, dstRgbFactor, srcAlphaFactor, dstAlphaFactor);
}

void BlendingEnableState::commitState(IGraphicsContext& device, bool blendTest)
{
	debugLogApiError("BlendingEnableState::setBlendTest enter");
	if (native_cast(device).getCurrentRenderStates().enabledBlend == blendTest)
	{
		return;
	}
	native_cast(device).getCurrentRenderStates().enabledBlend = blendTest;
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

<<<<<<< HEAD
void ColorWriteMask::commitState(pvr::IGraphicsContext& device, const glm::bvec4 mask)
=======
void ColorWriteMask::commitState(IGraphicsContext& device, const glm::bvec4 mask)
>>>>>>> 1776432f... 4.3
{
	debugLogApiError("SetColorWriteMask::execute enter");
	if (native_cast(device).getCurrentRenderStates().colorWriteMask == mask)
	{
		return;
	}
	gl::ColorMask(mask[0], mask[1], mask[2], mask[3]);
	native_cast(device).getCurrentRenderStates().colorWriteMask = mask;
	debugLogApiError("SetColorWriteMask::execute exit");
}

<<<<<<< HEAD
void DepthFuncState::commitState(pvr::IGraphicsContext& device, ComparisonMode func)
=======
void DepthFuncState::commitState(IGraphicsContext& device, ComparisonMode func)
>>>>>>> 1776432f... 4.3
{
	debugLogApiError("FrameBufferWriteState::setDepthFunc enter");
	if (native_cast(device).getCurrentRenderStates().depthStencil.depthOp == func)
	{
		return;
	}
	native_cast(device).getCurrentRenderStates().depthStencil.depthOp = func;
	gl::DepthFunc(nativeGles::ConvertToGles::comparisonMode(func));
	debugLogApiError("FrameBufferWriteState::setDepthFunc exit");
}

<<<<<<< HEAD
void StencilClearState::commitState(pvr::IGraphicsContext& device, int32 clearStencil)
=======
void StencilClearState::commitState(IGraphicsContext& device, int32 clearStencil)
>>>>>>> 1776432f... 4.3
{
	debugLogApiError("StencilClearState::clearStencil enter");
	if (native_cast(device).getCurrentRenderStates().depthStencil.clearStencilValue == clearStencil)
	{
		return;
	}
	native_cast(device).getCurrentRenderStates().depthStencil.clearStencilValue = clearStencil;
	gl::ClearStencil(clearStencil);
	debugLogApiError("StencilClearState::clearStencil exit");
}

<<<<<<< HEAD
void StencilTestState::commitState(pvr::IGraphicsContext& device, bool flag)
=======
void StencilTestState::commitState(IGraphicsContext& device, bool flag)
>>>>>>> 1776432f... 4.3
{
	debugLogApiError("StencilClearState::setStencilTest enter");
	if (native_cast(device).getCurrentRenderStates().depthStencil.enableStencilTest == flag)
	{
		return;
	}
	native_cast(device).getCurrentRenderStates().depthStencil.enableStencilTest = flag;
	(flag ? gl::Enable(GL_STENCIL_TEST) : gl::Disable(GL_STENCIL_TEST));
	debugLogApiError("StencilClearState::setStencilTest exit");
}

<<<<<<< HEAD
void StencilOpFrontState::commitState(pvr::IGraphicsContext& device, StencilOp opStencilFail, StencilOp opDepthFail,
=======
void StencilOpFrontState::commitState(IGraphicsContext& device, StencilOp opStencilFail, StencilOp opDepthFail,
>>>>>>> 1776432f... 4.3
                                      StencilOp opDepthStencilPass)
{
	debugLogApiError("StencilOpFrontState::commitState enter");
	platform::ContextGles::RenderStatesTracker& currentStates = native_cast(device).getCurrentRenderStates();
	if ((currentStates.depthStencil.stencilFailOpFront == opStencilFail) &&
	    (currentStates.depthStencil.depthFailOpFront == opDepthFail) &&
	    (currentStates.depthStencil.depthStencilPassOpFront == opDepthStencilPass))
	{
		return;
	}
	currentStates.depthStencil.stencilFailOpFront = opStencilFail;
	currentStates.depthStencil.depthFailOpFront = opDepthFail;
	currentStates.depthStencil.depthStencilPassOpFront = opDepthStencilPass;

	gl::StencilOpSeparate(GL_FRONT,  nativeGles::ConvertToGles::stencilOp(opStencilFail),
	                      nativeGles::ConvertToGles::stencilOp(opDepthFail),
	                      nativeGles::ConvertToGles::stencilOp(opDepthStencilPass));

	debugLogApiError("StencilOpFrontState::commitState exit");
}

<<<<<<< HEAD
void StencilOpBackState::commitState(pvr::IGraphicsContext& device, StencilOp opStencilFail,
=======
void StencilOpBackState::commitState(IGraphicsContext& device, StencilOp opStencilFail,
>>>>>>> 1776432f... 4.3
                                     StencilOp opDepthFail, StencilOp opDepthStencilPass)
{
	debugLogApiError("StencilOpBackState::commitState enter");
	platform::ContextGles::RenderStatesTracker& currentStates =
<<<<<<< HEAD
	  static_cast<platform::ContextGles&>(device).getCurrentRenderStates();
=======
	  native_cast(device).getCurrentRenderStates();
>>>>>>> 1776432f... 4.3
	if ((currentStates.depthStencil.stencilFailOpBack == opStencilFail) &&
	    (currentStates.depthStencil.depthFailOpBack == opDepthFail) &&
	    (currentStates.depthStencil.depthStencilPassOpBack == opDepthStencilPass))
	{
		return;
	}
	currentStates.depthStencil.stencilFailOpBack = opStencilFail;
	currentStates.depthStencil.depthFailOpBack = opDepthFail;
	currentStates.depthStencil.depthStencilPassOpBack = opDepthStencilPass;
	gl::StencilOpSeparate(GL_BACK, nativeGles::ConvertToGles::stencilOp(opStencilFail),
	                      nativeGles::ConvertToGles::stencilOp(opDepthFail),
	                      nativeGles::ConvertToGles::stencilOp(opDepthStencilPass));
	debugLogApiError("StencilOpBackState::commitState exit");
}

void ScissorTestState::commitState(IGraphicsContext& device, bool enable)
{
	debugLogApiError("StencilOpBackState::commitState enter");
	if (native_cast(device).getCurrentRenderStates().enabledScissorTest ==
	    enable) { return; }

	native_cast(device).getCurrentRenderStates().enabledScissorTest = enable;
	(enable ? gl::Enable(GL_SCISSOR_TEST) : gl::Disable(GL_SCISSOR_TEST));
	debugLogApiError("StencilOpBackState::commitState exit");
}

<<<<<<< HEAD
void StencilCompareOpFront::commitState(pvr::IGraphicsContext& device, pvr::ComparisonMode cmp)
{
	debugLogApiError("StencilComapreOpFront::commitState enter");
	platform::ContextGles::RenderStatesTracker& recordedStates =
	  static_cast<platform::ContextGles&>(device).getCurrentRenderStates();
=======
void StencilCompareOpFront::commitState(IGraphicsContext& device, ComparisonMode cmp)
{
	debugLogApiError("StencilComapreOpFront::commitState enter");
	platform::ContextGles::RenderStatesTracker& recordedStates =
	  native_cast(device).getCurrentRenderStates();
>>>>>>> 1776432f... 4.3
	if (cmp != recordedStates.depthStencil.stencilOpFront)
	{
		gl::StencilFuncSeparate(GL_FRONT, nativeGles::ConvertToGles::comparisonMode(cmp), recordedStates.depthStencil.refFront,
		                        recordedStates.depthStencil.readMaskFront);
		recordedStates.depthStencil.stencilOpFront =  cmp;
	}
	debugLogApiError("StencilComapreOpFront::commitState exit");
}

<<<<<<< HEAD
void StencilCompareOpBack::commitState(pvr::IGraphicsContext& device, pvr::ComparisonMode cmp)
{
	debugLogApiError("StencilCompareOpBack::commitState enter");
	platform::ContextGles::RenderStatesTracker& recordedStates =
	  static_cast<platform::ContextGles&>(device).getCurrentRenderStates();
=======
void StencilCompareOpBack::commitState(IGraphicsContext& device, ComparisonMode cmp)
{
	debugLogApiError("StencilCompareOpBack::commitState enter");
	platform::ContextGles::RenderStatesTracker& recordedStates =
	  native_cast(device).getCurrentRenderStates();
>>>>>>> 1776432f... 4.3
	if (cmp != recordedStates.depthStencil.stencilOpBack)
	{
		gl::StencilFuncSeparate(GL_BACK, nativeGles::ConvertToGles::comparisonMode(cmp), recordedStates.depthStencil.refBack,
		                        recordedStates.depthStencil.readMaskBack);
		recordedStates.depthStencil.stencilOpBack = cmp;
	}
	debugLogApiError("StencilCompareOpBack::commitState exit");
}

GraphicsShaderProgramState::GraphicsShaderProgramState()
{
	_isValid = false;
	_shaderProgram.construct();
}
GraphicsShaderProgramState::GraphicsShaderProgramState(const GraphicsShaderProgramState& shaderState)
{
	_isValid = true;
	_shaderProgram = shaderState._shaderProgram;
}
void GraphicsShaderProgramState::bind(IGraphicsContext& device) const
{
	platform::ContextGles::RenderStatesTracker& stateTracker =  native_cast(device).getCurrentRenderStates();
	if (stateTracker.lastBoundProgram != _shaderProgram->handle)
	{
		gl::UseProgram(_shaderProgram->handle);
		stateTracker.lastBoundProgram = _shaderProgram->handle;
	}
	debugLogApiError("GraphicsShaderProgramState::bind exit");
}

void GraphicsShaderProgramState::destroy()
{
	gl::DeleteProgram(_shaderProgram->handle); _shaderProgram.reset(); _isValid = false;
	debugLogApiError("GraphicsShaderProgramState::destoy exit");
}
void GraphicsShaderProgramState::generate()
{
	if (!_shaderProgram.isValid()) { _shaderProgram.construct(0); }
	_shaderProgram->handle = gl::CreateProgram(); _isValid = true;
	debugLogApiError("GraphicsShaderProgramState::generate exit");
}

bool GraphicsShaderProgramState::saveProgramBinary(Stream& outFile)
{
#if (!defined(BUILD_API_MAX)||(BUILD_API_MAX>=30))
	// validate the program
	GLint linked;
	gl::GetProgramiv(_shaderProgram->handle, GL_LINK_STATUS, &linked);
	if (!linked) { return false; }

	// get the length of the shader binary program in memory.
	GLsizei length = 0;
	gl::GetProgramiv(_shaderProgram->handle, GL_PROGRAM_BINARY_LENGTH, &length);

	// No binary?
	if (length == 0) { return false; }

	std::vector<byte> shaderBinary;
	shaderBinary.resize(length);

	GLenum binaryFmt = 0;
	GLsizei lengthWritten = 0;
	gl::GetProgramBinary(_shaderProgram->handle, length, &lengthWritten, &binaryFmt, &shaderBinary[0]);

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
	if (!_shaderProgram.isValid()) { _shaderProgram.construct(0); }
	_shaderProgram->handle = gl::CreateProgram(); _isValid = true;
	debugLogApiError("ComputeShaderProgramState::generate exit");
}
void ComputeShaderProgramState::bind(IGraphicsContext& device)
{
	platform::ContextGles::RenderStatesTracker& stateTracker =  native_cast(device).getCurrentRenderStates();
	if (stateTracker.lastBoundProgram != _shaderProgram->handle)
	{
		gl::UseProgram(_shaderProgram->handle);
		stateTracker.lastBoundProgram = _shaderProgram->handle;
	}
	debugLogApiError("ComputeShaderProgramState::bind exit");
}

}// namespace impl
}// namespace api
}// namespace pvr
