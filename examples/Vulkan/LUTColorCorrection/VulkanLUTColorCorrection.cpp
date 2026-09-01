/*!
\brief Shows how to perform color correction (grading)
\file VulkanLUTColorCorrection.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
// #include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRVk/VulkanBase.h"

/// <summary>Incremental rotation angle applied to the model each frame.</summary
const float RotateY = glm::pi<float>() / 150;

/// <summary>Directional light vector in world space.</summary>
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);

/// <summary>Default size used for LUT textures.</summary>
#define DEFAULT_LUT_SIZE 16U

/// <summary>Flag value indicating usage of a 3D LUT.</summary>
#define USE_3D_LUT 1U

/// <summary>Default flag indicating whether to use 2D or 3D LUT (0 = 2D).</summary>
#define DEFAULT_USE_2D_OR_3D_LUT 0U

/// <summary>Vertex attribute bindings mapping shader inputs to buffer locations.</summary>
const pvr::utils::VertexBindings VertexAttribBindings[] =
{
	{ "POSITION", 0 }, ///< Vertex position attribute
	{ "NORMAL", 1 }, ///< Vertex normal attribute
	{ "UV0", 2 }, ///< Texture coordinate attribute
	{ "TANGENT", 3 }, ///< Tangent vector for normal mappin
};

/// <summary>Enumeration of supported LUT texture sizes.</summary
enum class LutSize
{
	LUT_16 = 16,
	LUT_32 = 32
};

// Content file names
/// <summary>Fragment shader file for main rendering pipeline.</summary>
const char FragShaderSrcFile[]{ "FragShader.fsh.spv" };

/// <summary>Vertex shader file for main rendering pipeline.</summary>
const char VertShaderSrcFile[]{ "VertShader.vsh.spv" };

/// <summary>Fragment shader file for postprocessing pass.</summary>
const char PostProcessFragShaderSrcFile[]{ "PostProcessFragShader.fsh.spv" };

/// <summary>Vertex shader file for postprocessing pass.</summary>
const char PostProcessVertShaderSrcFile[]{ "PostProcessVertShader.vsh.spv" };

/// <summary>Base color texture for the 3D model.</summary>
const std::string StatueTexFile{ "Marble" };

/// <summary>Normal map texture for the 3D model.</summary>
const std::string StatueNormalMapFile{ "MarbleNormalMap" };

/// <summary>Default LUT texture name used at startup.</summary>
const std::string initialInputLUTTexFile{ "lut_warm" };

/// <summary>Main scene file containing the 3D model and camera setup.</summary>
const char SceneFile[]{ "Satyr.pod" };

/// <summary>Platform agnostic command line argument parser.</summary>
pvr::CommandLine _cmdLine{};

/// <summary>Class implementing the Shell functions.</summary>
class VulkanLUTColorCorrection : public VulkanBase
{
	struct DeviceResources
	{
		/// <summary>Encapsulation of a Vulkan instance.</summary>
		pvrvk::Instance instance;

		/// <summary>Callbacks and messengers for debug messages.</summary>
		pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;

		/// <summary>Encapsulation of a Vulkan logical device.</summary>
		pvrvk::Device device;

		/// <summary>Encapsulation of a Vulkan swapchain.</summary>
		pvrvk::Swapchain swapchain;

		/// <summary>Command pool to allocate command buffers.</summary>
		pvrvk::CommandPool commandPool;

		/// <summary>Descriptor pool where to get descriptor sets allocated from.</summary>
		pvrvk::DescriptorPool descriptorPool;
		
		/// <summary>Main graphics queue.</summary>
		pvrvk::Queue queue;

		/// <summary>Vulkan Memory Allocator instance.</summary>
		pvr::utils::vma::Allocator vmaAllocator;

		 /// <summary>Semaphores used when acquiring swapchain images.</summary>
		std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;

		/// <summary>Semaphores used to signal presentation completion.</summary>
		std::vector<pvrvk::Semaphore> presentationSemaphores;

		/// <summary>Fences used for CPU-GPU synchronization per frame.</summary>
		std::vector<pvrvk::Fence> perFrameResourcesFences;

		/// <summary>Vertex buffer objects for meshes.</summary>
		std::vector<pvrvk::Buffer> vbos;

		/// <summary>Index buffer objects for meshes.</summary>
		std::vector<pvrvk::Buffer> ibos;

		/// <summary>Descriptor set layout for textures.</summary>
		pvrvk::DescriptorSetLayout texLayout;

		/// <summary>Descriptor set layout for dynamic uniform buffers.</summary>
		pvrvk::DescriptorSetLayout uboLayoutDynamic;

		/// <summary>Pipeline layout combining descriptor set layouts.</summary>
		pvrvk::PipelineLayout pipelayout;

		/// <summary>Descriptor set containing texture samplers.</summary>
		pvrvk::DescriptorSet texDescSet;

		/// <summary>Command buffers for each swapchain image.</summary>
		std::vector<pvrvk::CommandBuffer> cmdBuffers;

		/// <summary>Framebuffers used in the postprocessing pass.</summary>
		std::vector<pvrvk::Framebuffer> postProcessFramebuffers;

		/// <summary>Descriptor sets for uniform buffers per swapchain image.</summary>
		std::vector<pvrvk::DescriptorSet> uboDescSets;

		/// <summary>Structured buffer view for managing UBO data layout.</summary>
		pvr::utils::StructuredBufferView structuredBufferView;

		/// <summary>Uniform buffer storing frame/object data.</summary>
		pvrvk::Buffer ubo;

		/// <summary>Pipeline cache to speed up pipeline creation.</summary>
		pvrvk::PipelineCache pipelineCache;

		/// <summary>Offscreen color images used for rendering before postprocessing.</summary>
		std::vector<pvrvk::Image> offscreenColorImages;

		/// <summary>Images view for the offscreen color images.</summary>
		std::vector<pvrvk::ImageView> offscreenColorImageViews;

		/// <summary>Offscreen depth images.</summary>
		std::vector<pvrvk::Image> offscreenDepthImages;

		/// <summary>Images view for the offscreen depth images.</summary>
		std::vector<pvrvk::ImageView> offscreenDepthImageViews;

		/// <summary>Framebuffers for the offscreen render pass.</summary>
		std::vector<pvrvk::Framebuffer> offscreenFramebuffers;

		/// <summary>Framebuffer for the offscreen render pass.</summary>
		pvrvk::RenderPass offscreenRenderPass;

		/// <summary>Render pass used for postprocessing.</summary>
		pvrvk::RenderPass postProcessRenderPass;

		/// <summary>Pipeline used during offscreen rendering.</summary>
		pvrvk::GraphicsPipeline offscreenPipe;

		/// <summary>2D LUT image for color correction.</summary>
		pvrvk::Image lut2DImage;

		/// <summary>3D LUT image for color correction.</summary>
		pvrvk::Image lut3DImage;

		/// <summary>Image view for the 3D LUT.</summary>
		pvrvk::ImageView lut3DImageView;

		/// <summary>Image view for the 2D LUT.</summary>
		pvrvk::ImageView lut2DImageView;

		/// <summary>Descriptor sets for postprocessing pass.</summary>
		std::vector<pvrvk::DescriptorSet> postDescSets;

		/// <summary>Descriptor set layout for postprocessing.</summary>
		pvrvk::DescriptorSetLayout postLayout;

		/// <summary>Pipeline layout for postprocessing.</summary>
		pvrvk::PipelineLayout postPipelineLayout;

		/// <summary>Graphics pipeline used for postprocessing.</summary>
		pvrvk::GraphicsPipeline postPipeline;

		/// <summary>Sampler used in postprocessing shaders.</summary>
		pvrvk::Sampler postProcessSampler;

		/// <summary>UI renderer used to display overlay text and UI elements.</summary>
		pvr::ui::UIRenderer uiRenderer;

		/// <summary>UI text element displaying current LUT name.</summary>
		pvr::ui::Text lutNameText;

		/// <summary>Presentation surface.</summary>
		pvrvk::Surface surface;

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

	/// <summary>Mesh uniform buffer data passed to shaders.</summary>
	struct UboPerMeshData
	{
		/// <summary>MVP matrix.</summary>
		glm::mat4 mvpMtx;

		/// <summary>Light direction transformed into model space.</summary>
		glm::vec3 lightDirModel;
	};

	/// <summary>Enumeration of available LUT modes.</summary>
	enum class LutMode : int
	{
		/// <summary>Warm color grading using 2D LUT.</summary>
		Warm2D = 0,

		/// <summary>Warm color grading using 3D LUT.</summary>
		Warm3D,

		/// <summary>Cool color grading using 2D LUT.</summary>
		Cool2D,

		/// <summary>Cool color grading using 3D LUT.</summary>
		Cool3D,

		/// <summary>Total number of LUT modes.</summary>
		Size
	};

	/// <summary>3D model loaded from a POD file.</summary>
	pvr::assets::ModelHandle _scene;

	/// <summary>Combined projection and view matrix.</summary>
	glm::mat4 _viewProj;

	/// <summary>Current frame index used for synchronization.</summary>
	uint32_t _frameId;
	
	/// <summary>Rotation and translation of the model around the Y axis.</summary>
	float _angleY;

	/// <summary>Container holding all Vulkan application resources.</summary>
	std::unique_ptr<DeviceResources> _deviceResources;

	/// <summary>Current LUT texture file name.</summary>
	std::string _currentLUTFile{ initialInputLUTTexFile };

	/// <summary>Number of images in the swapchain.</summary>
	uint32_t _swapchainLength;

	/// <summary>Index selecting the current LUT mode.</summary>
	uint8_t _currentLutIndex{ 0 };

	/// <summary>Current LUT size (16 or 32).</summary>
	LutSize _currentLutSize{ LutSize::LUT_16 };

	/// <summary>Numeric LUT size used for texture creation.</summary>
	uint8_t _lutSize{ DEFAULT_LUT_SIZE };

	/// <summary>Flag indicating whether to use a 3D LUT (1) or 2D LUT (0).</summary>
	uint8_t _use3DLUT{ DEFAULT_USE_2D_OR_3D_LUT };

	/// <summary>Indicates that LUT parameters have changed and need updating.</summary>
	bool _newLUTValues{ false };

	/// <summary>Flag indicating whether ASTC texture compression format is supported.</summary>
	bool _astcSupported{ false };
	
	/// <summary>Accumulated time for automatic LUT switching.</summary>
	float _timeCounter{ 0.0f };

	/// <summary>Interval (in seconds) between LUT switches.</summary>
	const float _switchInterval{ 5000.0f };
	
	/// <summary>Flag indicating whether the automatic showcase mode is active.</summary>
	bool _showcaseEnabled{ true };

	/// <summary>Stores the previously applied LUT file name to detect when the LUT content changes.</summary>
	std::string _previousLUTFile;
	/// <summary>Stores the previously used LUT size (16 or 32) to detect when a reload of LUT resources is required.</summary>
	uint8_t _previousLutSize{ 0 };


public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createUbo();
	void createLUTResources();
	void resetLUTResources();
	void createOffscreenResources();
	void createPostProcessRenderPass();
	void createPostProcessFramebuffer();
	void createPipeline();
	void createPostProcessPipeline();
	void createPostProcessSamplerDescriptor();
	void constructLUT3DImageSlices(pvrvk::CommandBuffer& uploadBuffer);
	void createImageSamplerDescriptor(pvrvk::CommandBuffer& imageUploadCmd);
	void drawMesh(pvrvk::CommandBuffer& cmdBuffers, int i32NodeIndex);
	void recordPostProcessCommandBuffer();
	void eventMappedInput(pvr::SimplifiedInput key);
	void applyCurrentLut();
	void updatePostProcessDescriptors();
	void updateShowcase(float deltaTime);
};

/// <summary>Code in printHelp() will be called by Shell once per run, before the rendering context is created
/// if the program is ran with '-options' argument.
/// Prints the available command-line options supported by the application.</summary>
/// <returns>None.</returns>
static void printHelp()
{
	std::cout << std::endl;
	std::cout << "Supported command line options:"																		<< std::endl;
	std::cout << "    -options              : Displays this help message."												<< std::endl;
	std::cout << "    -t or - texture       : Texture to be used as LUT input from the following: lut_cool, lut_warm."	<< std::endl;
	std::cout << "    -s or - size          : Size of the LUT texture: 16, 32."											<< std::endl;
	std::cout << "    -3d or - 3D			: Use 3D LUT texture, otherwise the 2D LUT texture is used."									<< std::endl;
	std::cout << "                                                                  "									<< std::endl;
	std::cout << "Example command line call: -t=lut_cool -s=32 -3d=1												 "	<< std::endl;
	std::cout << "                                                                  "									<< std::endl;
}


/// <summary>Loads the textures required for this training course.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::createImageSamplerDescriptor(pvrvk::CommandBuffer& imageUploadCmd)
{
	pvrvk::Device& device = _deviceResources->device;
	pvrvk::ImageView texBase;
	pvrvk::ImageView texNormalMap;

	// create the bilinear sampler
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	pvrvk::Sampler samplerMipBilinear = device->createSampler(samplerInfo);

	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	pvrvk::Sampler samplerTrilinear = device->createSampler(samplerInfo);

	texBase = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, (StatueTexFile + (_astcSupported ? "_astc.pvr" : ".pvr")).c_str(), true, imageUploadCmd, *this,
		pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_NONE, nullptr);
	texNormalMap = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, (StatueNormalMapFile + (_astcSupported ? "_astc.pvr" : ".pvr")).c_str(), true, imageUploadCmd,
		*this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_NONE, nullptr);

	std::string texBaseName = "Base Diffuse ImageView";
	texBase->setObjectName(texBaseName);
	texBase->setObjectTag(static_cast<uint64_t>(0), texBaseName.size(), texBaseName.c_str());

	std::string texNormalMapName = "Base Diffuse ImageView";
	texNormalMap->setObjectName(texNormalMapName);
	texNormalMap->setObjectTag(static_cast<uint64_t>(1), texNormalMapName.size(), texNormalMapName.c_str());

	// create the descriptor set
	_deviceResources->texDescSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texLayout);

	std::string textureDescriptorSetName = "TextureDescriptorSet";
	_deviceResources->texDescSet->setObjectName(textureDescriptorSetName);
	_deviceResources->texDescSet->setObjectTag(static_cast<uint64_t>(2), textureDescriptorSetName.size(), textureDescriptorSetName.c_str());

	pvrvk::WriteDescriptorSet writeDescSets[2] = { pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->texDescSet, 0),
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->texDescSet, 1) };
	writeDescSets[0].setImageInfo(0, pvrvk::DescriptorImageInfo(texBase, samplerMipBilinear));
	writeDescSets[1].setImageInfo(0, pvrvk::DescriptorImageInfo(texNormalMap, samplerTrilinear));

	_deviceResources->device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
}

/// <summary>Loads and initializes UBOs used in the shaders.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::createUbo()
{
	std::vector<pvrvk::WriteDescriptorSet> descUpdate{ _swapchainLength };
	{
		pvr::utils::StructuredMemoryDescription desc;

		desc.addElement("MVPMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("LightDirModel", pvr::GpuDatatypes::vec3);
		desc.addElement("LutSize", pvr::GpuDatatypes::uinteger);
		desc.addElement("Use3DLUT", pvr::GpuDatatypes::uinteger);

		_deviceResources->structuredBufferView.initDynamic(desc, _scene->getNumMeshNodes() * _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->ubo = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->structuredBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
		_deviceResources->structuredBufferView.pointToMappedMemory(_deviceResources->ubo->getDeviceMemory()->getMappedData());

		std::string ObjectUBOName = "Object Ubo";
		_deviceResources->ubo->setObjectName(ObjectUBOName);
		_deviceResources->ubo->setObjectTag(static_cast<uint64_t>(2), ObjectUBOName.size(), ObjectUBOName.c_str());
	}

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->uboDescSets.push_back(_deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutDynamic));

		std::string uboDescriptorSetName = "UboDescriptorSetSwapchain" + std::to_string(i);
		_deviceResources->uboDescSets[i]->setObjectName(uboDescriptorSetName);
		_deviceResources->queue->setObjectTag(static_cast<uint64_t>(2), uboDescriptorSetName.size(), uboDescriptorSetName.c_str());

		descUpdate[i]
			.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->uboDescSets[i])
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->ubo, 0, _deviceResources->structuredBufferView.getDynamicSliceSize()));
	}
	_deviceResources->device->updateDescriptorSets(static_cast<const pvrvk::WriteDescriptorSet*>(descUpdate.data()), _swapchainLength, nullptr, 0);
}

/// <summary>Loads and compiles the shaders and create a pipeline.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::createPipeline()
{
	pvrvk::PipelineColorBlendAttachmentState colorAttachemtState;
	pvrvk::GraphicsPipelineCreateInfo pipeInfo;
	colorAttachemtState.setBlendEnable(false);

	//--- create the texture-sampler descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); /* binding 0*/
		descSetLayoutInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); /* binding 1*/
		_deviceResources->texLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the ubo descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); /*binding 0*/
		_deviceResources->uboLayoutDynamic = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the pipeline layout
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo
			.addDescSetLayout(_deviceResources->texLayout) /*set 0*/
			.addDescSetLayout(_deviceResources->uboLayoutDynamic); /*set 1*/
		_deviceResources->pipelayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	pvrvk::Rect2D rect;
	rect = pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight());

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

	pipeInfo.subpass = 0;
	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.enableDepthTest(true);
	pipeInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
	pipeInfo.depthStencil.enableDepthWrite(true);

	VkPipelineOfflineCreateInfo pipelineOfflineCreateInfo{};
	
	pvr::utils::populateInputAssemblyFromMesh(mesh, VertexAttribBindings, sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipeInfo.vertexInput, pipeInfo.inputAssembler);

	pipeInfo.renderPass = _deviceResources->offscreenRenderPass;
	_deviceResources->offscreenPipe = _deviceResources->device->createGraphicsPipeline(pipeInfo, _deviceResources->pipelineCache);
	_deviceResources->offscreenPipe->setObjectName("OffscreenPipeline");
}

