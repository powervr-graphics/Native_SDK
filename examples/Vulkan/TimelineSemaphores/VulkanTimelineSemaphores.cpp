/*!
\brief Shows how to use timeline semaphore feature
\file VulkanTimelineSemaphores.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "TimelineData.h"
#include "PVRVk/TimelineSemaphoreVk.h"

// shader attributes
const pvr::utils::VertexBindings VertexAttribBindings[] = { { "POSITION", 0 }, { "NORMAL", 1 }, { "UV0", 2 } };

// shader uniforms
namespace Uniform {
enum Enum
{
	MVPMatrix
};
}

// Source and binary shaders
constexpr char FragShaderSrcFile[] = "FragShader.fsh.spv";
constexpr char VertShaderSrcFile[] = "VertShader.vsh.spv";
constexpr char ComputeShaderSrcFile[] = "ComputeShader.csh.spv";

// POD _scene files
constexpr char SceneFile[] = "Plane.pod";

/// <summary>Class implementing the Shell functions.</summary>
class VulkanTimelineSemaphores : public pvr::Shell
{
	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
		pvrvk::Device device;
		pvrvk::Swapchain swapchain;
		pvrvk::CommandPool graphicsCommandPool;
		pvrvk::CommandPool computeCommandPool;
		pvrvk::DescriptorPool descriptorPool;
		pvrvk::Queue graphicsQueue;
		pvrvk::Queue computeQueue;
		pvr::utils::vma::Allocator vmaAllocator;

		std::vector<pvrvk::TimelineSemaphore> timelineSemaphores;

		// Number of times frame was executed, we need to keep track of that because timeline semaphore values cant decrease
		std::vector<uint64_t> semaphoreIterations;
		// The amount of semaphore values increase per frame
		const uint64_t semaphoreCycleValue{ 10 };

		// We still need normal semaphores to synchronise with swapchain, as if now timeline semaphores do not work well with it
		std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;
		std::vector<pvrvk::Semaphore> presentationSemaphores;

		std::vector<pvrvk::Fence> perFrameResourcesFences;
		std::vector<pvrvk::Fence> endOfComputeFences;

		std::vector<pvrvk::Buffer> vbos;
		std::vector<pvrvk::Buffer> ibos;
		pvrvk::DescriptorSetLayout texLayout;
		pvrvk::DescriptorSetLayout uboLayoutDynamic;
		pvrvk::DescriptorSetLayout computeDescriptorSetLayout;

		pvrvk::PipelineLayout graphicsPipelineLayout;
		pvrvk::PipelineLayout computePipelineLayout;

		std::vector<pvrvk::DescriptorSet> texDescSet;
		std::vector<pvrvk::DescriptorSet> computeDescriptorSets;
		std::vector<pvrvk::DescriptorSet> uboDescSets;

		pvrvk::GraphicsPipeline graphicsPipeline;
		pvrvk::ComputePipeline computePipeline;

		std::vector<pvrvk::CommandBuffer> graphicsCommandBuffers;
		std::vector<pvrvk::CommandBuffer> computeCommandBuffers;

		std::vector<pvrvk::Framebuffer> onScreenFramebuffer;
		pvr::utils::StructuredBufferView structuredBufferView;
		pvrvk::Buffer ubo;
		pvrvk::PipelineCache pipelineCache;

		std::vector<std::vector<pvrvk::ImageView>> noiseImages;

		// Descriptor set cannot have empty bound, for simplicity of this demo, we use 2x2 image that we do not sample
		pvrvk::ImageView firstComputeIterationPlaceholderImage;

		pvrvk::Sampler samplerNearest;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;
		std::string uiDescription{};

		~DeviceResources()
		{
			if (device) { device->waitIdle(); }
			if (swapchain)
			{
				uint32_t l = swapchain->getSwapchainLength();
				for (uint32_t i = 0; i < l; ++i)
				{
					if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
				}
			}
		}
	};

	struct UboPerMeshData
	{
		glm::mat4 mvpMtx;
	};

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and view matrix
	glm::mat4 _viewProj;
	uint32_t _frameId{ 0 };
	// The translation and Rotate parameter of Model
	float _angleY{ .0f };
	std::unique_ptr<DeviceResources> _deviceResources;

	uint32_t _swapchainLength;
	
	static constexpr int _numberOfNoiseLayers{ 4 };
	static constexpr int _computeTextureResolution{ 128 };

public:
	pvr::Result initApplication() override;
	pvr::Result initView() override;
	pvr::Result releaseView() override;
	pvr::Result quitApplication() override;
	pvr::Result renderFrame() override;

	void createImageSamplerDescriptor(pvrvk::CommandBuffer imageUploadCmd);
	void createUbo();
	void createGraphicsPipeline();
	void createComputePipeline();
	void drawMesh(pvrvk::CommandBuffer& cmdBuffers, int i32NodeIndex);
	void recordGraphicsCommandBuffer();
	void recordComputeCommandBuffer(const uint32_t& currentFrameId, const uint32_t& noiseTextureId);

	void submitComputeWork(const uint32_t& currentFrameId, const uint64_t& semaphoreWaitValue, const uint64_t& semaphoreSignalValue, const uint16_t& textureIndex);
	void updateComputeDescriptorSets(const uint32_t& readImageIndex, const uint32_t& writeImageIdex, const uint32_t& currentFrameIndex);
	uint64_t getAccumulatedSemaphoreValueIncrease(const uint32_t swapchainIndex) const;
	bool checkTimelineSemaphoreCompatibility();

private:
	void updateModelMatrix(const uint32_t swapchainIndex, const uint32_t planeIndex);
	void createStructuredBufferView();
	void updateUboDescriptorSets();
	void createGraphicsPipelineInfo();
	void checkIfTimelineSeamphoreFeatureWasEnabled();
	void createPools();
	void createSyncObjectsAndCommandBuffers();
	void createSwapchainAndFramebuffer(const pvrvk::Surface& surface);
	void resizeSwapchainVectors();
	void setupTextures();
	void initUIRenderer();
	void setupViewAndProjection();
	void createDevicesAndQueues(pvrvk::Surface& surface);
	void renderComputeNoiseLayers(const uint32_t swapchainIndex);
	void addTimelineInfoToUIOss(const uint32_t swapchainIndex, int i, std::ostringstream& uiOss) const;
	pvrvk::ImageMemoryBarrier transitionFromReadOnlyToGeneral(const pvrvk::Image& noiseImage) const;
	pvrvk::ImageMemoryBarrier transitionFromGeneralToReadOnly(const pvrvk::Image& noiseImage) const;
	void updatePushConstants(const uint32_t& noiseTextureId, const uint32_t computeOperationIndex);
};

/// <summary>Creates the image sampler descriptor for noise images and updates them in a descriptor set.</summary>
/// <param name="imageUploadCmd">The command buffer used for image uploading.</param>
void VulkanTimelineSemaphores::createImageSamplerDescriptor(pvrvk::CommandBuffer imageUploadCmd)
{
	pvrvk::Device& device = _deviceResources->device;

	// create the bilinear sampler
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	pvrvk::Sampler samplerMipBilinear = device->createSampler(samplerInfo);

	for (int textDescriptorIndex = 0; textDescriptorIndex < _numberOfNoiseLayers; textDescriptorIndex++)
	{
		// create the descriptor set
		_deviceResources->texDescSet.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texLayout));
		_deviceResources->texDescSet[textDescriptorIndex]->setObjectName("TextureDescriptorSet");
		pvrvk::WriteDescriptorSet writeDescSets[] = { pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->texDescSet[textDescriptorIndex], 0) };
		writeDescSets[0].setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->noiseImages[0][textDescriptorIndex], samplerMipBilinear));

		_deviceResources->device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
	}
}
/// <summary>Creates Uniform Buffer Object</summary>
void VulkanTimelineSemaphores::createUbo()
{
	createStructuredBufferView();
	updateUboDescriptorSets();
}

/// <summary>Updates the Uniform Buffer Object descriptor sets for the entire swapchain length.</summary>
void VulkanTimelineSemaphores::updateUboDescriptorSets()
{
	std::vector<pvrvk::WriteDescriptorSet> descUpdate{ _swapchainLength };

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->uboDescSets[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutDynamic);
		_deviceResources->uboDescSets[i]->setObjectName("UBOSwapchain" + std::to_string(i) + "DescriptorSet");

		descUpdate[i]
			.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->uboDescSets[i])
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->ubo, 0, _deviceResources->structuredBufferView.getDynamicSliceSize()));
	}

	_deviceResources->device->updateDescriptorSets(descUpdate.data(), _deviceResources->swapchain->getSwapchainLength(), nullptr, 0);
}

/// <summary>Creates a structured buffer view with MVPMatrix.</summary>
void VulkanTimelineSemaphores::createStructuredBufferView()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("MVPMatrix", pvr::GpuDatatypes::mat4x4);

	_deviceResources->structuredBufferView.initDynamic(desc, _scene->getNumMeshNodes() * _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
	_deviceResources->ubo = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->structuredBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->ubo->setObjectName("UBO");
	_deviceResources->structuredBufferView.pointToMappedMemory(_deviceResources->ubo->getDeviceMemory()->getMappedData());
	_deviceResources->ubo->setObjectName("Object Ubo");
}

/// <summary>Creates the graphics pipeline, sets up descriptor set layouts, pipeline layout, and the graphics pipeline info.</summary>
void VulkanTimelineSemaphores::createGraphicsPipeline()
{
	//--- create the texture-sampler descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); /*binding 0*/
		_deviceResources->texLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the ubo descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->uboLayoutDynamic = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the pipeline layout
	{
		pvrvk::PushConstantRange pushConstantRange;
		pushConstantRange.setStageFlags(pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		pushConstantRange.setOffset(0);
		pushConstantRange.setSize(sizeof(glm::vec3));

		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo
			.addDescSetLayout(_deviceResources->texLayout) /*set 0*/
			.addDescSetLayout(_deviceResources->uboLayoutDynamic) /*set 1*/
			.addPushConstantRange(pushConstantRange);

		_deviceResources->graphicsPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	createGraphicsPipelineInfo();
}

/// <summary>Configures the graphics pipeline info by specifying viewports, blending, shaders, etc.</summary>
void VulkanTimelineSemaphores::createGraphicsPipelineInfo()
{
	pvrvk::GraphicsPipelineCreateInfo pipeInfo;

	pvrvk::PipelineColorBlendAttachmentState colorAttachemtState;
	colorAttachemtState.setBlendEnable(false);

	const pvrvk::Rect2D rect(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight());
	pipeInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(static_cast<float>(rect.getOffset().getX()), static_cast<float>(rect.getOffset().getY()), static_cast<float>(rect.getExtent().getWidth()),
			static_cast<float>(rect.getExtent().getHeight())),
		rect);
	pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	pipeInfo.colorBlend.setAttachmentState(0, colorAttachemtState);

	std::unique_ptr<pvr::Stream> vertSource = getAssetStream(VertShaderSrcFile);
	std::unique_ptr<pvr::Stream> fragSource = getAssetStream(FragShaderSrcFile);

	pipeInfo.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertSource->readToEnd<uint32_t>())));
	pipeInfo.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>())));

	const pvr::assets::Mesh& mesh = _scene->getMesh(0);
	pipeInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToPVRVk(mesh.getPrimitiveType()));
	pipeInfo.pipelineLayout = _deviceResources->graphicsPipelineLayout;
	pipeInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pipeInfo.subpass = 0;
	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.enableDepthTest(false);
	pipeInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
	pipeInfo.depthStencil.enableDepthWrite(false);
	pvr::utils::populateInputAssemblyFromMesh(mesh, VertexAttribBindings, sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipeInfo.vertexInput, pipeInfo.inputAssembler);
	_deviceResources->graphicsPipeline = _deviceResources->device->createGraphicsPipeline(pipeInfo, _deviceResources->pipelineCache);
	_deviceResources->graphicsPipeline->setObjectName("TimelineGraphicsPipeline");
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanTimelineSemaphores::initApplication()
{
	_scene = pvr::assets::loadModel(*this, SceneFile);

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanTimelineSemaphores::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanTimelineSemaphores::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create Vulkan 1.1 instance and retrieve compatible physical devices
	// Timeline Semaphore is Vulkan 1.2 feature, but it is usually available in 1.1 with extention
	pvr::utils::VulkanVersion VulkanVersion(1, 1, 0);

	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), VulkanVersion, pvr::utils::InstanceExtensions(VulkanVersion));

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	createDevicesAndQueues(surface);

	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("initView"));

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	createSwapchainAndFramebuffer(surface);
	resizeSwapchainVectors();
	createPools();

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// load the pipelines
	createGraphicsPipeline();
	createComputePipeline();

	createSyncObjectsAndCommandBuffers();
	setupTextures();
	initUIRenderer();
	createUbo();
	setupViewAndProjection();
	recordGraphicsCommandBuffer();

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);
	return pvr::Result::Success;
}

