/*!
\brief Contains implementations of functions for the UIRenderer class.
\file PVRUtils/Vulkan/UIRendererVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRCore/IO/BufferStream.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/SamplerVk.h"
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRUtils/Vulkan/UIRendererVk.h"
#include "PVRUtils/ArialBoldFont.h"
#include "PVRUtils/PowerVRLogo.h"
#include "PVRUtils/Vulkan/UIRendererShader_vk.vsh.h"
#include "PVRUtils/Vulkan/UIRendererShader_vk.fsh.h"
#include "PVRUtils/Vulkan/HelperVk.h"
using std::map;
using std::vector;
using namespace pvrvk;
const uint32_t MaxDescUbo = 200;
const uint32_t MaxCombinedImageSampler = 200;
namespace pvr {
namespace ui {
const glm::vec2 BaseScreenDim(640, 480);
namespace {

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
	debug_assertion(_device.isValid(),  "NULL Context");
	GraphicsPipelineCreateInfo pipelineDesc;
	PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_texDescLayout);

	if (!_uboMvpDescLayout.isNull())
	{
		pipeLayoutInfo.addDescSetLayout(_uboMvpDescLayout);
	}
	if (!_uboMaterialLayout.isNull())
	{
		pipeLayoutInfo.addDescSetLayout(_uboMaterialLayout);
	}

	_pipelineLayout = _device->createPipelineLayout(pipeLayoutInfo);
	if (!_pipelineLayout.isValid())
	{
		Log(LogLevel::Critical, "UIRenderer PipelinelineLayout could not be created.");
		return Result::UnknownError;
	}
	pipelineDesc.pipelineLayout = _pipelineLayout;
	// Text_ pipe
	Shader vs;
	Shader fs;

	vs = _device->createShader(BufferStream("", spv_UIRendererShader_vk_vsh,
	                                        sizeof(spv_UIRendererShader_vk_vsh)).readToEnd<uint32_t>());

	fs = _device->createShader(BufferStream("", spv_UIRendererShader_vk_fsh,
	                                        sizeof(spv_UIRendererShader_vk_fsh)).readToEnd<uint32_t>());

	if (vs.isNull() || fs.isNull())
	{
		Log(LogLevel::Critical, "UIRenderer shaders could not be created.");
		return Result::UnknownError;
	}
	pipelineDesc.vertexShader.setShader(vs);
	pipelineDesc.fragmentShader.setShader(fs);
	VertexInputAttributeDescription posAttrib(0, 0, VkFormat::e_R32G32B32A32_SFLOAT, 0);
	VertexInputAttributeDescription texAttrib(1, 0, VkFormat::e_R32G32_SFLOAT, sizeof(float) * 4);
	pipelineDesc.vertexInput
	.addInputBinding(VertexInputBindingDescription(0, sizeof(float) * 6, VkVertexInputRate::e_VERTEX))
	.addInputAttribute(posAttrib)
	.addInputAttribute(texAttrib);

	PipelineColorBlendAttachmentState attachmentState(true, VkBlendFactor::e_SRC_ALPHA,
	    VkBlendFactor::e_ONE_MINUS_SRC_ALPHA, VkBlendOp::e_ADD, CombineAllFlags<VkColorComponentFlags>::Flags);
	pipelineDesc.colorBlend.setAttachmentState(0, attachmentState);
	pipelineDesc.depthStencil.enableDepthTest(false).enableDepthWrite(false);
	pipelineDesc.rasterizer.setCullMode(VkCullModeFlags::e_NONE);
	pipelineDesc.inputAssembler.setPrimitiveTopology(VkPrimitiveTopology::e_TRIANGLE_LIST);
	pipelineDesc.viewport.setViewportAndScissor(0, Viewport(0, 0, _screenDimensions.x, _screenDimensions.y),
	    Rect2Di(0, 0, static_cast<int32_t>(_screenDimensions.x), static_cast<int32_t>(_screenDimensions.y)));
	pipelineDesc.renderPass = _renderpass;
	pipelineDesc.subpass = _subpass;
	pipelineDesc.flags = VkPipelineCreateFlags::e_ALLOW_DERIVATIVES_BIT;
	_pipeline = _device->createGraphicsPipeline(pipelineDesc);
	if (_pipeline.isNull())
	{
		Log(LogLevel::Critical, "UIRenderer pipeline not be created.");
		return Result::UnknownError;
	}
	return Result::Success;
}

pvr::Result UIRenderer::init_CreateDescriptorSetLayout()
{
	assertion(_device.isValid(), "NULL GRAPHICS CONTEXT");
	DescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, MaxCombinedImageSampler);
	descPoolInfo.setMaxDescriptorSets(MaxCombinedImageSampler);
	descPoolInfo.addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, MaxDescUbo);
	descPoolInfo.setMaxDescriptorSets(descPoolInfo.getMaxDescriptorSets() + MaxDescUbo);

	_descPool = getDevice()->createDescriptorPool(descPoolInfo);
	if (!_descPool.isValid())
	{
		Log("Failed to create UIRenderer Descriptorpool");
		return Result::UnknownError;
	}

	DescriptorSetLayoutCreateInfo layoutInfo;

	// CombinedImagesampler Layout
	layoutInfo.setBinding(0, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
	                      1, VkShaderStageFlags::e_FRAGMENT_BIT);
	_texDescLayout = _device->createDescriptorSetLayout(layoutInfo);

	if (_texDescLayout.isNull())
	{
		Log("Failed to create UIRenderer's CombinedImageSampler DescriptorSetLayout");
		return Result::UnknownError;
	}

	// Mvp ubo Layout
	layoutInfo.clear().setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC,
	                              1, VkShaderStageFlags::e_VERTEX_BIT);
	_uboMvpDescLayout = _device->createDescriptorSetLayout(layoutInfo);
	if (_uboMvpDescLayout.isNull())
	{
		Log("Failed to create UIRenderer's model-view-projection DescriptorSetLayout");
		return Result::UnknownError;
	}

	// material ubo layout
	layoutInfo.clear().setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1,
	                              VkShaderStageFlags::e_VERTEX_BIT | VkShaderStageFlags::e_FRAGMENT_BIT);
	_uboMaterialLayout = _device->createDescriptorSetLayout(layoutInfo);
	if (_uboMaterialLayout.isNull())
	{
		Log("Failed to create UIRenderer's material DescriptorSetLayout");
		return Result::UnknownError;
	}

	return Result::Success;
}

Font UIRenderer::createFont(const ImageView& image,
                            const TextureHeader& tex, const Sampler& sampler)
{
	Font font;
	font.construct(*this, image, tex, sampler);
	_fonts.push_back(font);
	return font;
}

MatrixGroup UIRenderer::createMatrixGroup()
{
	MatrixGroup group;
	group.construct(*this, generateGroupId());
	_sprites.push_back(group);
	group->commitUpdates();
	return group;
}

PixelGroup UIRenderer::createPixelGroup()
{
	PixelGroup group;
	group.construct(*this, generateGroupId());
	_sprites.push_back(group);
	group->commitUpdates();
	return group;
}

void UIRenderer::setUpUboPoolLayouts(uint32_t numInstances, uint32_t numSprites)
{
	debug_assertion(numInstances >= numSprites, "Maximum number of "
	                "instances must be atleast the same as maximum number of sprites");

	pvrvk::Device device = getDevice()->getReference();
	// mvp pool
	_uboMvp.initlayout(device, numInstances);
	// material pool
	_uboMaterial.initlayout(device, numSprites);
}

bool UIRenderer::setUpUboPools(uint32_t numInstances, uint32_t numSprites)
{
	debug_assertion(numInstances >= numSprites, "Maximum number of "
	                "instances must be atleast the same as maximum number of sprites");

	// mvp pool
	pvrvk::Device device = getDevice()->getReference();
	if (!_uboMvp.init(device, _uboMvpDescLayout, getDescriptorPool(), *this))
	{
		return false;
	}
	// material pool
	if (!_uboMaterial.init(device, _uboMaterialLayout, getDescriptorPool(), *this))
	{
		return false;
	}

	void* memory = 0;
	if ((_uboMvp._memory_cached->map(&memory, 0, _uboMvp._structuredBufferView.getSize() + _uboMaterial._structuredBufferView.getSize()) != VkResult::e_SUCCESS) || !memory)
	{
		Log("UIRenderer::updateMaterial: Could not map memory");
		return false;
	}
	_uboMvp._structuredBufferView.pointToMappedMemory((void*)size_t((size_t)memory + _uboMvp._memoffset_cached), 0);
	_uboMaterial._structuredBufferView.pointToMappedMemory((void*)size_t((size_t)memory + _uboMaterial._memoffset_cached), 0);

	return true;
}

Image UIRenderer::createImage(const ImageView& tex, const Sampler& sampler)
{
	return createImageFromAtlas(tex, Rect2Df(0.0f, 0.0f, 1.0f, 1.0f), sampler);
}

pvr::ui::Image UIRenderer::createImageFromAtlas(const ImageView& tex, const Rect2Df& uv, const Sampler& sampler)
{
	Image image;
	image.construct(*this, tex, tex->getImage()->getWidth(), tex->getImage()->getHeight(), sampler);
	if (!image->init()) { image.reset(); return image; }
	_sprites.push_back(image);
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
	_textElements.push_back(spriteText);
	return spriteText;
}

Text UIRenderer::createText(const TextElement& textElement)
{
	Text text; text.construct(*this, textElement);
	if (!text->init())
	{
		text.reset();
		return text;
	}
	_sprites.push_back(text);
	text->commitUpdates();
	return text;
}

bool UIRenderer::init(uint32_t width, uint32_t height, bool fullscreen, const RenderPass& renderpass, uint32_t subpass, CommandPool& cmdPool, Queue& queue,
                      bool createDefaultLogo, bool createDefaultTitle, bool createDefaultFont, uint32_t maxNumInstances, uint32_t maxNumSprites)
{
	release();
	_mustEndCommandBuffer = false;
	_device = renderpass->getDevice();

	// create pool layouts
	setUpUboPoolLayouts(maxNumInstances, maxNumSprites);

	{
		auto TMPBUFFER = utils::createBuffer(_device, _uboMvp._structuredBufferView.getDynamicSliceSize(),
		                                       VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT | VkBufferUsageFlags::e_INDEX_BUFFER_BIT | VkBufferUsageFlags::e_VERTEX_BUFFER_BIT,
		                                       (VkMemoryPropertyFlags(0)));
		if (TMPBUFFER.isNull())
		{
			Log("UIRenderer::Could not initialize suballocator for UIRenderer");
			return false;
		}
		_bufferAllocator = utils::createMemorySuballocator();

		_bufferAllocator->init(_device->getReference(),
		                       VkDeviceSize(align(_uboMvp._structuredBufferView.getSize(), TMPBUFFER->getMemoryRequirement().alignment) +
		                                    align(_uboMaterial._structuredBufferView.getSize(), TMPBUFFER->getMemoryRequirement().alignment)),
		                       TMPBUFFER->getMemoryRequirement(), VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}

	_screenDimensions = glm::vec2(width, height);
	_renderpass = renderpass;
	_subpass = subpass;
	// screen rotated?
	if (_screenDimensions.y > _screenDimensions.x && fullscreen)
	{
		rotateScreen90degreeCCW();
	}

	// create the commandbuffer
	CommandBuffer cmdBuffer = cmdPool->allocateCommandBuffer();
	cmdBuffer->begin();
	Result res;
	utils::ImageUploadResults sdkLogoImageResult;
	utils::ImageUploadResults defaultFontResult;
	if (((res = init_CreateDescriptorSetLayout()) == Result::Success) &&
	    ((res = init_CreatePipelineAndRenderPass()) == Result::Success))
	{
		if (!setUpUboPools(maxNumInstances, maxNumSprites)) { return false; }
		if (!init_CreateDefaultSampler()) { return false;  }
		if (createDefaultLogo && (sdkLogoImageResult = init_CreateDefaultSdkLogo(cmdBuffer)).getImageView().isNull())
		{
			return false;
		}
		if (createDefaultFont && (defaultFontResult = init_CreateDefaultFont(cmdBuffer)).getImageView().isNull())
		{
			return false;
		}
		if (createDefaultTitle)
		{
			if (!init_CreateDefaultTitle()) { return false; }
		}
	}
	cmdBuffer->end();
	SubmitInfo submitInfo;
	submitInfo.commandBuffers = &cmdBuffer;
	submitInfo.numCommandBuffers = 1;
	Fence fence = queue->getDevice()->createFence();
	queue->submit(&submitInfo, 1, fence);
	fence->wait();
	return true;
}

TextElement UIRenderer::createTextElement(const std::string& text, const Font& font)
{
	TextElement spriteText;
	spriteText.construct(*this, text, font);
	_textElements.push_back(spriteText);
	return spriteText;
}

bool UIRenderer::init_CreateDefaultSampler()
{
	pvrvk::SamplerCreateInfo samplerDesc;

	samplerDesc.wrapModeU = VkSamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerDesc.wrapModeV = VkSamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerDesc.wrapModeW = VkSamplerAddressMode::e_CLAMP_TO_EDGE;

	samplerDesc.mipMapMode = VkSamplerMipmapMode::e_NEAREST;
	samplerDesc.minFilter = VkFilter::e_LINEAR;
	samplerDesc.magFilter = VkFilter::e_LINEAR;
	_samplerBilinear = _device->createSampler(samplerDesc);
	if (_samplerBilinear.isNull())
	{
		Log("UIRenderer initialisation: Failed to create the default bilinear sampler. "
		    "This should never have happened...");
		return false;
	}
	samplerDesc.mipMapMode = VkSamplerMipmapMode::e_LINEAR;
	_samplerTrilinear = _device->createSampler(samplerDesc);
	if (_samplerTrilinear.isNull())
	{
		Log("UIRenderer initialisation: Failed to create the default trilinear sampler. "
		    "This should never have happened...");
		return false;
	}
	return true;
}

utils::ImageUploadResults UIRenderer::init_CreateDefaultSdkLogo(
  CommandBuffer& cmdBuffer)
{
	utils::ImageUploadResults sdkLogoImage;
	Stream::ptr_type sdkLogo = Stream::ptr_type(
	                             new BufferStream("", _PowerVR_512x256_RG_pvr,
	                                 _PowerVR_512x256_RG_pvr_size));
	Texture sdkTex;
	if (!assets::textureLoad(sdkLogo, TextureFileFormat::PVR, sdkTex))
	{
		Log(LogLevel::Warning, "UIRenderer: Could not create the PowerVR SDK Logo.");
		return sdkLogoImage;
	}
	sdkTex.setPixelFormat(GeneratePixelType2<'l', 'a', 8, 8>::ID);
	Device device = getDevice()->getReference();
	sdkLogoImage = utils::uploadImage(device, sdkTex, true, cmdBuffer);
	_sdkLogo = createImage(sdkLogoImage.getImageView(), _samplerBilinear);
	if (_sdkLogo.isNull())
	{
		Log(LogLevel::Warning,
		    "UIRenderer initialisation: Could not create the PowerVR SDK Logo."
		    "Errors will be gotten if trying to render getSdkLogo().");
		return sdkLogoImage;
	}
	_sdkLogo->setAnchor(Anchor::BottomRight, glm::vec2(.98f, -.98f));
	float scalefactor = .3f * getRenderingDim().x / BaseScreenDim.x;

	if (scalefactor > 1) { scalefactor = 1; }
	else if (scalefactor > .5) { scalefactor = .5; }
	else if (scalefactor > .25) { scalefactor = .25; }
	else if (scalefactor > .125) { scalefactor = .125; }
	else { scalefactor = .0625; }

	_sdkLogo->setScale(glm::vec2(scalefactor));
	_sdkLogo->commitUpdates();
	return sdkLogoImage;
}

bool UIRenderer::init_CreateDefaultTitle()
{
	_defaultTitle = createText(createTextElement("DefaultTitle", _defaultFont));
	_defaultDescription = createText(createTextElement("", _defaultFont));
	_defaultControls = createText(createTextElement("", _defaultFont));
	if (_defaultTitle.isNull())
	{
		Log(LogLevel::Warning, "UIRenderer initialisation: Could not create the PowerVR "
		    "Description text. Errors will be gotten if trying to render getDefaultDescription().");
		return false;
	}
	else
	{
		_defaultTitle->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f))->setScale(glm::vec2(.8, .8));
		_defaultTitle->commitUpdates();
	}

	if (_defaultDescription.isNull())
	{
		Log(LogLevel::Warning,
		    "UIRenderer initialisation: Could not create the Demo Description text. Errors will be gotten if trying to render getDefaultDescription().");
		return false;
	}
	else
	{
		_defaultDescription->setAnchor(Anchor::TopLeft, glm::vec2(-.98f, .98f -
		                               _defaultTitle->getFont()->getFontLineSpacing() / static_cast<float>(getRenderingDimY()) * 1.5f))
		->setScale(glm::vec2(.60, .60));
		_defaultDescription->commitUpdates();
	}

	if (_defaultControls.isNull())
	{
		Log(LogLevel::Warning,
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

utils::ImageUploadResults UIRenderer::init_CreateDefaultFont(
  CommandBuffer& cmdBuffer)
{
	Texture fontTex;
	Stream::ptr_type arialFontTex;
	float maxRenderDim = glm::max<float>(getRenderingDimX(), getRenderingDimY());
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
	utils::ImageUploadResults uploadResult;
	if (!assets::textureLoad(arialFontTex, TextureFileFormat::PVR, fontTex))
	{
		Log(LogLevel::Warning, "UIRenderer initialisation: Could not create "
		    "the default font. Errors will be gotten if trying to render with getDefaultFont().");
		return uploadResult;
	}
	fontTex.setPixelFormat(GeneratePixelType1<'a', 8>::ID);
	Device device = getDevice()->getReference();
	uploadResult = utils::uploadImage(device, fontTex, true, cmdBuffer);
	_defaultFont = createFont(uploadResult.getImageView(), fontTex);
	if (_defaultFont.isNull())
	{
		Log(LogLevel::Warning, "UIRenderer initialisation: Could not create the default font. Errors will be"
		    " gotten if trying to render with getDefaultFont().");
	}
	return uploadResult;
}

void UIRenderer::UboMaterial::initlayout(Device& device, uint32_t numArrayId)
{
	_numArrayId = numArrayId;
	// create the descriptor set & buffer if not already created
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("uv", GpuDatatypes::mat4x4);
	desc.addElement("color", GpuDatatypes::vec4);
	desc.addElement("alphaMode", GpuDatatypes::Integer);

	_structuredBufferView.initDynamic(desc, _numArrayId, pvr::BufferUsageFlags::UniformBuffer,
	                                  static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
}

bool UIRenderer::UboMaterial::init(Device& device, DescriptorSetLayout& descLayout, DescriptorPool& pool, UIRenderer& uirenderer)
{
	_buffer = uirenderer.suballocateBuffer((size_t)_structuredBufferView.getSize(), VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT);
	if (!_buffer.isValid())
	{
		Log("Failed to create UIRenderer Material buffer");
		return false;
	}

	if (!_uboDescSetSet.isValid())
	{
		_uboDescSetSet = pool->allocateDescriptorSet(descLayout);
	}

	this->_memoffset_cached = static_cast<size_t>(utils::SuballocatedMemory(_buffer->getDeviceMemory())->offset());
	this->_memory_cached = utils::SuballocatedMemory(_buffer->getDeviceMemory())->memory();
	WriteDescriptorSet writeDescSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _uboDescSetSet, 0, 0);
	writeDescSet.setBufferInfo(0, DescriptorBufferInfo(_buffer, 0, _structuredBufferView.getDynamicSliceSize()));
	device->updateDescriptorSets(&writeDescSet, 1, nullptr, 0);

	return true;
}

void UIRenderer::UboMvp::initlayout(Device& device, uint32_t numElements)
{
	_numArrayId = numElements;
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("mvp", GpuDatatypes::mat4x4);

	_structuredBufferView.initDynamic(desc, _numArrayId, pvr::BufferUsageFlags::UniformBuffer,
	                                  static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
}

bool UIRenderer::UboMvp::init(Device& device, DescriptorSetLayout& descLayout, DescriptorPool& pool, UIRenderer& uirenderer)
{
	_buffer = uirenderer.suballocateBuffer(_structuredBufferView.getSize(), VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT);

	if (!_uboDescSetSet.isValid())
	{
		_uboDescSetSet = pool->allocateDescriptorSet(descLayout);
	}
	this->_memoffset_cached = static_cast<size_t>(utils::SuballocatedMemory(_buffer->getDeviceMemory())->offset());
	this->_memory_cached = utils::SuballocatedMemory(_buffer->getDeviceMemory())->memory();

	WriteDescriptorSet writeDescSet(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _uboDescSetSet, 0, 0);
	writeDescSet.setBufferInfo(0, DescriptorBufferInfo(_buffer, 0, _structuredBufferView.getDynamicSliceSize()));
	device->updateDescriptorSets(&writeDescSet, 1, nullptr, 0);
	return true;
}

void UIRenderer::UboMaterial::updateMaterial(uint32_t arrayIndex, const glm::vec4& color,
    int32_t alphaMode, const glm::mat4& uv)
{
	_structuredBufferView.getElement(static_cast<uint32_t>(MaterialBufferElement::UVMtx), 0, arrayIndex).setValue(uv);
	_structuredBufferView.getElement(static_cast<uint32_t>(MaterialBufferElement::Color), 0, arrayIndex).setValue(color);
	_structuredBufferView.getElement(static_cast<uint32_t>(MaterialBufferElement::AlphaMode), 0, arrayIndex).setValue(alphaMode);
}

void UIRenderer::UboMvp::updateMvp(uint32_t bufferArrayId, const glm::mat4x4& mvp)
{
	_structuredBufferView.getElement(0, 0, bufferArrayId).setValue(mvp);
}
}// namespace ui
}// namespace pvr
//!\endcond