/// <summary>Loads and compiles the shaders and create a pipeline for the postprocessing pass.</summary>
/// <returns>Return true if no error occurred.</returns>
void VulkanLUTColorCorrection::createPostProcessPipeline()
{
	// Define the descriptor set layout
	pvrvk::DescriptorSetLayoutCreateInfo layoutInfo;
	// Binding for sSceneTexture
	layoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	// Binding for 3D LUT
	layoutInfo.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	// Binding for 2D LUT
	layoutInfo.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->postLayout = _deviceResources->device->createDescriptorSetLayout(layoutInfo);

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	pipeLayoutInfo.addDescSetLayout(_deviceResources->postLayout);
	pipeLayoutInfo.addDescSetLayout(_deviceResources->uboLayoutDynamic);
	_deviceResources->postPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	std::unique_ptr<pvr::Stream> vertSource = getAssetStream(PostProcessVertShaderSrcFile);
	std::unique_ptr<pvr::Stream> fragSource = getAssetStream(PostProcessFragShaderSrcFile);

	pvrvk::GraphicsPipelineCreateInfo pipeInfo;

	pipeInfo.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertSource->readToEnd<uint32_t>())));
	pipeInfo.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>())));

	// Set primitive topology to draw triangles
	pipeInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

	pipeInfo.pipelineLayout = _deviceResources->postPipelineLayout;
	pipeInfo.renderPass = _deviceResources->postProcessRenderPass;
	pipeInfo.subpass = 0;

	pipeInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

	pvrvk::Rect2D rect;

	rect = pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight());

	pipeInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(static_cast<float>(rect.getOffset().getX()), static_cast<float>(rect.getOffset().getY()), static_cast<float>(rect.getExtent().getWidth()),
			static_cast<float>(rect.getExtent().getHeight())),
		rect);

	pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);

	_deviceResources->postPipeline = _deviceResources->device->createGraphicsPipeline(pipeInfo, _deviceResources->pipelineCache);

	_deviceResources->postPipeline->setObjectName("LUTColorCorrectionGraphicsPostProcessPipeline");
}

