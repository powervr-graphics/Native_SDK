/*!
\brief OpenGL ES implementations of the EffectApi class.
\file PVRApi/OGLES/EffectApi.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/EffectApi.h"
#include "PVRCore/Maths.h"
#include "PVRCore/StringFunctions.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/GraphicsPipelineGles.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRApi/OGLES/TextureGles.h"
#include "PVRApi/ApiObjects/PipelineLayout.h"
#include "PVRCore/Texture/TextureDefines.h"
#include <functional>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>

using std::vector;
namespace pvr {
using namespace api;
namespace legacyPfx {
namespace impl {

uint32 EffectApi_::loadSemantics(const IGraphicsContext*, bool isAttribute)
{
	uint32 semanticIdx, nCount, nCountUnused;
	int32 nLocation;

	/*
	  Loop over the parameters searching for their semantics. If
	  found/recognised, it should be placed in the output array.
	*/
	nCount = 0;
	nCountUnused = 0;
	IndexedArray<EffectApiSemantic, StringHash>& mySemanticList = isAttribute ? _attributes : _uniforms;
	std::vector<assets::EffectSemantic>& myAssetEffectSemantics = isAttribute ? _assetEffect.attributes : _assetEffect.uniforms;

	GLint boundProgram;
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &boundProgram);
	gl::UseProgram(native_cast(_pipe));

	for (semanticIdx = 0; semanticIdx < myAssetEffectSemantics.size(); ++semanticIdx)
	{
		assets::EffectSemantic& assetSemantic = myAssetEffectSemantics[semanticIdx];
		// Semantic found for this parameter.
		if (isAttribute)
		{
			nLocation = _pipe->getAttributeLocation(assetSemantic.variableName.c_str());
		}
		else
		{
			nLocation = _pipe->getUniformLocation(assetSemantic.variableName.c_str());
		}

		if (nLocation != -1)
		{
			EffectApiSemantic semanticToInsert;
			semanticToInsert.location = nLocation;
			semanticToInsert.semanticIndex = semanticIdx;
			semanticToInsert.variableName = assetSemantic.variableName;
			if (!isAttribute && strings::startsWith(assetSemantic.semantic.str(), "TEXTURE"))
			{
				//IF the string is TEXTUREXXXX, the texture ordinal is the texture unit.
				gl::Uniform1i(nLocation, assetSemantic.semantic.str().length() >= 8 ? atoi(assetSemantic.semantic.c_str() + 7) : 0);
			}
			mySemanticList.insertAt(semanticIdx, assetSemantic.semantic, std::move(semanticToInsert));

			++nCount;
		}
		else
		{
			Log(Log.Warning, "[EffectFile: %s Effect: %s] Variable not used by GLSL code: Semantic:%s VariableName:%s",
			    _assetEffect.fileName.c_str(), _assetEffect.getMaterial().getEffectName().c_str(),
			    assetSemantic.semantic.str().c_str(), assetSemantic.variableName.c_str());
			++nCountUnused;
		}
	}
	gl::UseProgram(boundProgram);
	platform::ContextGles::RenderStatesTracker& stateTracker =  native_cast(*_context).getCurrentRenderStates();
	stateTracker.lastBoundProgram = boundProgram;
	return nCount;
}

void EffectApi_::setTexture(uint32 nIdx, const api::TextureView& tex)
{
	using namespace assets;
	if (nIdx < (uint32)_effectTexSamplers.size())
	{
		if (!tex.isValid()) { return; }// validate
		// Get the texture details from the PFX Parser. This contains details such as mipmapping and filter modes.
<<<<<<< HEAD
		//const StringHash& TexName = m_parser->getEffect(m_effect).textures[nIdx].name;
		//int32 iTexIdx = m_parser->findTextureByName(TexName);
		if (types::ImageBaseType::Image2D != types::imageViewTypeToImageBaseType(m_effectTexSamplers[nIdx].getTextureViewType())) { return; }
		m_effectTexSamplers[nIdx].texture = tex;
=======
		//const StringHash& TexName = _parser->getEffect(_effect).textures[nIdx].name;
		//int32 iTexIdx = _parser->findTextureByName(TexName);
		if (types::ImageBaseType::Image2D != types::imageViewTypeToImageBaseType(_effectTexSamplers[nIdx].getTextureViewType())) { return; }
		_effectTexSamplers[nIdx].texture = tex;
>>>>>>> 1776432f... 4.3
	}
}

