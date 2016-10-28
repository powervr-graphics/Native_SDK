/*!*********************************************************************************************************************
\file         PVRUIRenderer\UIRenderer.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Contains implementations of functions for the UIRenderer class.
***********************************************************************************************************************/
#include "PVRUIRenderer/UIRenderer.h"
#include "PVRCore/BufferStream.h"
#include "PVRApi/TextureUtils.h"
#include "PVRUIRenderer/ArialBoldFont.h"
#include "PVRUIRenderer/PowerVRLogo.h"
#include "PVRUIRenderer/UIRendererShaders_ES.h"
#include "PVRUIRenderer/UIRendererShader_vk.vsh.h"
#include "PVRUIRenderer/UIRendererShader_vk.fsh.h"
using std::map;
using std::vector;
const pvr::uint32 MaxDescUbo = 200;
const pvr::uint32 MaxCombinedImageSampler = 200;
namespace pvr {
using namespace types;
namespace ui {
const glm::vec2 BaseScreenDim(640, 480);
Result UIRenderer::init_CreatePipelineAndRenderPass()
{
	assertion(m_context.isValid() ,  "NULL Context");
	api::GraphicsPipelineCreateParam pipelineDesc;
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(m_texDescLayout);
	if (!m_uboDescLayout.isNull())
	{
		pipeLayoutInfo.addDescSetLayout(m_uboDescLayout);
	}
	m_pipelineLayout = m_context->createPipelineLayout(pipeLayoutInfo);
	if (!m_pipelineLayout.isValid())
	{
		Log(Log.Critical, "UIRenderer PipelinelineLayout could not be created.");
		return Result::UnknownError;
	}
	pipelineDesc.pipelineLayout = m_pipelineLayout;
	// Text_ pipe
	api::Shader vs;
	api::Shader fs;
	switch (getContext()->getApiType())
	{
	case pvr::Api::OpenGLES2:
	case pvr::Api::OpenGLES3:
	case pvr::Api::OpenGLES31:
		vs = m_context->createShader(BufferStream("", _print3DShader_glsles200_vsh, _print3DShader_glsles200_vsh_size),
		                             ShaderType::VertexShader);

		fs = m_context->createShader(BufferStream("", _print3DShader_glsles200_fsh, _print3DShader_glsles200_fsh_size),
		                             ShaderType::FragmentShader);
		break;
	case pvr::Api::Vulkan:
		vs = m_context->createShader(BufferStream("", spv_UIRendererShader_vk_vsh, sizeof(spv_UIRendererShader_vk_vsh)),
		                             ShaderType::VertexShader);

		fs = m_context->createShader(BufferStream("", spv_UIRendererShader_vk_fsh, sizeof(spv_UIRendererShader_vk_fsh)),
		                             ShaderType::FragmentShader);
		break;
	// Suppress the warning
	case pvr::Api::Unspecified:
	case pvr::Api::Count:
		assertion(false, "Invalid Api");
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
	pipelineDesc.renderPass = m_renderpass;
	pipelineDesc.subPass = m_subpass;
	m_pipeline = m_context->createParentableGraphicsPipeline(pipelineDesc);
	if (m_pipeline.isNull())
	{
		Log(Log.Critical, "UIRenderer pipeline not be created.");
		return Result::UnknownError;
	}
	if (getContext()->getApiType() <= pvr::Api::OpenGLESMaxVersion)
	{
		const char8* attributes[] = { "myVertex", "myUV" };
		if (api::logApiError("UIRenderer::createPipelineAndRenderPass createGraphicsPipeline"))
		{
			return Result::UnknownError;
		}
		const char8* textProgramUni[] = { "myMVPMatrix", "fontTexture", "varColor", "alphaMode", "myUVMatrix" };
		if (api::logApiError("UIRenderer::createPipelineAndRenderPass setUniforms"))
		{
			return Result::UnknownError;
		}

		m_pipeline->getAttributeLocation(attributes, 2, m_programData.attributes);
		m_pipeline->getUniformLocation(textProgramUni, sizeof(textProgramUni) / sizeof(textProgramUni[0]), m_programData.uniforms);
		if (api::logApiError("UIRenderer::createPipelineAndRenderPass getUniformLocation"))
		{
			return Result::UnknownError;
		}
	}
	return Result::Success;
}

pvr::Result UIRenderer::init_CreateDescriptorSetLayout()
{
	assertion(m_context.isValid() ,  "NULL GRAPHICS CONTEXT");

	m_descPool = getContext()->createDescriptorPool(api::DescriptorPoolCreateParam()
	             .addDescriptorInfo(types::DescriptorType::UniformBuffer, MaxDescUbo)
	             .addDescriptorInfo(types::DescriptorType::CombinedImageSampler, MaxCombinedImageSampler)
	             .setMaxDescriptorSets(MaxDescUbo + MaxCombinedImageSampler));

	if (!m_descPool.isValid())
	{
		Log("Failed to create UIRenderer Descriptorpool");
		return pvr::Result::UnknownError;
	}

	api::DescriptorSetLayoutCreateParam defaultDesc;
	defaultDesc.setBinding(0, DescriptorType::CombinedImageSampler, 1, ShaderStageFlags::Fragment);
	m_texDescLayout = m_context->createDescriptorSetLayout(defaultDesc);
	if (m_context->getApiType() > Api::OpenGLESMaxVersion)// use uniform buffer
	{
		api::DescriptorSetLayoutCreateParam uboDesc;
		uboDesc.setBinding(0, DescriptorType::UniformBuffer, 1, ShaderStageFlags::Vertex | ShaderStageFlags::Fragment);
		m_uboDescLayout = m_context->createDescriptorSetLayout(uboDesc);
		if (m_uboDescLayout.isNull()) { return Result::UnknownError; }
	}
	if (m_texDescLayout.isNull()) { return Result::UnknownError; }
	return Result::Success;
}

Font UIRenderer::createFont(const assets::Texture& tex)
{
	Font font;
	api::TextureView apiTexture;
	if (utils::textureUpload(m_context, tex, apiTexture) != Result::Success)
	{
		Log("UIRenderer::CreateFont::Failed to create font");
	}
	font.construct(*this, apiTexture, tex);
	font->setSampler(m_samplerBilinear);
	font->commitUpdates();
	return font;
}

Font UIRenderer::createFont(api::TextureView& texture, const assets::TextureHeader& tex)
{
	Font font;
	font.construct(*this, texture, tex);
	font->setSampler(m_samplerBilinear);
	font->commitUpdates();
	return font;
}

Image UIRenderer::createImage(const assets::Texture& texture)
{
	Image image;
	api::TextureView apiTexture;
	utils::textureUpload(m_context, texture, apiTexture);
	return createImage(apiTexture, texture.getWidth(), texture.getHeight());
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

Image UIRenderer::createImage(api::TextureView& tex, int32 width, int32 height)
{
	Image image;
	image.construct(*this, tex, width, height);
	image->setSampler(tex->getResource()->getFormat().mipmapLevels > 1 ? m_samplerTrilinear : m_samplerBilinear);
	image->commitUpdates();
	return image;
}

pvr::ui::Image UIRenderer::createImageFromAtlas(api::TextureView& tex, const Rectanglef& uv,
    uint32 atlasWidth, uint32 atlasHeight)
{
	Image image;
	image.construct(*this, tex, atlasWidth, atlasHeight);
	image->setSampler(tex->getResource()->getFormat().mipmapLevels > 1 ? m_samplerTrilinear : m_samplerBilinear);


	// construct the scaling matrix
	// calculate the scale factor
	// convert from texel to normalize coord
	image->setUV(uv);
	image->commitUpdates();
	return image;
}


Text UIRenderer::createText(const std::wstring& text, const Font& font)
{
	Text spriteText;
	spriteText.construct(*this, text, font);
	spriteText->commitUpdates();
	return spriteText;
}

Text UIRenderer::createText(const std::string& text, const Font& font)
{
	Text spriteText;
	spriteText.construct(*this, text, font);
	spriteText->commitUpdates();
	return spriteText;
}
Text UIRenderer::createText(const Font& font)
{
	Text spriteText;
	spriteText.construct(*this, font);
	spriteText->commitUpdates();
	return spriteText;
}

bool UIRenderer::init_CreateDefaultSampler()
{
	api::SamplerCreateParam samplerDesc;
	samplerDesc.mipMappingFilter = SamplerFilter::None;
	samplerDesc.minificationFilter = SamplerFilter::Linear;
	samplerDesc.magnificationFilter = SamplerFilter::Linear;
	m_samplerBilinear = m_context->createSampler(samplerDesc);
	if (m_samplerBilinear.isNull())
	{
		Log("UIRenderer initialisation: Failed to create the default bilinear sampler. This should never have happened...");
		return false;
	}
	samplerDesc.mipMappingFilter = SamplerFilter::Linear;
	m_samplerTrilinear = m_context->createSampler(samplerDesc);
	if (m_samplerTrilinear.isNull())
	{
		Log("UIRenderer initialisation: Failed to create the default trilinear sampler. This should never have happened...");
		return false;
	}
	return true;
}

bool UIRenderer::init_CreateDefaultSdkLogo()
{
	Stream::ptr_type sdkLogo = Stream::ptr_type(new BufferStream("", _PowerVR_512x256_RG_pvr, _PowerVR_512x256_RG_pvr_size));
	assets::Texture sdkTex;
	if (textureLoad(sdkLogo, assets::TextureFileFormat::PVR, sdkTex) != Result::Success)
	{
		Log(Log.Warning, "UIRenderer: Could not create the PowerVR SDK Logo.");
		return false;
	}
	sdkTex.setPixelFormat(assets::GeneratePixelType2<'l', 'a', 8, 8>::ID);

	m_sdkLogo = createImage(sdkTex);
	if (m_sdkLogo.isNull())
	{
		Log(Log.Warning,
		    "UIRenderer initialisation: Could not create the PowerVR SDK Logo. Errors will be gotten if trying to render getSdkLogo().");
		return false;
	}
	m_sdkLogo->setAnchor(Anchor::BottomRight, glm::vec2(.98f, -.98f));

	pvr::float32 scalefactor = .3f * getRenderingDim().x / BaseScreenDim.x;

	if (scalefactor > 1) { scalefactor = 1; }
	else if (scalefactor > .5) { scalefactor = .5; }
	else if (scalefactor > .25) { scalefactor = .25; }
	else if (scalefactor > .125) { scalefactor = .125; }
	else { scalefactor = .0625; }

	m_sdkLogo->setScale(glm::vec2(scalefactor));
	m_sdkLogo->commitUpdates();
	return true;
}

bool UIRenderer::init_CreateDefaultTitle()
{
	m_defaultTitle = createText("DefaultTitle", m_defaultFont);
	m_defaultDescription = createText(m_defaultFont);
	m_defaultControls = createText(m_defaultFont);

	if (m_defaultTitle.isNull())
	{
		Log(Log.Warning,
		    "UIRenderer initialisation: Could not create the PowerVR Description text. Errors will be gotten if trying to render getDefaultDescription().");
		return false;
	}
	else
	{
		m_defaultTitle->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f))->setScale(glm::vec2(.8, .8));
		m_defaultTitle->commitUpdates();
	}

	if (m_defaultDescription.isNull())
	{
		Log(Log.Warning,
		    "UIRenderer initialisation: Could not create the Demo Description text. Errors will be gotten if trying to render getDefaultDescription().");
		return false;
	}
	else
	{
		m_defaultDescription->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f -
		                                m_defaultTitle->getFont()->getFontLineSpacing() / (float)getRenderingDimY() * 1.5f))
		->setScale(glm::vec2(.60, .60));
		m_defaultDescription->commitUpdates();
	}