/// <summary>Loads and create a descriptor set for the postprocessing pass.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::createPostProcessSamplerDescriptor()
{
	pvrvk::SamplerCreateInfo samplerInfo;

	samplerInfo.wrapModeU = samplerInfo.wrapModeV = samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;

	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;

	_deviceResources->postProcessSampler = _deviceResources->device->createSampler(samplerInfo);
	
	
	_deviceResources->postDescSets.resize(_swapchainLength);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->postDescSets[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->postLayout);

		pvrvk::WriteDescriptorSet writeDescSets[3] = { pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->postDescSets[i], 0),
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->postDescSets[i], 1),
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->postDescSets[i], 2) };

		writeDescSets[0].setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->offscreenColorImageViews[i], _deviceResources->postProcessSampler));

		writeDescSets[1].setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->lut3DImageView, _deviceResources->postProcessSampler));

		writeDescSets[2].setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->lut2DImageView, _deviceResources->postProcessSampler));

		_deviceResources->device->updateDescriptorSets(writeDescSets, ARRAY_SIZE(writeDescSets), nullptr, 0);
	}
}

/// <summary>Create image container, renderpass and framebuffer for the offscreen pass.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::createOffscreenResources()
{
	uint32_t swapchainWidth = _deviceResources->swapchain->getDimension().getWidth();
	uint32_t swapchainHeight = _deviceResources->swapchain->getDimension().getHeight();

	// Define the size of the image
	pvrvk::Extent3D imageExtent(swapchainWidth, swapchainHeight, 1);

	// Create depth image
	pvrvk::Format depthFormat = pvrvk::Format::e_D16_UNORM;

	// Create renderpass
	pvrvk::RenderPassCreateInfo renderPassInfo;

		renderPassInfo.setAttachmentDescription(0,
			pvrvk::AttachmentDescription::createColorDescription(
				pvrvk::Format::e_R8G8B8A8_UNORM,
				pvrvk::ImageLayout::e_UNDEFINED,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
				pvrvk::AttachmentLoadOp::e_CLEAR,
				pvrvk::AttachmentStoreOp::e_STORE));

		renderPassInfo.setAttachmentDescription(1,
			pvrvk::AttachmentDescription::createDepthStencilDescription(
				depthFormat,
				pvrvk::ImageLayout::e_UNDEFINED,
				pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				pvrvk::AttachmentLoadOp::e_CLEAR,
				pvrvk::AttachmentStoreOp::e_DONT_CARE));

		pvrvk::SubpassDescription subpass;
		subpass.setColorAttachmentReference(0,
			pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

		subpass.setDepthStencilAttachmentReference(
			pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

		renderPassInfo.setSubpass(0, subpass);

		_deviceResources->offscreenRenderPass =
			_deviceResources->device->createRenderPass(renderPassInfo);


	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// Create color image
		pvrvk::Image colorImage = pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8G8B8A8_UNORM, imageExtent,
				pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);

		pvrvk::ImageView colorView = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(colorImage));

		_deviceResources->offscreenColorImages.push_back(colorImage);
		_deviceResources->offscreenColorImageViews.push_back(colorView);

		// Create depth image
		pvrvk::Image depthImage = pvr::utils::createImage(_deviceResources->device,
			pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, depthFormat, imageExtent, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);

		pvrvk::ImageView depthView = _deviceResources->device->createImageView(
			pvrvk::ImageViewCreateInfo(depthImage, pvrvk::ImageViewType::e_2D, depthFormat, pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_DEPTH_BIT)));

		_deviceResources->offscreenDepthImages.push_back(depthImage);
		_deviceResources->offscreenDepthImageViews.push_back(depthView);

		// Create framebuffer
		pvrvk::FramebufferCreateInfo fbInfo;
		fbInfo.setRenderPass(_deviceResources->offscreenRenderPass);
		fbInfo.setAttachment(0, colorView);
		fbInfo.setAttachment(1, depthView);
		fbInfo.setDimensions(swapchainWidth, swapchainHeight);

		_deviceResources->offscreenFramebuffers.push_back(_deviceResources->device->createFramebuffer(fbInfo));

	}
}

