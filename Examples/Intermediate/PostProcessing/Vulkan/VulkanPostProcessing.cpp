/*!********************************************************************************************
\File         VulkanPostProcessing.cpp
\Title        Bloom
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Shows how to do a bloom effect
***********************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

pvr::utils::VertexBindings_Name VertexBindings[] =
{
	{ "POSITION", "inVertex" },
	{ "NORMAL", "inNormal" },
	{ "UV0", "inTexCoord" },
};

enum class Config
{
	MaxSwapChain = 4
};

/**********************************************************************************************
Consts
**********************************************************************************************/
const glm::vec4 LightPosition(-1.5f, 0.0f, 10.0f, 0.0);

/**********************************************************************************************
Content file names
***********************************************************************************************/
const char FragShaderSrcFile[] = "FragShader_vk.fsh.spv";
const char VertShaderSrcFile[] = "VertShader_vk.vsh.spv";
const char PreBloomFragShaderSrcFile[] = "PreBloomFragShader_vk.fsh.spv";
const char PreBloomVertShaderSrcFile[] = "PreBloomVertShader_vk.vsh.spv";
const char PostBloomFragShaderSrcFile[] = "PostBloomFragShader_vk.fsh.spv";
const char PostBloomVertShaderSrcFile[] = "PostBloomVertShader_vk.vsh.spv";
const char BlurFragSrcFile[] = "BlurFragShader_vk.fsh.spv";
const char BlurVertSrcFile[] = "BlurVertShader_vk.vsh.spv";

// PVR texture files
const char BaseTexFile[] = "Marble.pvr";
// POD scene files
const char SceneFile[] = "scene.pod";

struct StaticUbo
{
	pvr::utils::StructuredBufferView structuredBufferView;
	pvrvk::Buffer buffer;
	pvrvk::DescriptorSet sets[static_cast<uint32_t>(Config::MaxSwapChain)];
};

struct DynamicUbo
{
	pvr::utils::StructuredBufferView structuredBufferView;
	pvrvk::Buffer buffer;
	pvrvk::DescriptorSet sets[static_cast<uint32_t>(Config::MaxSwapChain)];
};

struct BlurPass
{
	pvr::utils::StructuredBufferView structuredBufferView;
	pvrvk::Buffer buffer;
	pvrvk::DescriptorSet perVertDescriptorSet;
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::DescriptorSet texDescSet[static_cast<uint32_t>(Config::MaxSwapChain)]; // per swapchain
	pvrvk::Framebuffer framebuffer[static_cast<uint32_t>(Config::MaxSwapChain)];
};

struct RenderScenePass
{
	DynamicUbo uboDynamic;
	StaticUbo uboStatic;

	pvrvk::GraphicsPipeline pipeline;
	pvrvk::Rect2Di renderArea;
	pvrvk::DescriptorSet texDescriptor;
	enum class UboDynamicElements
	{
		MVInv, MVPMatrix, LightDirection
	};

	enum class UboStaticElements
	{
		Shininess
	};
};

struct PreBloomPass
{
	pvrvk::Framebuffer framebuffer[static_cast<uint32_t>(Config::MaxSwapChain)];
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::DescriptorSet descTex;
	pvr::utils::StructuredBufferView structuredBufferView;
	pvrvk::Buffer buffer;
	pvrvk::DescriptorSet descIntensity;

	DynamicUbo uboDynamic;
	StaticUbo uboStatic;
};

struct PostBloomPass
{
	pvrvk::GraphicsPipeline pipeline;

	pvr::utils::StructuredBufferView structuredBufferView;
	pvrvk::Buffer buffer;
	pvr::Multi<pvrvk::DescriptorSet> uboBloomConfigs;

	pvrvk::DescriptorSet texDescSet[static_cast<uint32_t>(Config::MaxSwapChain)];//per swapchain
};

/*!********************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************/
class VulkanPostProcessing : public pvr::Shell
{
	struct DeviceResrcs
	{
		pvrvk::Instance instance;
		pvrvk::Device device;
		pvrvk::DescriptorPool descriptorPool;
		pvrvk::CommandPool commandPool;
		pvrvk::Swapchain swapchain;

		pvrvk::Semaphore semaphoreImageAcquired[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameAcquireFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Semaphore semaphorePresent[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameCommandBufferFence[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

		// Textures
		pvrvk::ImageView baseTex;
		pvrvk::ImageView bloomMapTex;

		// Samplers
		pvrvk::Sampler sceneSamplerClamp;

		// Vbos and Ibos
		std::vector<pvrvk::Buffer> vbos;
		std::vector<pvrvk::Buffer> ibos;

		// Command Buffers
		pvrvk::CommandBuffer mainCmdBloom[static_cast<uint32_t>(Config::MaxSwapChain)];
		pvrvk::CommandBuffer mainCmdNoBloom[static_cast<uint32_t>(Config::MaxSwapChain)];

		pvrvk::SecondaryCommandBuffer preBloomCommandBuffer[static_cast<uint32_t>(Config::MaxSwapChain)];
		pvrvk::SecondaryCommandBuffer noBloomCommandBuffer[static_cast<uint32_t>(Config::MaxSwapChain)];
		pvrvk::SecondaryCommandBuffer noBloomUiRendererCommandBuffer[static_cast<uint32_t>(Config::MaxSwapChain)];
		pvrvk::SecondaryCommandBuffer bloomUiRendererCommandBuffer[static_cast<uint32_t>(Config::MaxSwapChain)];

		pvrvk::SecondaryCommandBuffer horizontalBlurCommandBuffer[static_cast<uint32_t>(Config::MaxSwapChain)];
		pvrvk::SecondaryCommandBuffer verticalBlurCommandBuffer[static_cast<uint32_t>(Config::MaxSwapChain)];

		pvrvk::SecondaryCommandBuffer postBloomCommandBuffer[static_cast<uint32_t>(Config::MaxSwapChain)];

		// descriptor layouts
		pvrvk::DescriptorSetLayout texSamplerLayoutFrag;
		pvrvk::DescriptorSetLayout postBloomTexLayoutFrag;
		pvrvk::DescriptorSetLayout uboLayoutVert;
		pvrvk::DescriptorSetLayout uboLayoutFrag;
		pvrvk::DescriptorSetLayout uboLayoutDynamicVert;

		// Renderpasses
		PreBloomPass preBloomPass;
		RenderScenePass renderScenePass;
		PostBloomPass postBloomPass;
		BlurPass horizontalBlurPass;
		BlurPass verticalBlurPass;

		// 3D Model
		pvr::assets::ModelHandle scene;

		pvrvk::Queue queues[1];

		pvr::Multi<pvrvk::ImageView> depthStencilImages;
		std::vector<pvr::utils::ImageUploadResults> uiImageUploadResult;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
	};

	std::unique_ptr<DeviceResrcs> _deviceResources;

	uint32_t _frameId;

	float _bloomIntensity;
	bool _applyBloom;
	bool _drawObject;
	bool _animating;

	float _rotation;
	VkFormat _depthStencilImageFormat;

	glm::mat4 _worldMatrix;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;

	float _blurTexelOffset;
	uint32_t _blurDimension;
	uint32_t semaphoreIndex;

public:
	VulkanPostProcessing() : _bloomIntensity(1.f), semaphoreIndex(0) {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	bool createOnScreenFramebuffer();
	void calculateBlurTexelOffsets();
	bool createDescriptors();
	bool createPipelines();
	void createBuffers();
	void createDescriptorSetLayouts();
	bool createBlurFramebuffer(pvrvk::CommandBuffer imageTransCmdBuffer);
	bool createPreBloomFramebuffer(pvrvk::CommandBuffer imageTransCmdBuffer);
	void recordBloomCommands(uint32_t swapchain);
	void recordCommandsPreBloom(uint32_t swapchain);
	void recordNoBloomCommands(uint32_t swapchain);
	void recordCommandsBlur(pvrvk::SecondaryCommandBuffer& commandBuffer, BlurPass& pass, uint32_t swapchain);
	void recordCommandsPostBloom(uint32_t swapchain);
	void recordCommandUIRenderer(uint32_t swapchain);
	void recordCommandsNoBloom(uint32_t swapchain);
	void updateSubtitleText();
	void updatePostBloomConfig(uint32_t swapchain);
	void drawMesh(int i32NodeIndex, pvrvk::SecondaryCommandBuffer& commandBuffer);
	void eventMappedInput(pvr::SimplifiedInput e);
	void updateAnimation();
	void updateBloomIntensity(float _bloomIntensity);
	void recordCommandBuffers();
	void createCommandBuffers(uint32_t swapchain);
	void prepareSwapchainForRendering(pvrvk::CommandBuffer commandBuffer, uint32_t swapchain);
};

void VulkanPostProcessing::prepareSwapchainForRendering(
  pvrvk::CommandBuffer commandBuffer, uint32_t swapchain)
{
	pvrvk::ImageMemoryBarrier barrier;
	barrier.image = _deviceResources->swapchain->getImage(swapchain);
	barrier.srcAccessMask = VkAccessFlags::e_MEMORY_READ_BIT;
	barrier.dstAccessMask = VkAccessFlags::e_MEMORY_WRITE_BIT;
	barrier.oldLayout = VkImageLayout::e_PRESENT_SRC_KHR;
	barrier.newLayout = VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL;
	barrier.srcQueueFamilyIndex  = barrier.dstQueueFamilyIndex =
	                                 commandBuffer->getCommandPool()->getQueueFamilyId();
}

void VulkanPostProcessing::calculateBlurTexelOffsets()
{
	// Texel offset for blur filter kernel
	_blurTexelOffset = 1.0f / (float)_blurDimension;
	// Altered weights for the faster filter kernel
	float w1 = 0.0555555f;
	float w2 = 0.2777777f;
	float intraTexelOffset = (w1 / (w1 + w2)) * _blurTexelOffset;
	_blurTexelOffset += intraTexelOffset;
}

/*!********************************************************************************************
\return Return true if no error occurred
\brief  Loads the textures required for this training course
***********************************************************************************************/
bool VulkanPostProcessing::createDescriptors()
{
	// sampler clamp
	pvrvk::SamplerCreateInfo samplerDesc;
	samplerDesc.minFilter = VkFilter::e_LINEAR;
	samplerDesc.mipMapMode = VkSamplerMipmapMode::e_NEAREST;
	samplerDesc.magFilter = VkFilter::e_LINEAR;
	samplerDesc.wrapModeU = VkSamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerDesc.wrapModeV = VkSamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerDesc.wrapModeW = VkSamplerAddressMode::e_CLAMP_TO_EDGE;
	_deviceResources->sceneSamplerClamp = _deviceResources->device->createSampler(samplerDesc);

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets(6 * _deviceResources->swapchain->getSwapchainLength() + 5);
	pvrvk::WriteDescriptorSet* writeDescSet = &writeDescSets[0];
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// render pass descriptor set dynamic ubo
		{
			_deviceResources->renderScenePass.uboDynamic.sets[i] =
			  _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutDynamicVert);

			writeDescSet->set(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC,
			                  _deviceResources->renderScenePass.uboDynamic.sets[i]);
			writeDescSet->setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->renderScenePass.uboDynamic.buffer,
			                            0, _deviceResources->renderScenePass.uboDynamic.structuredBufferView.getDynamicSliceSize()));
			++writeDescSet;
		}