/// <summary>Creates devices and queues. Checks for, and enables timeline semaphore extension.</summary>
/// <param name="surface">The surface to be used for creating devices and queues.</param>
void VulkanTimelineSemaphores::createDevicesAndQueues(pvrvk::Surface& surface)
{
	if (!checkTimelineSemaphoreCompatibility()) throw pvrvk::ErrorInitializationFailed("No physical device with timelineSemaphores feature support is found.");

	// Setup device extentions
	pvr::utils::DeviceExtensions deviceExtensions =
		pvr::utils::DeviceExtensions(); // Add these device features to the physical device, since they're all connected by a pNext chain, we only need to explicitly attach the top feature
	VkPhysicalDeviceFeatures2 deviceFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_FEATURES_2) };

	VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineSemaphoreFeatures = { static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR) };

	timelineSemaphoreFeatures.timelineSemaphore = true;
	deviceFeatures.pNext = &timelineSemaphoreFeatures;

	// Add these device features to the physical device, since they're all connected by a pNext chain, we only need to explicitly attach the top feature
	deviceExtensions.addExtensionFeatureVk<VkPhysicalDeviceFeatures2>(&deviceFeatures);
	deviceExtensions.addExtension("VK_KHR_timeline_semaphore");

	const pvr::utils::QueuePopulateInfo queuePopulateInfos[2] = { { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface }, { pvrvk::QueueFlags::e_COMPUTE_BIT } };

	pvr::utils::QueueAccessInfo queueAccessInfos[2];
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queuePopulateInfos, 2, queueAccessInfos, deviceExtensions);

	checkIfTimelineSeamphoreFeatureWasEnabled();

	_deviceResources->graphicsQueue = _deviceResources->device->getQueue(queueAccessInfos[0].familyId, queueAccessInfos[0].queueId);

	if (!(queueAccessInfos[1].familyId != static_cast<uint32_t>(-1) &&
		    queueAccessInfos[1].queueId != static_cast<uint32_t>(-1)))
	{
		Log(LogLevel::Error, "Multiple queues are not supported (e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI)");
	}
	_deviceResources->computeQueue = _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);

	_deviceResources->graphicsQueue->setObjectName("GraphicsQueue");
	_deviceResources->computeQueue->setObjectName("ComputeQueue");
}