/// <summary>Loads and create 2D and 3D image containers for the LUT output texture.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::createLUTResources()
{
	uint32_t imageSize = _lutSize;

	pvrvk::ImageUsageFlags imageUsage = pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT;

	// Creation of the 3D Image Resources
	// Define the size of the image
	pvrvk::Extent3D imageExtent3D(imageSize, imageSize, imageSize);

	_deviceResources->lut3DImage =
		pvr::utils::createImage(_deviceResources->device, pvrvk::ImageCreateInfo(pvrvk::ImageType::e_3D, pvrvk::Format::e_R8G8B8A8_UNORM, imageExtent3D, imageUsage),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
	_deviceResources->lut3DImage->setObjectName("3DLUTImage");

	pvrvk::ImageViewCreateInfo viewInfo3D(_deviceResources->lut3DImage);

	viewInfo3D.setViewType(pvrvk::ImageViewType::e_3D);

	_deviceResources->lut3DImageView = _deviceResources->device->createImageView(viewInfo3D);
	_deviceResources->lut3DImageView->setObjectName("3DLUTImageView");

	// Creation of the 2D Image Resources
	// Define the size of the image
	pvrvk::Extent3D imageExtent2D(imageSize * imageSize, imageSize, 1);

	_deviceResources->lut2DImage =
		pvr::utils::createImage(_deviceResources->device, pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_R8G8B8A8_UNORM, imageExtent2D, imageUsage),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, _deviceResources->vmaAllocator);
	_deviceResources->lut2DImage->setObjectName("2DLUTImage");

	pvrvk::ImageViewCreateInfo viewInfo2D(_deviceResources->lut2DImage);

	viewInfo2D.setViewType(pvrvk::ImageViewType::e_2D);

	_deviceResources->lut2DImageView = _deviceResources->device->createImageView(viewInfo2D);
	_deviceResources->lut2DImageView->setObjectName("2DLUTImageView");
}

/// <summary>Resets 3D and 2D image and imageview containers.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::resetLUTResources()
{
	_deviceResources->lut3DImageView.reset();
	_deviceResources->lut2DImageView.reset();
	_deviceResources->lut3DImage.reset();
	_deviceResources->lut2DImage.reset();
}

