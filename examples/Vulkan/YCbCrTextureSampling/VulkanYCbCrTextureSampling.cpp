/*!
\brief Shows how to load and display images using the YCbCr colour format
\file VulkanYCbCrTextureSampling.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRCore/commandline/CommandLine.h"

const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);

// shader attributes
const pvr::utils::VertexBindings VertexAttribBindings[] = {
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
};

// shader uniforms
namespace Uniform {
enum Enum
{
	MVPMatrix,
	LightDir,
	NumUniforms
};
}

// Content file names

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader.fsh.spv";
const char VertShaderSrcFile[] = "VertShader.vsh.spv";

// POD _scene files
const char SceneFile[] = "Plane.pod";

/// <summary>Class implementing the Shell functions.</summary>
class VulkanYCbCrTextureSampling : public pvr::Shell
{
	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
		pvrvk::Device device;
		pvrvk::Swapchain swapchain;
		pvrvk::CommandPool commandPool;
		pvrvk::DescriptorPool descriptorPool;
		pvrvk::Queue queue;
		pvr::utils::vma::Allocator vmaAllocator;
		std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;
		std::vector<pvrvk::Semaphore> presentationSemaphores;
		std::vector<pvrvk::Fence> perFrameResourcesFences;
		std::vector<pvrvk::Buffer> vbos;
		std::vector<pvrvk::Buffer> ibos;
		pvrvk::DescriptorSetLayout texLayout;
		pvrvk::DescriptorSetLayout uboLayoutDynamic;
		pvrvk::PipelineLayout pipelayout;
		pvrvk::DescriptorSet texDescSet;
		pvrvk::GraphicsPipeline pipe;
		std::vector<pvrvk::CommandBuffer> cmdBuffers; // per swapchain
		std::vector<pvrvk::Framebuffer> onScreenFramebuffer; // per swapchain
		std::vector<pvrvk::DescriptorSet> uboDescSets;
		pvr::utils::StructuredBufferView structuredBufferView;
		pvrvk::Buffer ubo;
		pvrvk::PipelineCache pipelineCache;
		pvrvk::Sampler ycbcrSampler;
		pvrvk::ImageView texBase;
		VkSamplerYcbcrConversion ycbcrSamplerConversion;
		VkSamplerYcbcrConversionInfo ycbcrInfo;
		VkSamplerYcbcrConversionCreateInfo conversionInfo;
		VkDescriptorSetLayoutBinding image_binding;
		VkPhysicalDeviceSamplerYcbcrConversionFeatures physicalDeviceSamplerYcbcrConversionFeatures;
		VkFormatProperties formatProperties;

		// Command line arguments
		pvr::platform::CommandLineParser::ParsedCommandLine parsedCommandLine;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		~DeviceResources()
		{
			if (device)
			{
				device->waitIdle();
				for (auto fence : perFrameResourcesFences)
				{
					if (fence) fence->wait();
				}
			}
		}
	};

	struct UboPerMeshData
	{
		glm::mat4 mvpMtx;
		glm::vec3 lightDirModel;
	};

	// PVR texture files
	std::string TextureFile = "ColourTest2_YUV_3-Plane_444_UNorm_sRGB.pvr";

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and view matrix
	glm::mat4 _viewProj;

	uint32_t _frameId;
	// The rotation, translation and scale parameters of the model
	float _angleX;
	float _angleY;
	float _angleZ;
	float _offsetX;
	float _offsetY;
	float _offsetZ;
	float _scale;
	std::unique_ptr<DeviceResources> _deviceResources;

	bool _cosited = false;
	bool _midpoint = false;

	uint32_t _swapchainLength;

	/// <summary>Compatible physical device number.</summary>
	uint32_t _physicalDeviceNumber;

	/// <summary>Format of the texture to display.</summary>
	pvrvk::Format _textureFormat;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createImageSamplerDescriptor(pvrvk::CommandBuffer& imageUploadCmd);
	void createUbo();
	void createPipeline();
	void drawMesh(pvrvk::CommandBuffer& cmdBuffers, int i32NodeIndex);
	void recordCommandBuffer();
	glm::mat4 animate();
};

/// <summary>Loads the textures required for this training course.</summary>
/// <returns>Return true if no error occurred.</returns>
void VulkanYCbCrTextureSampling::createImageSamplerDescriptor(pvrvk::CommandBuffer& imageUploadCmd)
{
	pvrvk::Device& device = _deviceResources->device;

	if (!(static_cast<VkFormat>(_textureFormat) >= VkFormat::VK_FORMAT_G8B8G8R8_422_UNORM && static_cast<VkFormat>(_textureFormat) <= VkFormat::VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM))
	{
		// Non-YCbCr codepath

		// create the bilinear sampler
		pvrvk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
		samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
		samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
		_deviceResources->ycbcrSampler = device->createSampler(samplerInfo);

		_deviceResources->texBase = pvr::utils::loadAndUploadImageAndView(device, TextureFile.c_str(), true, imageUploadCmd, *this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_NONE);

		_deviceResources->texBase->setObjectName("Base diffuse ImageView");

		return;
	}

	_deviceResources->conversionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
	_deviceResources->conversionInfo.pNext = nullptr;
	_deviceResources->conversionInfo.format = static_cast<VkFormat>(_textureFormat);
	_deviceResources->conversionInfo.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
	_deviceResources->conversionInfo.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
	_deviceResources->conversionInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	_deviceResources->conversionInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	_deviceResources->conversionInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	_deviceResources->conversionInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	if (_cosited)
	{
		_deviceResources->conversionInfo.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
		_deviceResources->conversionInfo.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
	}
	else
	{
		_deviceResources->conversionInfo.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
		_deviceResources->conversionInfo.yChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
	}
	_deviceResources->conversionInfo.chromaFilter = VK_FILTER_NEAREST;
	_deviceResources->conversionInfo.forceExplicitReconstruction = VK_FALSE;
	if (!device->getVkBindings().vkCreateSamplerYcbcrConversionKHR(device->getVkHandle(), &_deviceResources->conversionInfo, nullptr, &_deviceResources->ycbcrSamplerConversion))
	{
		Log(LogLevel::Information, "Successfully created YCbCr conversion.");
	}
	else
	{
		setExitMessage("Failed to create YCbCr conversion sampler.");
		return;
	}

	_deviceResources->ycbcrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
	_deviceResources->ycbcrInfo.pNext = nullptr;
	_deviceResources->ycbcrInfo.conversion = _deviceResources->ycbcrSamplerConversion;

	// Create the YCbCr sampler
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.pNext = &_deviceResources->ycbcrInfo;
	samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	samplerInfo.wrapModeU = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerInfo.wrapModeV = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerInfo.enableAnisotropy = VK_FALSE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.lodMinimum = 0.0f;
	samplerInfo.lodMaximum = 1.0f;
	_deviceResources->ycbcrSampler = device->createSampler(samplerInfo);

	_deviceResources->image_binding.binding = 0;
	_deviceResources->image_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	_deviceResources->image_binding.descriptorCount = 1;
	_deviceResources->image_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	_deviceResources->image_binding.pImmutableSamplers = &(_deviceResources->ycbcrSampler->getVkHandle());

	_deviceResources->texBase = pvr::utils::loadAndUploadImageAndView(device, TextureFile.c_str(), true, imageUploadCmd, *this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
		pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_NONE,
		&_deviceResources->ycbcrInfo);

	_deviceResources->texBase->setObjectName("Base diffuse ImageView");
}

void VulkanYCbCrTextureSampling::createUbo()
{
	//--- create the ubo descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->uboLayoutDynamic = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	std::vector<pvrvk::WriteDescriptorSet> descUpdate{ _swapchainLength };
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("MVPMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("LightDirModel", pvr::GpuDatatypes::vec3);

		_deviceResources->structuredBufferView.initDynamic(desc, _scene->getNumMeshNodes() * _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->ubo = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->structuredBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
		_deviceResources->ubo->setObjectName("Object UBO");
		_deviceResources->structuredBufferView.pointToMappedMemory(_deviceResources->ubo->getDeviceMemory()->getMappedData());
	}

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->uboDescSets[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutDynamic);
		_deviceResources->uboDescSets[i]->setObjectName("UBOSwapchain" + std::to_string(i) + "DescriptorSet");

		descUpdate[i]
			.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->uboDescSets[i])
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->ubo, 0, _deviceResources->structuredBufferView.getDynamicSliceSize()));
	}
	_deviceResources->device->updateDescriptorSets(static_cast<const pvrvk::WriteDescriptorSet*>(descUpdate.data()), _swapchainLength, nullptr, 0);
}

/// <summary>Loads and compiles the shaders and create a pipeline.</summary>
/// <returns>Return true if no error occurred.</returns>
void VulkanYCbCrTextureSampling::createPipeline()
{
	pvrvk::PipelineColorBlendAttachmentState colorAttachemtState;
	pvrvk::GraphicsPipelineCreateInfo pipeInfo;
	colorAttachemtState.setBlendEnable(false);

	//--- create the texture-sampler descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, _deviceResources->ycbcrSampler);
		_deviceResources->texLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);

		// Create the descriptor set
		// Add the ycbcrSampler to pImmutableSamplers when setting up the descriptor set layout binding
		// Conversion must be fixed at pipeline creation time, through use of a combined image sampler with an immutable sampler in DescriptorSetLayoutBinding
		_deviceResources->texDescSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texLayout);
		_deviceResources->texDescSet->setObjectName("TextureDescriptorSet");

		pvrvk::WriteDescriptorSet writeDescSets[1] = { pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->texDescSet, 0) };
		writeDescSets[0].setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->texBase, _deviceResources->ycbcrSampler));

		_deviceResources->device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
	}

	//--- create the pipeline layout
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo
			.addDescSetLayout(_deviceResources->texLayout) /*set 0*/
			.addDescSetLayout(_deviceResources->uboLayoutDynamic); /*set 1*/
		_deviceResources->pipelayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

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
	pipeInfo.pipelineLayout = _deviceResources->pipelayout;
	pipeInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pipeInfo.subpass = 0;
	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.enableDepthTest(true);
	pipeInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
	pipeInfo.depthStencil.enableDepthWrite(true);
	pvr::utils::populateInputAssemblyFromMesh(mesh, VertexAttribBindings, sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipeInfo.vertexInput, pipeInfo.inputAssembler);
	_deviceResources->pipe = _deviceResources->device->createGraphicsPipeline(pipeInfo, _deviceResources->pipelineCache);
	_deviceResources->pipe->setObjectName("YCbCrTextureSamplingGraphicsPipeline");
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanYCbCrTextureSampling::initApplication()
{
	// Load the scene
	_scene = pvr::assets::loadModel(*this, SceneFile);
	_angleX = 0.0f;
	_angleY = 0.0f;
	_angleZ = 0.0f;
	_offsetX = 0.0f;
	_offsetY = 0.0f;
	_offsetZ = 0.0f;
	_scale = 1.0f;
	_frameId = 0;

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanYCbCrTextureSampling::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanYCbCrTextureSampling::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create instance and retrieve compatible physical devices
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), pvr::utils::VulkanVersion(1, 1, 0));

	if (!_deviceResources->instance->getNumPhysicalDevices())
	{
		setExitMessage("Unable to find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Read the command line arguments
	_deviceResources->parsedCommandLine = getCommandLine();
	if (_deviceResources->parsedCommandLine.getOptionsList().empty()) { Log(LogLevel::Information, "No input arguments provided. Default values will be used."); }

	if (_deviceResources->parsedCommandLine.hasOption("-t") || _deviceResources->parsedCommandLine.hasOption("-texture"))
	{
		_deviceResources->parsedCommandLine.getStringOption("-t", TextureFile);
		_deviceResources->parsedCommandLine.getStringOption("-texture", TextureFile);
	}

	// Check for a physical device that supports the YCbCr Vulkan extensions
	std::vector<std::string> extensions;
	std::vector<std::int32_t> compatibleDevices;
	extensions.push_back("VK_KHR_bind_memory2");
	extensions.push_back("VK_KHR_maintenance1");
	extensions.push_back("VK_KHR_sampler_ycbcr_conversion");

	compatibleDevices = pvr::utils::validatePhysicalDeviceExtensions(_deviceResources->instance, extensions);
	if (compatibleDevices.empty())
	{
		setExitMessage("Unable to find a Vulkan physical device compatible with YCbCr.");
		return pvr::Result::UnknownError;
	}
	uint32_t physicalDeviceNumber = compatibleDevices[0];

	// Create the surface
	pvrvk::Surface surface = pvr::utils::createSurface(
		_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(physicalDeviceNumber), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	// Enable VK_KHR_sampler_ycbcr_conversion and extensions it requires
	pvr::utils::DeviceExtensions deviceExtensions = pvr::utils::DeviceExtensions();
	deviceExtensions.addExtension("VK_KHR_bind_memory2");
	deviceExtensions.addExtension("VK_KHR_maintenance1");
	deviceExtensions.addExtension("VK_KHR_sampler_ycbcr_conversion");

	// Enable VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES
	_deviceResources->physicalDeviceSamplerYcbcrConversionFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
	_deviceResources->physicalDeviceSamplerYcbcrConversionFeatures.pNext = nullptr;
	_deviceResources->physicalDeviceSamplerYcbcrConversionFeatures.samplerYcbcrConversion = VK_TRUE;
	deviceExtensions.addExtensionFeatureVk<VkPhysicalDeviceSamplerYcbcrConversionFeatures>(
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES, &_deviceResources->physicalDeviceSamplerYcbcrConversionFeatures);

	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device =
		pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(physicalDeviceNumber), &queuePopulateInfo, 1, &queueAccessInfo, deviceExtensions);
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);
	_deviceResources->queue->setObjectName("GraphicsQueue");

	pvr::utils::beginQueueDebugLabel(_deviceResources->queue, pvrvk::DebugUtilsLabel("initView"));

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(physicalDeviceNumber)->getSurfaceCapabilities(surface);

	// Validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	//---------------
	// Create the swapchain, on screen framebuffers and renderpass
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));
	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	_swapchainLength = _deviceResources->swapchain->getSwapchainLength();

	_deviceResources->imageAcquiredSemaphores.resize(_swapchainLength);
	_deviceResources->presentationSemaphores.resize(_swapchainLength);
	_deviceResources->perFrameResourcesFences.resize(_swapchainLength);
	_deviceResources->cmdBuffers.resize(_swapchainLength);
	_deviceResources->uboDescSets.resize(_swapchainLength);

	//---------------
	// Create the command pool and descriptor set pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	_deviceResources->commandPool->setObjectName("Main Command Pool");

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(8 * _swapchainLength))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, static_cast<uint16_t>(8 * _swapchainLength))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, static_cast<uint16_t>(8 * _swapchainLength))
																						  .setMaxDescriptorSets(static_cast<uint16_t>(8 * _swapchainLength)));
	_deviceResources->descriptorPool->setObjectName("Main Descriptor Pool");
	_deviceResources->descriptorPool->setObjectName("DescriptorPool");

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// Create the per swapchain command buffers
		_deviceResources->cmdBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->cmdBuffers[i]->setObjectName("MainCommandBufferSwapchain" + std::to_string(i));
		if (i == 0) { _deviceResources->cmdBuffers[0]->begin(); }

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->presentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));

		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameResourcesFences[i]->setObjectName("FenceSwapchain" + std::to_string(i));
	}

	// Load the vbo and ibo data
	bool requiresCommandBufferSubmission = false;
	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_scene, _deviceResources->vbos, _deviceResources->ibos, _deviceResources->cmdBuffers[0],
		requiresCommandBufferSubmission, _deviceResources->vmaAllocator);

	// Query support in case a multiplanar texture is to be displayed. VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT or
	// VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT must be supported
	auto assetStream = this->getAssetStream(TextureFile.c_str());
	pvr::Texture outTexture = pvr::textureLoad(*assetStream, pvr::getTextureFormatFromFilename(TextureFile.c_str()));
	if (!outTexture.getDataSize()) { throw pvrvk::ErrorValidationFailedEXT("TextureUtils.h:textureUpload:: Invalid texture supplied, please verify inputs."); }
	bool isDecompressed = false;
	_textureFormat = pvr::utils::convertToPVRVkPixelFormat(outTexture.getPixelFormat(), outTexture.getColorSpace(), outTexture.getChannelType(), isDecompressed);

	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceFormatProperties(
		_deviceResources->device->getPhysicalDevice()->getVkHandle(), static_cast<VkFormat>(_textureFormat), &_deviceResources->formatProperties);

	if ((static_cast<VkFormat>(_textureFormat) >= VkFormat::VK_FORMAT_G8B8G8R8_422_UNORM) && (static_cast<VkFormat>(_textureFormat) <= VkFormat::VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM))
	{
		_midpoint = _deviceResources->formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
		_cosited = _deviceResources->formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT;

		if ((_midpoint || _cosited) == false)
		{
			setExitMessage("Neither VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT nor VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT is supported.");
			return pvr::Result::UnknownError;
		}
	}

	// Create the image samplers
	createImageSamplerDescriptor(_deviceResources->cmdBuffers[0]);
	_deviceResources->cmdBuffers[0]->end();

	pvr::utils::beginQueueDebugLabel(_deviceResources->queue, pvrvk::DebugUtilsLabel("Batching Application Resource Upload"));

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();

	pvr::utils::endQueueDebugLabel(_deviceResources->queue);

	// Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("YCbCr Texture Sampling");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	// Create the uniform buffers
	createUbo();

	glm::vec3 from, to, up;
	float fov;
	_scene->getCameraProperties(0, fov, from, to, up);

	// Check if the screen is rotated
	const bool bRotate = this->isScreenRotated();

	// Calculate the projection and rotate it by 90 degree if the screen is rotated.
	_viewProj = (bRotate ? pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, static_cast<float>(this->getHeight()), static_cast<float>(this->getWidth()),
							   _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f)
						 : pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()),
							   _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar()));

	_viewProj = _viewProj * glm::lookAt(from, to, up);

	//---------------
	// Load the pipeline
	createPipeline();

	// Record the command buffers
	recordCommandBuffer();

	pvr::utils::endQueueDebugLabel(_deviceResources->queue);

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanYCbCrTextureSampling::releaseView()
{
	// Destroy each pure vulkan object
	_deviceResources->device->getVkBindings().vkDestroySamplerYcbcrConversionKHR(_deviceResources->device->getVkHandle(), _deviceResources->ycbcrSamplerConversion, nullptr);

	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Manipulate the model matrix according to the animation parameters.</summary>
/// <returns>Return the model matrix with the transformation for the current frame.</returns>
glm::mat4 VulkanYCbCrTextureSampling::animate()
{
	glm::mat4 modelMatrix(1.0f);

	// Apply X rotation
	float angleXMagnitude = 0.0f;
	float angleXOffset = 0.0f;
	uint64_t angleXDelay = 0;
	_angleX = sin(angleXMagnitude * 0.0001f * (getTime() + angleXDelay) + angleXOffset);
	modelMatrix *= glm::rotate(_angleX, glm::vec3(1.0f, 0.0f, 0.0f));

	// Apply Y rotation
	float angleYMagnitude = 5.0f;
	float angleYOffset = 0.0f;
	uint64_t angleYDelay = 0;
	_angleY = sin(angleYMagnitude * 0.0001f * (getTime() + angleYDelay) + angleYOffset);
	modelMatrix *= glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f));

	// Apply Z rotation
	float angleZMagnitude = 0.0f;
	float angleZOffset = 0.0f;
	uint64_t angleZDelay = 0;
	_angleZ = sin(angleZMagnitude * 0.0001f * (getTime() + angleZDelay) + angleZOffset);
	modelMatrix *= glm::rotate(_angleZ, glm::vec3(0.0f, 0.0f, 1.0f));

	// Apply scaling
	float scaleMagnitude = 5.0f;
	float scaleOffset = 3.0f;
	uint64_t scaleDelay = 500;
	_scale = sin(scaleMagnitude * 0.0001f * (getTime() + scaleDelay)) + scaleOffset;
	modelMatrix *= glm::scale(glm::vec3(_scale));

	// Apply translation
	float offsetYMagnitude = 4.0f;
	float offsetYOffset = 0.0f;
	uint64_t offsetYDelay = 0;
	_offsetY = sin(offsetYMagnitude * 0.0001f * (getTime() + offsetYDelay) + offsetYOffset);
	_offsetX = sqrt(1 - (_offsetY * _offsetY));
	// modelMatrix *= glm::translate(glm::vec3(_offsetX, _offsetY, _offsetZ));

	return modelMatrix;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanYCbCrTextureSampling::renderFrame()
{
	pvr::utils::beginQueueDebugLabel(_deviceResources->queue, pvrvk::DebugUtilsLabel("renderFrame"));

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	// Calculate the model matrix
	const glm::mat4 mModel = animate();

	// Set light Direction in model space
	//  The inverse of a rotation matrix is the transposed matrix
	//  Because of v * M = transpose(M) * v, this means:
	//  v * R == inverse(R) * v
	//  So we don't have to actually invert or transpose the matrix
	//  to transform back from world space to model space

	//---------------
	// update the ubo
	UboPerMeshData srcWrite;
	srcWrite.lightDirModel = glm::vec3(LightDir * mModel);
	srcWrite.mvpMtx = _viewProj * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());

	_deviceResources->structuredBufferView.getElementByName("MVPMatrix", 0, swapchainIndex).setValue(srcWrite.mvpMtx);
	_deviceResources->structuredBufferView.getElementByName("LightDirModel", 0, swapchainIndex).setValue(srcWrite.lightDirModel);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->ubo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->ubo->getDeviceMemory()->flushRange(
			_deviceResources->structuredBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->structuredBufferView.getDynamicSliceSize());
	}

	//---------------
	// SUBMIT
	pvr::utils::beginQueueDebugLabel(_deviceResources->queue, pvrvk::DebugUtilsLabel("Submitting per frame command buffers"));

	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	pvr::utils::endQueueDebugLabel(_deviceResources->queue);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	//---------------
	// PRESENT
	pvr::utils::beginQueueDebugLabel(_deviceResources->queue, pvrvk::DebugUtilsLabel("Presenting swapchain image to the screen"));

	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	pvr::utils::endQueueDebugLabel(_deviceResources->queue);

	_frameId = (_frameId + 1) % _swapchainLength;

	pvr::utils::endQueueDebugLabel(_deviceResources->queue);

	return pvr::Result::Success;
}