/// <summary>Setups the view and projection matrices based on the scene's camera properties.</summary>
void VulkanTimelineSemaphores::setupViewAndProjection()
{
	glm::vec3 from, to, up;
	float fov;
	_scene->getCameraProperties(0, fov, from, to, up);

	// Is the screen rotated
	const bool bRotate = isScreenRotated();

	//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
	_viewProj = (bRotate ? pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, static_cast<float>(getHeight()), static_cast<float>(getWidth()), _scene->getCamera(0).getNear(),
							   _scene->getCamera(0).getFar(), glm::pi<float>() * .5f)
						 : pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, static_cast<float>(getWidth()), static_cast<float>(getHeight()), _scene->getCamera(0).getNear(),
							   _scene->getCamera(0).getFar()));

	// Set camera to look at planes from up
	_viewProj = _viewProj * glm::lookAt(glm::vec3(0, 15.0f, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1.0f));
}

/// <summary>Initializes the User Interface renderer.</summary>
void VulkanTimelineSemaphores::initUIRenderer()
{
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->graphicsCommandPool, _deviceResources->graphicsQueue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Timeline Semaphores");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
}

/// <summary>Configures the textures for the application. Descriptor set cannot be empty, for simplicity we use dummy texture, so that compute shader is same for
/// all of the iterations. (First noise layer does not sample texture)</summary>
void VulkanTimelineSemaphores::setupTextures()
{ // Create a single time submit command buffer for uploading resources
	pvrvk::CommandBuffer uploadBuffer = _deviceResources->graphicsCommandPool->allocateCommandBuffer();
	uploadBuffer->setObjectName("InitView : Upload Command Buffer");
	uploadBuffer->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);

	// load the vbo and ibo data
	bool requiresCommandBufferSubmission = false;
	pvr::utils::appendSingleBuffersFromModel(
		_deviceResources->device, *_scene, _deviceResources->vbos, _deviceResources->ibos, uploadBuffer, requiresCommandBufferSubmission, _deviceResources->vmaAllocator);

	{
		// Create compute textures
		_deviceResources->computeCommandBuffers[0]->begin();

		// Create placeholder image for fist iteration of compute shader
		{
			std::vector<unsigned char> dummyData(2 * 2 * 1, 255);
			pvr::TextureHeader textureHeader(pvr::PixelFormat::R_8(), 2, 2);
			pvr::Texture dummyTexture(textureHeader, dummyData.data());

			_deviceResources->firstComputeIterationPlaceholderImage = pvr::utils::uploadImageAndView(_deviceResources->device, dummyTexture, true,
				_deviceResources->computeCommandBuffers[0], pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_STORAGE_BIT,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
		}
		
		std::vector<unsigned char> computeTextureData(_computeTextureResolution * _computeTextureResolution, 0);
		pvr::TextureHeader textureHeader(pvr::PixelFormat::R_8(), _computeTextureResolution, _computeTextureResolution);

		pvr::Texture compTexture(textureHeader, computeTextureData.data());
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
		{
			for (uint32_t j = 0; j < _numberOfNoiseLayers; j++)
			{
				pvrvk::ImageView noiseImageView = pvr::utils::uploadImageAndView(_deviceResources->device, compTexture, true, _deviceResources->computeCommandBuffers[0],
					pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_STORAGE_BIT, pvrvk::ImageLayout::e_GENERAL, _deviceResources->vmaAllocator,
					_deviceResources->vmaAllocator);

				// change layour from e_GENERAL to e_SHADER_READ_ONLY_OPTIMAL
				pvrvk::ImageMemoryBarrier imageMemoryBarrier;
				imageMemoryBarrier.setOldLayout(pvrvk::ImageLayout::e_GENERAL);
				imageMemoryBarrier.setNewLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
				imageMemoryBarrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
				imageMemoryBarrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
				imageMemoryBarrier.setImage(noiseImageView->getImage());
				imageMemoryBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 1, 0, 1));
				imageMemoryBarrier.setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
				imageMemoryBarrier.setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);

				pvrvk::MemoryBarrierSet generalToShaderReadOnlySet;
				generalToShaderReadOnlySet.addBarrier(imageMemoryBarrier);

				_deviceResources->computeCommandBuffers[0]->pipelineBarrier(
					pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, generalToShaderReadOnlySet);

				_deviceResources->noiseImages[i].push_back(noiseImageView);
			}
		}

		_deviceResources->computeCommandBuffers[0]->end();

		// Summit creating of compute textures to command buffer
		pvrvk::SubmitInfo submit;
		submit.commandBuffers = &_deviceResources->computeCommandBuffers[0];
		submit.numCommandBuffers = 1;
		_deviceResources->computeQueue->submit(&submit, 1);
		_deviceResources->computeQueue->waitIdle();
	}

	// create the image samplers
	createImageSamplerDescriptor(uploadBuffer);
	uploadBuffer->end();

	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("Batching Application Resource Upload"));

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &uploadBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->graphicsQueue->submit(&submitInfo, 1);
	_deviceResources->graphicsQueue->waitIdle();

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);
}