/// <summary>Initializes the renderpass for the postprocessing pass.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::createPostProcessRenderPass()
{
	pvrvk::RenderPassCreateInfo renderPassInfo;

	// color attachment
	renderPassInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(_deviceResources->swapchain->getImageFormat(), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_PRESENT_SRC_KHR,
			pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE));

	pvrvk::SubpassDescription subpass;
	subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

	renderPassInfo.setSubpass(0, subpass);

	pvrvk::SubpassDependency externalDependencies[2];

	externalDependencies[0] = pvrvk::SubpassDependency(pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT,
			pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::DependencyFlags::e_BY_REGION_BIT);

	externalDependencies[1] = pvrvk::SubpassDependency(0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT,
			pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::DependencyFlags::e_BY_REGION_BIT);

	renderPassInfo.addSubpassDependency(externalDependencies[0]);
	renderPassInfo.addSubpassDependency(externalDependencies[1]);

	_deviceResources->postProcessRenderPass = _deviceResources->device->createRenderPass(renderPassInfo);

	_deviceResources->postProcessRenderPass->setObjectName("PostProcessRenderPass");
}

/// <summary>Construct the postprocessing framebuffer with the postprocessing renderpass.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::createPostProcessFramebuffer()
{
	uint32_t width = _deviceResources->swapchain->getDimension().getWidth();
	uint32_t height = _deviceResources->swapchain->getDimension().getHeight();

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		pvrvk::FramebufferCreateInfo fbInfo;

		fbInfo.setRenderPass(_deviceResources->postProcessRenderPass);
		fbInfo.setAttachment(0, _deviceResources->swapchain->getImageView(i));
		fbInfo.setDimensions(width, height);

		_deviceResources->postProcessFramebuffers.push_back(_deviceResources->device->createFramebuffer(fbInfo));
	}
}

/// <summary>Construct and update the 2D and 3D LUT texture from the input LUT texture.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::constructLUT3DImageSlices(pvrvk::CommandBuffer& uploadBuffer)
{
	const int size = _lutSize;
	const uint8_t inputChannelNo = 3;
	const uint8_t outputChannelNo = 4;
	// allocate buffer for 3D LUT
	std::vector<uint8_t> data(size * size * size * outputChannelNo, 0);
	std::vector<uint8_t> data3D(size * size * size * outputChannelNo, 0);
	std::unique_ptr<pvr::Stream> lutStream = getAssetStream(_currentLUTFile);

	pvr::Texture lutTexture = pvr::textureLoad(*lutStream);

	uint32_t width = lutTexture.getWidth();
	uint32_t rowStride = width * outputChannelNo;
	const uint8_t* src = lutTexture.getDataPointer();
	
	for (uint32_t b = 0; b < size; ++b)
	{
		for (uint32_t g = 0; g < size; ++g)
		{
			for (uint32_t r = 0; r < size; ++r)
			{
				// source is horizontal strip
				uint32_t srcIndex = ((b * size + g * size * size) + r) * inputChannelNo;
				// destination is 3D
				uint32_t dstIndex = ((b * size * size) + (g * size) + r) * outputChannelNo;

				data3D[dstIndex + 0] = src[srcIndex + 0];
				data3D[dstIndex + 1] = src[srcIndex + 1];
				data3D[dstIndex + 2] = src[srcIndex + 2];
				data3D[dstIndex + 3] = 255;
			}
		}
	}

	// upload as a 3D image
	pvr::utils::ImageUpdateInfo updateInfo3D;

	updateInfo3D.offsetX = 0;
	updateInfo3D.offsetY = 0;
	updateInfo3D.offsetZ = 0;

	updateInfo3D.imageWidth = size;
	updateInfo3D.imageHeight = size;
	updateInfo3D.imageDepth = size;

	updateInfo3D.dataWidth = size;
	updateInfo3D.dataHeight = size;

	updateInfo3D.mipLevel = 0;
	updateInfo3D.arrayIndex = 0;
	updateInfo3D.cubeFace = 0;
	updateInfo3D.numPlanes = 1;
	updateInfo3D.planeIndex = 0;

	updateInfo3D.data = data3D.data();
	updateInfo3D.dataSize = data3D.size();

	// convert LUT data from RGB to RGBA
	for (uint32_t b = 0; b < size; ++b)
	{
		for (uint32_t g = 0; g < size; ++g)
		{
			for (uint32_t r = 0; r < size; ++r)
			{
				uint32_t dstIndex = ((b * size + g * size * size) + r) * outputChannelNo;
				uint32_t srcIndex = ((b * size + g * size * size) + r) * inputChannelNo;

				data[dstIndex + 0] = src[srcIndex + 0];
				data[dstIndex + 1] = src[srcIndex + 1];
				data[dstIndex + 2] = src[srcIndex + 2];
				data[dstIndex + 3] = 255;
			}
		}
	}

	// upload as a 2D image
	pvr::utils::ImageUpdateInfo updateInfo2D;

	updateInfo2D.offsetX = 0;
	updateInfo2D.offsetY = 0;
	updateInfo2D.offsetZ = 0;

	updateInfo2D.imageWidth = size * size;
	updateInfo2D.imageHeight = size;
	updateInfo2D.imageDepth = 1;

	updateInfo2D.dataWidth = size * size;
	updateInfo2D.dataHeight = size;

	updateInfo2D.mipLevel = 0;
	updateInfo2D.arrayIndex = 0;
	updateInfo2D.cubeFace = 0;
	updateInfo2D.numPlanes = 1;
	updateInfo2D.planeIndex = 0;

	updateInfo2D.data = data.data();
	updateInfo2D.dataSize = data.size();

	// Populate the images with the constructed data
	pvr::utils::updateImage(_deviceResources->device, uploadBuffer, &updateInfo3D, 1, pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, false,
		_deviceResources->lut3DImage, _deviceResources->vmaAllocator, false);

	pvrvk::MemoryBarrierSet barrier3D;

	// Specify explicitly the shader read to wait for the transfer operation
	barrier3D.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, _deviceResources->lut3DImage,
		pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
		_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

	barrier3D.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, _deviceResources->lut2DImage,
		pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
		_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

	uploadBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, barrier3D);

	pvr::utils::updateImage(_deviceResources->device, uploadBuffer, &updateInfo2D, 1, pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, false,
		_deviceResources->lut2DImage, _deviceResources->vmaAllocator, false);

	pvrvk::MemoryBarrierSet barrier2D;

	// Specify explicitly the shader read to wait for the transfer operation
	barrier2D.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, _deviceResources->lut2DImage,
		pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
		_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

	barrier2D.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, _deviceResources->lut3DImage,
		pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL,
		_deviceResources->queue->getFamilyIndex(), _deviceResources->queue->getFamilyIndex()));

	uploadBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, barrier2D);
}