		// pre-bloom pass descriptor set
		{
			_deviceResources->preBloomPass.uboDynamic = _deviceResources->renderScenePass.uboDynamic;
		}

		// horizontal blur descriptor set
		{
			_deviceResources->horizontalBlurPass.texDescSet[i] =
			  _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texSamplerLayoutFrag);
			writeDescSet->set(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
			                  _deviceResources->horizontalBlurPass.texDescSet[i]);

			writeDescSet->setImageInfo(0, pvrvk::DescriptorImageInfo(
			                             _deviceResources->preBloomPass.framebuffer[i]->getAttachment(1), _deviceResources->sceneSamplerClamp));
			++writeDescSet;
		}

		// vertical blur pass descriptor set
		{
			_deviceResources->verticalBlurPass.texDescSet[i] =
			  _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texSamplerLayoutFrag);

			writeDescSet->set(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
			                  _deviceResources->verticalBlurPass.texDescSet[i]);

			writeDescSet->setImageInfo(0, pvrvk::DescriptorImageInfo(
			                             _deviceResources->horizontalBlurPass.framebuffer[i]->getAttachment(0),
			                             _deviceResources->sceneSamplerClamp));

			++writeDescSet;
		}

		// post bloom descriptor set
		{
			_deviceResources->postBloomPass.texDescSet[i] =
			  _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->postBloomTexLayoutFrag);

			writeDescSet->set(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
			                  _deviceResources->postBloomPass.texDescSet[i]);

			writeDescSet->setImageInfo(0, pvrvk::DescriptorImageInfo(
			                             _deviceResources->preBloomPass.framebuffer[i]->getAttachment(0),
			                             _deviceResources->sceneSamplerClamp));
			++writeDescSet;

			writeDescSet->set(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
			                  _deviceResources->postBloomPass.texDescSet[i], 1);

			writeDescSet->setImageInfo(0, pvrvk::DescriptorImageInfo(
			                             _deviceResources->verticalBlurPass.framebuffer[i]->getAttachment(0),
			                             _deviceResources->sceneSamplerClamp));
			++writeDescSet;
		}
		// bloom config
		{
			_deviceResources->postBloomPass.uboBloomConfigs.add(
			  _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutFrag));

			writeDescSet->set(VkDescriptorType::e_UNIFORM_BUFFER, _deviceResources->postBloomPass.uboBloomConfigs[i]);

			writeDescSet->setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->postBloomPass.buffer,
			                            _deviceResources->postBloomPass.structuredBufferView.getDynamicSliceOffset(i),
			                            _deviceResources->postBloomPass.structuredBufferView.getDynamicSliceSize()));
			++writeDescSet;
		}
	}

	// pre bloom pass
	{
		// create the intensity descriptor
		_deviceResources->preBloomPass.descIntensity =
		  _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutFrag);

		writeDescSet->set(VkDescriptorType::e_UNIFORM_BUFFER,
		                  _deviceResources->preBloomPass.descIntensity);
		writeDescSet->setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->preBloomPass.buffer,
		                            0, _deviceResources->preBloomPass.structuredBufferView.getDynamicSliceSize()));
		++writeDescSet;
	}

	// set up the render scene pass static descriptors
	{
		{
			_deviceResources->renderScenePass.uboStatic.sets[0] =
			  _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutVert);

			writeDescSet->set(VkDescriptorType::e_UNIFORM_BUFFER, _deviceResources->renderScenePass.uboStatic.sets[0]);
			writeDescSet->setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->renderScenePass.uboStatic.buffer,
			                            0, _deviceResources->renderScenePass.uboStatic.structuredBufferView.getDynamicSliceSize()));
			++writeDescSet;
		}

		{
			_deviceResources->renderScenePass.texDescriptor =
			  _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texSamplerLayoutFrag);

			writeDescSet->set(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->renderScenePass.texDescriptor);
			writeDescSet->setImageInfo(0, pvrvk::DescriptorImageInfo(
			                             _deviceResources->baseTex, _deviceResources->sceneSamplerClamp));
			++writeDescSet;
		}

		// copy the texture descriptor from the render scene pass
		_deviceResources->preBloomPass.uboStatic = _deviceResources->renderScenePass.uboStatic;
		_deviceResources->preBloomPass.descTex = _deviceResources->renderScenePass.texDescriptor;
	}

	// blur pass (horizontal)
	{
		_deviceResources->horizontalBlurPass.perVertDescriptorSet =
		  _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutVert);
		writeDescSet->set(VkDescriptorType::e_UNIFORM_BUFFER, _deviceResources->horizontalBlurPass.perVertDescriptorSet);
		writeDescSet->setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->horizontalBlurPass.buffer,
		                            0, _deviceResources->horizontalBlurPass.structuredBufferView.getDynamicSliceSize()));
		++writeDescSet;
	}

	// blur pass1 (vertical)
	{
		_deviceResources->verticalBlurPass.perVertDescriptorSet =
		  _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutVert);

		writeDescSet->set(VkDescriptorType::e_UNIFORM_BUFFER, _deviceResources->verticalBlurPass.perVertDescriptorSet);
		writeDescSet->setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->verticalBlurPass.buffer,
		                            0, _deviceResources->verticalBlurPass.structuredBufferView.getDynamicSliceSize()));
		++writeDescSet;
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
	return true;
}

