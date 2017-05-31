/*!*********************************************************************************************************************
\File         VulkanIntroducingPVRAssets.cpp
\Title        Introducing the PowerVR Framework
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        Shows how to load POD files and play the animation with basic  lighting
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRNativeApi/NativeVk.h"

/*!*********************************************************************************************************************
Content file names
***********************************************************************************************************************/
const char VertShaderFileName[] = "VertShader_vk.spv";
const char FragShaderFileName[] = "FragShader_vk.spv";
const char SceneFileName[] = "GnomeToy.pod"; // POD scene files


void vkSuccessOrExit(VkResult result, const char* msg)
{
	if (result != VK_SUCCESS)
	{
		pvr::Log("%s Vulkan Raised an error", msg);
		exit(0);
	}
}

struct UboPerMeshData
{
	glm::mat4 mvpMtx;
	glm::mat4 worldViewIT;
	// pad to 256 bits
	char padding[128];
};

struct UboStaticData
{
	glm::vec4 lightDir;
};

struct MaterialDescSet
{
	pvr::native::HDescriptorSet_ descriptor;
	pvr::native::HTexture_ texture;
	pvr::native::HImageView_ view;
};
struct BufferDescriptor
{
	pvr::native::HBuffer_ buffer;
	pvr::native::HDescriptorSet_ descriptor;
	pvr::uint32 numBuffers;
};

struct GraphicsPipelineCreate
{
	enum ShaderStage
	{
		Vertex, Fragment
	};
	VkGraphicsPipelineCreateInfo vkPipeInfo;
	VkPipelineShaderStageCreateInfo shaderStages[2];
	VkPipelineColorBlendStateCreateInfo cb;
	VkPipelineInputAssemblyStateCreateInfo ia;
	VkPipelineDepthStencilStateCreateInfo ds;
	VkPipelineVertexInputStateCreateInfo vi;
	VkPipelineViewportStateCreateInfo vp;
	VkPipelineMultisampleStateCreateInfo ms;
	VkPipelineRasterizationStateCreateInfo rs;

	void reset()
	{
		memset(&vkPipeInfo, 0, sizeof(vkPipeInfo));
		memset(&shaderStages, 0, sizeof(shaderStages));
		memset(&cb, 0, sizeof(cb));
		memset(&ia, 0, sizeof(ia));
		memset(&ds, 0, sizeof(ds));
		memset(&vi, 0, sizeof(vi));
		memset(&vp, 0, sizeof(vp));
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

		shaderStages[ShaderStage::Vertex].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[ShaderStage::Vertex].stage = VK_SHADER_STAGE_VERTEX_BIT;

		shaderStages[ShaderStage::Fragment].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[ShaderStage::Fragment].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		ia.primitiveRestartEnable = VK_FALSE;

		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi.pNext = NULL;
		vi.flags = 0;
		vi.vertexBindingDescriptionCount = 0;
		vi.vertexAttributeDescriptionCount = 0;

		cb.attachmentCount = 1;
		cb.pNext = NULL;
		cb.flags = 0;
		cb.logicOp = VK_LOGIC_OP_COPY;
		cb.logicOpEnable = VK_FALSE;

		vkPipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		vkPipeInfo.pColorBlendState = &cb;
		vkPipeInfo.pDepthStencilState = &ds;
		vkPipeInfo.pInputAssemblyState = &ia;
		vkPipeInfo.pMultisampleState = &ms;
		vkPipeInfo.pRasterizationState = &rs;
		vkPipeInfo.pTessellationState = NULL;
		vkPipeInfo.pVertexInputState = &vi;
		vkPipeInfo.pViewportState = &vp;
		vkPipeInfo.pDynamicState = NULL;
		vkPipeInfo.pStages = shaderStages;
		vkPipeInfo.stageCount = 2;
		resetDepthStencil().resetRasterizer().resetMultisample();

	}
	GraphicsPipelineCreate() { reset(); }

	GraphicsPipelineCreate& resetRasterizer()
	{
		memset(&rs, 0, sizeof(rs));
		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.cullMode = VK_CULL_MODE_BACK_BIT;
		rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rs.polygonMode = VK_POLYGON_MODE_FILL;
		rs.lineWidth = 1.0;
		return *this;
	}

	GraphicsPipelineCreate& resetMultisample()
	{
		memset(&ms, 0, sizeof(ms));
		ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		ms.minSampleShading = 0.0f;
		return *this;
	}

	GraphicsPipelineCreate& resetDepthStencil()
	{
		memset(&ds, 0, sizeof(ds));
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.depthTestEnable = VK_TRUE;
		ds.depthWriteEnable = VK_TRUE;
		ds.depthCompareOp = VK_COMPARE_OP_LESS;
		ds.minDepthBounds = 0.0f;
		ds.maxDepthBounds = 1.0f;

		ds.front.writeMask = 0xff;
		ds.front.compareMask = 0xff;
		ds.front.compareOp = VK_COMPARE_OP_LESS;
		ds.front.depthFailOp = ds.front.passOp = ds.front.failOp = VK_STENCIL_OP_KEEP;
		ds.back = ds.front;
		return *this;
	}
};
typedef std::vector<pvr::native::HFbo_> MultiFbo;
/*!********************************************************************************************************************
Class implementing the pvr::Shell functions.
***********************************************************************************************************************/
class VulkanIntroducingPVRAssets : public pvr::Shell
{
	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and Model View matrices
	glm::mat4 _projMtx;
	glm::mat4 _viewMtx;

	// Variables to handle the animation in a time-based manner
	float _frame;