/// <summary>Checks for user input from the touchscreen or keyboard/mouse setup and update the LUT output.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::eventMappedInput(pvr::SimplifiedInput key)
{
	// disable showcase mode on user interaction
	_showcaseEnabled = false;

	switch (key)
	{
		// mobile tap or left mouse click to select in the left 30% of screen. On keyboard is key '1'
		// cycles between texture sizes
	case pvr::SimplifiedInput::Action2:
		_currentLutIndex--;
		_currentLutIndex = _currentLutIndex % (int)LutMode::Size;

		if (_currentLutIndex < 0) _currentLutIndex = 3;
		break;

		// mobile tap or left mouse click to select in the right 30% of screen. On keyboard is key '2'
		// cycles between texture names
	case pvr::SimplifiedInput::Action3:
		_currentLutIndex++;
		_currentLutIndex = _currentLutIndex % (int)LutMode::Size;

		if (_currentLutIndex > 3) _currentLutIndex = 0;
		break;

		// cycles between texture names
	case pvr::SimplifiedInput::Left:
		_currentLutIndex--;
		_currentLutIndex = _currentLutIndex % (int)LutMode::Size;

		if (_currentLutIndex < 0) _currentLutIndex = 3;
		break;

		// cycles between texture names
	case pvr::SimplifiedInput::Right:
		_currentLutIndex++;
		_currentLutIndex = _currentLutIndex % (int)LutMode::Size;

		if (_currentLutIndex > 3) _currentLutIndex = 0;
		break;

		// cycles between texture sizes
	case pvr::SimplifiedInput::Up:
	case pvr::SimplifiedInput::Down:
		if(_currentLutSize == LutSize::LUT_16)
			_currentLutSize = LutSize::LUT_32;
		else
			_currentLutSize = LutSize::LUT_16;
		break;

		// closes the application
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;

	default: return;
	}

	_lutSize = static_cast<uint8_t>(_currentLutSize);
	
	_newLUTValues = true;
}

/// <summary>Updates the displayed lut texture based on the user input.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::applyCurrentLut()
{
	std::string newFile;
	uint8_t newUse3D;

	switch (_currentLutIndex)
	{
	// 2D warm LUT
	case (int)LutMode::Warm2D:
		newFile = "lut_warm_" + std::to_string(_lutSize) + ".pvr";
		newUse3D = 0;
		break;

	// 3D warm LUT
	case (int)LutMode::Warm3D:
		newFile = "lut_warm_" + std::to_string(_lutSize) + ".pvr";
		newUse3D = 1;
		break;

	// 2D cool LUT
	case (int)LutMode::Cool2D:
		newFile = "lut_cool_" + std::to_string(_lutSize) + ".pvr";
		newUse3D = 0;
		break;

	// 3D cool LUT
	case (int)LutMode::Cool3D:
		newFile = "lut_cool_" + std::to_string(_lutSize) + ".pvr";
		newUse3D = 1;
		break;

	default: break;
	}

	
	bool lutFileChanged = (newFile != _previousLUTFile);
	bool lutSizeChanged = (_lutSize != _previousLutSize);

	
	_use3DLUT = newUse3D;
	_currentLUTFile = newFile;


	// Update the UI text
	if (_use3DLUT == 1)
		_deviceResources->lutNameText->setText(_currentLUTFile + "  3D LUT ");
	else
		_deviceResources->lutNameText->setText(_currentLUTFile + "  2D LUT ");

	_deviceResources->lutNameText->commitUpdates();

	
	if (lutFileChanged || lutSizeChanged)
	{
		resetLUTResources();
		createLUTResources();

		pvrvk::CommandBuffer uploadCmd = _deviceResources->commandPool->allocateCommandBuffer();
		uploadCmd->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);

		constructLUT3DImageSlices(uploadCmd);

		uploadCmd->end();

		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &uploadCmd;
		submitInfo.numCommandBuffers = 1;

		_deviceResources->queue->submit(&submitInfo, 1);
		_deviceResources->queue->waitIdle();

		updatePostProcessDescriptors();
		recordPostProcessCommandBuffer();
	}
	
	_previousLUTFile = newFile;
	_previousLutSize = _lutSize;


}

/// <summary>Updates the write descriptor sets for all swapchain images.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::updatePostProcessDescriptors()
{
	std::vector<pvrvk::WriteDescriptorSet> writes;

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		writes.clear();

		writes.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->postDescSets[i], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->offscreenColorImageViews[i], _deviceResources->postProcessSampler)));

		writes.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->postDescSets[i], 1)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->lut3DImageView, _deviceResources->postProcessSampler)));

		writes.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->postDescSets[i], 2)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->lut2DImageView, _deviceResources->postProcessSampler)));

		_deviceResources->device->updateDescriptorSets(writes.data(), 3, nullptr, 0);
	}
}