void VulkanPostProcessing::createBuffers()
{
	// dynamic ubos
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("MVInv", pvr::GpuDatatypes::mat4x4);
		desc.addElement("MVPMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("LightDirection", pvr::GpuDatatypes::vec3);

		_deviceResources->renderScenePass.uboDynamic.structuredBufferView.initDynamic(desc, _deviceResources->scene->getNumMeshNodes() * _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));

		_deviceResources->renderScenePass.uboDynamic.buffer = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->renderScenePass.uboDynamic.structuredBufferView.getSize(),
		    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);
	}

	// static ubos
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("Shininess", pvr::GpuDatatypes::Float);

		_deviceResources->renderScenePass.uboStatic.structuredBufferView.init(desc);
		_deviceResources->renderScenePass.uboStatic.buffer = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->renderScenePass.uboStatic.structuredBufferView.getSize(),
		    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);

		// update the buffer once
		void* memory;
		_deviceResources->renderScenePass.uboStatic.buffer->getDeviceMemory()->map(&memory);
		const float shininess = 0.6f;
		_deviceResources->renderScenePass.uboStatic.structuredBufferView.pointToMappedMemory(memory);
		_deviceResources->renderScenePass.uboStatic.structuredBufferView.getElementByName("Shininess").setValue(&shininess);
		_deviceResources->renderScenePass.uboStatic.buffer->getDeviceMemory()->unmap();
	}

	// bloom intensity buffer
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("BloomIntensity", pvr::GpuDatatypes::Float);

		_deviceResources->preBloomPass.structuredBufferView.init(desc);
		_deviceResources->preBloomPass.buffer = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->preBloomPass.structuredBufferView.getSize(),
		                                        VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);

		// update the initial bloom intensity
		void* memory;
		_deviceResources->preBloomPass.buffer->getDeviceMemory()->map(&memory);
		const float bloomIntensity = 1.0f;
		_deviceResources->preBloomPass.structuredBufferView.pointToMappedMemory(memory);
		_deviceResources->preBloomPass.structuredBufferView.getElementByName("BloomIntensity").setValue(&bloomIntensity);
		_deviceResources->preBloomPass.buffer->getDeviceMemory()->unmap();
	}

	// blur pass (horizontal)
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("TexelOffsetX", pvr::GpuDatatypes::Float);
		desc.addElement("TexelOffsetY", pvr::GpuDatatypes::Float);

		_deviceResources->horizontalBlurPass.structuredBufferView.init(desc);
		_deviceResources->horizontalBlurPass.buffer = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->horizontalBlurPass.structuredBufferView.getSize(),
		    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);

		// set the const values
		void* memory;
		_deviceResources->horizontalBlurPass.buffer->getDeviceMemory()->map(&memory);
		const float texelOffsetY = 0.0f;
		_deviceResources->horizontalBlurPass.structuredBufferView.pointToMappedMemory(memory);
		_deviceResources->horizontalBlurPass.structuredBufferView.getElementByName("TexelOffsetX").setValue(&_blurTexelOffset);
		_deviceResources->horizontalBlurPass.structuredBufferView.getElementByName("TexelOffsetY").setValue(&texelOffsetY);
		_deviceResources->horizontalBlurPass.buffer->getDeviceMemory()->unmap();
	}

	// blur pass (vertical)
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("TexelOffsetX", pvr::GpuDatatypes::Float);
		desc.addElement("TexelOffsetY", pvr::GpuDatatypes::Float);

		_deviceResources->verticalBlurPass.structuredBufferView.init(desc);
		_deviceResources->verticalBlurPass.buffer = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->verticalBlurPass.structuredBufferView.getSize(),
		    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);

		// set the const values
		void* memory;
		_deviceResources->verticalBlurPass.buffer->getDeviceMemory()->map(&memory);
		const float texelOffsetX = 0.0f;
		_deviceResources->verticalBlurPass.structuredBufferView.pointToMappedMemory(memory);
		_deviceResources->verticalBlurPass.structuredBufferView.getElementByName("TexelOffsetX").setValue(&texelOffsetX);
		_deviceResources->verticalBlurPass.structuredBufferView.getElementByName("TexelOffsetY").setValue(&_blurTexelOffset);
		_deviceResources->verticalBlurPass.buffer->getDeviceMemory()->unmap();
	}

	// post bloom config
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("sTexFactor", pvr::GpuDatatypes::Float);
		desc.addElement("sBlurTexFactor", pvr::GpuDatatypes::Float);

		_deviceResources->postBloomPass.structuredBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		    static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
		_deviceResources->postBloomPass.buffer = pvr::utils::createBuffer(_deviceResources->device,_deviceResources->postBloomPass.structuredBufferView.getSize(),
		    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);

		// map the entire buffer
		void* memory;
		_deviceResources->postBloomPass.buffer->getDeviceMemory()->map(&memory);
		_deviceResources->postBloomPass.structuredBufferView.pointToMappedMemory(memory);
		const float textureFactors = 1.0f;

		// set the const values - per swap chain
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			_deviceResources->postBloomPass.structuredBufferView.getElementByName("sTexFactor", 0, i).setValue(&textureFactors);
			_deviceResources->postBloomPass.structuredBufferView.getElementByName("sBlurTexFactor", 0, i).setValue(&textureFactors);
		}
		// unmap the entire buffer
		_deviceResources->postBloomPass.buffer->getDeviceMemory()->unmap();
	}
}

void VulkanPostProcessing::createDescriptorSetLayouts()
{
	{
		pvrvk::DescriptorSetLayoutCreateInfo layoutDesc;
		layoutDesc.setBinding(0, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER,
		                      1, VkShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->texSamplerLayoutFrag = _deviceResources->device->createDescriptorSetLayout(layoutDesc);
	}

	{
		pvrvk::DescriptorSetLayoutCreateInfo layoutDesc;
		layoutDesc.setBinding(0, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
		layoutDesc.setBinding(1, VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->postBloomTexLayoutFrag = _deviceResources->device->createDescriptorSetLayout(layoutDesc);
	}

	{
		pvrvk::DescriptorSetLayoutCreateInfo layoutDesc;
		layoutDesc.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER, 1, VkShaderStageFlags::e_VERTEX_BIT);
		_deviceResources->uboLayoutVert = _deviceResources->device->createDescriptorSetLayout(layoutDesc);
	}

	{
		pvrvk::DescriptorSetLayoutCreateInfo layoutDesc;
		layoutDesc.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER, 1, VkShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->uboLayoutFrag = _deviceResources->device->createDescriptorSetLayout(layoutDesc);
	}

	{
		pvrvk::DescriptorSetLayoutCreateInfo layoutDesc;
		layoutDesc.setBinding(0, VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1,
		                      VkShaderStageFlags::e_VERTEX_BIT);
		_deviceResources->uboLayoutDynamicVert = _deviceResources->device->createDescriptorSetLayout(layoutDesc);
	}
}

/*!********************************************************************************************
\brief  Loads and compiles the shaders and links the shader programs
\return Return true if no error occurred required for this training course
***********************************************************************************************/
bool VulkanPostProcessing::createPipelines()
{
	const pvr::assets::Mesh& mesh = _deviceResources->scene->getMesh(0);
	const pvrvk::Extent2D& dim = _deviceResources->swapchain->getDimension();

	// create render scene pass pipeline
	{
		pvrvk::GraphicsPipelineCreateInfo basicPipeDesc;

		// enable backface culling
		basicPipeDesc.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);
		// disable blending
		basicPipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// enable depth testing
		basicPipeDesc.depthStencil.setDepthCompareFunc(VkCompareOp::e_LESS);
		basicPipeDesc.depthStencil.enableDepthTest(true);
		basicPipeDesc.depthStencil.enableDepthWrite(true);
		basicPipeDesc.depthStencil.enableAllStates(true);
		basicPipeDesc.viewport.setViewportAndScissor(0,
		    pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(dim.width), static_cast<float>(dim.height)),
		    pvrvk::Rect2Di(0, 0, dim.width, dim.height));

		pvr::Stream::ptr_type vs = getAssetStream(VertShaderSrcFile);
		basicPipeDesc.vertexShader.setShader(
		  _deviceResources->device->createShader(vs->readToEnd<uint32_t>()));

		basicPipeDesc.fragmentShader.setShader(
		  _deviceResources->device->createShader(getAssetStream(FragShaderSrcFile)->readToEnd<uint32_t>()));

		pvr::utils::populateInputAssemblyFromMesh(
		  mesh, VertexBindings, 3, basicPipeDesc.vertexInput,
		  basicPipeDesc.inputAssembler);

		// create pipeline layout
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(_deviceResources->texSamplerLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutDynamicVert);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutVert);
		basicPipeDesc.pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

		basicPipeDesc.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		basicPipeDesc.subpass = 0;
		_deviceResources->renderScenePass.pipeline = _deviceResources->device->createGraphicsPipeline(basicPipeDesc);

		if (_deviceResources->renderScenePass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed To Create the RenderScenePass Pipeline");
			return false;
		}
	}

	// create prebloom pass pipeline
	{
		pvrvk::GraphicsPipelineCreateInfo prebloomPipeDesc;

		// enable backface culling
		prebloomPipeDesc.rasterizer.setCullMode(VkCullModeFlags::e_BACK_BIT);

		// enable depth testing
		prebloomPipeDesc.depthStencil.setDepthCompareFunc(VkCompareOp::e_LESS);
		prebloomPipeDesc.depthStencil.enableDepthTest(true);
		prebloomPipeDesc.depthStencil.enableDepthWrite(true);

		prebloomPipeDesc.vertexShader =
		  _deviceResources->device->createShader(getAssetStream(PreBloomVertShaderSrcFile)->readToEnd<uint32_t>());

		prebloomPipeDesc.fragmentShader =
		  _deviceResources->device->createShader(getAssetStream(PreBloomFragShaderSrcFile)->readToEnd<uint32_t>());

		pvr::utils::populateInputAssemblyFromMesh(mesh, VertexBindings, 3, prebloomPipeDesc.vertexInput, prebloomPipeDesc.inputAssembler);

		// set blending states - disable blending
		prebloomPipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		prebloomPipeDesc.colorBlend.setAttachmentState(1, pvrvk::PipelineColorBlendAttachmentState());

		// create pipeline layout
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(_deviceResources->texSamplerLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutDynamicVert);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutVert);

		prebloomPipeDesc.pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

		prebloomPipeDesc.renderPass = _deviceResources->preBloomPass.framebuffer[0]->getRenderPass();
		prebloomPipeDesc.subpass = 0;

		prebloomPipeDesc.viewport.setViewportAndScissor(0,
		    pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(dim.width), static_cast<float>(dim.height)),
		    pvrvk::Rect2Di(0, 0, dim.width, dim.height));

		_deviceResources->preBloomPass.pipeline =
		  _deviceResources->device->createGraphicsPipeline(prebloomPipeDesc);

		if (_deviceResources->preBloomPass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed to Create preBloom pipeline");
			return false;
		}
	}

	// create Post-Bloom Pipeline
	{
		pvrvk::GraphicsPipelineCreateInfo postbloomPipeDesc;

		// enable back face culling
		postbloomPipeDesc.rasterizer.setCullMode(VkCullModeFlags::e_FRONT_BIT);

		// set counter clockwise winding order for front faces
		postbloomPipeDesc.rasterizer.setFrontFaceWinding(VkFrontFace::e_COUNTER_CLOCKWISE);
		postbloomPipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		postbloomPipeDesc.depthStencil.enableDepthTest(false);
		postbloomPipeDesc.depthStencil.enableDepthWrite(false);
		postbloomPipeDesc.depthStencil.enableStencilTest(false);

		postbloomPipeDesc.vertexShader =
		  _deviceResources->device->createShader(getAssetStream(PostBloomVertShaderSrcFile)->readToEnd<uint32_t>());

		postbloomPipeDesc.fragmentShader =
		  _deviceResources->device->createShader(getAssetStream(PostBloomFragShaderSrcFile)->readToEnd<uint32_t>());

		postbloomPipeDesc.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		postbloomPipeDesc.subpass = 0;

		// setup vertex inputs
		postbloomPipeDesc.vertexInput.clear();
		postbloomPipeDesc.inputAssembler.setPrimitiveTopology(VkPrimitiveTopology::e_TRIANGLE_LIST);

		// create pipeline layout
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.setDescSetLayout(0, _deviceResources->postBloomTexLayoutFrag);
		pipeLayoutInfo.setDescSetLayout(1, _deviceResources->uboLayoutFrag);

		postbloomPipeDesc.pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

		postbloomPipeDesc.viewport.setViewportAndScissor(0,
		    pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(dim.width), static_cast<float>(dim.height)),
		    pvrvk::Rect2Di(0, 0, dim.width, dim.height));

		_deviceResources->postBloomPass.pipeline = _deviceResources->device->createGraphicsPipeline(postbloomPipeDesc);

		if (_deviceResources->postBloomPass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed to Create postBloom pipeline");
			return false;
		}
	}

	//   Blur Pipeline
	{
		pvrvk::GraphicsPipelineCreateInfo blurPipeDesc;

		// enable back face culling
		blurPipeDesc.rasterizer.setCullMode(VkCullModeFlags::e_FRONT_BIT);

		// set counter clockwise winding order for front faces
		blurPipeDesc.rasterizer.setFrontFaceWinding(VkFrontFace::e_COUNTER_CLOCKWISE);

		// set blending states - disable blending
		blurPipeDesc.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		blurPipeDesc.depthStencil.enableDepthTest(false);
		blurPipeDesc.depthStencil.enableDepthWrite(false);
		blurPipeDesc.depthStencil.enableStencilTest(false);

		blurPipeDesc.vertexShader = _deviceResources->device->createShader(
		                              getAssetStream(BlurVertSrcFile)->readToEnd<uint32_t>());

		blurPipeDesc.fragmentShader = _deviceResources->device->createShader(
		                                getAssetStream(BlurFragSrcFile)->readToEnd<uint32_t>());

		// setup vertex inputs
		blurPipeDesc.vertexInput.clear();
		blurPipeDesc.inputAssembler.setPrimitiveTopology(VkPrimitiveTopology::e_TRIANGLE_STRIP);

		const pvrvk::Rect2Di region(0, 0, _deviceResources->horizontalBlurPass.framebuffer[0]->getDimensions().width,
		                            _deviceResources->horizontalBlurPass.framebuffer[0]->getDimensions().height);

		blurPipeDesc.viewport.setViewportAndScissor(0, pvrvk::Viewport(region), region);

		// create pipeline layout
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(_deviceResources->texSamplerLayoutFrag);
		pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutVert);
		blurPipeDesc.pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

		blurPipeDesc.renderPass = _deviceResources->horizontalBlurPass.framebuffer[0]->getRenderPass();
		blurPipeDesc.subpass = 0;

		_deviceResources->horizontalBlurPass.pipeline = _deviceResources->verticalBlurPass.pipeline =
		      _deviceResources->device->createGraphicsPipeline(blurPipeDesc);

		if (_deviceResources->horizontalBlurPass.pipeline.isValid() == false)
		{
			this->setExitMessage("Failed to Create Blur pipeline");
			return false;
		}
	}
	return true;
}