	// The Vertex buffer object handle array.
	std::vector<pvr::native::HBuffer_> _vbos;
	std::vector<pvr::native::HBuffer_> _ibos;
	MultiFbo _fboOnScreen;
	pvr::native::HRenderPass_ _renderPass;
	std::vector<pvr::native::HCommandBuffer_> _commandBuffers;
	std::vector<MaterialDescSet> _diffuseTextures;
	std::vector<BufferDescriptor> _dynamicUboDescriptors;
	BufferDescriptor _staticLightUboDescriptor;
	pvr::api::Sampler _samplerTrilinear;
	pvr::native::HDescriptorSetLayout_ _texLayout;
	pvr::native::HDescriptorSetLayout_ _uboLayoutDynamic;
	pvr::native::HDescriptorSetLayout_ _uboLayoutStatic;
	pvr::uint32 _perMeshUboSizePerItem;
	pvr::native::HPipelineLayout_ _pipelineLayout;
	pvr::native::HPipeline_ _pipeline;
	pvr::native::HSampler_ _sampler;
	pvr::native::HDescriptorPool_ _descriptorPool;

	struct DrawPass
	{
		std::vector<glm::mat4> worldViewProj;
		std::vector<glm::mat4> worldViewIT;
		std::vector<glm::vec3> dirLight;
		glm::mat4 scale;
	};
	DrawPass _drawPass;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void writeVertexIndexBuffer();
	bool initDescriptors();
	bool loadShader(pvr::Stream::ptr_type stream, VkShaderModule& outShader);
	void recordCommandBuffer();
	void createPipeline();
	void createPipelineLayout();
	void createDescriptorSetLayouts();
	MultiFbo createOnScreenFbo(pvr::native::HRenderPass_& _renderPass);
	void setupVertexAttribs(VkVertexInputBindingDescription* bindings, VkVertexInputAttributeDescription* attributes, VkPipelineVertexInputStateCreateInfo& createInfo);
	void initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state);
	pvr::native::HRenderPass_ createOnScreenRenderPass(VkAttachmentLoadOp colorLoad = VK_ATTACHMENT_LOAD_OP_CLEAR,
	    VkAttachmentStoreOp colorStore = VK_ATTACHMENT_STORE_OP_STORE,
	    VkAttachmentLoadOp dsLoad = VK_ATTACHMENT_LOAD_OP_CLEAR,
	    VkAttachmentStoreOp dsStore = VK_ATTACHMENT_STORE_OP_DONT_CARE);
	void createUboDescriptor(pvr::native::HBuffer_& buffers, pvr::uint32 range, pvr::native::HDescriptorSetLayout_& descSetLayout,
	                         bool isDynamic, pvr::native::HDescriptorSet_& outDescSet);
	void createCombinedImageSamplerDescriptor(pvr::native::HImageView_& images, pvr::native::HSampler_ _sampler,
	    pvr::native::HDescriptorSetLayout_& descSetLayout, pvr::native::HDescriptorSet_& outDescSet);
	VkDevice& getDevice() { return getPlatformContext().getNativePlatformHandles().context.device; }
	void updateBuffer(pvr::native::HBuffer_& buffer, pvr::uint32 offset, pvr::uint32 size, void* data);
	pvr::Result loadTexturePVR(const pvr::StringHash& filename, pvr::native::HTexture_& outTex, pvr::native::HImageView_& outImageView);
	void createDescriptorLayout(VkShaderStageFlagBits stages, VkDescriptorType type, pvr::native::HDescriptorSetLayout_& outLayout);
};

struct DescripotSetComp
{
	pvr::int32 id;
	DescripotSetComp(pvr::int32 id) : id(id) {}
	bool operator()(std::pair<pvr::int32, pvr::api::DescriptorSet> const& pair) { return pair.first == id; }
};

void VulkanIntroducingPVRAssets::initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state)
{
	state.blendEnable = VK_FALSE;
	state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	state.colorBlendOp = VK_BLEND_OP_ADD;

	state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	state.alphaBlendOp = VK_BLEND_OP_ADD;
}