/// <summary></summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::updateShowcase(float deltaTime)
{
	if (!_showcaseEnabled) return;

	_timeCounter += deltaTime;

	if (_timeCounter < _switchInterval) return;

	// reset timer
	_timeCounter = 0.0f;

	// cycle to the next LUT mode
	_currentLutIndex++;

	if (_currentLutIndex >= static_cast<uint8_t>(LutMode::Size))
	{
		_currentLutIndex = 0;

		// switch LUT size when we finish one full cycle
		if (_currentLutSize == LutSize::LUT_16)
			_currentLutSize = LutSize::LUT_32;
		else
			_currentLutSize = LutSize::LUT_16;

		_lutSize = static_cast<uint8_t>(_currentLutSize);
	}

	// mark LUT update required
	_newLUTValues = true;
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanLUTColorCorrection::initApplication()
{
	std::cout << "Use command-line parameter -options to see the available command-line options for this demo." << std::endl;

	_cmdLine = this->getCommandLine();

	bool help = false;

	_cmdLine.getBoolOptionSetTrueIfPresent("-options", help);

	if (help)
	{
		printHelp();

		return pvr::Result::ExitRenderFrame;
	}

	// Load the scene
	_scene = pvr::assets::loadModel(*this, SceneFile);
	_angleY = 0.0f;
	_frameId = 0;

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanLUTColorCorrection::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanLUTColorCorrection::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// create Vulkan 1.0 instance and retrieve compatible physical devices
	pvr::utils::VulkanVersion vulkanVersion(1, 0, 0);

	pvr::utils::InstanceLayers instanceLayers = pvr::utils::InstanceLayers();
	
	pvr::utils::InstanceExtensions instanceExtensions = pvr::utils::InstanceExtensions(vulkanVersion);

	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), vulkanVersion, instanceExtensions, instanceLayers);

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable to find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// create the surface
	_deviceResources->surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT };

	queuePopulateInfo.surface = _deviceResources->surface;

	pvr::utils::QueueAccessInfo queueAccessInfo;
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	std::vector<std::uint8_t> _vectorPipelineCache;

	
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	std::string queueName = "GraphicsQueue";
	_deviceResources->queue->setObjectName(queueName);
	_deviceResources->queue->setObjectTag(static_cast<uint64_t>(2), queueName.size(), queueName.c_str());

	// check ASTC device support
	_astcSupported = pvr::utils::isSupportedFormat(_deviceResources->device->getPhysicalDevice(), pvrvk::Format::e_ASTC_4x4_UNORM_BLOCK);

	// extract the LUT texture name from the command line
	if (_cmdLine.hasOption("-t"))
	{
		// disable showcase mode on command line interaction
		_showcaseEnabled = false;

		bool stringOptionResult = _cmdLine.getStringOption("-t", _currentLUTFile);

		if (false == stringOptionResult) { _currentLUTFile = initialInputLUTTexFile; }
	}
	else if (_cmdLine.hasOption("-texture"))
	{
		// disable showcase mode on command line interaction
		_showcaseEnabled = false;

		bool stringOptionResult = _cmdLine.getStringOption("-texture", _currentLUTFile);

		if (false == stringOptionResult) { _currentLUTFile = initialInputLUTTexFile; }
	}

	// extract the LUT texture size from the command line
	if (_cmdLine.hasOption("-s"))
	{
		// disable showcase mode on command line interaction
		_showcaseEnabled = false;

		std::string sizeString;
		bool intOptionResult = _cmdLine.getStringOption("-s", sizeString);

		_lutSize = std::stoi(sizeString);

		if (false == intOptionResult) { _lutSize = DEFAULT_LUT_SIZE; }
	}
	else if (_cmdLine.hasOption("-size"))
	{
		// disable showcase mode on command line interaction
		_showcaseEnabled = false;

		std::string sizeString;
		bool intOptionResult = _cmdLine.getStringOption("-size", sizeString);

		_lutSize = std::stoi(sizeString);

		if (false == intOptionResult) { _lutSize = DEFAULT_LUT_SIZE; }
	}

	// extract the LUT texture format (2D or 3D) from the command line
	if (_cmdLine.hasOption("-3d"))
	{
		// disable showcase mode on command line interaction
		_showcaseEnabled = false;

		_use3DLUT = 1;
	}
	else if (_cmdLine.hasOption("-3D"))
	{
		// disable showcase mode on command line interaction
		_showcaseEnabled = false;

		_use3DLUT = 1;
	}

	if (_currentLUTFile == "lut_warm")
	{
		if(_use3DLUT == 0)
			_currentLutIndex = 0;
		else if(_use3DLUT == 1)
			_currentLutIndex = 1;
	}
	else if (_currentLUTFile == "lut_cool")
	{
		if (_use3DLUT == 0)
			_currentLutIndex = 2;
		else if (_use3DLUT == 1)
			_currentLutIndex = 3;
	}

	_currentLUTFile = _currentLUTFile + "_" + std::to_string(_lutSize) + ".pvr";


	pvr::utils::beginQueueDebugLabel(_deviceResources->queue, pvrvk::DebugUtilsLabel("initView"));

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	} //---------------
		// create the swapchain, on screen framebuffers and renderpass
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, _deviceResources->surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));
	_deviceResources->swapchain = swapChainCreateOutput.swapchain;

	auto format = _deviceResources->swapchain->getImageFormat();
	std::cout << "Swapchain format: " << (int)format << std::endl;
	_swapchainLength = _deviceResources->swapchain->getSwapchainLength();

	_deviceResources->imageAcquiredSemaphores.resize(_swapchainLength);
	_deviceResources->presentationSemaphores.resize(_swapchainLength);
	_deviceResources->perFrameResourcesFences.resize(_swapchainLength);
	_deviceResources->cmdBuffers.resize(_swapchainLength);

	//---------------
	// create the command pool and descriptor set pool
	pvrvk::CommandPoolCreateInfo commandPoolCreateInfo = pvrvk::CommandPoolCreateInfo(_deviceResources->queue->getFamilyIndex());

	commandPoolCreateInfo.setFlags(pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT);
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(commandPoolCreateInfo);

	std::string mainCommandPoolName = "Main Command Pool";
	_deviceResources->commandPool->setObjectName(mainCommandPoolName);
	_deviceResources->commandPool->setObjectTag(static_cast<uint64_t>(2), mainCommandPoolName.size(), mainCommandPoolName.c_str());

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
			.addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, (_swapchainLength * 3) + 2)
			.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _swapchainLength)
			.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _swapchainLength)
			.setMaxDescriptorSets((_swapchainLength * 2) + 1));

	std::string descriptorPoolName = "DescriptorPool";
	_deviceResources->descriptorPool->setObjectName("DescriptorPool");
	_deviceResources->descriptorPool->setObjectTag(static_cast<uint64_t>(2), descriptorPoolName.size(), descriptorPoolName.c_str());

	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// create the per swapchain command buffers
		_deviceResources->cmdBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();

		std::string cmdBufferName = "Main CommandBuffer [" + std::to_string(i) + "]";
		_deviceResources->cmdBuffers[i]->setObjectName(cmdBufferName);
		_deviceResources->cmdBuffers[i]->setObjectTag(static_cast<uint64_t>(3), cmdBufferName.size(), cmdBufferName.c_str());

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();

		std::string presentationSemaphoreName = "PresentationSemaphoreSwapchain" + std::to_string(i);
		_deviceResources->presentationSemaphores[i]->setObjectName(presentationSemaphoreName);
		_deviceResources->presentationSemaphores[i]->setObjectTag(static_cast<uint64_t>(2), presentationSemaphoreName.size(), presentationSemaphoreName.c_str());

		std::string imageAcquiredSemaphoreName = "ImageAcquiredSemaphoreSwapchain" + std::to_string(i);
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName(imageAcquiredSemaphoreName);
		_deviceResources->imageAcquiredSemaphores[i]->setObjectTag(static_cast<uint64_t>(2), imageAcquiredSemaphoreName.size(), imageAcquiredSemaphoreName.c_str());

		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);

		std::string fenceSwapchainName = "FenceSwapchain" + std::to_string(i);
		_deviceResources->perFrameResourcesFences[i]->setObjectName(fenceSwapchainName);
		_deviceResources->perFrameResourcesFences[i]->setObjectTag(static_cast<uint64_t>(2), fenceSwapchainName.size(), fenceSwapchainName.c_str());
	}

	// create a single time submit command buffer for uploading resources
	pvrvk::CommandBuffer uploadBuffer = _deviceResources->commandPool->allocateCommandBuffer();

	std::string uploadBufferName = "InitView : Upload Command Buffer";
	uploadBuffer->setObjectName(uploadBufferName);
	uploadBuffer->setObjectTag(static_cast<uint64_t>(2), uploadBufferName.size(), uploadBufferName.c_str());

	uploadBuffer->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);

	// load the vbo and ibo data
	bool requiresCommandBufferSubmission = false;
	pvr::utils::appendSingleBuffersFromModel(
		_deviceResources->device, *_scene, _deviceResources->vbos, _deviceResources->ibos, uploadBuffer, requiresCommandBufferSubmission, _deviceResources->vmaAllocator);

	// create the offscreen image, image viewer, framebuffer and renderbuffer
	createOffscreenResources();

	// create postprocessing renderpass and framebuffer
	createPostProcessRenderPass();
	createPostProcessFramebuffer();

	// load the pipeline
	createPipeline();

	createPostProcessPipeline();

	// create the LUT 3D image
	createLUTResources();

	createPostProcessSamplerDescriptor();

	// create the image samplers
	createImageSamplerDescriptor(uploadBuffer);

	uploadBuffer->end();

	pvr::utils::beginQueueDebugLabel(_deviceResources->queue, pvrvk::DebugUtilsLabel("Batching Application Resource Upload"));

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &uploadBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle();

	pvr::utils::endQueueDebugLabel(_deviceResources->queue);

	//  Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->postProcessRenderPass, 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("LUTColorCorrection");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	
	_deviceResources->lutNameText = _deviceResources->uiRenderer.createText(_currentLUTFile + "  3D?: " + std::to_string(_use3DLUT));
	_deviceResources->lutNameText->setPixelOffset(glm::vec2(430.f, 370.f));
	_deviceResources->lutNameText->setScale(glm::vec2(0.5f, 0.5f));
	_deviceResources->lutNameText->commitUpdates();

	// create the uniform buffers
	createUbo();

	applyCurrentLut();
	glm::vec3 from, to, up;
	float fov;
	_scene->getCameraProperties(0, fov, from, to, up);

	// Is the screen rotated
	const bool bRotate = this->isScreenRotated();

	//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
	_viewProj = (bRotate ? pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, static_cast<float>(this->getHeight()), static_cast<float>(this->getWidth()),
							   _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f)
						 : pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()),
							   _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar()));

	_viewProj = _viewProj * glm::lookAt(from, to, up);

	// record the command buffers
	recordPostProcessCommandBuffer();

	pvr::utils::endQueueDebugLabel(_deviceResources->queue);

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanLUTColorCorrection::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanLUTColorCorrection::renderFrame()
{
	pvr::utils::beginQueueDebugLabel(_deviceResources->queue, pvrvk::DebugUtilsLabel("renderFrame"));

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	// Calculate the model matrix
	const glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
	_angleY += -RotateY * 0.05f * getFrameTime();

	updateShowcase(getFrameTime());

	if (_newLUTValues)
	{
		_newLUTValues = false;
		applyCurrentLut();
	}
	
	// Update the ubo
	UboPerMeshData srcWrite;

	// Set light Direction in model space
	//  The inverse of a rotation matrix is the transposed matrix
	//  Because of v * M = transpose(M) * v, this means:
	//  v * R == inverse(R) * v
	//  So we don't have to actually invert or transpose the matrix
	//  to transform back from world space to model space
	srcWrite.lightDirModel = glm::vec3(LightDir * mModel);
	srcWrite.mvpMtx = _viewProj * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());

	_deviceResources->structuredBufferView.getElementByName("MVPMatrix", 0, swapchainIndex).setValue(srcWrite.mvpMtx);
	_deviceResources->structuredBufferView.getElementByName("LightDirModel", 0, swapchainIndex).setValue(srcWrite.lightDirModel);
	_deviceResources->structuredBufferView.getElementByName("LutSize", 0, swapchainIndex).setValue(_lutSize);
	_deviceResources->structuredBufferView.getElementByName("Use3DLUT", 0, swapchainIndex).setValue(_use3DLUT);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->ubo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->ubo->getDeviceMemory()->flushRange(
			_deviceResources->structuredBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->structuredBufferView.getDynamicSliceSize());
	}

	// Submit
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

	// Present
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
void VulkanLUTColorCorrection::drawMesh(pvrvk::CommandBuffer& cmdBuffers, int nodeIndex)
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