/*!********************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering
        context is created.
        Used to initialize variables that are not dependent on it (e.g. external modules,
        loading meshes, etc.)
        If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************/
pvr::Result VulkanPostProcessing::initApplication()
{
	_deviceResources.reset(new DeviceResrcs());
	this->setStencilBitsPerPixel(0);

	// Apply bloom per default
	_applyBloom = true;
	_drawObject = true;
	_animating = true;

	_rotation = 0.0f;

	// Load the scene
	if (!pvr::assets::helper::loadModel(*this, SceneFile, _deviceResources->scene))
	{
		this->setExitMessage("Error: Couldn't load the %s file\n", SceneFile);
		return pvr::Result::UnknownError;
	}

	// calculate initial view matrix
	float fov;
	glm::vec3 from, to, up;
	_deviceResources->scene->getCameraProperties(0, fov, from, to, up);
	_viewMatrix = glm::lookAt(from, to, up);
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return Return  Result::Success if no error occured
\brief  Code in quitApplication() will be called by Shell once per run, just before exiting the program.
quitApplication() will not be called every time the rendering context is lost, only before application exit.
***********************************************************************************************/
pvr::Result VulkanPostProcessing::quitApplication()
{
	//Instructs the Asset Manager to free all resources
	return pvr::Result::Success;
}

/*!********************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change
        in the rendering context. Used to initialize variables that are dependent on the rendering
        context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************/
pvr::Result VulkanPostProcessing::initView()
{
	_frameId = 0;
	pvrvk::Surface surface;
	bool result = pvr::utils::createInstanceAndSurface(this->getApplicationName(), this->getWindow(),
	              this->getDisplay(), _deviceResources->instance, surface);
	if (!result)
	{
		return pvr::Result::UnknownError;
	}

	// look for a queue from queue family 0
	pvr::utils::QueuePopulateInfo queueCreateInfo =
	{
		VkQueueFlags::e_GRAPHICS_BIT | VkQueueFlags::e_COMPUTE_BIT, surface,
	};
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0),
	                           &queueCreateInfo, 1, &queueAccessInfo);

	if (!_deviceResources->device.isValid())
	{
		setExitMessage("failed to create Logical device");
		return pvr::Result::UnknownError;
	}
	_deviceResources->queues[0] = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	VkImageUsageFlags swapchainImageUsage = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, VkImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= VkImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	if (!pvr::utils::createSwapchainAndDepthStencilImageView(_deviceResources->device, surface, getDisplayAttributes(),
	    _deviceResources->swapchain, _deviceResources->depthStencilImages, swapchainImageUsage))
	{
		setExitMessage("Failed to create SwapChain/Depth stencil");
		return pvr::Result::UnknownError;
	}

	if (!_deviceResources->swapchain.isValid())
	{
		return pvr::Result::UnknownError;
	}

	// Calculates the projection matrix
	float fov = _deviceResources->scene->getCamera(0).getFOV();
	bool bRotate = isFullScreen() && isScreenRotated();
	if (bRotate)
	{
		_projectionMatrix = pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, (float)getHeight(), (float)getWidth(),
		                    _deviceResources->scene->getCamera(0).getNear(), _deviceResources->scene->getCamera(0).getFar(), glm::pi<float>() * .5f);
	}
	else
	{
		_projectionMatrix = pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, (float)getWidth(), (float)getHeight(),
		                    _deviceResources->scene->getCamera(0).getNear(), _deviceResources->scene->getCamera(0).getFar());
	}

	_blurDimension = 256;

	// create the commandpool and the descriptor pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
	                                  _deviceResources->queues[0]->getQueueFamilyId(),
	                                  VkCommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);

	_deviceResources->descriptorPool =  _deviceResources->device->createDescriptorPool(
	                                      pvrvk::DescriptorPoolCreateInfo()
	                                      .setMaxDescriptorSets(45)
	                                      .addDescriptorInfo(VkDescriptorType::e_COMBINED_IMAGE_SAMPLER, 15)
	                                      .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 15)
	                                      .addDescriptorInfo(VkDescriptorType::e_UNIFORM_BUFFER, 15));

	//  Initialize VBO data
	// Load vertex data of all meshes in the scene into VBOs
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	// Interleaving data improves the memory access pattern and cache efficiency,
	// thus it can be read faster by the hardware.
	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device,
	    *_deviceResources->scene, _deviceResources->vbos, _deviceResources->ibos);

	// create the initial commandbuffer which will be used for layout transition.
	_deviceResources->mainCmdNoBloom[0] = _deviceResources->commandPool->allocateCommandBuffer();
	_deviceResources->mainCmdNoBloom[0]->begin();

	// Create framebuffers and do initial image transition
	if (!createOnScreenFramebuffer())
	{
		return pvr::Result::NotInitialized;
	}

	if (!createBlurFramebuffer(_deviceResources->mainCmdNoBloom[0]))
	{
		setExitMessage("Failed to create BlurFramebuffer");
		return pvr::Result::NotInitialized;
	}

	// create Framebuffer used for the pre bloom pass
	if (!createPreBloomFramebuffer(_deviceResources->mainCmdNoBloom[0]))
	{
		setExitMessage("Failed to bloom framebuffer");
		return pvr::Result::NotInitialized;
	}

	// calculate the texel offsets used in the blurring passes
	calculateBlurTexelOffsets();

	// create demo buffers
	createBuffers();

	// create the descriptor set layouts and pipeline layouts
	createDescriptorSetLayouts();

	//  Load textures
	// Load Textures
	pvr::utils::ImageUploadResults uploadResult = pvr::utils::loadAndUploadImage(_deviceResources->device, BaseTexFile,
	    true, _deviceResources->mainCmdNoBloom[0], *this);
	if (uploadResult.getResult() != pvr::Result::Success)
	{
		setExitMessage("FAILED to load texture %s.", BaseTexFile);
		return pvr::Result::UnknownError;
	}
	_deviceResources->baseTex = uploadResult.getImageView();

	if (!createDescriptors())
	{
		setExitMessage("Failed to create DescriptorSets");
		return pvr::Result::NotInitialized;
	}

	// create the graphics pipelines used throughout the demo
	if (!createPipelines())
	{
		setExitMessage("Failed to create Pipelines");
		return pvr::Result::NotInitialized;
	}

	if (!_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0, _deviceResources->commandPool,
	                                       _deviceResources->queues[0]))
	{
		setExitMessage("Failed to initialize the UIRenderer\n");
		return pvr::Result::NotInitialized;
	}

	_deviceResources->uiRenderer.getDefaultTitle()->setText("PostProcessing");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText(
	  "Left / right: Rendering mode\n"
	  "Up / down: Bloom intensity\n"
	  "Action:     Pause\n"
	);
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	updateSubtitleText();
	_deviceResources->mainCmdNoBloom[0]->end();

	pvrvk::SubmitInfo submiInfo;
	submiInfo.commandBuffers = &_deviceResources->mainCmdNoBloom[0];
	submiInfo.numCommandBuffers = 1;
	_deviceResources->queues[0]->submit(&submiInfo, 1);

	_deviceResources->device->waitIdle();
	_deviceResources->uiImageUploadResult.clear();
	recordCommandBuffers();

	// create the semaphores
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->semaphorePresent[i] = _deviceResources->device->createSemaphore();
		_deviceResources->semaphoreImageAcquired[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameCommandBufferFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameAcquireFence[i] = _deviceResources->device->createFence(VkFenceCreateFlags::e_SIGNALED_BIT);
	}

	return pvr::Result::Success;
}