void EffectApi_::setDefaultUniformValue(const char8* const pszName, const assets::EffectSemanticData& psDefaultValue)
{
	using namespace assets;
	GLint nLocation = gl::GetUniformLocation(_program.program->handle, pszName);
	switch (psDefaultValue.type)
	{
	case types::SemanticDataType::Mat2:
		gl::UniformMatrix2fv(nLocation, 1, GL_FALSE, psDefaultValue.dataF32);
		break;
	case types::SemanticDataType::Mat3:
		gl::UniformMatrix3fv(nLocation, 1, GL_FALSE, psDefaultValue.dataF32);
		break;
	case types::SemanticDataType::Mat4:
		gl::UniformMatrix4fv(nLocation, 1, GL_FALSE, psDefaultValue.dataF32);
		break;
	case types::SemanticDataType::Vec2:
		gl::Uniform2fv(nLocation, 1, psDefaultValue.dataF32);
		break;
	case types::SemanticDataType::RGB:
	case types::SemanticDataType::Vec3:
		gl::Uniform3fv(nLocation, 1, psDefaultValue.dataF32);
		break;
	case types::SemanticDataType::RGBA:
	case types::SemanticDataType::Vec4:
		gl::Uniform4fv(nLocation, 1, psDefaultValue.dataF32);
		break;
	case types::SemanticDataType::IVec2:
		gl::Uniform2iv(nLocation, 1, psDefaultValue.dataI32);
		break;
	case types::SemanticDataType::IVec3:
		gl::Uniform3iv(nLocation, 1, psDefaultValue.dataI32);
		break;
	case types::SemanticDataType::IVec4:
		gl::Uniform4iv(nLocation, 1, psDefaultValue.dataI32);
		break;
	case types::SemanticDataType::BVec2:
		gl::Uniform2i(nLocation, psDefaultValue.dataBool[0] ? 1 : 0, psDefaultValue.dataBool[1] ? 1 : 0);
		break;
	case types::SemanticDataType::BVec3:
		gl::Uniform3i(nLocation, psDefaultValue.dataBool[0] ? 1 : 0, psDefaultValue.dataBool[1] ? 1 : 0,
		              psDefaultValue.dataBool[2] ? 1 : 0);
		break;
	case types::SemanticDataType::BVec4:

		gl::Uniform4i(nLocation, psDefaultValue.dataBool[0] ? 1 : 0, psDefaultValue.dataBool[1] ? 1 : 0,
		              psDefaultValue.dataBool[2] ? 1 : 0, psDefaultValue.dataBool[3] ? 1 : 0);
		break;
	case types::SemanticDataType::Float:
		gl::Uniform1f(nLocation, psDefaultValue.dataF32[0]);
		break;
	case types::SemanticDataType::Int1:
		gl::Uniform1i(nLocation, psDefaultValue.dataI32[0]);
		break;
	case types::SemanticDataType::Bool1:
		gl::Uniform1i(nLocation, psDefaultValue.dataBool[0] ? 1 : 0);
		break;
	case types::SemanticDataType::Count:
	case types::SemanticDataType::None:
	default:
		break;
	}
}

Result EffectApi_::buildSemanticTables(uint32& uiUnknownSemantics)
{
	loadSemantics(NULL, false);
	loadSemantics(NULL, true);

	return Result::Success;
}

EffectApi_::EffectApi_(const GraphicsContext& context, utils::AssetLoadingDelegate& effectDelegate) :
	_isLoaded(false), _delegate(&effectDelegate), _context(context) {}

<<<<<<< HEAD
EffectApi_::EffectApi_(GraphicsContext& context, AssetLoadingDelegate& effectDelegate) :
	m_isLoaded(false), m_delegate(&effectDelegate), m_context(context) {}