	if (m_defaultControls.isNull())
	{
		Log(Log.Warning,
		    "UIRenderer initialisation: Could not create the Demo Controls text. Errors will be gotten if trying to render getDefaultControls().");
		return false;
	}
	else
	{
		m_defaultControls->setAnchor(Anchor::BottomLeft, glm::vec2(-.98f, -.98f))->setScale(glm::vec2(.5, .5));
		m_defaultControls->commitUpdates();
	}
	return true;
}


bool UIRenderer::init_CreateDefaultFont()
{
	assets::Texture fontTex;
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

	if (textureLoad(arialFontTex, assets::TextureFileFormat::PVR, fontTex) != Result::Success)
	{
		Log(Log.Warning, "UIRenderer initialisation: Could not create the default font. Errors will be gotten if trying to render with getDefaultFont().");
		return false;
	}
	fontTex.setPixelFormat(assets::GeneratePixelType1<'a', 8>::ID);

	m_defaultFont = createFont(fontTex);
	if (m_defaultFont.isNull())
	{
		Log(Log.Warning, "UIRenderer initialisation: Could not create the default font. Errors will be gotten if trying to render with getDefaultFont().");
		return false;
	}

	return true;
}

pvr::uint64 UIRenderer::generateGroupId()
{
	static pvr::uint64 groupId = 1;
	return groupId++;
}

}// namespace ui
}// namespace pvr