void VulkanPostProcessing::createCommandBuffers(uint32_t swapchain)
{
	if (!_deviceResources->mainCmdNoBloom[swapchain].isValid())
	{
		_deviceResources->mainCmdNoBloom[swapchain] = _deviceResources->commandPool->allocateCommandBuffer();
	}
	if (!_deviceResources->mainCmdBloom[swapchain].isValid())
	{
		_deviceResources->mainCmdBloom[swapchain] = _deviceResources->commandPool->allocateCommandBuffer();
	}
	if (!_deviceResources->preBloomCommandBuffer[swapchain].isValid())
	{
		_deviceResources->preBloomCommandBuffer[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
	}
	if (!_deviceResources->noBloomCommandBuffer[swapchain].isValid())
	{
		_deviceResources->noBloomCommandBuffer[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
	}
	if (!_deviceResources->noBloomUiRendererCommandBuffer[swapchain].isValid())
	{
		_deviceResources->noBloomUiRendererCommandBuffer[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
	}
	if (!_deviceResources->bloomUiRendererCommandBuffer[swapchain].isValid())
	{
		_deviceResources->bloomUiRendererCommandBuffer[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
	}
	if (!_deviceResources->horizontalBlurCommandBuffer[swapchain].isValid())
	{
		_deviceResources->horizontalBlurCommandBuffer[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
	}
	if (!_deviceResources->verticalBlurCommandBuffer[swapchain].isValid())
	{
		_deviceResources->verticalBlurCommandBuffer[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
	}
	if (!_deviceResources->postBloomCommandBuffer[swapchain].isValid())
	{
		_deviceResources->postBloomCommandBuffer[swapchain] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
	}
}

void VulkanPostProcessing::recordCommandBuffers()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		createCommandBuffers(i);
		recordCommandUIRenderer(i);

		// record no bloom command buffer
		recordNoBloomCommands(i);

		// record bloom command buffer
		recordBloomCommands(i);
	}
}

/*!********************************************************************************************
\brief  Create the blur framebuffer
\return Return  true on success
***********************************************************************************************/
bool VulkanPostProcessing::createBlurFramebuffer(pvrvk::CommandBuffer imageLayoutTransCmd)
{
	VkFormat colorFmt = VkFormat::e_R8G8B8A8_UNORM;
	// create the render passes.
	pvrvk::RenderPassCreateInfo blurRenderPassDesc;

	pvrvk::SubPassDescription subpass;
	// use the first color attachment
	subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

	// setup subpasses
	blurRenderPassDesc.setAttachmentDescription(0, pvrvk::AttachmentDescription::createColorDescription(colorFmt,
	    VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	blurRenderPassDesc.setSubPass(0, subpass);

	// create renderpass
	pvrvk::RenderPass blurRenderPass = _deviceResources->device->createRenderPass(blurRenderPassDesc);

	pvrvk::FramebufferCreateInfo blurFramebufferDesc;
	blurFramebufferDesc.setRenderPass(blurRenderPass);

	// blur at a much lower resolution
	blurFramebufferDesc.setDimensions(_blurDimension, _blurDimension);
	pvrvk::DeviceWeakPtr device = _deviceResources->device;

	// for each swapchain
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// blur pass0
		{
			// call the non-sparse as createImage2D ??
			// can we have single function for 2d, 3d, 1d
			// can we have a function, mirrors vulkan and helpers for 3d, 3d etc.
			pvrvk::Image colorTex = pvr::utils::createImage(device, VkImageType::e_2D, colorFmt, pvrvk::Extent3D(_blurDimension, _blurDimension, 1u),
			                        VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT | VkImageUsageFlags::e_SAMPLED_BIT);

			pvr::utils::setImageLayout(colorTex, VkImageLayout::e_UNDEFINED,
			                           VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, imageLayoutTransCmd);


			if (colorTex.isNull())
			{
				return false;
			}
			// set framebuffer color attachments
			blurFramebufferDesc.setAttachment(0, device->createImageView(colorTex));

			// create the blur pass framebuffer
			_deviceResources->horizontalBlurPass.framebuffer[i] = _deviceResources->device->createFramebuffer(blurFramebufferDesc);

			if (!_deviceResources->horizontalBlurPass.framebuffer[i].isValid())
			{
				Log("Failed to create blur framebuffer");
				return false;
			}
		}
		// blur pass1
		{
			pvrvk::Image colorTex  = pvr::utils::createImage(device, VkImageType::e_2D, colorFmt, pvrvk::Extent3D(_blurDimension, _blurDimension, 1u),
			                         VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT | VkImageUsageFlags::e_SAMPLED_BIT);

			pvr::utils::setImageLayout(colorTex, VkImageLayout::e_UNDEFINED,
			                           VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, imageLayoutTransCmd);

			// set framebuffer color attachments
			blurFramebufferDesc.setAttachment(0, _deviceResources->device->createImageView(colorTex));

			// create the blur pass framebuffer
			_deviceResources->verticalBlurPass.framebuffer[i] = _deviceResources->device->createFramebuffer(blurFramebufferDesc);

			if (!_deviceResources->verticalBlurPass.framebuffer[i].isValid())
			{
				Log("Failed to create blur framebuffer");
				return false;
			}
		}
	}

	return true;
}

bool VulkanPostProcessing::createPreBloomFramebuffer(pvrvk::CommandBuffer imageTransCmdBuffer)
{
	// color and depth image formats
	VkFormat dsFormat = VkFormat::e_D16_UNORM;
	VkFormat colorFormat = VkFormat::e_R8G8B8A8_UNORM;

	// depth texture storage
	pvr::Multi<pvrvk::Image> depthTexture;

	// color texture storage
	pvr::Multi<pvrvk::Image> colorTexture;
	pvr::Multi<pvrvk::Image> filterTexture;

	// create the render pass
	pvrvk::RenderPassCreateInfo renderPassInfo;
	const pvrvk::AttachmentDescription dsInfo = pvrvk::AttachmentDescription::createDepthStencilDescription(dsFormat,
	    VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	    VkAttachmentLoadOp::e_CLEAR, VkAttachmentStoreOp::e_DONT_CARE, VkAttachmentLoadOp::e_CLEAR, VkAttachmentStoreOp::e_DONT_CARE);

	const pvrvk::AttachmentDescription colorInfo = pvrvk::AttachmentDescription::createColorDescription(colorFormat,
	    VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL);

	renderPassInfo.setAttachmentDescription(0, colorInfo).setAttachmentDescription(1, colorInfo).setAttachmentDescription(2, dsInfo);

	// configure the subpass
	pvrvk::SubPassDescription subpass; subpass
	.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL))
	.setColorAttachmentReference(1, pvrvk::AttachmentReference(1, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL))
	.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(2, VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	renderPassInfo.setSubPass(0, subpass);

	// create the renderpass
	pvrvk::RenderPass renderPass = _deviceResources->device->createRenderPass(renderPassInfo);

	// pre bloom render area uses the full screen dimensions
	pvrvk::ImageAreaSize imageSize(pvrvk::ImageLayersSize(), pvrvk::Extent2D(getWidth(), getHeight()));
	// create the framebuffer
	pvrvk::FramebufferCreateInfo framebufferInfo;
	framebufferInfo.setRenderPass(renderPass);
	framebufferInfo.setDimensions(imageSize.width, imageSize.height);
	pvrvk::DeviceWeakPtr device = _deviceResources->device;

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// create depth texture as transient
		depthTexture[i] = pvr::utils::createImage(device, VkImageType::e_2D, dsFormat, pvrvk::Extent3D(getWidth(), getHeight(), 1u),
		                                      VkImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT,
		                                      VkImageCreateFlags(0), pvrvk::ImageLayersSize(), VkSampleCountFlags::e_1_BIT,
		                                      VkMemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT);

		pvr::utils::setImageLayout(depthTexture[i], VkImageLayout::e_UNDEFINED,
		                           VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, imageTransCmdBuffer);

		// color and filter textures will be sampled
		colorTexture[i] = pvr::utils::createImage(device, VkImageType::e_2D, colorFormat, pvrvk::Extent3D(getWidth(), getHeight(), 1u),
		                                      VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT | VkImageUsageFlags::e_SAMPLED_BIT);

		pvr::utils::setImageLayout(colorTexture[i], VkImageLayout::e_UNDEFINED,
		                           VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, imageTransCmdBuffer);

		filterTexture[i] = pvr::utils::createImage(device, VkImageType::e_2D, colorFormat, pvrvk::Extent3D(getWidth(), getHeight(), 1u),
		                                       VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT | VkImageUsageFlags::e_SAMPLED_BIT);

		pvr::utils::setImageLayout(filterTexture[i], VkImageLayout::e_UNDEFINED,
		                           VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, imageTransCmdBuffer);

		// set color attachments
		framebufferInfo.setAttachment(0, device->createImageView(colorTexture[i]));
		framebufferInfo.setAttachment(1, device->createImageView(filterTexture[i]));

		// set depth stencil attachment
		framebufferInfo.setAttachment(2, device->createImageView(depthTexture[i]));

		// create the framebuffer
		_deviceResources->preBloomPass.framebuffer[i] = _deviceResources->device->createFramebuffer(framebufferInfo);

		if (!_deviceResources->preBloomPass.framebuffer[i].isValid())
		{
			Log("Failed to create the rendering framebuffer");
			return false;
		}
	}
	return true;
}

/*!********************************************************************************************
\return Return Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before
a change in the rendering context.
***********************************************************************************************/
pvr::Result VulkanPostProcessing::releaseView()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_deviceResources->perFrameAcquireFence[i]->wait();
		_deviceResources->perFrameAcquireFence[i]->reset();

		_deviceResources->perFrameCommandBufferFence[i]->wait();
		_deviceResources->perFrameCommandBufferFence[i]->reset();
	}

	_deviceResources->device->waitIdle();// make sure the device is free
	_deviceResources.reset();
	return pvr::Result::Success;
}

void VulkanPostProcessing::updatePostBloomConfig(uint32_t swapchain)
{
	if (_applyBloom)
	{
		float config[] = {(_drawObject ? 1.f : 0.0f), 1.f};
		void* memory;
		_deviceResources->postBloomPass.buffer->getDeviceMemory()->map(&memory, _deviceResources->postBloomPass.structuredBufferView.getDynamicSliceOffset(swapchain),
		    _deviceResources->postBloomPass.structuredBufferView.getDynamicSliceSize());
		_deviceResources->postBloomPass.structuredBufferView.pointToMappedMemory(memory, swapchain);
		_deviceResources->postBloomPass.structuredBufferView.getElementByName("sTexFactor", 0, swapchain).setValue(&config[0]);
		_deviceResources->postBloomPass.structuredBufferView.getElementByName("sBlurTexFactor", 0, swapchain).setValue(&config[1]);
		_deviceResources->postBloomPass.buffer->getDeviceMemory()->unmap();
	}
}

/*!*********************************************************************************************************************
\brief Update the animation
***********************************************************************************************************************/
void VulkanPostProcessing::updateAnimation()
{
	// Calculate the mask and light _rotation based on the passed time
	float const twoPi = glm::pi<float>() * 2.f;

	if (_animating)
	{
		_rotation += glm::pi<float>() * getFrameTime() * 0.0002f;
		// wrap it
		if (_rotation > twoPi) { _rotation -= twoPi; }
	}

	// Calculate the model, viewMatrix and projection matrix
	_worldMatrix = glm::rotate((-_rotation), glm::vec3(0.f, 1.f, 0.f)) *
	               glm::scale(glm::vec3(1.65f));

	float fov;
	fov = _deviceResources->scene->getCamera(0).getFOV(0);

	glm::mat4x4 viewProj = _projectionMatrix * _viewMatrix;
	// Simple rotating directional light in model-space)
	const glm::mat4& mvInv = glm::inverse(_viewMatrix * _worldMatrix *
	                                      _deviceResources->scene->getWorldMatrix(_deviceResources->scene->getNode(0).getObjectId()));

	const glm::mat4& mvp = viewProj * _worldMatrix *
	                       _deviceResources->scene->getWorldMatrix(_deviceResources->scene->getNode(0).getObjectId());

	// map the current swap chain slice only
	void* memory;
	uint32_t mappedDynamicSlice = _deviceResources->swapchain->getSwapchainIndex() * _deviceResources->scene->getNumMeshNodes();
	_deviceResources->renderScenePass.uboDynamic.buffer->getDeviceMemory()->map(&memory,
	    _deviceResources->renderScenePass.uboDynamic.structuredBufferView.getDynamicSliceOffset(mappedDynamicSlice),
	    _deviceResources->renderScenePass.uboDynamic.structuredBufferView.getDynamicSliceSize() * _deviceResources->scene->getNumMeshNodes());

	_deviceResources->renderScenePass.uboDynamic.structuredBufferView.pointToMappedMemory(memory, mappedDynamicSlice);

	for (uint32_t i = 0; i < _deviceResources->scene->getNumMeshNodes(); ++i)
	{
		uint32_t dynamicSlice = i + mappedDynamicSlice;
		_deviceResources->renderScenePass.uboDynamic.structuredBufferView.getElement(static_cast<uint32_t>(RenderScenePass::UboDynamicElements::MVInv),
		    0, dynamicSlice).setValue(mvInv);
		_deviceResources->renderScenePass.uboDynamic.structuredBufferView.getElement(static_cast<uint32_t>(RenderScenePass::UboDynamicElements::MVPMatrix),
		    0, dynamicSlice).setValue(mvp);
		_deviceResources->renderScenePass.uboDynamic.structuredBufferView.getElement(static_cast<uint32_t>(RenderScenePass::UboDynamicElements::LightDirection),
		    0, dynamicSlice).setValue(glm::normalize(glm::vec3(glm::inverse(_worldMatrix) * LightPosition)));
	}
	_deviceResources->renderScenePass.uboDynamic.buffer->getDeviceMemory()->unmap();
}

void VulkanPostProcessing::updateBloomIntensity(float bloomIntensity)
{
	this->_bloomIntensity = bloomIntensity;
	void* memory;
	_deviceResources->preBloomPass.buffer->getDeviceMemory()->map(&memory);
	_deviceResources->preBloomPass.structuredBufferView.pointToMappedMemory(memory);
	_deviceResources->preBloomPass.structuredBufferView.getElementByName("BloomIntensity").setValue(&_bloomIntensity);
	_deviceResources->preBloomPass.buffer->getDeviceMemory()->unmap();
}

/*!********************************************************************************************
\return Return Result::Suceess if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************/
pvr::Result VulkanPostProcessing::renderFrame()
{
	// wait and reset the fence before using it.
	_deviceResources->perFrameAcquireFence[_frameId]->wait();
	_deviceResources->perFrameAcquireFence[_frameId]->reset();
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->semaphoreImageAcquired[_frameId], _deviceResources->perFrameAcquireFence[_frameId]);
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->wait();
	_deviceResources->perFrameCommandBufferFence[swapchainIndex]->reset();

	updateAnimation();
	pvrvk::Queue& queue = _deviceResources->queues[0];
	pvrvk::SubmitInfo submitInfo;
	VkPipelineStageFlags submitWaitFlags = VkPipelineStageFlags::e_TRANSFER_BIT;
	submitInfo.waitDestStages = &submitWaitFlags;
	submitInfo.waitSemaphores = &_deviceResources->semaphoreImageAcquired[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->semaphorePresent[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.numCommandBuffers = 1;
	if (_applyBloom)
	{
		submitInfo.commandBuffers = &_deviceResources->mainCmdBloom[swapchainIndex];
	}
	else
	{
		submitInfo.commandBuffers = &_deviceResources->mainCmdNoBloom[swapchainIndex];
	}
	queue->submit(&submitInfo, 1, _deviceResources->perFrameCommandBufferFence[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		if (_deviceResources->swapchain->supportsUsage(VkImageUsageFlags::e_TRANSFER_SRC_BIT))
		{
			pvr::utils::takeScreenshot(_deviceResources->swapchain, swapchainIndex, _deviceResources->commandPool, queue, this->getScreenshotFileName());
		}
		else
		{
			Log(LogLevel::Warning, "Could not take screenshot as the swapchain does not support TRANSFER_SRC_BIT");
		}
	}

	pvrvk::PresentInfo presentInfo;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.numSwapchains = 1;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &_deviceResources->semaphorePresent[_frameId];
	presentInfo.swapchains = &_deviceResources->swapchain;
	queue->present(presentInfo);
	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

bool VulkanPostProcessing::createOnScreenFramebuffer()
{
	// Create on screen Framebuffers
	return pvr::utils::createOnscreenFramebufferAndRenderpass(_deviceResources->swapchain,
	       &_deviceResources->depthStencilImages[0], _deviceResources->onScreenFramebuffer);
}

/*!********************************************************************************************
\brief  update the subtitle sprite
***********************************************************************************************/
void VulkanPostProcessing::updateSubtitleText()
{
	if (_applyBloom)
	{
		if (_drawObject)
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText(
			  pvr::strings::createFormatted("Object with bloom effect, intensity % 2.1f", _bloomIntensity));
		}
		else
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText(
			  pvr::strings::createFormatted("Bloom effect textures, intensity % 2.1f", _bloomIntensity));
		}
	}
	else
	{
		if (_drawObject)
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText("Object without bloom");
		}
		else
		{
			_deviceResources->uiRenderer.getDefaultDescription()->setText("Use up - down to draw object and / or bloom textures");
		}
	}
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
}

/*!********************************************************************************************
\brief  Handles user input and updates live variables accordingly.
***********************************************************************************************/
void VulkanPostProcessing::eventMappedInput(pvr::SimplifiedInput e)
{
	static int mode = 0;
	//Object+Bloom, object, bloom
	switch (e)
	{
	case pvr::SimplifiedInput::Left:
		if (--mode < 0) { mode = 2; }
		_applyBloom = (mode != 1); _drawObject = (mode != 2);
		updateSubtitleText();
		_deviceResources->device->waitIdle();
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			updatePostBloomConfig(i);
		}
		recordCommandBuffers();
		break;
	case pvr::SimplifiedInput::Right:
		++mode %= 3;
		_applyBloom = (mode != 1); _drawObject = (mode != 2);
		updateSubtitleText();
		_deviceResources->device->waitIdle();
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			updatePostBloomConfig(i);
		}
		recordCommandBuffers();
		break;
	case pvr::SimplifiedInput::Up:
		updateSubtitleText();
		updateBloomIntensity(std::min(_bloomIntensity + 0.1f, 5.f));
		_deviceResources->device->waitIdle();
		recordCommandBuffers();
		break;
	case pvr::SimplifiedInput::Down:
		updateBloomIntensity(std::max(_bloomIntensity - 0.1f, 0.f));
		updateSubtitleText();
		_deviceResources->device->waitIdle();
		recordCommandBuffers();
		break;
	case pvr::SimplifiedInput::ActionClose:
		this->exitShell();
		break;
	case pvr::SimplifiedInput::Action1:
	case pvr::SimplifiedInput::Action2:
	case pvr::SimplifiedInput::Action3:
		_animating = !_animating;
		break;
	default:
		break;
	}
}