/// <summary>Creates swapchain and associated frame buffers.</summary>
/// <param name="surface">The surface for which the swapchain and frame buffers are created.</param>
void VulkanTimelineSemaphores::createSwapchainAndFramebuffer(const pvrvk::Surface& surface)
{
	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	} //---------------

	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));
	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	_swapchainLength = _deviceResources->swapchain->getSwapchainLength();
}
/// <summary>Resizes vectors, that hold members for each of swapchain elements</summary>
void VulkanTimelineSemaphores::resizeSwapchainVectors()
{
	_deviceResources->timelineSemaphores.resize(_swapchainLength);
	_deviceResources->semaphoreIterations.resize(_swapchainLength);
	_deviceResources->imageAcquiredSemaphores.resize(_swapchainLength);
	_deviceResources->presentationSemaphores.resize(_swapchainLength);
	_deviceResources->perFrameResourcesFences.resize(_swapchainLength);
	_deviceResources->endOfComputeFences.resize(_swapchainLength);
	_deviceResources->uboDescSets.resize(_swapchainLength);
	_deviceResources->graphicsCommandBuffers.resize(_swapchainLength);
	_deviceResources->onScreenFramebuffer.resize(_swapchainLength);
	_deviceResources->noiseImages.resize(_swapchainLength);
}

/// <summary>Creates synchronization objects and command buffers for rendering and compute operations,
/// so fences, binary semaphores and timeline semaphores.
/// All timeline semaphores are set to 3. Timeline semaphores can be signalled both from CPU and GPU.</summary>
void VulkanTimelineSemaphores::createSyncObjectsAndCommandBuffers()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// create the per swapchain command buffers
		_deviceResources->graphicsCommandBuffers[i] = _deviceResources->graphicsCommandPool->allocateCommandBuffer();
		_deviceResources->graphicsCommandBuffers[i]->setObjectName("MainCommandBufferSwapchain" + std::to_string(i));

		// create compute command buffers
		for (int k = 0; k < 4; k++)
		{
			_deviceResources->computeCommandBuffers.push_back(_deviceResources->computeCommandPool->allocateCommandBuffer());
			_deviceResources->computeCommandBuffers[4 * i + k]->setObjectName(std::string("Main Compute CommandBuffer [") + std::to_string(i) + "]" + "[" + std::to_string(k) + "]");
		}

		// create classic sync objects
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->presentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));

		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->endOfComputeFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_NONE);
		_deviceResources->perFrameResourcesFences[i]->setObjectName("FenceSwapchain" + std::to_string(i));
		_deviceResources->endOfComputeFences[i]->setObjectName("EndOfComputeFenceSwapchain" + std::to_string(i));

		// create timeline semaphores
		pvrvk::SemaphoreCreateInfo createInfo{};
		_deviceResources->timelineSemaphores[i] = _deviceResources->device->createTimelineSemaphore(createInfo);
		_deviceResources->timelineSemaphores[i]->setObjectName("TimelineSemaphoreSwapchain" + std::to_string(i));
	}

	{
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
		{
			VkSemaphoreSignalInfo signalInfo = {};
			signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
			signalInfo.semaphore = _deviceResources->timelineSemaphores[i]->getVkHandle();
			signalInfo.value = 3;

			auto result = _deviceResources->device->getVkBindings().vkSignalSemaphoreKHR(_deviceResources->device->getVkHandle(), &signalInfo);

			if (result != VK_SUCCESS) { Log(LogLevel::Error, "Error signaling timeline semaphore"); }
		}
	}
}

/// <summary>Creates graphics, descriptor, and command pools.</summary>
void VulkanTimelineSemaphores::createPools()
{ // Create the compute command pool
	_deviceResources->computeCommandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->computeQueue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	_deviceResources->computeCommandPool->setObjectName("Compute Command Pool");
	// Create the graphics command pool and descriptor set pool
	_deviceResources->graphicsCommandPool = _deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(_deviceResources->graphicsQueue->getFamilyIndex()));
	_deviceResources->graphicsCommandPool->setObjectName("Main Command Pool");

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 16)
																						  .setMaxDescriptorSets(32));
	_deviceResources->descriptorPool->setObjectName("DescriptorPool");
}

