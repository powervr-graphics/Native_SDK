/*!
\brief Contains implementations of functions for the UIRenderer class.
\file PVREngineUtils/UIRenderer.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVREngineUtils/UIRenderer.h"
#include "PVRCore/IO/BufferStream.h"
#include "PVREngineUtils/AssetUtils.h"
#include "PVREngineUtils/ArialBoldFont.h"
#include "PVREngineUtils/PowerVRLogo.h"
#include "PVREngineUtils/UIRendererShaders_ES.h"
#include "PVREngineUtils/UIRendererShader_vk.vsh.h"
#include "PVREngineUtils/UIRendererShader_vk.fsh.h"

using std::map;
using std::vector;
const pvr::uint32 MaxDescUbo = 200;
const pvr::uint32 MaxCombinedImageSampler = 200;
namespace pvr {
using namespace types;
namespace ui {
const glm::vec2 BaseScreenDim(640, 480);
namespace{

enum class MaterialBufferElement
{
	UVMtx,
	Color,
	AlphaMode
};

enum class UboDescSetBindingId
{
	MVP,
	Material
};

}
Result UIRenderer::init_CreatePipelineAndRenderPass()
{
	debug_assertion(_context.isValid() ,  "NULL Context");
	api::GraphicsPipelineCreateParam pipelineDesc;
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_texDescLayout);

	if (!_uboMvpDescLayout.isNull())
	{
		pipeLayoutInfo.addDescSetLayout(_uboMvpDescLayout);
	}
	if (!_uboMaterialLayout.isNull())
	{
		pipeLayoutInfo.addDescSetLayout(_uboMaterialLayout);
	}

	_pipelineLayout = _context->createPipelineLayout(pipeLayoutInfo);
	if (!_pipelineLayout.isValid())
	{
		Log(Log.Critical, "UIRenderer PipelinelineLayout could not be created.");
		return Result::UnknownError;
	}
	pipelineDesc.pipelineLayout = _pipelineLayout;
	// Text_ pipe
	api::Shader vs;
	api::Shader fs;
	switch (getContext()->getApiType())
	{
	case pvr::Api::OpenGLES2:
	case pvr::Api::OpenGLES3:
	case pvr::Api::OpenGLES31:
		vs = _context->createShader(BufferStream("", _print3DShader_glsles200_vsh, _print3DShader_glsles200_vsh_size),
		                            ShaderType::VertexShader);

		fs = _context->createShader(BufferStream("", _print3DShader_glsles200_fsh, _print3DShader_glsles200_fsh_size),
		                            ShaderType::FragmentShader);
		break;
	case pvr::Api::Vulkan:
		vs = _context->createShader(BufferStream("", spv_UIRendererShader_vk_vsh, sizeof(spv_UIRendererShader_vk_vsh)),
		                            ShaderType::VertexShader);

		fs = _context->createShader(BufferStream("", spv_UIRendererShader_vk_fsh, sizeof(spv_UIRendererShader_vk_fsh)),
		                            ShaderType::FragmentShader);
		break;
	// Suppress the warning
	case pvr::Api::Unspecified:
	case pvr::Api::Count:
		debug_assertion(false, "Invalid Api");
		break;
	}
	if (vs.isNull() || fs.isNull())
	{
		Log(Log.Critical, "UIRenderer shaders could not be created.");
		return Result::UnknownError;
	}
	pipelineDesc.vertexShader.setShader(vs);
	pipelineDesc.fragmentShader.setShader(fs);
	pipelineDesc.es2TextureBindings.setTextureUnit(0, "fontTexture");
	api::VertexAttributeInfo posAttrib(0, DataType::Float32, 4, 0, "myVertex");
	api::VertexAttributeInfo texAttrib(1, DataType::Float32, 2, sizeof(float32) * 4, "myUV");
	pipelineDesc.vertexInput
	.setInputBinding(0, sizeof(float32) * 6, StepRate::Vertex)
	.addVertexAttribute(0, posAttrib)
	.addVertexAttribute(0, texAttrib);

	types::BlendingConfig attachmentState(true, BlendFactor::SrcAlpha,
	                                      BlendFactor::OneMinusSrcAlpha, BlendOp::Add, ColorChannel::All);
	pipelineDesc.colorBlend.setAttachmentState(0, attachmentState);
	pipelineDesc.depthStencil.setDepthTestEnable(false).setDepthWrite(false);
	pipelineDesc.rasterizer.setCullFace(Face::None);
	pipelineDesc.inputAssembler.setPrimitiveTopology(PrimitiveTopology::TriangleList);
	pipelineDesc.renderPass = _renderpass;

	pipelineDesc.multiSample
	.enableState(_renderpass->getCreateParam().getNumRasterizationSamples() != types::SampleCount::Count1)
	.setNumRasterizationSamples(_renderpass->getCreateParam().getNumRasterizationSamples());

	pipelineDesc.subPass = _subpass;
	_pipeline = _context->createParentableGraphicsPipeline(pipelineDesc);
	if (_pipeline.isNull())
	{
		Log(Log.Critical, "UIRenderer pipeline not be created.");
		return Result::UnknownError;
	}
	if (getContext()->getApiType() <= pvr::Api::OpenGLESMaxVersion)
	{
		const char8* attributes[] = { "myVertex", "myUV" };
		const char8* textProgramUni[] = { "myMVPMatrix", "fontTexture", "varColor", "alphaMode", "myUVMatrix" };
		_pipeline->getAttributeLocation(attributes, 2, _programData.attributes);
		_pipeline->getUniformLocation(textProgramUni, sizeof(textProgramUni) / sizeof(textProgramUni[0]), _programData.uniforms);
	}
	return Result::Success;
}

pvr::Result UIRenderer::init_CreateDescriptorSetLayout()
{
	assertion(_context.isValid() , "NULL GRAPHICS CONTEXT");
	api::DescriptorPoolCreateParam descPoolInfo;
	descPoolInfo.addDescriptorInfo(types::DescriptorType::CombinedImageSampler, MaxCombinedImageSampler);
	descPoolInfo.setMaxDescriptorSets(MaxCombinedImageSampler);
	if(getContext()->getApiType() > Api::OpenGLESMaxVersion)
	{
		descPoolInfo.addDescriptorInfo(types::DescriptorType::UniformBufferDynamic, MaxDescUbo);
		descPoolInfo.setMaxDescriptorSets(descPoolInfo.getMaxSetCount() + MaxDescUbo);
	}
	_descPool = getContext()->createDescriptorPool(descPoolInfo);
	if (!_descPool.isValid())
	{
		Log("Failed to create UIRenderer Descriptorpool");
		return Result::UnknownError;
	}


	api::DescriptorSetLayoutCreateParam layoutInfo;

	// CombinedImagesampler Layout
	layoutInfo.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	_texDescLayout = _context->createDescriptorSetLayout(layoutInfo);

	if (_texDescLayout.isNull())
	{
		Log("Failed to create UIRenderer's CombinedImageSampler DescriptorSetLayout" );
		return Result::UnknownError;
	}

	// Mvp ubo Layout
	layoutInfo.clear().setBinding(0, DescriptorType::UniformBufferDynamic, 1, ShaderStageFlags::Vertex);
	_uboMvpDescLayout = _context->createDescriptorSetLayout(layoutInfo);
	if (_uboMvpDescLayout.isNull())
	{
		Log("Failed to create UIRenderer's model-view-projection DescriptorSetLayout" );
		return Result::UnknownError;
	}

	// material ubo layout
	layoutInfo.clear().setBinding(0, DescriptorType::UniformBufferDynamic, 1, ShaderStageFlags::Vertex | ShaderStageFlags::Fragment);
	_uboMaterialLayout = _context->createDescriptorSetLayout(layoutInfo);
	if (_uboMaterialLayout.isNull())
	{
		Log("Failed to create UIRenderer's material DescriptorSetLayout" );
		return Result::UnknownError;
	}

	return Result::Success;
}

Font UIRenderer::createFont(const Texture& tex, const api::Sampler& sampler)
{
	api::TextureView apiTexture = _context->uploadTexture(tex);
	if (apiTexture.isNull())
	{
		Log("UIRenderer::CreateFont::Failed to create font");
	}
	return createFont(apiTexture, tex, sampler);
}

Font UIRenderer::createFont(api::TextureView& texture, const TextureHeader& tex, const api::Sampler& sampler)
{
	Font font;
	font.construct(*this, texture, tex, sampler);
	return font;
}

Image UIRenderer::createImage(const Texture& texture, const api::Sampler& sampler)
{
	auto tex = _context->uploadTexture(texture);
	return createImage(tex, texture.getWidth(), texture.getHeight(), sampler);
}

MatrixGroup UIRenderer::createMatrixGroup()
{
	MatrixGroup group;
	group.construct(*this, generateGroupId());
	group->commitUpdates();
	return group;
}

PixelGroup UIRenderer::createPixelGroup()
{
	PixelGroup group;
	group.construct(*this, generateGroupId());
	group->commitUpdates();
	return group;
}

bool UIRenderer::setUpUboPools(uint32 numInstances, uint32 numSprites)
{
	debug_assertion(numInstances >= numSprites, "Maximum number of instances must be atleast the same as maximum number of sprites");
	// create the ubo descriptor set

		// mvp pool
	if(!_uboMvp.init(getContext(), _uboMvpDescLayout, getDescriptorPool(), numInstances))
	{
		return false;
	}
		// material pool
	if(!_uboMaterial.init(getContext(), _uboMaterialLayout, getDescriptorPool(), numSprites))
	{
		return false;
	}

	return true;
}

Image UIRenderer::createImage(api::TextureView& tex, int32 width, int32 height, const api::Sampler& sampler)
{
	return createImageFromAtlas(tex,Rectanglef(0.0f,0.0f,1.0f,1.0f), width, height, sampler);
}

pvr::ui::Image UIRenderer::createImageFromAtlas(api::TextureView& tex, const Rectanglef& uv,
    uint32 atlasWidth, uint32 atlasHeight, const api::Sampler& sampler)
{
	Image image;
	image.construct(*this, tex, atlasWidth, atlasHeight, sampler);
	if(!image->init()){ image.reset(); return image; }
	// construct the scaling matrix
	// calculate the scale factor
	// convert from texel to normalize coord
	image->setUV(uv);
	image->commitUpdates();
	return image;
}

TextElement UIRenderer::createTextElement(const std::wstring& text, const Font& font)
{
	TextElement spriteText;
	spriteText.construct(*this, text, font);
	return spriteText;
}

Text UIRenderer::createText(const TextElement &textElement)
{
	Text text; text.construct(*this, textElement);
	if(!text->init())
	{
		text.reset();
		return text;
	}
	text->commitUpdates();
	return text;
}

TextElement UIRenderer::createTextElement(const std::string& text, const Font& font)
{
	TextElement spriteText;
	spriteText.construct(*this, text, font);
	return spriteText;
}

bool UIRenderer::init_CreateDefaultSampler()
{
	api::SamplerCreateParam samplerDesc;
	samplerDesc.mipMappingFilter = SamplerFilter::None;
	samplerDesc.minificationFilter = SamplerFilter::Linear;
	samplerDesc.magnificationFilter = SamplerFilter::Linear;
	_samplerBilinear = _context->createSampler(samplerDesc);
	if (_samplerBilinear.isNull())
	{
		Log("UIRenderer initialisation: Failed to create the default bilinear sampler. This should never have happened...");
		return false;
	}
	samplerDesc.mipMappingFilter = SamplerFilter::Linear;
	_samplerTrilinear = _context->createSampler(samplerDesc);
	if (_samplerTrilinear.isNull())
	{
		Log("UIRenderer initialisation: Failed to create the default trilinear sampler. This should never have happened...");
		return false;
	}
	return true;
}

bool UIRenderer::init_CreateDefaultSdkLogo()
{
	Stream::ptr_type sdkLogo = Stream::ptr_type(new BufferStream("", _PowerVR_512x256_RG_pvr, _PowerVR_512x256_RG_pvr_size));
	Texture sdkTex;
	if (assets::textureLoad(sdkLogo, TextureFileFormat::PVR, sdkTex) != Result::Success)
	{
		Log(Log.Warning, "UIRenderer: Could not create the PowerVR SDK Logo.");
		return false;
	}
	sdkTex.setPixelFormat(GeneratePixelType2<'l', 'a', 8, 8>::ID);

	_sdkLogo = createImage(sdkTex);
	if (_sdkLogo.isNull())
	{
		Log(Log.Warning,
		    "UIRenderer initialisation: Could not create the PowerVR SDK Logo. Errors will be gotten if trying to render getSdkLogo().");
		return false;
	}
	_sdkLogo->setAnchor(Anchor::BottomRight, glm::vec2(.98f, -.98f));
	pvr::float32 scalefactor = .3f * getRenderingDim().x / BaseScreenDim.x;

	if (scalefactor > 1) { scalefactor = 1; }
	else if (scalefactor > .5) { scalefactor = .5; }
	else if (scalefactor > .25) { scalefactor = .25; }
	else if (scalefactor > .125) { scalefactor = .125; }
	else { scalefactor = .0625; }

	_sdkLogo->setScale(glm::vec2(scalefactor));
	_sdkLogo->commitUpdates();
	return true;
}

bool UIRenderer::init_CreateDefaultTitle()
{
	_defaultTitle = createText(createTextElement("DefaultTitle", _defaultFont));
	_defaultDescription = createText(createTextElement("",_defaultFont));
	_defaultControls = createText(createTextElement("",_defaultFont));
	if (_defaultTitle.isNull())
	{
		Log(Log.Warning,
		    "UIRenderer initialisation: Could not create the PowerVR Description text. Errors will be gotten if trying to render getDefaultDescription().");
		return false;
	}
	else
	{
		_defaultTitle->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f))->setScale(glm::vec2(.8, .8));
		_defaultTitle->commitUpdates();
	}

	if (_defaultDescription.isNull())
	{
		Log(Log.Warning,
		    "UIRenderer initialisation: Could not create the Demo Description text. Errors will be gotten if trying to render getDefaultDescription().");
		return false;
	}
	else
	{
		_defaultDescription->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f -
		                               _defaultTitle->getFont()->getFontLineSpacing() / (float)getRenderingDimY() * 1.5f))
		->setScale(glm::vec2(.60, .60));
		_defaultDescription->commitUpdates();
	}

	if (_defaultControls.isNull())
	{
		Log(Log.Warning,
		    "UIRenderer initialisation: Could not create the Demo Controls text. Errors will be gotten if trying to render getDefaultControls().");
		return false;
	}
	else
	{
		_defaultControls->setAnchor(Anchor::BottomLeft, glm::vec2(-.98f, -.98f))->setScale(glm::vec2(.5, .5));
		_defaultControls->commitUpdates();
	}
	return true;
}

bool UIRenderer::init_CreateDefaultFont()
{
	Texture fontTex;
	Stream::ptr_type arialFontTex;
	pvr::float32 maxRenderDim = glm::max<pvr::float32>(getRenderingDimX(), getRenderingDimY());
	// pick the right font size of this resolution.
	if (maxRenderDim <= 800)
	{
		arialFontTex = Stream::ptr_type(new BufferStream("", _arialbd_36_pvr, _arialbd_36_pvr_size));
	}
	else if (maxRenderDim <= 1000)
	{
		arialFontTex = Stream::ptr_type(new BufferStream("", _arialbd_46_pvr, _arialbd_46_pvr_size));
	}
	else
	{
		arialFontTex = Stream::ptr_type(new BufferStream("", _arialbd_56_pvr, _arialbd_56_pvr_size));
	}

	if (assets::textureLoad(arialFontTex, TextureFileFormat::PVR, fontTex) != Result::Success)
	{
		Log(Log.Warning, "UIRenderer initialisation: Could not create the default font. Errors will be"
			" gotten if trying to render with getDefaultFont().");
		return false;
	}
	fontTex.setPixelFormat(GeneratePixelType1<'a', 8>::ID);

	_defaultFont = createFont(fontTex);
	if (_defaultFont.isNull())
	{
		Log(Log.Warning, "UIRenderer initialisation: Could not create the default font. Errors will be"
			" gotten if trying to render with getDefaultFont().");
		return false;
	}
	return true;
}


bool UIRenderer::UboMaterial::init(GraphicsContext &context, api::DescriptorSetLayout &descLayout,
    api::DescriptorPool& pool, uint32 numArrayId)
{
	_numArrayId = numArrayId;
	if(_numArrayId && context->getApiType() > Api::OpenGLESMaxVersion)
	{
		// create the descriptor set & buffer if not already created
		_buffer = utils::StructuredMemoryView();
		_buffer.addEntryPacked("uv", types::GpuDatatypes::mat4x4);
		_buffer.addEntryPacked("color", types::GpuDatatypes::vec4);
		_buffer.addEntryPacked("alphaMode", types::GpuDatatypes::integer);

		_buffer.finalize(context, _numArrayId, types::BufferBindingUse::UniformBuffer, true);

		_buffer
			.connectWithBuffer(0, context->createBufferView(context->createBuffer(_buffer.getAlignedTotalSize(),
				pvr::types::BufferBindingUse::UniformBuffer, true), 0, _buffer.getAlignedElementSize()));

		if(!_buffer.getConnectedBuffer(0).isValid())
		{
			Log("Failed to create UIRenderer Material buffer");
			return false;
		}

		if(!_uboDescSetSet.isValid())
		{
			_uboDescSetSet = pool->allocateDescriptorSet(descLayout);
		}
		_uboDescSetSet->update(api::DescriptorSetUpdate().setDynamicUbo(0,_buffer.getConnectedBuffer(0)));
	}
	return true;
}

bool UIRenderer::UboMvp::init(GraphicsContext &context, api::DescriptorSetLayout &descLayout,
    api::DescriptorPool& pool, uint32 numElements)
{
	_numArrayId = numElements;
	if(_numArrayId && context->getApiType() > Api::OpenGLESMaxVersion)
	{
		_buffer = utils::StructuredBufferView();
		_buffer.addEntryPacked("mvp",types::GpuDatatypes::mat4x4);
		_buffer.finalize(context, _numArrayId, types::BufferBindingUse::UniformBuffer, true);
		// create the buffer
		_buffer.connectWithBuffer(0, context->createBufferView(context->createBuffer(_buffer.getAlignedTotalSize(),
		    pvr::types::BufferBindingUse::UniformBuffer, true),0, _buffer.getAlignedElementSize()));
		if(!_buffer.getConnectedBuffer(0).isValid())
		{
			Log("Failed to create UIRenderer Model-view-projection buffer");
			return false;
		}
		if(!_uboDescSetSet.isValid())
		{
			_uboDescSetSet = pool->allocateDescriptorSet(descLayout);
		}
		_uboDescSetSet->update(api::DescriptorSetUpdate().setDynamicUbo(0,_buffer.getConnectedBuffer(0)));
	}
	return true;
}

void UIRenderer::UboMaterial::updateMaterial(uint32 arrayIndex, const glm::vec4& color,
    int32 alphaMode,const glm::mat4& uv)
{
	_buffer.mapArrayIndex(0, arrayIndex);
	_buffer
		.setValue((uint32)MaterialBufferElement::UVMtx, uv)
			.setValue((uint32)MaterialBufferElement::Color, color)
			.setValue((uint32)MaterialBufferElement::AlphaMode, alphaMode, 0);
	_buffer.unmap(0);

}
}// namespace ui
}// namespace pvr