/*!********************************************************************************************
\param  nodeIndex Node index of the mesh to draw
\brief  Draws a Model::Mesh after the model viewMatrix matrix has been set and the material prepared.
***********************************************************************************************/
void VulkanPostProcessing::drawMesh(int nodeIndex, pvrvk::SecondaryCommandBuffer& commandBuffer)
{
	int meshIndex = _deviceResources->scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Model::Mesh& mesh = _deviceResources->scene->getMesh(meshIndex);
	// bind the VBO for the mesh
	commandBuffer->bindVertexBuffer(_deviceResources->vbos[meshIndex], 0, 0);
	// bind the index buffer, won't hurt if the handle is 0
	commandBuffer->bindIndexBuffer(_deviceResources->ibos[meshIndex], 0, pvr::utils::convertToVk(mesh.getFaces().getDataType()));

	if (mesh.getMeshInfo().isIndexed)
	{
		// Indexed Triangle list
		commandBuffer->drawIndexed(0, mesh.getNumFaces() * 3);
	}
	else
	{
		// Non-Indexed Triangle list
		commandBuffer->draw(0, mesh.getNumFaces() * 3);
	}
}

void VulkanPostProcessing::recordCommandUIRenderer(uint32_t swapchain)
{
	_deviceResources->noBloomUiRendererCommandBuffer[swapchain]->begin(_deviceResources->onScreenFramebuffer[swapchain], 0,
	    VkCommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	_deviceResources->uiRenderer.beginRendering(_deviceResources->noBloomUiRendererCommandBuffer[swapchain]);

	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->noBloomUiRendererCommandBuffer[swapchain]->end();

	_deviceResources->bloomUiRendererCommandBuffer[swapchain]->begin(_deviceResources->onScreenFramebuffer[swapchain], 0,
	    VkCommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	_deviceResources->uiRenderer.beginRendering(
	  _deviceResources->bloomUiRendererCommandBuffer[swapchain]);
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->bloomUiRendererCommandBuffer[swapchain]->end();
}

void VulkanPostProcessing::recordCommandsNoBloom(uint32_t swapchain)
{
	_deviceResources->noBloomCommandBuffer[swapchain]->begin(
	  _deviceResources->onScreenFramebuffer[swapchain], 0,
	  VkCommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	// Simple rotating directional light in model-space
	// Use simple shader program to render the mask
	_deviceResources->noBloomCommandBuffer[swapchain]->bindPipeline(_deviceResources->renderScenePass.pipeline);

	// Bind descriptor Sets
	// bind the albedo texture
	_deviceResources->noBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->renderScenePass.pipeline->getPipelineLayout(), 0, _deviceResources->renderScenePass.texDescriptor);

	uint32_t uboOffset = static_cast<uint32_t>(_deviceResources->renderScenePass.uboDynamic.structuredBufferView.getDynamicSliceOffset(swapchain));

	_deviceResources->noBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->renderScenePass.pipeline->getPipelineLayout(), 1, _deviceResources->renderScenePass.uboDynamic.sets[swapchain], &uboOffset, 1);

	_deviceResources->noBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->renderScenePass.pipeline->getPipelineLayout(), 1, _deviceResources->renderScenePass.uboDynamic.sets[swapchain], &uboOffset, 1);

	_deviceResources->noBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->renderScenePass.pipeline->getPipelineLayout(), 1, _deviceResources->renderScenePass.uboDynamic.sets[swapchain], &uboOffset, 1);

	_deviceResources->noBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->renderScenePass.pipeline->getPipelineLayout(), 2, _deviceResources->renderScenePass.uboStatic.sets[0]);

	// Draw the mesh
	drawMesh(0, _deviceResources->noBloomCommandBuffer[swapchain]);
	_deviceResources->noBloomCommandBuffer[swapchain]->end();
}