/// <summary>Checks if the VK_KHR_timeline_semaphore feature was enabled.</summary>
void VulkanTimelineSemaphores::checkIfTimelineSeamphoreFeatureWasEnabled()
{
	VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures = {};
	timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
	timelineSemaphoreFeatures.pNext = nullptr;

	VkPhysicalDeviceFeatures2 deviceFeatures = {};
	deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures.pNext = &timelineSemaphoreFeatures;

	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceFeatures2(_deviceResources->instance->getPhysicalDevice(0)->getVkHandle(), &deviceFeatures);

	if (timelineSemaphoreFeatures.timelineSemaphore) { Log_Info("VK_KHR_timeline_semaphore was enabled"); }
	else
	{
		Log(LogLevel::Error, "Required extension VK_KHR_timeline_semaphore not supported");
		throw pvrvk::ErrorInitializationFailed("Required extension VK_KHR_timeline_semaphore not supported");
	}
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanTimelineSemaphores::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanTimelineSemaphores::renderFrame()
{
	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("renderFrame"));
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	_deviceResources->uiDescription = "Noise texture nr | Required semaphore value\n";

	renderComputeNoiseLayers(swapchainIndex);

	_deviceResources->endOfComputeFences[swapchainIndex]->wait();
	_deviceResources->endOfComputeFences[swapchainIndex]->reset();

	//	 This is how you can get value of timeline semaphore
	//	{
	//		uint64_t timeSemaphoreValue{0};
	//
	//		_deviceResources->device->getVkBindings().vkGetSemaphoreCounterValueKHR(
	//			_deviceResources->device->getVkHandle(),
	//			_deviceResources->timelineSemaphores[swapchainIndex]->getVkHandle(), &timeSemaphoreValue
	//			);
	//	}

	_deviceResources->uiRenderer.getDefaultDescription()->setText(_deviceResources->uiDescription);
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	updateModelMatrix(swapchainIndex, 0);

	//---------------
	// SUBMIT
	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("Submitting per frame command buffers"));

	std::vector<pvrvk::Semaphore> semaphoresToSignalAfterSubmit;
	semaphoresToSignalAfterSubmit.push_back(_deviceResources->timelineSemaphores[_frameId]);
	semaphoresToSignalAfterSubmit.push_back(_deviceResources->presentationSemaphores[_frameId]);

	std::vector<pvrvk::Semaphore> waitSemaphores;
	waitSemaphores.push_back(_deviceResources->timelineSemaphores[_frameId]);
	waitSemaphores.push_back(_deviceResources->imageAcquiredSemaphores[_frameId]);

	pvrvk::TimelineSemaphoreSubmitInfo mySemaphoreSubmitInfo;
	std::vector<pvrvk::Semaphore> mySemaphoreSubmitInfos;

	const uint64_t sumbitInfoWaitValues[] = { 7 + getAccumulatedSemaphoreValueIncrease(swapchainIndex), 0 };
	const uint64_t signalValues[] = { 3 + getAccumulatedSemaphoreValueIncrease(swapchainIndex) + _deviceResources->semaphoreCycleValue, 0 };

	mySemaphoreSubmitInfo.waitSemaphoreValueCount = 2;
	mySemaphoreSubmitInfo.waitSemaphoreValues = sumbitInfoWaitValues;
	mySemaphoreSubmitInfo.signalSemaphoreValueCount = 2;
	mySemaphoreSubmitInfo.signalSemaphoreValues = signalValues;

	pvrvk::SubmitInfo submitInfo;
	std::vector<pvrvk::PipelineStageFlags> pipeWaitStageFlags(waitSemaphores.size(), pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
	submitInfo.commandBuffers = &_deviceResources->graphicsCommandBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &waitSemaphores[0];
	submitInfo.numWaitSemaphores = static_cast<uint32_t>(waitSemaphores.size());
	submitInfo.signalSemaphores = &semaphoresToSignalAfterSubmit[0];
	submitInfo.numSignalSemaphores = static_cast<uint32_t>(semaphoresToSignalAfterSubmit.size());
	submitInfo.timelineSemaphoreSubmitInfo = &mySemaphoreSubmitInfo;

	submitInfo.waitDstStageMask = pipeWaitStageFlags.data();

	_deviceResources->graphicsQueue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);

	_deviceResources->semaphoreIterations[swapchainIndex]++;

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->graphicsQueue, _deviceResources->graphicsCommandPool, _deviceResources->swapchain, swapchainIndex,
			this->getScreenshotFileName(), _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	//---------------
	// PRESENT
	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("Presenting swapchain image to the screen"));

	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->graphicsQueue->present(presentInfo);

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);

	return pvr::Result::Success;
}

/// <summary>Executes the compute shaders to generate noise layers. Also updates UI description to display timeline semaphores values.</summary>
/// <param name="swapchainIndex">Index of the current swapchain in usage.</param>
void VulkanTimelineSemaphores::renderComputeNoiseLayers(const uint32_t swapchainIndex)
{
	// For better performance than adding strings
	std::ostringstream uiOss;

	for (int i = 0; i < _numberOfNoiseLayers; i++)
	{
		// This calculates, what value should timeline semaphore wait for, and what value to ping
		// 3 is starting value. So every frame computeCanStartValue is 10 * frame number + 3.
		const uint64_t computeCanStartValue = 3 + getAccumulatedSemaphoreValueIncrease(swapchainIndex);
		uint64_t waitValue = computeCanStartValue + i;

		updateComputeDescriptorSets((i == 0) ? 0 : i - 1, i, swapchainIndex);

		recordComputeCommandBuffer(_frameId, i);
		submitComputeWork(_frameId, waitValue, waitValue + 1, static_cast<uint16_t>(i));

		addTimelineInfoToUIOss(swapchainIndex, static_cast<uint16_t>(i), uiOss);
	}
	_deviceResources->uiDescription += uiOss.str();
}

/// <summary>Adds information about timeline semaphores values to the provided string stream.</summary>
/// <param name="swapchainIndex">Index of the swapchain image in use.</param>
/// <param name="i">Index of the current noise layer.</param>
/// <param name="uiOss">Reference to the output string stream.</param>
void VulkanTimelineSemaphores::addTimelineInfoToUIOss(const uint32_t swapchainIndex, int i, std::ostringstream& uiOss) const
{
	uiOss << "            " << (i + 1) << "            |            " << (3 + getAccumulatedSemaphoreValueIncrease(swapchainIndex) + i) << "\n";
}