void VulkanIntroducingPVRAssets::setupVertexAttribs(VkVertexInputBindingDescription* bindings,
    VkVertexInputAttributeDescription* attributes, VkPipelineVertexInputStateCreateInfo& createInfo)
{
	const auto& attribs = _scene->getMesh(0).getVertexAttributes();
	bindings[0].binding = 0;
	bindings[0].stride = _scene->getMesh(0).getStride(0);
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_BEGIN_RANGE;
	createInfo.vertexAttributeDescriptionCount = 0;
	createInfo.vertexBindingDescriptionCount = 1;
	for (pvr::uint32 i = 0; i < attribs.size(); ++i)
	{
		attributes[i].location = i;
		attributes[i].offset = attribs[i].getOffset();
		attributes[i].format = pvr::nativeVk::ConvertToVk::dataFormat(attribs[i].getVertexLayout().dataType, attribs[i].getN());
		attributes[i].binding = 0;
		++createInfo.vertexAttributeDescriptionCount;
	}
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRAssets::initApplication()
{
	// Load the scene
	if ((_scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(getAssetStream(SceneFileName)))).isNull())
	{
		this->setExitMessage("ERROR: Couldn't load the %s file\n", SceneFileName);
		return pvr::Result::UnknownError;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (_scene->getNumCameras() == 0)
	{
		this->setExitMessage("ERROR: The _scene does not contain a camera\n");
		return pvr::Result::InvalidData;
	}

	// Ensure that all meshes use an indexed triangle list
	for (pvr::uint32 i = 0; i < _scene->getNumMeshes(); ++i)
	{
		if (_scene->getMesh(i).getPrimitiveType() != pvr::types::PrimitiveTopology::TriangleList ||
		    _scene->getMesh(i).getFaces().getDataSize() == 0)
		{
			this->setExitMessage("ERROR: The meshes in the _scene should use an indexed triangle list\n");
			return pvr::Result::InvalidData;
		}
	}

	// Initialize variables used for the animation
	_frame = 0;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRAssets::quitApplication() { return pvr::Result::Success; }

pvr::native::HRenderPass_ VulkanIntroducingPVRAssets::createOnScreenRenderPass(VkAttachmentLoadOp colorLoad, VkAttachmentStoreOp colorStore,
    VkAttachmentLoadOp dsLoad, VkAttachmentStoreOp dsStore)
{
	// create the renderpass used for rendering to the screen
	VkRenderPassCreateInfo renderPassInfo = {};
	VkAttachmentDescription attachmentDesc[2] = { 0 };
	VkSubpassDescription subpass = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachmentDesc;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.subpassCount = 1;

	// color attachment
	attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc[0].format = getPlatformContext().getNativeDisplayHandle().onscreenFbo.colorFormat;
	attachmentDesc[0].loadOp = colorLoad;
	attachmentDesc[0].storeOp = colorStore;
	attachmentDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// depth stencil attachment
	attachmentDesc[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc[1].format = getPlatformContext().getNativeDisplayHandle().onscreenFbo.depthStencilFormat;
	attachmentDesc[1].loadOp = dsLoad;
	attachmentDesc[1].storeOp = dsStore;
	attachmentDesc[1].stencilLoadOp = dsLoad;
	attachmentDesc[1].stencilStoreOp = dsStore;

	VkAttachmentReference attachmentRef[2] =
	{
		{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
		{ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
	};

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = attachmentRef;
	subpass.pDepthStencilAttachment = &attachmentRef[1];
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	VkRenderPass outRenderpass;
	vkSuccessOrExit(vk::CreateRenderPass(getPlatformContext().getNativePlatformHandles().context.device,
	                                     &renderPassInfo, NULL, &outRenderpass), "Failed to create renderpass");
	return outRenderpass;
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change  in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRAssets::initView()
{
	vk::initVk(getPlatformContext().getNativePlatformHandles().context.instance,
	           getPlatformContext().getNativePlatformHandles().context.device);

	VkPhysicalDeviceProperties props = {};
	vk::GetPhysicalDeviceProperties(getPlatformContext().getNativePlatformHandles().context.physicalDevice, &props);

	//Calculate offset for UBO
	pvr::uint32 minUboDynamicOffset = (pvr::uint32)props.limits.minUniformBufferOffsetAlignment;
	pvr::uint32 structSize = sizeof(UboPerMeshData);

	if (structSize < minUboDynamicOffset)
	{
		_perMeshUboSizePerItem = minUboDynamicOffset;
	}
	else
	{
		_perMeshUboSizePerItem =
		  ((structSize / minUboDynamicOffset) * minUboDynamicOffset +
		   ((structSize % minUboDynamicOffset) == 0 ? 0 : minUboDynamicOffset));
	}

	// create the renderpass
	_renderPass = createOnScreenRenderPass();

	// create the framebuffer
	_fboOnScreen = createOnScreenFbo(_renderPass);

	// create the descriptor pool
	VkDescriptorPoolSize descriptorTypesRequired[3];
	VkDescriptorPoolCreateInfo poolInfo = {};

	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.poolSizeCount = 3;
	descriptorTypesRequired[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorTypesRequired[0].descriptorCount = 10;

	descriptorTypesRequired[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorTypesRequired[1].descriptorCount = 10;

	descriptorTypesRequired[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorTypesRequired[2].descriptorCount = 10;

	poolInfo.pPoolSizes = descriptorTypesRequired;
	poolInfo.maxSets = 10;
	vkSuccessOrExit(vk::CreateDescriptorPool(getDevice(), &poolInfo, NULL,
	                &_descriptorPool.handle), "Failed to create descirptor pool");

	writeVertexIndexBuffer();

	// We check the _scene contains at least one light
	if (_scene->getNumLights() == 0)
	{
		pvr::Log("The _scene does not contain a light\n");
		return pvr::Result::InvalidData;
	}

	// create the descriptor set layouts
	createDescriptorSetLayouts();

	// create the _pipeline layout
	createPipelineLayout();

	// create the graphics _pipeline
	createPipeline();

	// create the descriptor sets
	initDescriptors();

	// record the rendering commands
	recordCommandBuffer();

	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(), (float)this->getHeight() / (float)this->getWidth(),
		                                  _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, _scene->getCamera(0).getFOV(), (float)this->getWidth() / (float)this->getHeight(),
		                                  _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar());
	}


	//update the light direction ubo only once.
	glm::vec3 lightDir3;
	_scene->getLightDirection(0, lightDir3);
	UboStaticData src{ glm::vec4(glm::normalize(lightDir3), 1.0f) };

	// update the static light buffer
	updateBuffer(_staticLightUboDescriptor.buffer, 0, sizeof(src), &src);

	vk::QueueWaitIdle(getPlatformContext().getNativePlatformHandles().mainQueue());

	return pvr::Result::Success;
}

struct ReleaseBuffer
{
	VkDevice device;
	ReleaseBuffer(VkDevice& device) : device(device) {}
	void operator()(pvr::native::HBuffer_ & buffer)
	{
		vk::DestroyBuffer(device, buffer.buffer, NULL);
		vk::FreeMemory(device, buffer.memory, NULL);
		buffer.buffer = VK_NULL_HANDLE;
		buffer.memory = VK_NULL_HANDLE;
	}
};

struct ReleaseFbo
{
	VkDevice device;
	ReleaseFbo(VkDevice& device) : device(device) {}
	void operator()(pvr::native::HFbo_& fbo) { vk::DestroyFramebuffer(device, fbo.handle, NULL); }
};
struct ReleaseCommandBuffer
{
	VkDevice device;
	VkCommandPool pool;
	ReleaseCommandBuffer(VkDevice& device, VkCommandPool& cmdPool) : device(device), pool(cmdPool) {}
	void operator()(pvr::native::HCommandBuffer_& cmd) { vk::FreeCommandBuffers(device, pool, 1, &cmd.handle); }
};

struct ReleaseMaterialDescriptor
{
	VkDevice device;
	pvr::native::HDescriptorPool_ pool;
	ReleaseMaterialDescriptor(VkDevice& device, pvr::native::HDescriptorPool_& pool) : device(device), pool(pool) {}
	void operator()(MaterialDescSet& desc)
	{
		vk::FreeDescriptorSets(device, pool, 1, &desc.descriptor.handle);
		vk::DestroyImage(device, desc.texture.image, NULL);
		vk::FreeMemory(device, desc.texture.memory, NULL);
		vk::DestroyImageView(device, desc.view.handle, NULL);
	}
};

struct ReleaseBufferDescriptor
{
	VkDevice device;
	pvr::native::HDescriptorPool_ pool;
	ReleaseBufferDescriptor(VkDevice& device, pvr::native::HDescriptorPool_& pool) : device(device), pool(pool) {}
	void operator()(BufferDescriptor& buffer)
	{
		vk::FreeDescriptorSets(device, pool, 1, &buffer.descriptor.handle);
		vk::FreeMemory(device, buffer.buffer.memory, NULL);
		vk::DestroyBuffer(device, buffer.buffer.buffer, NULL);
	}
};

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRAssets::releaseView()
{
	auto& handles = getPlatformContext().getNativePlatformHandles();
	vk::QueueWaitIdle(handles.mainQueue());

	// release the buffer resources
	std::for_each(_vbos.begin(), _vbos.end(), ReleaseBuffer(getDevice()));
	std::for_each(_ibos.begin(), _ibos.end(), ReleaseBuffer(getDevice()));

	// release the fbo
	std::for_each(_fboOnScreen.begin(), _fboOnScreen.end(), ReleaseFbo(getDevice()));

	// releaes the command buffers
	std::for_each(_commandBuffers.begin(), _commandBuffers.end(), ReleaseCommandBuffer(getDevice(),
	              getPlatformContext().getNativePlatformHandles().universalCommandPool));

	// release the renderpass
	vk::DestroyRenderPass(getDevice(), _renderPass, NULL);

	// release the textures
	std::for_each(_diffuseTextures.begin(), _diffuseTextures.end(), ReleaseMaterialDescriptor(getDevice(), _descriptorPool));

	// release the ubos
	std::for_each(_dynamicUboDescriptors.begin(), _dynamicUboDescriptors.end(), ReleaseBufferDescriptor(getDevice(), _descriptorPool));
	ReleaseBufferDescriptor(getDevice(), _descriptorPool).operator()(_staticLightUboDescriptor);

	// release _sampler
	vk::DestroySampler(getDevice(), _sampler, NULL);

	// release graphics _pipeline
	vk::DestroyPipeline(getDevice(), _pipeline, NULL);

	// release descriptor set layouts
	vk::DestroyDescriptorSetLayout(getDevice(), _texLayout, NULL);
	vk::DestroyDescriptorSetLayout(getDevice(), _uboLayoutDynamic, NULL);
	vk::DestroyDescriptorSetLayout(getDevice(), _uboLayoutStatic, NULL);

	// release _pipeline layout
	vk::DestroyPipelineLayout(getDevice(), _pipelineLayout, NULL);

	// release the descriptor pool
	vk::DestroyDescriptorPool(getDevice(), _descriptorPool, NULL);
	return pvr::Result::Success;
}

inline static void submit_command_buffers(VkQueue queue, VkDevice device, VkCommandBuffer* cmdBuffs,
    pvr::uint32 numCmdBuffs = 1, VkSemaphore* waitSems = NULL, pvr::uint32 numWaitSems = 0,
    VkSemaphore* signalSems = NULL, pvr::uint32 numSignalSems = 0, VkFence fence = VK_NULL_HANDLE)
{
	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkSubmitInfo nfo = {};
	nfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	nfo.pNext = 0;
	nfo.waitSemaphoreCount = numWaitSems;
	nfo.pWaitSemaphores = waitSems;
	nfo.pWaitDstStageMask = &pipeStageFlags;
	nfo.pCommandBuffers = cmdBuffs;
	nfo.commandBufferCount = numCmdBuffs;
	nfo.pSignalSemaphores = signalSems;
	nfo.signalSemaphoreCount = numSignalSems;
	vkSuccessOrExit(vk::QueueSubmit(queue, 1, &nfo, fence), "CommandBufferBase::submitCommandBuffers failed");
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every _frame.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRAssets::renderFrame()
{
	auto& handles = getPlatformContext().getNativePlatformHandles();

	//  Calculates the _frame number to animate in a time-based manner.
	//  get the time in milliseconds.
	_frame += (float)getFrameTime() / 30.f; // design-time target fps for animation
	if (_frame >= _scene->getNumFrames() - 1) { _frame = 0; }

	// Sets the _scene animation to this _frame
	_scene->setCurrentFrame(_frame);

	//  We can build the world view matrix from the camera position, target and an up vector.
	pvr::float32 fov;
	glm::vec3 cameraPos, cameraTarget, cameraUp;
	_scene->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
	_viewMtx = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	pvr::uint32 swapchainindex = getPlatformContext().getSwapChainIndex();

	// update the model ubo
	// only update the current swapchain ubo
	std::vector<UboPerMeshData> tempMtx(_scene->getNumMeshNodes());
	for (pvr::uint32 i = 0; i < _scene->getNumMeshNodes(); ++i)
	{
		tempMtx[i].mvpMtx = _viewMtx * _scene->getWorldMatrix(i);
		tempMtx[i].worldViewIT = glm::inverseTranspose(tempMtx[i].mvpMtx);
		tempMtx[i].mvpMtx = _projMtx * tempMtx[i].mvpMtx;

		updateBuffer(_dynamicUboDescriptors[swapchainindex].buffer, 0, _perMeshUboSizePerItem * static_cast<pvr::uint32>(tempMtx.size()), tempMtx.data());
	}

	// submit the current swap chain command buffer
	submit_command_buffers(handles.mainQueue(), handles.context.device, &_commandBuffers[swapchainindex].handle, 1,
	                       &handles.semaphoreCanBeginRendering[swapchainindex], handles.semaphoreCanBeginRendering[swapchainindex] != 0,
	                       &handles.semaphoreFinishedRendering[swapchainindex], handles.semaphoreFinishedRendering[swapchainindex] != 0,
	                       handles.fenceRender[swapchainindex]);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Pre-record the rendering commands
***********************************************************************************************************************/
void VulkanIntroducingPVRAssets::recordCommandBuffer()
{
	_commandBuffers.resize(getPlatformContext().getSwapChainLength());

	// allocate the command buffers
	VkCommandBufferAllocateInfo sAllocateInfo = {};
	sAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	sAllocateInfo.pNext = NULL;
	sAllocateInfo.commandPool = getPlatformContext().getNativePlatformHandles().universalCommandPool;
	sAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	sAllocateInfo.commandBufferCount = 1;

	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		vkSuccessOrExit(vk::AllocateCommandBuffers(getDevice(), &sAllocateInfo, &_commandBuffers[i].handle), "");
	}

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	VkClearValue clearVals[2] = { 0 };
	clearVals[0].color.float32[0] = 0.00f;
	clearVals[0].color.float32[1] = 0.70f;
	clearVals[0].color.float32[2] = .67f;
	clearVals[0].color.float32[3] = 1.0f;
	clearVals[1].depthStencil.depth = 1.0f;
	clearVals[1].depthStencil.stencil = 0;
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		vkSuccessOrExit(vk::BeginCommandBuffer(_commandBuffers[i].handle, &cmdBufferBeginInfo),
		                "Failed to begin commandbuffer");

		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = NULL;
		renderPassBeginInfo.renderPass = _renderPass.handle;
		renderPassBeginInfo.framebuffer = _fboOnScreen[i].handle;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = { getWidth(), getHeight() };
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = &clearVals[0];
		vk::CmdBeginRenderPass(_commandBuffers[i].handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vk::CmdBindPipeline(_commandBuffers[i].handle, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.handle);

		// A _scene is composed of nodes. There are 3 types of nodes:
		// - MeshNodes :
		// references a mesh in the getMesh().
		// These nodes are at the beginning of of the Nodes array.
		// And there are nNumMeshNode number of them.
		// This way the .pod format can instantiate several times the same mesh
		// with different attributes.
		// - lights
		// - cameras
		// To draw a _scene, you must go through all the MeshNodes and draw the referenced meshes.
		pvr::uint32 uboOffset = 0;
		VkDeviceSize vertexBufferOffset = 0;
		vk::CmdBindDescriptorSets(_commandBuffers[i].handle, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout.handle, 2,
		                          1, &_staticLightUboDescriptor.descriptor.handle, 0, 0);
		for (unsigned int j = 0; j < _scene->getNumMeshNodes(); ++j)
		{
			const pvr::assets::Model::Node* pNode = &_scene->getMeshNode(j);
			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &_scene->getMesh(pNode->getObjectId());
			uboOffset = _perMeshUboSizePerItem * j;
			vk::CmdBindDescriptorSets(_commandBuffers[i].handle, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout.handle, 0,
			                          1, &_diffuseTextures[pNode->getMaterialIndex()].descriptor.handle, 0, 0);

			vk::CmdBindDescriptorSets(_commandBuffers[i].handle, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout.handle, 1,
			                          1, &_dynamicUboDescriptors[i].descriptor.handle, 1, &uboOffset);

			vk::CmdBindVertexBuffers(_commandBuffers[i].handle, 0, 1, &_vbos[pNode->getObjectId()].buffer, &vertexBufferOffset);
			vk::CmdBindIndexBuffer(_commandBuffers[i].handle, _ibos[pNode->getObjectId()].buffer, 0,
			                       (pMesh->getFaces().getDataTypeSize() == 16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32));

			//Now that the model-view matrix is set and the materials ready,
			//call another function to actually draw the mesh.
			vk::CmdDrawIndexed(_commandBuffers[i].handle, pMesh->getNumFaces() * 3, 1, 0, 0, 0);
		}
		vk::CmdEndRenderPass(_commandBuffers[i].handle);
		vk::EndCommandBuffer(_commandBuffers[i].handle);
	}
}

bool VulkanIntroducingPVRAssets::loadShader(pvr::Stream::ptr_type stream, VkShaderModule& outShader)
{
	pvr::assertion(stream.get() != NULL && "Invalid Shader source");
	VkShaderModuleCreateInfo shaderInfo = {};
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	std::vector<pvr::uint32> readData(stream->getSize());
	size_t read;
	stream->read(stream->getSize(), 1, readData.data(), read);
	shaderInfo.codeSize = stream->getSize();
	shaderInfo.pCode = readData.data();
	vkSuccessOrExit(vk::CreateShaderModule(getDevice(), &shaderInfo, NULL, &outShader),
	                "Failed to create the shader");
	return true;
}

void VulkanIntroducingPVRAssets::createPipelineLayout()
{
	// Create the _pipeline layout
	VkPipelineLayoutCreateInfo sPipelineLayoutCreateInfo = {};
	sPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	sPipelineLayoutCreateInfo.pNext = NULL;
	sPipelineLayoutCreateInfo.flags = 0;
	sPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	sPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

	VkDescriptorSetLayout descLayouts[] =
	{
		_texLayout.handle,
		_uboLayoutDynamic.handle,
		_uboLayoutStatic
	};
	sPipelineLayoutCreateInfo.setLayoutCount = sizeof(descLayouts) / sizeof(descLayouts[0]);
	sPipelineLayoutCreateInfo.pSetLayouts = descLayouts;
	vk::CreatePipelineLayout(getDevice(), &sPipelineLayoutCreateInfo, NULL, &_pipelineLayout.handle);
}

void VulkanIntroducingPVRAssets::createDescriptorSetLayouts()
{
	// create the texture descriptor layout
	createDescriptorLayout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _texLayout);

	// create the matrices ubo descriptor layout
	createDescriptorLayout(VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _uboLayoutDynamic);

	// create the static light ubo descriptor layout
	createDescriptorLayout(VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _uboLayoutStatic);
}

void VulkanIntroducingPVRAssets::createPipeline()
{
	// load the shader modules
	VkShaderModule vertexShaderModule = {};
	loadShader(getAssetStream(VertShaderFileName), vertexShaderModule);
	VkShaderModule fragmentShaderModule = {};
	loadShader(getAssetStream(FragShaderFileName), fragmentShaderModule);

	//These arrays are then used in the graphics _pipeline creation
	VkVertexInputAttributeDescription attributes[16];
	VkVertexInputBindingDescription bindings[16];
	VkRect2D scissors[1];
	VkViewport viewports[1];
	VkSampleMask sampleMask = 0xffffffff;

	//The various CreateInfos needed for a graphics _pipeline
	GraphicsPipelineCreate pipeCreate;

	pipeCreate.vi.pVertexAttributeDescriptions = attributes;
	pipeCreate.vi.pVertexBindingDescriptions = bindings;

	// setup the vertex attributes and bindings
	setupVertexAttribs(bindings, attributes, pipeCreate.vi);

	// setup the color blend attachment state
	VkPipelineColorBlendAttachmentState attachments[1];
	pipeCreate.cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipeCreate.cb.pNext = 0;
	pipeCreate.cb.flags = 0;
	pipeCreate.cb.logicOpEnable = 0;
	pipeCreate.cb.logicOp = VK_LOGIC_OP_SET;
	pipeCreate.cb.attachmentCount = 1;
	pipeCreate.cb.pAttachments = attachments;
	pipeCreate.cb.blendConstants[0] = 0.0f;
	pipeCreate.cb.blendConstants[1] = 0.0f;
	pipeCreate.cb.blendConstants[2] = 0.0f;
	pipeCreate.cb.blendConstants[3] = 0.0f;

	initColorBlendAttachmentState(attachments[0]);

	//Set up the _pipeline state
	pipeCreate.vkPipeInfo.pNext = NULL;

	pipeCreate.ms.pSampleMask = &sampleMask;

	scissors[0].offset.x = 0;
	scissors[0].offset.y = 0;
	scissors[0].extent = { getWidth(), getHeight() };
	pipeCreate.vp.pScissors = scissors;

	viewports[0].minDepth = 0.0f;
	viewports[0].maxDepth = 1.0f;
	viewports[0].x = 0;
	viewports[0].y = 0;
	viewports[0].width = static_cast<pvr::float32>(getWidth());
	viewports[0].height = static_cast<pvr::float32>(getHeight());

	pipeCreate.vp.pViewports = viewports;
	pipeCreate.vp.viewportCount = 1;
	pipeCreate.vp.scissorCount = 1;

	pipeCreate.vkPipeInfo.layout = _pipelineLayout;
	pipeCreate.vkPipeInfo.renderPass = _renderPass.handle;
	pipeCreate.vkPipeInfo.subpass = 0;
	pipeCreate.shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipeCreate.shaderStages[0].module = vertexShaderModule;
	pipeCreate.shaderStages[0].pName = "main";
	pipeCreate.shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipeCreate.shaderStages[1].module = fragmentShaderModule;
	pipeCreate.shaderStages[1].pName = "main";

	pipeCreate.vkPipeInfo.flags |= VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
	pipeCreate.vkPipeInfo.basePipelineIndex = -1;

	vkSuccessOrExit(vk::CreateGraphicsPipelines(getDevice(), VK_NULL_HANDLE, 1, &pipeCreate.vkPipeInfo, NULL, &_pipeline.handle), "Failed to create the _pipeline");

	// destroy the shader module not required anymore
	vk::DestroyShaderModule(getDevice(), vertexShaderModule, NULL);
	vk::DestroyShaderModule(getDevice(), fragmentShaderModule, NULL);
}

MultiFbo VulkanIntroducingPVRAssets::createOnScreenFbo(pvr::native::HRenderPass_ & renderPass)
{
	MultiFbo outFbo(getPlatformContext().getSwapChainLength());
	VkFramebufferCreateInfo fboInfo = {};
	fboInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fboInfo.flags = 0;
	fboInfo.width = getPlatformContext().getNativeDisplayHandle().displayExtent.width;
	fboInfo.height = getPlatformContext().getNativeDisplayHandle().displayExtent.height;
	fboInfo.layers = 1;
	fboInfo.renderPass = renderPass;
	fboInfo.attachmentCount = 2;
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		VkImageView imageViews[] =
		{
			getPlatformContext().getNativeDisplayHandle().onscreenFbo.colorImageViews[i],
			getPlatformContext().getNativeDisplayHandle().onscreenFbo.depthStencilImageView[i]
		};
		fboInfo.pAttachments = imageViews;
		vkSuccessOrExit(vk::CreateFramebuffer(getDevice(), &fboInfo, NULL, &outFbo[i].handle), "Failed to create the fbo");
	}
	return outFbo;
}

pvr::Result VulkanIntroducingPVRAssets::loadTexturePVR(const pvr::StringHash& filename,
    pvr::native::HTexture_ & outTexHandle, pvr::native::HImageView_ & outImageView)
{
	pvr::Texture tempTexture;
	pvr::Result result;

	// get the texture asset
	pvr::Stream::ptr_type assetStream = this->getAssetStream(filename);
	if (!assetStream.get())
	{
		pvr::Log(pvr::Log.Error, "AssetStore.loadTexture error for filename %s : File not found", filename.c_str());
		return pvr::Result::NotFound;
	}

	// read the texture asset
	result = pvr::assets::textureLoad(assetStream, pvr::TextureFileFormat::PVR, tempTexture);

	if (result != pvr::Result::Success)
	{
		pvr::Log(pvr::Log.Error, "AssetStore.loadTexture error for filename %s : Failed to load texture with code %s.",
		         filename.c_str(), pvr::Log.getResultCodeString(result));
		return result;
	}

	// upload the texture asset
	pvr::utils::vulkan::TextureUploadResults texUploadResults = pvr::utils::vulkan::textureUpload(getPlatformContext(), tempTexture, true);

	if (texUploadResults.getResult() != pvr::Result::Success)
	{
		pvr::Log(pvr::Log.Error, "AssetStore.loadTexture error for filename %s : Failed to load texture with code %s.",
		         filename.c_str(), pvr::Log.getResultCodeString(result));
		return result;
	}

	outTexHandle.image = texUploadResults.getImage().image;
	outTexHandle.memory = texUploadResults.getImage().memory;

	// create the imageView
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = outTexHandle.image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	VkFormat viewFormat = pvr::nativeVk::ConvertToVk::pixelFormat(tempTexture.getPixelFormat(), tempTexture.getColorSpace(), tempTexture.getChannelType());
	if (texUploadResults.isDecompressed())
	{
		viewFormat = pvr::nativeVk::ConvertToVk::pixelFormat(texUploadResults.getPixelFormat(), tempTexture.getColorSpace(), tempTexture.getChannelType());
	}

	viewInfo.format = viewFormat;
	viewInfo.components =
	{
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = tempTexture.getNumberOfMIPLevels();
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = tempTexture.getNumberOfArrayMembers();

	vkSuccessOrExit(vk::CreateImageView(getDevice(), &viewInfo, NULL, &outImageView.handle),
	                "Failed to create the image view");

	return result;
}

/*!*********************************************************************************************************************
\brief  Create combined texture and _sampler descriptor set for the materials in the _scene
\return Return true on success
***********************************************************************************************************************/
bool VulkanIntroducingPVRAssets::initDescriptors()
{
	//  create the _sampler
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = false;
		samplerInfo.compareEnable = false;
		samplerInfo.unnormalizedCoordinates = false;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.maxLod = 100.0f;

		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		vkSuccessOrExit(vk::CreateSampler(getDevice(), &samplerInfo, NULL, &_sampler.handle),
		                "failed to create the _sampler");
	}

	// load the demo materials
	pvr::uint32 numMaterials = _scene->getNumMaterials();
	pvr::uint32 i = 0;
	while (i < _scene->getNumMaterials() && _scene->getMaterial(i).defaultSemantics().getDiffuseTextureIndex() != -1)
	{
		const pvr::assets::Model::Material& material = _scene->getMaterial(i);

		MaterialDescSet matDescSet;
		// Load the diffuse texture map
		if (loadTexturePVR(_scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName(),
		                   matDescSet.texture, matDescSet.view) != pvr::Result::Success)
		{
			pvr::Log("Failed to load texture %s", _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str());
			return false;
		}
		createCombinedImageSamplerDescriptor(matDescSet.view, _sampler, _texLayout, matDescSet.descriptor);
		_diffuseTextures.push_back(matDescSet);
		++i;
	}

	// create the matrices ubos (one per swap chain)
	_dynamicUboDescriptors.resize(getPlatformContext().getSwapChainLength());
	for (pvr::uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		// create the dynamic descriptor
		if (!pvr::utils::vulkan::createBuffer(getPlatformContext(), pvr::types::BufferBindingUse::UniformBuffer,
		                                      _perMeshUboSizePerItem * _scene->getNumMeshNodes(), true,
		                                      _dynamicUboDescriptors[i].buffer))
		{
			return false;
		}
		createUboDescriptor(_dynamicUboDescriptors[i].buffer, _perMeshUboSizePerItem, _uboLayoutDynamic, true, _dynamicUboDescriptors[i].descriptor);
	}

	{
		// create the static ubo
		if (!pvr::utils::vulkan::createBuffer(getPlatformContext(), pvr::types::BufferBindingUse::UniformBuffer, _perMeshUboSizePerItem,
		                                      true, _staticLightUboDescriptor.buffer))
		{
			return false;
		}
		createUboDescriptor(_staticLightUboDescriptor.buffer, _perMeshUboSizePerItem, _uboLayoutStatic, false, _staticLightUboDescriptor.descriptor);
	}

	return true;
}

void VulkanIntroducingPVRAssets::createUboDescriptor(pvr::native::HBuffer_& buffers, pvr::uint32 range,
    pvr::native::HDescriptorSetLayout_ & descSetLayout, bool isDynamic, pvr::native::HDescriptorSet_ & outDescSet)
{
	// create the ubo descriptor set
	VkDescriptorSetAllocateInfo descAllocInfo = {};
	descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descAllocInfo.pNext = NULL;
	descAllocInfo.descriptorSetCount = 1;
	descAllocInfo.pSetLayouts = &descSetLayout.handle;
	descAllocInfo.descriptorPool = _descriptorPool;
	vkSuccessOrExit(vk::AllocateDescriptorSets(getDevice(), &descAllocInfo, &outDescSet.handle), "Failed to allocate descriptor set");

	VkWriteDescriptorSet writeDesc = {};
	VkDescriptorBufferInfo bufferInfo = {};

	writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDesc.descriptorType = (isDynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writeDesc.dstSet = outDescSet.handle;
	writeDesc.dstBinding = 0;
	writeDesc.descriptorCount = 1;
	writeDesc.pBufferInfo = &bufferInfo;

	bufferInfo.buffer = buffers.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = range;

	vk::UpdateDescriptorSets(getDevice(), 1, &writeDesc, 0, 0);
}

void VulkanIntroducingPVRAssets::writeVertexIndexBuffer()
{
	void* ptr = 0;
	_vbos.resize(_scene->getNumMeshes());
	_ibos.resize(_scene->getNumMeshes());

	// Load vertex data of all meshes in the _scene into VBOs
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	// Interleaving data improves the memory access pattern and cache efficiency,
	// thus it can be read faster by the hardware.
	for (pvr::uint32 i = 0; i < _scene->getNumMeshes(); ++i)
	{
		// Load vertex data into buffer object
		const pvr::assets::Mesh& mesh = _scene->getMesh(i);
		size_t size = mesh.getDataSize(0);
		VkMappedMemoryRange range = {};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.offset = 0;
		range.size = size;
		pvr::utils::vulkan::createBuffer(getPlatformContext(), pvr::types::BufferBindingUse::VertexBuffer, static_cast<pvr::uint32>(size), true, _vbos[i]);
		range.memory = _vbos[i].memory;
		vk::MapMemory(getDevice(), _vbos[i].memory, 0, size, 0, (void**)&ptr);
		memcpy(ptr, mesh.getData(0), size);
		vk::UnmapMemory(getDevice(), _vbos[i].memory);

		// Load index data into buffer object if available
		if (mesh.getFaces().getData())
		{
			size = mesh.getFaces().getDataSize();
			pvr::utils::vulkan::createBuffer(getPlatformContext(), pvr::types::BufferBindingUse::IndexBuffer, static_cast<pvr::uint32>(size), true, _ibos[i]);
			vk::MapMemory(getDevice(), _ibos[i].memory, 0, size, 0, (void**)&ptr);
			memcpy(ptr, mesh.getFaces().getData(), size);
			range.memory = _ibos[i].memory;
			range.offset = 0;
			range.size = size;
			vk::UnmapMemory(getDevice(), _ibos[i].memory);
		}
	}
}

void VulkanIntroducingPVRAssets::updateBuffer(pvr::native::HBuffer_ & buffer, pvr::uint32 offset, pvr::uint32 size, void* data)
{
	void* tmpData = NULL;
	vk::MapMemory(getDevice(), buffer.memory, offset, size, 0, (void**)&tmpData);
	memcpy(tmpData, data, size);
	VkMappedMemoryRange memRange = {};
	memRange.offset = offset;
	memRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memRange.pNext = 0;
	memRange.size = size;
	memRange.memory = buffer.memory;
	vk::UnmapMemory(getDevice(), buffer.memory);
}

void VulkanIntroducingPVRAssets::createCombinedImageSamplerDescriptor(pvr::native::HImageView_ & images,
    pvr::native::HSampler_ _sampler, pvr::native::HDescriptorSetLayout_ & descSetLayout,
    pvr::native::HDescriptorSet_ & outDescSet)
{
	// create the image _sampler descriptor set
	VkDescriptorSetAllocateInfo descSetAlloc = {};
	descSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descSetAlloc.descriptorSetCount = 1;
	descSetAlloc.pNext = NULL;
	descSetAlloc.descriptorPool = _descriptorPool;
	descSetAlloc.pSetLayouts = &descSetLayout.handle;
	vkSuccessOrExit(vk::AllocateDescriptorSets(getDevice(), &descSetAlloc, &outDescSet.handle), "Failed to allocate descriptor set");

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.sampler = _sampler;
	imageInfo.imageView = images;

	VkWriteDescriptorSet descSetWrite = {};
	descSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descSetWrite.descriptorCount = 1;
	descSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descSetWrite.dstArrayElement = 0;
	descSetWrite.dstBinding = 0;
	descSetWrite.pImageInfo = &imageInfo;
	descSetWrite.dstSet = outDescSet.handle;
	vk::UpdateDescriptorSets(getDevice(), 1, &descSetWrite, 0, NULL);
}

void VulkanIntroducingPVRAssets::createDescriptorLayout(VkShaderStageFlagBits stages, VkDescriptorType type,
    pvr::native::HDescriptorSetLayout_& outLayout)
{
	VkDescriptorSetLayoutBinding descBindings[1];
	descBindings[0].binding = 0;
	descBindings[0].descriptorCount = 1;
	descBindings[0].pImmutableSamplers = NULL;
	descBindings[0].stageFlags = stages;
	descBindings[0].descriptorType = type;

	VkDescriptorSetLayoutCreateInfo descLayoutInfo;
	descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descLayoutInfo.flags = 0;
	descLayoutInfo.pNext = 0;
	descLayoutInfo.pBindings = descBindings;
	descLayoutInfo.bindingCount = 1;

	vkSuccessOrExit(vk::CreateDescriptorSetLayout(getDevice(), &descLayoutInfo, NULL,
	                &outLayout.handle), "Failed to create descriptorset layout");
}

pvr::GraphicsContextStrongReference pvr::createGraphicsContext()
{
	return pvr::GraphicsContextStrongReference();
}

/*!*********************************************************************************************************************
\brief  This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.
\return Return an auto ptr to the demo supplied by the user
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanIntroducingPVRAssets()); }