void VulkanPostProcessing::recordNoBloomCommands(uint32_t swapchain)
{
	recordCommandsNoBloom(swapchain);

	_deviceResources->mainCmdNoBloom[swapchain]->begin();
	const pvrvk::ClearValue clearValues[]  =
	{
		pvrvk::ClearValue(0.00f, 0.70f, 0.67f, 1.f),
		pvrvk::ClearValue::createDefaultDepthStencilClearValue()
	};
	_deviceResources->mainCmdNoBloom[swapchain]->beginRenderPass(_deviceResources->onScreenFramebuffer[swapchain], _deviceResources->onScreenFramebuffer[swapchain]->getRenderPass(),
	    pvrvk::Rect2Di(0, 0, getWidth(), getHeight()), false, clearValues, ARRAY_SIZE(clearValues));

	_deviceResources->mainCmdNoBloom[swapchain]->executeCommands(_deviceResources->noBloomCommandBuffer[swapchain]);
	_deviceResources->mainCmdNoBloom[swapchain]->executeCommands(_deviceResources->noBloomUiRendererCommandBuffer[swapchain]);
	_deviceResources->mainCmdNoBloom[swapchain]->endRenderPass();
	_deviceResources->mainCmdNoBloom[swapchain]->end();
}

void VulkanPostProcessing::recordCommandsPreBloom(uint32_t swapchain)
{
	_deviceResources->preBloomCommandBuffer[swapchain]->begin(
	  _deviceResources->preBloomPass.framebuffer[swapchain], 0, VkCommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	// filter the bright portion of the image
	_deviceResources->preBloomCommandBuffer[swapchain]->bindPipeline(_deviceResources->preBloomPass.pipeline);

	uint32_t uboOffset = static_cast<uint32_t>(_deviceResources->renderScenePass.uboDynamic.structuredBufferView.getDynamicSliceOffset(swapchain));

	// bind the pre bloom descriptor sets
	_deviceResources->preBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->preBloomPass.pipeline->getPipelineLayout(), 0, _deviceResources->preBloomPass.descTex);

	_deviceResources->preBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->preBloomPass.pipeline->getPipelineLayout(), 1, _deviceResources->preBloomPass.descIntensity);

	_deviceResources->preBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->preBloomPass.pipeline->getPipelineLayout(), 2, _deviceResources->preBloomPass.uboDynamic.sets[swapchain], &uboOffset, 1);

	_deviceResources->preBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->preBloomPass.pipeline->getPipelineLayout(), 3, _deviceResources->preBloomPass.uboStatic.sets[0]);

	drawMesh(0, _deviceResources->preBloomCommandBuffer[swapchain]);
	_deviceResources->preBloomCommandBuffer[swapchain]->end();
}

