/*!*********************************************************************************************************************
\file         PVRUIRenderer\UIRenderer.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief     	  Contains implementations of functions for the UIRenderer class.
***********************************************************************************************************************/
#include "PVRUIRenderer/UIRenderer.h"
#include "PVRCore/BufferStream.h"
#include "PVRApi/OGLES/TextureUtils.h"
#include "PVRUIRenderer/ArialBoldFont.h"
#include "PVRUIRenderer/PowerVRLogo.h"
#include "PVRUIRenderer/UIRendererShaders.h"
using std::map;
using std::vector;
namespace pvr {
namespace ui {
const glm::vec2 BaseScreenDim(640, 480);
Result::Enum UIRenderer::init_CreatePipelineAndRenderPass()
{
	PVR_ASSERT(m_context.isValid() && "NULL Context");
	api::GraphicsPipelineCreateParam pipelineDesc;
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(m_defaultLayout);
	m_pipelineLayout = m_context->createPipelineLayout(pipeLayoutInfo);
	pipelineDesc.pipelineLayout = m_pipelineLayout;
	// Text_ pipe
	{
		api::Shader vs = m_context->createShader(BufferStream("", _print3DShader_glsles200_vsh, _print3DShader_glsles200_vsh_size),
		                 ShaderType::VertexShader);
		api::Shader fs = m_context->createShader(BufferStream("", _print3DShader_glsles200_fsh, _print3DShader_glsles200_fsh_size),
		                 ShaderType::FragmentShader);
		if (vs.isNull() || fs.isNull())
		{
			Log(Log.Critical, "UIRenderer shaders could not be created.");
			return Result::UnknownError;
		}
		pipelineDesc.vertexShader.setShader(vs);
		pipelineDesc.fragmentShader.setShader(fs);

		api::VertexAttributeInfo posAttrib(0, DataType::Float32, 3, 0, "myVertex");
		api::VertexAttributeInfo texCoordAttrib(1, DataType::Float32, 2, sizeof(float32) * 4, "myUV");

		pipelineDesc.vertexInput.setInputBinding(0, sizeof(float32) * 6, api::StepRate::Vertex)
		.addVertexAttribute(0, posAttrib).addVertexAttribute(0, texCoordAttrib);

		api::ImageDataFormat colorFormat;
		m_context->getDisplayAttributes();
		api::getDisplayFormat(m_context->getDisplayAttributes(), &colorFormat, NULL);
		api::pipelineCreation::ColorBlendAttachmentState attachmentState(true, api::BlendFactor::SrcAlpha,
		    api::BlendFactor::OneMinusSrcAlpha, api::BlendOp::Add, api::ColorChannel::All);
		pipelineDesc.colorBlend.addAttachmentState(attachmentState);
		pipelineDesc.depthStencil.setDepthTestEnable(false).setDepthWrite(false);
		pipelineDesc.rasterizer.setCullFace(api::Face::None);
		pipelineDesc.inputAssembler.setPrimitiveTopology(pvr::PrimitiveTopology::TriangleList);
	}
	m_pipeline = m_context->createParentableGraphicsPipeline(pipelineDesc);
	if (m_pipeline.isNull())
	{
		Log(Log.Critical, "UIRenderer pipeline not be created.");
		return Result::UnknownError;
	}

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

	return Result::Success;
}

pvr::Result::Enum UIRenderer::init_CreateDescriptors()
{
	PVR_ASSERT(m_context.isValid() && "NULL GRAPHICS CONTEXT");
	api::DescriptorSetLayoutCreateParam defaultDesc;
	defaultDesc.addBinding(0, pvr::api::DescriptorType::CombinedImageSampler, 1, pvr::api::ShaderStageFlags::Fragment);
	m_defaultLayout = m_context->createDescriptorSetLayout(defaultDesc);
	if (m_defaultLayout.isNull()) { return Result::UnknownError; }
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
	image.construct(*this, apiTexture, texture.getWidth(), texture.getHeight());
	image->setSampler(m_samplerBilinear);
	image->commitUpdates();
	return image;
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
	image->setSampler(m_samplerBilinear);
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
	assets::SamplerCreateParam samplerDesc;
	samplerDesc.mipMappingFilter = SamplerFilter::Linear;
	samplerDesc.minificationFilter = SamplerFilter::Linear;
	samplerDesc.magnificationFilter = SamplerFilter::Linear;
	m_samplerBilinear = m_context->createSampler(samplerDesc);
	if (m_samplerBilinear.isNull())
	{
		Log("UIRenderer initialisation: Failed to create the default bilinear sampler. This should never have happened...");
		return false;
	}
	return true;
}

bool UIRenderer::init_CreateDefaultSdkLogo()
{
	Stream::ptr_type sdkLogo = Stream::ptr_type(new BufferStream("", _PowerVR_512x256_RG_pvr, _PowerVR_512x256_RG_pvr_size));
	assets::Texture sdkTex;
	if (textureLoad(sdkLogo, assets::TextureFileFormat::PVR, sdkTex))
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