/// <summary>Updates the model matrix.</summary>
/// <param name="swapchainIndex">Index of the swapchain image in use.</param>
/// <param name="planeIndex">Index of the plane.</param>
void VulkanTimelineSemaphores::updateModelMatrix(const uint32_t swapchainIndex, const uint32_t planeIndex)
{
	const float distanceBetweenTiles = 3.f;
	float xDisplacement = (_numberOfNoiseLayers * -0.5f) * distanceBetweenTiles + float(planeIndex) * distanceBetweenTiles;
	xDisplacement = .0f;
	// Calculate the model matrix
	const glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(xDisplacement, .0f, .0f)) * glm::scale(glm::vec3(1.4f));

	//---------------
	// update the ubo
	UboPerMeshData srcWrite{};
	srcWrite.mvpMtx = _viewProj * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());

	_deviceResources->structuredBufferView.getElementByName("MVPMatrix", 0, swapchainIndex).setValue(srcWrite.mvpMtx);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->ubo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->ubo->getDeviceMemory()->flushRange(
			_deviceResources->structuredBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->structuredBufferView.getDynamicSliceSize());
	}
}

/// <summary>Draws a assets::Mesh after the model view matrix has been set and the material prepared.</summary>
/// <pram =name"nodeIndex">Node index of the mesh to draw.</param>
void VulkanTimelineSemaphores::drawMesh(pvrvk::CommandBuffer& cmdBuffers, int nodeIndex)
{
	const uint32_t meshId = _scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = _scene->getMesh(meshId);

	// bind the VBO for the mesh
	cmdBuffers->bindVertexBuffer(_deviceResources->vbos[meshId], 0, 0);

	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		// Indexed Triangle list
		if (_deviceResources->ibos[meshId])
		{
			cmdBuffers->bindIndexBuffer(_deviceResources->ibos[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
			cmdBuffers->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			cmdBuffers->draw(0, mesh.getNumFaces() * 3, 0, 1);
		}
	}
	else
	{
		uint32_t offset = 0;
		for (uint32_t i = 0; i < mesh.getNumStrips(); ++i)
		{
			if (_deviceResources->ibos[meshId])
			{
				// Indexed Triangle strips
				cmdBuffers->bindIndexBuffer(_deviceResources->ibos[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
				cmdBuffers->drawIndexed(0, mesh.getStripLength(i) + 2, (int32_t)offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				cmdBuffers->draw(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

/// <summary>Pre-records commands into the graphics command buffer. This command buffer is used to draw 4 planes with layered noise generated using compute shader.
/// Additionally, UI is drawn. Graphics command buffer is not being recorded every frame.</summary>
void VulkanTimelineSemaphores::recordGraphicsCommandBuffer()
{
	const uint32_t numSwapchains = _deviceResources->swapchain->getSwapchainLength();
	pvrvk::ClearValue clearValues[2] = { pvrvk::ClearValue(0.0f, 0.45f, 0.41f, 1.f), pvrvk::ClearValue(1.f, 0u) };
	for (uint32_t i = 0; i < numSwapchains; ++i)
	{
		// begin recording commands for the current swap chain command buffer
		_deviceResources->graphicsCommandBuffers[i]->begin();
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->graphicsCommandBuffers[i], pvrvk::DebugUtilsLabel("Render Frame Commands"));

		// begin the render pass
		_deviceResources->graphicsCommandBuffers[i]->beginRenderPass(
			_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->graphicsCommandBuffers[i], pvrvk::DebugUtilsLabel("Mesh"));

		// calculate the dynamic offset to use
		const uint32_t dynamicOffset = _deviceResources->structuredBufferView.getDynamicSliceOffset(i);
		// enqueue the static states which won 't be changed throughout the frame
		_deviceResources->graphicsCommandBuffers[i]->bindPipeline(_deviceResources->graphicsPipeline);

		_deviceResources->graphicsCommandBuffers[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->graphicsPipelineLayout, 1, _deviceResources->uboDescSets[i], &dynamicOffset, 1);

		std::array<glm::vec3, _numberOfNoiseLayers> planePositions{};

		for (std::size_t j = 0; j < planePositions.size(); j++)
		{
			const float distanceBetweenTiles = 3.f;
			const float xDisplacement = (_numberOfNoiseLayers * -0.5f) * distanceBetweenTiles + float(j) * distanceBetweenTiles + 1.5f;
			planePositions[j] = glm::vec3{ xDisplacement, 0.0f, 0.0f };
		}

		for (int j = 0; j < _numberOfNoiseLayers; j++)
		{
			pvr::utils::beginCommandBufferDebugLabel(_deviceResources->graphicsCommandBuffers[i], pvrvk::DebugUtilsLabel("DrawingTextureNumber " + std::to_string(j + 1)));

			pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);
			_deviceResources->graphicsCommandBuffers[i]->bindDescriptorSet(
				pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->graphicsPipelineLayout, 0, _deviceResources->texDescSet[j], nullptr);

			_deviceResources->graphicsCommandBuffers[i]->pushConstants(
				_deviceResources->graphicsPipelineLayout, pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, sizeof(glm::vec3), &planePositions[j]);

			drawMesh(_deviceResources->graphicsCommandBuffers[i], 0);
			pvr::utils::endCommandBufferDebugLabel(_deviceResources->graphicsCommandBuffers[i]);
		}

		// record the ui renderer commands
		_deviceResources->uiRenderer.beginRendering(_deviceResources->graphicsCommandBuffers[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.endRendering();

		// end the renderpass
		_deviceResources->graphicsCommandBuffers[i]->endRenderPass();

		pvr::utils::endCommandBufferDebugLabel(_deviceResources->graphicsCommandBuffers[i]);

		// end recording commands for the current command buffer
		_deviceResources->graphicsCommandBuffers[i]->end();
	}
}

/// <summary>Checks if the timeline semaphore feature is compatible.</summary>
/// <returns>True if device is compatible to use timeline semaphores, false otherwise.</returns>
bool VulkanTimelineSemaphores::checkTimelineSemaphoreCompatibility()
{
	VkPhysicalDeviceTimelineSemaphoreFeatures timeLineSemaphoreFeature = { static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES) };

	VkPhysicalDeviceFeatures2 deviceFeatures2{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_FEATURES_2) };
	deviceFeatures2.pNext = &timeLineSemaphoreFeature; // Fill in all of these device features with one call
	_deviceResources->instance->getPhysicalDevice(0)->getInstance()->getVkBindings().vkGetPhysicalDeviceFeatures2(
		_deviceResources->instance->getPhysicalDevice(0)->getVkHandle(), &deviceFeatures2); // Logic if feature is not supported
	return (timeLineSemaphoreFeature.timelineSemaphore == VK_TRUE);
}

/// <summary>Creates a compute pipeline and components it needs, like: compute descriptor set layout, compute shader, compute descriptor sets, sampler.</summary>
void VulkanTimelineSemaphores::createComputePipeline()
{
	// Create compute pipeline layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descriptorSetLayoutParams;
		descriptorSetLayoutParams.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		descriptorSetLayoutParams.setBinding(1, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

		_deviceResources->computeDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descriptorSetLayoutParams);
	}

	//--- create the compute descritptor set
	{
		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength() * 4; ++i)
		{
			_deviceResources->computeDescriptorSets.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->computeDescriptorSetLayout));
			_deviceResources->computeDescriptorSets.back()->setObjectName("ComputeSwapchain" + std::to_string(i) + "DescriptorSet");
		}
	}

	{
		pvrvk::PushConstantRange pushConstantRange;
		pushConstantRange.setStageFlags(pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		pushConstantRange.setOffset(0);
		pushConstantRange.setSize(sizeof(NoiseComputePushConstant));

		pvrvk::PipelineLayoutCreateInfo createInfo;
		createInfo.addDescSetLayout(_deviceResources->computeDescriptorSetLayout).addPushConstantRange(pushConstantRange);
		_deviceResources->computePipelineLayout = _deviceResources->device->createPipelineLayout(createInfo);
	}

	const pvrvk::ShaderModule computeShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(ComputeShaderSrcFile)->readToEnd<uint32_t>()));

	// Create Smapler
	{
		pvrvk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
		samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
		samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;

		_deviceResources->samplerNearest = _deviceResources->device->createSampler(samplerInfo);
	}

	// Create compute pipeline
	pvrvk::ComputePipelineCreateInfo createInfo;
	createInfo.computeShader.setShader(computeShader);
	createInfo.pipelineLayout = _deviceResources->computePipelineLayout;
	_deviceResources->computePipeline = _deviceResources->device->createComputePipeline(createInfo, _deviceResources->pipelineCache);
	_deviceResources->computePipeline->setObjectName("TimelineSemaphoresComputePipeline");
}

/// <summary>Records commands into the compute command buffer. This is done every frame, not pre-recorded like in case of graphics command buffer.
/// Previous texture is sampled here. Then another noise texture is generated and overlayed on top inside of this compute shader.</summary>
/// <param name="currentFrameId">Current frame index</param>
/// <param name="noiseTextureId">Noise texture index</param>
void VulkanTimelineSemaphores::recordComputeCommandBuffer(const uint32_t& currentFrameId, const uint32_t& noiseTextureId)
{
	const uint32_t computeOperationIndex = currentFrameId * _numberOfNoiseLayers + noiseTextureId;
	pvrvk::CommandBuffer& mainCmdBuffer = _deviceResources->computeCommandBuffers[computeOperationIndex];

	const pvrvk::Image& noiseImage = _deviceResources->noiseImages[currentFrameId][noiseTextureId]->getImage();

	pvrvk::MemoryBarrierSet toGeneralBarrierSet;
	pvrvk::ImageMemoryBarrier toGeneralBarrier = transitionFromReadOnlyToGeneral(noiseImage);
	toGeneralBarrierSet.addBarrier(toGeneralBarrier);

	// Recording the Compute Commandbuffer
	mainCmdBuffer->reset();
	mainCmdBuffer->begin();
	pvr::utils::beginCommandBufferDebugLabel(mainCmdBuffer, pvrvk::DebugUtilsLabel("ComputeNoiseLayer " + std::to_string(noiseTextureId)));
	mainCmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, toGeneralBarrierSet);

	mainCmdBuffer->bindPipeline(_deviceResources->computePipeline);
	mainCmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->computePipelineLayout, 0, _deviceResources->computeDescriptorSets[computeOperationIndex]);

	updatePushConstants(noiseTextureId, computeOperationIndex);

	mainCmdBuffer->dispatch(_computeTextureResolution, _computeTextureResolution, 1);

	pvrvk::MemoryBarrierSet toReadOnlyBarrierSet;
	pvrvk::ImageMemoryBarrier toReadOnlyBarrier = transitionFromGeneralToReadOnly(noiseImage);
	toReadOnlyBarrierSet.addBarrier(toReadOnlyBarrier);

	mainCmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, toReadOnlyBarrierSet);

	mainCmdBuffer->end();
	pvr::utils::endCommandBufferDebugLabel(mainCmdBuffer);
}