/// <summary>Draws a assets::Mesh after the model view matrix has been set and the material prepared.</summary>
/// <pram =name"nodeIndex">Node index of the mesh to draw.</param>
void VulkanYCbCrTextureSampling::drawMesh(pvrvk::CommandBuffer& cmdBuffers, int nodeIndex)
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
				cmdBuffers->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
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

/// <summary>Pre-record the commands.</summary>
void VulkanYCbCrTextureSampling::recordCommandBuffer()
{
	const uint32_t numSwapchains = _swapchainLength;
	pvrvk::ClearValue clearValues[2] = { pvrvk::ClearValue(0.0f, 0.45f, 0.41f, 1.f), pvrvk::ClearValue(1.f, 0u) };
	for (uint32_t i = 0; i < numSwapchains; ++i)
	{
		// begin recording commands for the current swap chain command buffer
		_deviceResources->cmdBuffers[i]->begin();
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->cmdBuffers[i], pvrvk::DebugUtilsLabel("Render Frame Commands"));

		// begin the render pass
		_deviceResources->cmdBuffers[i]->beginRenderPass(
			_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->cmdBuffers[i], pvrvk::DebugUtilsLabel("Mesh"));

		// calculate the dynamic offset to use
		const uint32_t dynamicOffset = _deviceResources->structuredBufferView.getDynamicSliceOffset(i);
		// enqueue the static states which wont be changed through out the frame
		_deviceResources->cmdBuffers[i]->bindPipeline(_deviceResources->pipe);
		_deviceResources->cmdBuffers[i]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelayout, 0, _deviceResources->texDescSet, nullptr);
		_deviceResources->cmdBuffers[i]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelayout, 1, _deviceResources->uboDescSets[i], &dynamicOffset, 1);
		drawMesh(_deviceResources->cmdBuffers[i], 0);
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBuffers[i]);

		// record the ui renderer commands
		_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBuffers[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();

		// end the renderpass
		_deviceResources->cmdBuffers[i]->endRenderPass();

		pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBuffers[i]);

		// end recording commands for the current command buffer
		_deviceResources->cmdBuffers[i]->end();
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanYCbCrTextureSampling>(); }
