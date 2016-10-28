/*!*********************************************************************************************************************
\file         PVRApi\OGLES\EffectApi.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES implementations of the EffectApi class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/EffectApi.h"
#include "PVRCore/Maths.h"
#include "PVRCore/IGraphicsContext.h"
#include "PVRCore/StringFunctions.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRApi/ApiObjects/DescriptorSet.h"
#include "PVRApi/ApiObjects/PipelineLayout.h"
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
	IndexedArray<EffectApiSemantic, StringHash>& mySemanticList = isAttribute ? m_attributes : m_uniforms;
	std::vector<assets::EffectSemantic>& myAssetEffectSemantics = isAttribute ? m_assetEffect.attributes : m_assetEffect.uniforms;

	GLint boundProgram;
	gl::GetIntegerv(GL_CURRENT_PROGRAM, &boundProgram);
	gl::UseProgram(m_pipe->getNativeObject());

	for (semanticIdx = 0; semanticIdx < myAssetEffectSemantics.size(); ++semanticIdx)
	{
		assets::EffectSemantic& assetSemantic = myAssetEffectSemantics[semanticIdx];
		// Semantic found for this parameter.
		if (isAttribute)
		{
			nLocation = m_pipe->getAttributeLocation(assetSemantic.variableName.c_str());
		}
		else
		{
			nLocation = m_pipe->getUniformLocation(assetSemantic.variableName.c_str());
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
			    m_assetEffect.fileName.c_str(), m_assetEffect.getMaterial().getEffectName().c_str(),
			    assetSemantic.semantic.str().c_str(), assetSemantic.variableName.c_str());
			++nCountUnused;
		}
	}
	gl::UseProgram(boundProgram);

	return nCount;
}

void EffectApi_::setTexture(uint32 nIdx, const api::TextureView& tex)
{
	using namespace assets;
	if (nIdx < (uint32)m_effectTexSamplers.size())
	{
		if (!tex.isValid()) { return; }// validate
		// Get the texture details from the PFX Parser. This contains details such as mipmapping and filter modes.
		//const StringHash& TexName = m_parser->getEffect(m_effect).textures[nIdx].name;
		//int32 iTexIdx = m_parser->findTextureByName(TexName);
		if (types::ImageBaseType::Image2D != types::imageViewTypeToImageBaseType(m_effectTexSamplers[nIdx].getTextureViewType())) { return; }
		m_effectTexSamplers[nIdx].texture = tex;
	}
}

void EffectApi_::setDefaultUniformValue(const char8* const pszName, const assets::EffectSemanticData& psDefaultValue)
{
	using namespace assets;
	GLint nLocation = gl::GetUniformLocation(m_program.program->handle, pszName);
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


EffectApi_::EffectApi_(GraphicsContext& context, AssetLoadingDelegate& effectDelegate) :
	m_isLoaded(false), m_delegate(&effectDelegate), m_context(context) {}

Result EffectApi_::init(const assets::Effect& effect, api::GraphicsPipelineCreateParam& pipeDesc)
{
	uint32	 i;
	m_assetEffect = effect;
	Result pvrRslt;

	//--- Initialize each Texture
	for (i = 0; i < effect.textures.size(); ++i)
	{
		m_effectTexSamplers.insertAt(i, effect.textures[i].name, EffectApiTextureSampler());
		if (effect.textures[i].flags & assets::PVRTEX_CUBEMAP)
		{

			m_effectTexSamplers[i].texture.construct(m_context->createTexture());
		}
		else
		{
			api::TextureStore tex2d = m_context->createTexture();
			m_effectTexSamplers[i].texture.construct(tex2d);
		}

		m_effectTexSamplers[i].name = effect.textures[i].name;
		m_effectTexSamplers[i].fileName = effect.textures[i].fileName;
		m_effectTexSamplers[i].flags = 0;
		m_effectTexSamplers[i].unit = effect.textures[i].unit;

		// create the sampler
		assets::SamplerCreateParam samplerDesc;
		samplerDesc.minificationFilter = effect.textures[i].minFilter;
		samplerDesc.magnificationFilter = effect.textures[i].magFilter;
		samplerDesc.mipMappingFilter = effect.textures[i].mipFilter;

		samplerDesc.wrapModeU = effect.textures[i].wrapS;
		samplerDesc.wrapModeV = effect.textures[i].wrapT;
		samplerDesc.wrapModeW = effect.textures[i].wrapR;
		m_effectTexSamplers[i].sampler = m_context->createSampler(samplerDesc);
	}

	//--- register the custom semantics and load the requested textures
	if ((pvrRslt = loadTexturesForEffect()) != Result::Success) { return pvrRslt; }

	if (pipeDesc.pipelineLayout.isNull())
	{
		//--- create the descriptor set layout and pipeline layout
		pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
		for (pvr::uint8 ii = 0; ii < static_cast<pvr::uint8>(m_effectTexSamplers.size()); ++ii)
		{
			descSetLayoutInfo.setBinding(ii,  types::DescriptorType::CombinedImageSampler,
			                             0, types::ShaderStageFlags::Fragment);
		}
		m_descriptorSetLayout = m_context->createDescriptorSetLayout(descSetLayoutInfo);
		PipelineLayoutCreateParam pipeLayoutCreateInfo;
		pipeLayoutCreateInfo.addDescSetLayout(m_descriptorSetLayout);
		pipeDesc.pipelineLayout = m_context->createPipelineLayout(pipeLayoutCreateInfo);
	}

	//--- create the descriptor set
	pvr::api::DescriptorSetUpdate descriptorSetInfo;
	for (pvr::uint8 ii = 0; ii < static_cast<pvr::uint8>(m_effectTexSamplers.size()); ++ii)
	{
		descriptorSetInfo.setCombinedImageSamplerAtIndex(ii, m_effectTexSamplers[ii].unit,
		    m_effectTexSamplers[ii].texture, m_effectTexSamplers[ii].sampler);
	}
	if (m_effectTexSamplers.size())
	{
		m_descriptorSet = m_context->createDescriptorSetOnDefaultPool(m_descriptorSetLayout);
		if (m_descriptorSet->update(descriptorSetInfo))
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
	m_pipe = m_context->createParentableGraphicsPipeline(pipeDesc);
	if (!m_pipe.isValid()) { return Result::NotInitialized; }

//--- Build uniform table
	pvrRslt = buildSemanticTables(m_numUnknownUniforms);
	if (pvrRslt != Result::Success) { return pvrRslt; }

	m_isLoaded = true;
	return pvrRslt;
}

Result EffectApi_::loadTexturesForEffect()
{
	Result pvrRslt = Result::Success;

	for (IndexedArray<EffectApiTextureSampler>::iterator it = m_effectTexSamplers.begin(); it != m_effectTexSamplers.end(); ++it)
	{
		pvrRslt = it->value.init(*m_delegate);
		if (pvrRslt != Result::Success) { return pvrRslt; }
	}
	return Result::Success;
}

void EffectApi_::destroy()
{
	m_effectTexSamplers.clear();
	m_isLoaded = false;
}

Result EffectApi_::loadShadersForEffect(api::Shader& vertexShader, api::Shader& fragmentShader)
{
	using namespace assets;
	// initialize attributes to default values
	string vertShaderSrc;
	string fragShaderSrc;

	bool isVertShaderBinary = m_assetEffect.vertexShader.glslBinFile.length() != 0;
	bool isFragShaderBinary = m_assetEffect.fragmentShader.glslBinFile.length() != 0;

	types::ShaderBinaryFormat vertShaderBinFmt =  types::ShaderBinaryFormat::None;
	types::ShaderBinaryFormat fragShaderBinFmt =  types::ShaderBinaryFormat::None;

	// create vertex shader stream from source/ binary.
	BufferStream vertexShaderData(
	  (isVertShaderBinary ? m_assetEffect.vertexShader.glslBinFile.c_str() :
	   m_assetEffect.vertexShader.glslFile.c_str()),
	  (isVertShaderBinary ? m_assetEffect.vertexShader.glslBin.c_str() : m_assetEffect.vertexShader.glslCode.c_str()),
	  (isVertShaderBinary ? m_assetEffect.vertexShader.glslBin.length() : m_assetEffect.vertexShader.glslCode.length()));

	// create fragment shader stream from source/ binary.
	BufferStream fragmentShaderData(
	  (isFragShaderBinary ? m_assetEffect.fragmentShader.glslBinFile : m_assetEffect.fragmentShader.glslFile.c_str()),
	  (isFragShaderBinary ? m_assetEffect.fragmentShader.glslBin.c_str() :
	   m_assetEffect.fragmentShader.glslCode.c_str()),
	  (isFragShaderBinary ? m_assetEffect.fragmentShader.glslBin.length() :
	   m_assetEffect.fragmentShader.glslCode.length()));

	if (vertexShaderData.getSize() == 0)
	{
		Log(Log.Error, "Effect File: [%s] -- Could not find vertex shader [%s] when processing effect [%s]",
		    m_assetEffect.fileName.c_str(), m_assetEffect.vertexShader.name.c_str(), m_assetEffect.material.getEffectName().c_str());
	}
	if (fragmentShaderData.getSize() == 0)
	{
		Log(Log.Error, "Effect File: [%s] -- Could not find fragment shader [%s]  when processing effect [%s]",
		    m_assetEffect.fileName.c_str(), m_assetEffect.fragmentShader.name.c_str(), m_assetEffect.material.getEffectName().c_str());
	}

#if defined(GL_SGX_BINARY_IMG)
	if (isVertShaderBinary)	{ vertShaderBinFmt =  types::ShaderBinaryFormat::ImgSgx; }
	if (isFragShaderBinary)	{ fragShaderBinFmt =  types::ShaderBinaryFormat::ImgSgx; }
#endif
	// load the vertex and fragment shader

	if (vertShaderBinFmt !=  types::ShaderBinaryFormat::None)
	{
		vertexShader = m_context->createShader(vertexShaderData,  types::ShaderType::VertexShader, vertShaderBinFmt);
	}
	else // not a binary format so load it as normally
	{
		vertexShader = m_context->createShader(vertexShaderData,  types::ShaderType::VertexShader, 0, 0);
	}

	if (fragShaderBinFmt !=  types::ShaderBinaryFormat::None)
	{
		fragmentShader = m_context->createShader(fragmentShaderData,  types::ShaderType::FragmentShader, fragShaderBinFmt);
	}
	else // not a binary format so load it as normally
	{
		fragmentShader = m_context->createShader(fragmentShaderData,  types::ShaderType::FragmentShader, 0, 0);
	}

	if (vertexShader.isNull())
	{
		Log(Log.Error, "Effect File: [%s] -- Vertex Shader [%s] compilation error when processing effect [%s]",
		    m_assetEffect.fileName.c_str(), m_assetEffect.vertexShader.name.c_str(), m_assetEffect.material.getEffectName().c_str());
	}
	if (fragmentShader.isNull())
	{
		Log(Log.Error, "Effect File: [%s] -- Fragment Shader [%s] compilation error when processing effect [%s]",
		    m_assetEffect.fileName.c_str(), m_assetEffect.fragmentShader.name.c_str(), m_assetEffect.material.getEffectName().c_str());
	}
	return ((vertexShader.isValid() && fragmentShader.isValid()) ? Result::Success : Result::UnknownError);
}

void EffectApi_::setSampler(uint32 index, api::Sampler sampler)
{
	if (!sampler.isValid()) { return; }
	m_effectTexSamplers[index].sampler = sampler;
}

}// namespace gles
}// namespace api
}// namespace pvr
//!\endcond