void VulkanPostProcessing::recordCommandsBlur(
  pvrvk::SecondaryCommandBuffer& commandBuffer, BlurPass& pass, uint32_t swapchain)
{
	commandBuffer->begin(pass.framebuffer[swapchain], 0, VkCommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
	commandBuffer->bindPipeline(pass.pipeline);
	commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, pass.pipeline->getPipelineLayout(),
	                                 0, pass.texDescSet[swapchain]);

	commandBuffer->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS, pass.pipeline->getPipelineLayout(),
	                                 1, pass.perVertDescriptorSet);

	commandBuffer->draw(0, 3);
	commandBuffer->end();
}

void VulkanPostProcessing::recordCommandsPostBloom(uint32_t swapchain)
{
	_deviceResources->postBloomCommandBuffer[swapchain]->begin(_deviceResources->onScreenFramebuffer[swapchain], 0,
	    VkCommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	_deviceResources->postBloomCommandBuffer[swapchain]->bindPipeline(_deviceResources->postBloomPass.pipeline);

	_deviceResources->postBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->postBloomPass.pipeline->getPipelineLayout(), 0, _deviceResources->postBloomPass.texDescSet[swapchain]);

	_deviceResources->postBloomCommandBuffer[swapchain]->bindDescriptorSet(VkPipelineBindPoint::e_GRAPHICS,
	    _deviceResources->postBloomPass.pipeline->getPipelineLayout(), 1, _deviceResources->postBloomPass.uboBloomConfigs[swapchain]);

	_deviceResources->postBloomCommandBuffer[swapchain]->draw(0, 3);
	_deviceResources->postBloomCommandBuffer[swapchain]->end();
}

void VulkanPostProcessing::recordBloomCommands(uint32_t swapchain)
{
	recordCommandsPreBloom(swapchain);
	recordCommandsBlur(_deviceResources->horizontalBlurCommandBuffer[swapchain], _deviceResources->horizontalBlurPass, swapchain);
	recordCommandsBlur(_deviceResources->verticalBlurCommandBuffer[swapchain], _deviceResources->verticalBlurPass, swapchain);
	recordCommandsPostBloom(swapchain);

	_deviceResources->mainCmdBloom[swapchain]->begin();
	pvrvk::ClearValue clearValue[3] =
	{
		pvrvk::ClearValue(0.0, 0.70, 0.67, 1.0f),
		pvrvk::ClearValue(0.0, 0.0, 0.0, 1.f),
		pvrvk::ClearValue::createDefaultDepthStencilClearValue()
	};

	// pre bloom
	{
		_deviceResources->mainCmdBloom[swapchain]->beginRenderPass(_deviceResources->preBloomPass.framebuffer[swapchain],
		    pvrvk::Rect2Di(0, 0, getWidth(), getHeight()), false, clearValue, ARRAY_SIZE(clearValue));

		_deviceResources->mainCmdBloom[swapchain]->executeCommands(_deviceResources->preBloomCommandBuffer[swapchain]);
		_deviceResources->mainCmdBloom[swapchain]->endRenderPass();
	}

	clearValue[0].SetColorValue(0.0f, 0.0f, 1.f, 1.0f);
	// horizontal blur
	{
		_deviceResources->mainCmdBloom[swapchain]->beginRenderPass(_deviceResources->horizontalBlurPass.framebuffer[swapchain],
		    pvrvk::Rect2Di(0, 0, _deviceResources->horizontalBlurPass.framebuffer[swapchain]->getDimensions().width,
		                   _deviceResources->horizontalBlurPass.framebuffer[swapchain]->getDimensions().height), false, clearValue, 1);

		_deviceResources->mainCmdBloom[swapchain]->executeCommands(_deviceResources->horizontalBlurCommandBuffer[swapchain]);
		_deviceResources->mainCmdBloom[swapchain]->endRenderPass();
	}

	// vertical blur
	{
		_deviceResources->mainCmdBloom[swapchain]->beginRenderPass(_deviceResources->verticalBlurPass.framebuffer[swapchain],
		    pvrvk::Rect2Di(0, 0, _deviceResources->verticalBlurPass.framebuffer[swapchain]->getDimensions().width,
		                   _deviceResources->verticalBlurPass.framebuffer[swapchain]->getDimensions().height), false, clearValue, 1);

		_deviceResources->mainCmdBloom[swapchain]->executeCommands(_deviceResources->verticalBlurCommandBuffer[swapchain]);
		_deviceResources->mainCmdBloom[swapchain]->endRenderPass();
	}

	// post bloom
	{
		clearValue[1] = clearValue[2];
		_deviceResources->mainCmdBloom[swapchain]->beginRenderPass(_deviceResources->onScreenFramebuffer[swapchain],
		    pvrvk::Rect2Di(0, 0, getWidth(), getHeight()), false, clearValue, 2);
		_deviceResources->mainCmdBloom[swapchain]->executeCommands(_deviceResources->postBloomCommandBuffer[swapchain]);
		_deviceResources->mainCmdBloom[swapchain]->executeCommands(_deviceResources->bloomUiRendererCommandBuffer[swapchain]);
		_deviceResources->mainCmdBloom[swapchain]->endRenderPass();
	}

	// Transition image layouts
	pvrvk::MemoryBarrierSet barriers;
	const uint32_t queueFamilyId = _deviceResources->queues[0]->getQueueFamilyId();
	// transform back to color-attachment write from shader read
	barriers.addBarrier(pvrvk::ImageMemoryBarrier(VkAccessFlags::e_SHADER_READ_BIT, VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
	                    _deviceResources->horizontalBlurPass.framebuffer[swapchain]->getAttachment(0)->getImage(), pvrvk::ImageSubresourceRange(VkImageAspectFlags::e_COLOR_BIT),
	                    VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, queueFamilyId, queueFamilyId));

	barriers.addBarrier(pvrvk::ImageMemoryBarrier(VkAccessFlags::e_SHADER_READ_BIT, VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
	                    _deviceResources->verticalBlurPass.framebuffer[swapchain]->getAttachment(0)->getImage(), pvrvk::ImageSubresourceRange(VkImageAspectFlags::e_COLOR_BIT),
	                    VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, queueFamilyId, queueFamilyId));

	// transform back to color-attachment write from shader read
	barriers.addBarrier(pvrvk::ImageMemoryBarrier(VkAccessFlags::e_SHADER_READ_BIT, VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
	                    _deviceResources->preBloomPass.framebuffer[swapchain]->getAttachment(0)->getImage(),
	                    pvrvk::ImageSubresourceRange(VkImageAspectFlags::e_COLOR_BIT), VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
	                    VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, queueFamilyId, queueFamilyId));

	barriers.addBarrier(pvrvk::ImageMemoryBarrier(VkAccessFlags::e_SHADER_READ_BIT, VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
	                    _deviceResources->preBloomPass.framebuffer[swapchain]->getAttachment(1)->getImage(), pvrvk::ImageSubresourceRange(VkImageAspectFlags::e_COLOR_BIT),
	                    VkImageLayout::e_SHADER_READ_ONLY_OPTIMAL, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, queueFamilyId, queueFamilyId));

	_deviceResources->mainCmdBloom[swapchain]->pipelineBarrier(VkPipelineStageFlags::e_FRAGMENT_SHADER_BIT, VkPipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, barriers);

	_deviceResources->mainCmdBloom[swapchain]->end();
}

/*!********************************************************************************************
\return Return auto ptr to the demo supplied by the user
\brief  This function must be implemented by the user of the shell.
The user should return its Shell object defining the behaviour of the application.
***********************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanPostProcessing()); }