/// <summary>Pre-record the commands for the postprocessing pass.</summary>
/// <returns>None.</returns>
void VulkanLUTColorCorrection::recordPostProcessCommandBuffer()
{
	const uint32_t numSwapchains = _swapchainLength;

	for (uint32_t i = 0; i < numSwapchains; ++i)
	{
		const uint32_t dynamicOffset = _deviceResources->structuredBufferView.getDynamicSliceOffset(i);

		pvrvk::ClearValue clearValuesOffscreen[2] = { pvrvk::ClearValue(0.0f, 0.45f, 0.41f, 1.0f), pvrvk::ClearValue::createDefaultDepthStencilClearValue() };
		pvrvk::ClearValue clearValuesPostProcessing[1] = { pvrvk::ClearValue(0.0f, 0.45f, 0.41f, 1.0f) };
		
		// begin recording commands for the current swap chain command buffer
		_deviceResources->cmdBuffers[i]->begin();

		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->cmdBuffers[i], pvrvk::DebugUtilsLabel("OffscreenPass"));
		_deviceResources->cmdBuffers[i]->beginRenderPass(_deviceResources->offscreenFramebuffers[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValuesOffscreen, ARRAY_SIZE(clearValuesOffscreen));
		_deviceResources->cmdBuffers[i]->bindPipeline(_deviceResources->offscreenPipe);
		_deviceResources->cmdBuffers[i]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelayout, 0, _deviceResources->texDescSet);
		_deviceResources->cmdBuffers[i]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelayout, 1, _deviceResources->uboDescSets[i], &dynamicOffset, 1);
		drawMesh(_deviceResources->cmdBuffers[i], 0);
		_deviceResources->cmdBuffers[i]->endRenderPass();
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBuffers[i]);

		
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->cmdBuffers[i], pvrvk::DebugUtilsLabel("LUTPostProcessPass"));
		_deviceResources->cmdBuffers[i]->beginRenderPass(
			_deviceResources->postProcessFramebuffers[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValuesPostProcessing, ARRAY_SIZE(clearValuesPostProcessing));
		_deviceResources->cmdBuffers[i]->bindPipeline(_deviceResources->postPipeline);
		_deviceResources->cmdBuffers[i]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->postPipelineLayout, 1, _deviceResources->uboDescSets[i], &dynamicOffset, 1);
		_deviceResources->cmdBuffers[i]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->postPipelineLayout, 0, _deviceResources->postDescSets[i]);
		_deviceResources->cmdBuffers[i]->draw(0, 6);

		// UI renderpass
		_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBuffers[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->lutNameText->render();
		_deviceResources->uiRenderer.endRendering();

		_deviceResources->cmdBuffers[i]->endRenderPass();

		pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBuffers[i]);

		_deviceResources->cmdBuffers[i]->end();
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanLUTColorCorrection>(); }