/// <summary>Updates the push constants for the compute shaders. They store displacement of planes, direction of noise panning, sale of noise, and boolean (as int) if to sample
/// previous texture.</summary> <param name="noiseTextureId">The ID of the noise texture.</param> <param name="computeOperationIndex">Index of compute operation</param>
void VulkanTimelineSemaphores::updatePushConstants(const uint32_t& noiseTextureId, const uint32_t computeOperationIndex)
{
	constexpr std::array<std::array<float, 2>, _numberOfNoiseLayers> normalizedDirections = { { { { 0.707107, 0.707107 } }, { { -0.242536, -0.970143 } },
		{ { -0.832050, 0.554700 } }, { { 0.447214, 0.894427 } } } };

	const glm::vec2 offset =
		glm::vec2(normalizedDirections[noiseTextureId][0], normalizedDirections[noiseTextureId][1]) * (static_cast<float>(getTime()) / (4000.0f * (1.0f + noiseTextureId)));

	NoiseComputePushConstant constantData{ 10.0f * (noiseTextureId + 1.f), noiseTextureId == 0, offset };

	_deviceResources->computeCommandBuffers[computeOperationIndex]->pushConstants(
		_deviceResources->computePipelineLayout, pvrvk::ShaderStageFlags::e_COMPUTE_BIT, 0, sizeof(NoiseComputePushConstant), &constantData);
}