Result EffectApi_::init(const assets::Effect& effect, api::GraphicsPipelineCreateParam& pipeDesc)
{
	uint32	 i;
	m_assetEffect = effect;
=======
Result EffectApi_::init(const assets::Effect& effect, api::GraphicsPipelineCreateParam& pipeDesc)
{
	uint32   i;
	_assetEffect = effect;
>>>>>>> 1776432f... 4.3
	Result pvrRslt;

	//--- Initialize each Texture
	for (i = 0; i < effect.textures.size(); ++i)
	{
		_effectTexSamplers.insertAt(i, effect.textures[i].name, EffectApiTextureSampler());
		if (effect.textures[i].flags & PVRTEX_CUBEMAP)
		{

			_effectTexSamplers[i].texture = _context->createTextureView(_context->createTexture());
		}
		else
		{
			_effectTexSamplers[i].texture = _context->createTextureView(_context->createTexture());
		}

		_effectTexSamplers[i].name = effect.textures[i].name;
		_effectTexSamplers[i].fileName = effect.textures[i].fileName;
		_effectTexSamplers[i].flags = 0;
		_effectTexSamplers[i].unit = effect.textures[i].unit;

		// create the sampler
		assets::SamplerCreateParam samplerDesc;
		samplerDesc.minificationFilter = effect.textures[i].minFilter;
		samplerDesc.magnificationFilter = effect.textures[i].magFilter;
		samplerDesc.mipMappingFilter = effect.textures[i].mipFilter;

		samplerDesc.wrapModeU = effect.textures[i].wrapS;
		samplerDesc.wrapModeV = effect.textures[i].wrapT;
		samplerDesc.wrapModeW = effect.textures[i].wrapR;
		_effectTexSamplers[i].sampler = _context->createSampler(samplerDesc);
	}

	//--- register the custom semantics and load the requested textures
	if ((pvrRslt = loadTexturesForEffect()) != Result::Success) { return pvrRslt; }

	if (pipeDesc.pipelineLayout.isNull())
	{
		//--- create the descriptor set layout and pipeline layout
		pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
<<<<<<< HEAD
		for (pvr::uint8 ii = 0; ii < static_cast<pvr::uint8>(m_effectTexSamplers.size()); ++ii)
=======
		for (pvr::uint8 ii = 0; ii < static_cast<pvr::uint8>(_effectTexSamplers.size()); ++ii)
>>>>>>> 1776432f... 4.3
		{
			descSetLayoutInfo.setBinding(ii,  types::DescriptorType::CombinedImageSampler,
			                             0, types::ShaderStageFlags::Fragment);
		}
		_descriptorSetLayout = _context->createDescriptorSetLayout(descSetLayoutInfo);
		PipelineLayoutCreateParam pipeLayoutCreateInfo;
		pipeLayoutCreateInfo.addDescSetLayout(_descriptorSetLayout);
		pipeDesc.pipelineLayout = _context->createPipelineLayout(pipeLayoutCreateInfo);
	}

	//--- create the descriptor set
	pvr::api::DescriptorSetUpdate descriptorSetInfo;
<<<<<<< HEAD
	for (pvr::uint8 ii = 0; ii < static_cast<pvr::uint8>(m_effectTexSamplers.size()); ++ii)
	{
		descriptorSetInfo.setCombinedImageSamplerAtIndex(ii, m_effectTexSamplers[ii].unit,
		    m_effectTexSamplers[ii].texture, m_effectTexSamplers[ii].sampler);
=======
	for (pvr::uint8 ii = 0; ii < static_cast<pvr::uint8>(_effectTexSamplers.size()); ++ii)
	{
		descriptorSetInfo.setCombinedImageSamplerAtIndex(ii, _effectTexSamplers[ii].unit,
		    _effectTexSamplers[ii].texture, _effectTexSamplers[ii].sampler);
>>>>>>> 1776432f... 4.3
	}
	if (_effectTexSamplers.size())
	{
		_descriptorSet = _context->createDescriptorSetOnDefaultPool(_descriptorSetLayout);
		if (_descriptorSet->update(descriptorSetInfo))
		{
			Log("DescriptorSet update failed");
			return pvr::Result::UnknownError;
		}
	}
	//--- construct the pipeline
	api::Shader vertShader;
	api::Shader fragmentShader;

	//--- Load the shaders
	pvrRslt = loadShadersForEffect(vertShader, fragmentShader);
	if (pvrRslt != Result::Success) { return pvrRslt; }
	pipeDesc.vertexShader.setShader(vertShader);
	pipeDesc.fragmentShader.setShader(fragmentShader);

	//--- create and validate pipeline
	_pipe = _context->createParentableGraphicsPipeline(pipeDesc);
	if (!_pipe.isValid()) { return Result::NotInitialized; }

//--- Build uniform table
	pvrRslt = buildSemanticTables(_numUnknownUniforms);
	if (pvrRslt != Result::Success) { return pvrRslt; }

	_isLoaded = true;
	return pvrRslt;
}

Result EffectApi_::loadTexturesForEffect()
{
	Result pvrRslt = Result::Success;

	for (IndexedArray<EffectApiTextureSampler>::iterator it = _effectTexSamplers.begin(); it != _effectTexSamplers.end(); ++it)
	{
		pvrRslt = it->value.init(*_delegate);
		if (pvrRslt != Result::Success) { return pvrRslt; }
	}
	return Result::Success;
}

void EffectApi_::destroy()
{
	_effectTexSamplers.clear();
	_isLoaded = false;
}

Result EffectApi_::loadShadersForEffect(api::Shader& vertexShader, api::Shader& fragmentShader)
{
	using namespace assets;
	// initialize attributes to default values
	string vertShaderSrc;
	string fragShaderSrc;

	bool isVertShaderBinary = _assetEffect.vertexShader.glslBinFile.length() != 0;
	bool isFragShaderBinary = _assetEffect.fragmentShader.glslBinFile.length() != 0;

	types::ShaderBinaryFormat vertShaderBinFmt =  types::ShaderBinaryFormat::None;
	types::ShaderBinaryFormat fragShaderBinFmt =  types::ShaderBinaryFormat::None;

	// create vertex shader stream from source/ binary.
	BufferStream vertexShaderData(
	  (isVertShaderBinary ? _assetEffect.vertexShader.glslBinFile.c_str() :
	   _assetEffect.vertexShader.glslFile.c_str()),
	  (isVertShaderBinary ? _assetEffect.vertexShader.glslBin.c_str() : _assetEffect.vertexShader.glslCode.c_str()),
	  (isVertShaderBinary ? _assetEffect.vertexShader.glslBin.length() : _assetEffect.vertexShader.glslCode.length()));

	// create fragment shader stream from source/ binary.
	BufferStream fragmentShaderData(
	  (isFragShaderBinary ? _assetEffect.fragmentShader.glslBinFile : _assetEffect.fragmentShader.glslFile.c_str()),
	  (isFragShaderBinary ? _assetEffect.fragmentShader.glslBin.c_str() :
	   _assetEffect.fragmentShader.glslCode.c_str()),
	  (isFragShaderBinary ? _assetEffect.fragmentShader.glslBin.length() :
	   _assetEffect.fragmentShader.glslCode.length()));

	if (vertexShaderData.getSize() == 0)
	{
		Log(Log.Error, "Effect File: [%s] -- Could not find vertex shader [%s] when processing effect [%s]",
		    _assetEffect.fileName.c_str(), _assetEffect.vertexShader.name.c_str(), _assetEffect.material.getEffectName().c_str());
	}
	if (fragmentShaderData.getSize() == 0)
	{
		Log(Log.Error, "Effect File: [%s] -- Could not find fragment shader [%s]  when processing effect [%s]",
		    _assetEffect.fileName.c_str(), _assetEffect.fragmentShader.name.c_str(), _assetEffect.material.getEffectName().c_str());
	}

#if defined(GL_SGX_BINARY_IMG)
	if (isVertShaderBinary) { vertShaderBinFmt =  types::ShaderBinaryFormat::ImgSgx; }
	if (isFragShaderBinary) { fragShaderBinFmt =  types::ShaderBinaryFormat::ImgSgx; }
#endif
	// load the vertex and fragment shader

	if (vertShaderBinFmt !=  types::ShaderBinaryFormat::None)
	{
		vertexShader = _context->createShader(vertexShaderData,  types::ShaderType::VertexShader, vertShaderBinFmt);
	}
	else // not a binary format so load it as normally
	{
		vertexShader = _context->createShader(vertexShaderData,  types::ShaderType::VertexShader, 0, 0);
	}

	if (fragShaderBinFmt !=  types::ShaderBinaryFormat::None)
	{
		fragmentShader = _context->createShader(fragmentShaderData,  types::ShaderType::FragmentShader, fragShaderBinFmt);
	}
	else // not a binary format so load it as normally
	{
		fragmentShader = _context->createShader(fragmentShaderData,  types::ShaderType::FragmentShader, 0, 0);
	}

	if (vertexShader.isNull())
	{
		Log(Log.Error, "Effect File: [%s] -- Vertex Shader [%s] compilation error when processing effect [%s]",
		    _assetEffect.fileName.c_str(), _assetEffect.vertexShader.name.c_str(), _assetEffect.material.getEffectName().c_str());
	}
	if (fragmentShader.isNull())
	{
		Log(Log.Error, "Effect File: [%s] -- Fragment Shader [%s] compilation error when processing effect [%s]",
		    _assetEffect.fileName.c_str(), _assetEffect.fragmentShader.name.c_str(), _assetEffect.material.getEffectName().c_str());
	}
	return ((vertexShader.isValid() && fragmentShader.isValid()) ? Result::Success : Result::UnknownError);
}

void EffectApi_::setSampler(uint32 index, api::Sampler sampler)
{
	if (!sampler.isValid()) { return; }
	_effectTexSamplers[index].sampler = sampler;
}

}// namespace gles
}// namespace api
}// namespace pvr