/// <summary>Handles the transition of the image from general layout to read-only layout.</summary>
/// <param name="noiseImage">The noise image to be transitioned.</param>
/// <returns>A barrier to manage the transition.</returns>
pvrvk::ImageMemoryBarrier VulkanTimelineSemaphores::transitionFromGeneralToReadOnly(const pvrvk::Image& noiseImage) const
{
	pvrvk::ImageMemoryBarrier toReadOnlyBarrier;
	toReadOnlyBarrier.setOldLayout(pvrvk::ImageLayout::e_GENERAL);
	toReadOnlyBarrier.setNewLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
	toReadOnlyBarrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	toReadOnlyBarrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	toReadOnlyBarrier.setImage(noiseImage);

	toReadOnlyBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 1, 0, 1));

	toReadOnlyBarrier.setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_WRITE_BIT);
	toReadOnlyBarrier.setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	return toReadOnlyBarrier;
}

/// <summary>Handles the transition of the image from read-only layout to general layout.</summary>
/// <param name="noiseImage">The noise image to be transitioned.</param>
/// <returns>A barrier to manage the transition.</returns>
pvrvk::ImageMemoryBarrier VulkanTimelineSemaphores::transitionFromReadOnlyToGeneral(const pvrvk::Image& noiseImage) const
{
	pvrvk::ImageMemoryBarrier toGeneralBarrier;
	toGeneralBarrier.setOldLayout(pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL);
	toGeneralBarrier.setNewLayout(pvrvk::ImageLayout::e_GENERAL);
	toGeneralBarrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	toGeneralBarrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	toGeneralBarrier.setImage(noiseImage);
	toGeneralBarrier.setSubresourceRange(pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT, 0, 1, 0, 1));
	toGeneralBarrier.setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
	toGeneralBarrier.setDstAccessMask(pvrvk::AccessFlags::e_SHADER_WRITE_BIT);
	return toGeneralBarrier;
}

/// <summary>Submits compute work to the device for execution</summary>
/// <param name="currentFrameId">Current frame index</param>
/// <param name="semaphoreWaitValue">Value for semaphore wait</param>
/// <param name="semaphoreSignalValue">Value for semaphore signal</param>
/// <param name="textureIndex">Index of the noise texture used</param>
void VulkanTimelineSemaphores::submitComputeWork(const uint32_t& currentFrameId, const uint64_t& semaphoreWaitValue, const uint64_t& semaphoreSignalValue, const uint16_t& textureIndex)
{
	pvrvk::CommandBuffer& submitCmdBuffer = _deviceResources->computeCommandBuffers[currentFrameId * 4 + textureIndex];
	pvrvk::PipelineStageFlags computePipeWaitStageFlags[] = { pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT };

	// Set the timeline semaphores to be waited on and signalled
	pvrvk::Semaphore computeWaitSemaphores[] = { _deviceResources->timelineSemaphores[currentFrameId] };
	pvrvk::Semaphore computeSignalSemaphores[] = { _deviceResources->timelineSemaphores[currentFrameId] };

	pvrvk::TimelineSemaphoreSubmitInfo mySemaphoreSubmitInfo;

	const uint64_t sumbitInfoWaitValue = semaphoreWaitValue;
	const uint64_t signalValue = semaphoreSignalValue;

	mySemaphoreSubmitInfo.waitSemaphoreValueCount = 1;
	mySemaphoreSubmitInfo.waitSemaphoreValues = &sumbitInfoWaitValue;
	mySemaphoreSubmitInfo.signalSemaphoreValueCount = 1;
	mySemaphoreSubmitInfo.signalSemaphoreValues = &signalValue;

	// Submit
	pvrvk::SubmitInfo computeSubmitInfo;
	computeSubmitInfo.commandBuffers = &submitCmdBuffer;
	computeSubmitInfo.numCommandBuffers = 1;
	computeSubmitInfo.waitSemaphores = computeWaitSemaphores;
	computeSubmitInfo.numWaitSemaphores = 1;
	computeSubmitInfo.signalSemaphores = computeSignalSemaphores;
	computeSubmitInfo.waitDstStageMask = computePipeWaitStageFlags;
	computeSubmitInfo.numSignalSemaphores = 1;
	computeSubmitInfo.timelineSemaphoreSubmitInfo = &mySemaphoreSubmitInfo;

	pvrvk::Fence lastComputePassFence{ nullptr };

	if (textureIndex == 3) lastComputePassFence = _deviceResources->endOfComputeFences[currentFrameId];

	_deviceResources->computeQueue->submit(&computeSubmitInfo, 1, lastComputePassFence);
}

/// <summary>Updates the descriptor sets used by the compute shaders.</summary>
/// <param name="readImageIndex">Index of the image to read.</param>
/// <param name="writeImageIdex">Index of the image to write to.</param>
/// <param name="currentFrameIndex">Current frame index.</param>
void VulkanTimelineSemaphores::updateComputeDescriptorSets(const uint32_t& readImageIndex, const uint32_t& writeImageIdex, const uint32_t& currentFrameIndex)
{
	assert(readImageIndex >= 0 && readImageIndex < _numberOfNoiseLayers);
	assert(writeImageIdex >= 0 && writeImageIdex < _numberOfNoiseLayers);

	const uint32_t descriptorPingPongIndex = 4 * currentFrameIndex + writeImageIdex;
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	{
		const pvrvk::ImageView srcImageView =
			readImageIndex + writeImageIdex == 0 ? _deviceResources->firstComputeIterationPlaceholderImage : _deviceResources->noiseImages[currentFrameIndex][readImageIndex];

		assert(descriptorPingPongIndex < _deviceResources->computeDescriptorSets.size());
		assert(descriptorPingPongIndex >= 0);

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->computeDescriptorSets[descriptorPingPongIndex], 0)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(srcImageView, _deviceResources->samplerNearest, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->computeDescriptorSets[descriptorPingPongIndex], 1)
									.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->noiseImages[currentFrameIndex][writeImageIdex], pvrvk::ImageLayout::e_GENERAL)));
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Calculates semaphore value over time for given swapchain index.</summary>
/// <param name="swapchainIndex">Index of the swapchain.</param>
/// <returns>The accumulated semaphore value increase.</returns>
uint64_t VulkanTimelineSemaphores::getAccumulatedSemaphoreValueIncrease(const uint32_t swapchainIndex) const
{
	return _deviceResources->semaphoreIterations[swapchainIndex] * _deviceResources->semaphoreCycleValue;
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanTimelineSemaphores>(); }
