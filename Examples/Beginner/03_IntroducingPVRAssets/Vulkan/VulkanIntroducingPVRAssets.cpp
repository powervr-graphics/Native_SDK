/*!*********************************************************************************************************************
\File         VulkanIntroducingPVRAssets.cpp
\Title        Introducing the PowerVR Framework
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief        Shows how to load POD files and play the animation with basic  lighting
***********************************************************************************************************************/
#include "PVRShell/PVRShell.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRPlatformGlue/Vulkan/PlatformHandlesVulkanGlue.h"
#include "PVRNativeApi/TextureUtils.h"

#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"

#include "PVRNativeApi/BufferUtils.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
using namespace pvr;

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
	pvr::native::HTexture_  texture;
	pvr::native::HImageView_ view;
};
struct BufferDescriptor
{
	native::HBuffer_ buffer;
	native::HDescriptorSet_ descriptor;
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
	pvr::assets::ModelHandle scene;

	// Projection and Model View matrices
	glm::mat4 projMtx, viewMtx;

	// Variables to handle the animation in a time-based manner
	float frame;

	// The Vertex buffer object handle array.
	std::vector<native::HBuffer_> vbos;
	std::vector<native::HBuffer_> ibos;
	MultiFbo fboOnScreen;
	native::HRenderPass_ renderPass;
	std::vector<native::HCommandBuffer_> commandBuffer;
	std::vector<MaterialDescSet> diffuseTextures;
	std::vector<BufferDescriptor> uboDescriptorDynamic;
	BufferDescriptor uboDescriptorStatic;
	api::Sampler samplerTrilinear;
	native::HDescriptorSetLayout_ texLayout;
	native::HDescriptorSetLayout_ uboLayoutDynamic, uboLayoutStatic;
	uint32 perMeshUboSizePerItem;
	native::HPipelineLayout_ pipelineLayout;
	native::HPipeline_ pipeline;
	native::HSampler_ sampler;
	native::HDescriptorPool_ descriptorPool;

	struct DrawPass
	{
		std::vector<glm::mat4> worldViewProj;
		std::vector<glm::mat4> worldViewIT;
		std::vector<glm::vec3> dirLight;
		glm::mat4 scale;
	};
	DrawPass drawPass;

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

	MultiFbo createOnScreenFbo(native::HRenderPass_& renderPass);

	void setupVertexAttribs(VkVertexInputBindingDescription* bindings, VkVertexInputAttributeDescription* attributes,
	                        VkPipelineVertexInputStateCreateInfo& createInfo);

	void initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state);

	native::HRenderPass_ createOnScreenRenderPass(VkAttachmentLoadOp colorLoad = VK_ATTACHMENT_LOAD_OP_CLEAR,
	    VkAttachmentStoreOp colorStore = VK_ATTACHMENT_STORE_OP_STORE,
	    VkAttachmentLoadOp dsLoad = VK_ATTACHMENT_LOAD_OP_CLEAR,
	    VkAttachmentStoreOp dsStore = VK_ATTACHMENT_STORE_OP_DONT_CARE);

	void createUbo(native::HBuffer_& buffers, pvr::uint32 range, native::HDescriptorSetLayout_& descSetLayout,
	               bool isDynamic, native::HDescriptorSet_& outDescSet);

	void createCombinedImageSampler(native::HImageView_& images, native::HSampler_ sampler,
	                                native::HDescriptorSetLayout_& descSetLayout, native::HDescriptorSet_& outDescSet);

	void createPipeline();

	VkDevice& getDevice() { return getPlatformContext().getNativePlatformHandles().context.device; }

	void updateBuffer(native::HBuffer_& buffer, pvr::uint32 offset, pvr::uint32 size, void* data);

	pvr::Result loadTexturePVR(const pvr::StringHash& filename, native::HTexture_& outTex,
	                           native::HImageView_& outImageView);

	void createDescriptorLayout(VkShaderStageFlagBits stages, VkDescriptorType type,
	                            native::HDescriptorSetLayout_& outLayout);

};

struct DescripotSetComp
{
	pvr::int32 id;
	DescripotSetComp(pvr::int32 id) : id(id) {}
	bool operator()(std::pair<pvr::int32, pvr::api::DescriptorSet> const& pair)	{ return pair.first == id; }
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
	const auto& attribs = scene->getMesh(0).getVertexAttributes();
	bindings[0].binding = 0;
	bindings[0].stride = scene->getMesh(0).getStride(0);
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_BEGIN_RANGE;
	createInfo.vertexAttributeDescriptionCount = 0;
	createInfo.vertexBindingDescriptionCount = 1;
	for (pvr::uint32 i = 0; i < attribs.size(); ++i)
	{
		attributes[i].location = i;
		attributes[i].offset = attribs[i].getOffset();
		attributes[i].format = pvr::api::ConvertToVk::dataFormat(attribs[i].getVertexLayout().dataType, attribs[i].getN());
		attributes[i].binding = 0;
		++createInfo.vertexAttributeDescriptionCount;
	}
}

/*!*********************************************************************************************************************
\return Result::Success if no error occurred
\brief	Code in initApplication() will be called by Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.). If the rendering
context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRAssets::initApplication()
{
	// Load the scene
	if ((scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(getAssetStream(SceneFileName)))).isNull())
	{
		this->setExitMessage("ERROR: Couldn't load the %s file\n", SceneFileName);
		return pvr::Result::UnknownError;
	}

	// The cameras are stored in the file. We check it contains at least one.
	if (scene->getNumCameras() == 0)
	{
		this->setExitMessage("ERROR: The scene does not contain a camera\n");
		return pvr::Result::InvalidData;
	}

	// Ensure that all meshes use an indexed triangle list
	for (uint32 i = 0; i < scene->getNumMeshes(); ++i)
	{
		if (scene->getMesh(i).getPrimitiveType() != types::PrimitiveTopology::TriangleList ||
		    scene->getMesh(i).getFaces().getDataSize() == 0)
		{
			this->setExitMessage("ERROR: The meshes in the scene should use an indexed triangle list\n");
			return pvr::Result::InvalidData;
		}
	}
	// Initialize variables used for the animation
	frame = 0;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.
If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRAssets::quitApplication() { return pvr::Result::Success; }

native::HRenderPass_ VulkanIntroducingPVRAssets::createOnScreenRenderPass(VkAttachmentLoadOp colorLoad, VkAttachmentStoreOp colorStore,
    VkAttachmentLoadOp dsLoad, VkAttachmentStoreOp dsStore)
{
	VkRenderPassCreateInfo renderPassInfo = {};
	VkAttachmentDescription attachmentDesc[2] = { 0 };
	VkSubpassDescription subpass = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachmentDesc;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.subpassCount = 1;

	attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc[0].format = getPlatformContext().getNativeDisplayHandle().onscreenFbo.colorFormat;
	attachmentDesc[0].loadOp = colorLoad;
	attachmentDesc[0].storeOp = colorStore;
	attachmentDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

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
\return	Result::Success if no error occurred
\brief	Code in initView() will be called by Shell upon initialization or after a change  in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRAssets::initView()
{
	vk::initVk(getPlatformContext().getNativePlatformHandles().context.instance,
	           getPlatformContext().getNativePlatformHandles().context.device);

	VkPhysicalDeviceProperties props = {};
	vk::GetPhysicalDeviceProperties(getPlatformContext().getNativePlatformHandles().context.physicalDevice, &props);

	//Calculate offset for UBO
	uint32 minUboDynamicOffset = (uint32)props.limits.minUniformBufferOffsetAlignment;
	uint32 structSize = sizeof(UboPerMeshData);

	if (structSize < minUboDynamicOffset)
	{
		perMeshUboSizePerItem = minUboDynamicOffset;
	}
	else
	{
		perMeshUboSizePerItem =
		  ((structSize / minUboDynamicOffset) * minUboDynamicOffset +
		   ((structSize % minUboDynamicOffset) == 0 ? 0 : minUboDynamicOffset));
	}

	// create the framebuffer
	renderPass = createOnScreenRenderPass();
	fboOnScreen = createOnScreenFbo(renderPass);

	// create the descriptor pool
	VkDescriptorPoolSize descriptorTypesRequired[3];
	VkDescriptorPoolCreateInfo poolInfo = {};

	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.poolSizeCount = 3;
	descriptorTypesRequired[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorTypesRequired[0].descriptorCount = 50;

	descriptorTypesRequired[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorTypesRequired[1].descriptorCount = 50;

	descriptorTypesRequired[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorTypesRequired[2].descriptorCount = 50;

	poolInfo.pPoolSizes = descriptorTypesRequired;
	poolInfo.maxSets = 100;
	vkSuccessOrExit(vk::CreateDescriptorPool(getDevice(), &poolInfo, NULL,
	                &descriptorPool.handle), "Failed to create descirptor pool");

	writeVertexIndexBuffer();

	// We check the scene contains at least one light
	if (scene->getNumLights() == 0)
	{
		pvr::Log("The scene does not contain a light\n");
		return pvr::Result::InvalidData;
	}

	createPipeline();
	initDescriptors();
	recordCommandBuffer();

	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated() && this->isFullScreen();
	if (isRotated)
	{
		projMtx = math::perspective(pvr::Api::Vulkan, scene->getCamera(0).getFOV(), (float)this->getHeight() / (float)this->getWidth(),
		                            scene->getCamera(0).getNear(), scene->getCamera(0).getFar(), glm::pi<pvr::float32>() * .5f);
	}
	else
	{
		projMtx = math::perspective(pvr::Api::Vulkan, scene->getCamera(0).getFOV(), (float)this->getWidth() / (float)this->getHeight(),
		                            scene->getCamera(0).getNear(), scene->getCamera(0).getFar());
	}


	//update the light direction ubo only once.
	glm::vec3 lightDir3;
	scene->getLightDirection(0, lightDir3);
	UboStaticData src{ glm::vec4(glm::normalize(lightDir3), 1.0f) };
	updateBuffer(uboDescriptorStatic.buffer, 0, sizeof(src), &src);

	vk::QueueWaitIdle(getPlatformContext().getNativePlatformHandles().graphicsQueue);
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
	void operator()(native::HFbo_& fbo)	{	vk::DestroyFramebuffer(device, fbo.handle, NULL);	}
};
struct ReleaseCommandBuffer
{
	VkDevice device;
	VkCommandPool pool;
	ReleaseCommandBuffer(VkDevice& device, VkCommandPool& cmdPool) : device(device), pool(cmdPool) {}
	void operator()(native::HCommandBuffer_& cmd) {	vk::FreeCommandBuffers(device, pool, 1, &cmd.handle);	}
};

struct ReleaseMaterialDescriptor
{
	VkDevice device;
	pvr::native::HDescriptorPool_ pool;
	ReleaseMaterialDescriptor(VkDevice& device, native::HDescriptorPool_& pool) : device(device), pool(pool) {}
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
	ReleaseBufferDescriptor(VkDevice& device, native::HDescriptorPool_& pool) : device(device), pool(pool) {}
	void operator()(BufferDescriptor& buffer)
	{
		vk::FreeDescriptorSets(device, pool, 1, &buffer.descriptor.handle);
		vk::FreeMemory(device, buffer.buffer.memory, NULL);
		vk::DestroyBuffer(device, buffer.buffer.buffer, NULL);
	}
};

/*!*********************************************************************************************************************
\return	Result::Success if no error occurred
\brief	Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRAssets::releaseView()
{
	auto& handles = getPlatformContext().getNativePlatformHandles();
	vk::QueueWaitIdle(handles.graphicsQueue);

	std::for_each(vbos.begin(), vbos.end(), ReleaseBuffer(getDevice()));
	std::for_each(ibos.begin(), ibos.end(), ReleaseBuffer(getDevice()));
	std::for_each(fboOnScreen.begin(), fboOnScreen.end(), ReleaseFbo(getDevice()));
	std::for_each(commandBuffer.begin(), commandBuffer.end(), ReleaseCommandBuffer(getDevice(),
	              getPlatformContext().getNativePlatformHandles().commandPool));
	vk::DestroyRenderPass(getDevice(), renderPass, NULL);
	std::for_each(diffuseTextures.begin(), diffuseTextures.end(), ReleaseMaterialDescriptor(getDevice(), descriptorPool));
	std::for_each(uboDescriptorDynamic.begin(), uboDescriptorDynamic.end(), ReleaseBufferDescriptor(getDevice(), descriptorPool));
	ReleaseBufferDescriptor(getDevice(), descriptorPool).operator()(uboDescriptorStatic);
	vk::DestroySampler(getDevice(), sampler, NULL);
	vk::DestroyPipeline(getDevice(), pipeline, NULL);
	vk::DestroyDescriptorSetLayout(getDevice(), texLayout, NULL);
	vk::DestroyDescriptorSetLayout(getDevice(), uboLayoutDynamic, NULL);
	vk::DestroyDescriptorSetLayout(getDevice(), uboLayoutStatic, NULL);
	vk::DestroyPipelineLayout(getDevice(), pipelineLayout, NULL);
	vk::DestroyDescriptorPool(getDevice(), descriptorPool, NULL);
	return pvr::Result::Success;
}

inline static void submit_command_buffers(VkQueue queue, VkDevice device, VkCommandBuffer* cmdBuffs,
    uint32 numCmdBuffs = 1, VkSemaphore* waitSems = NULL, uint32 numWaitSems = 0,
    VkSemaphore* signalSems = NULL, uint32 numSignalSems = 0, VkFence fence = VK_NULL_HANDLE)
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
\brief	Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanIntroducingPVRAssets::renderFrame()
{
	auto& handles = getPlatformContext().getNativePlatformHandles();
	//	Calculates the frame number to animate in a time-based manner.
	//	get the time in milliseconds.
	frame += (float)getFrameTime() / 30.f; // design-time target fps for animation

	if (frame >= scene->getNumFrames() - 1)	{ frame = 0; }

	// Sets the scene animation to this frame
	scene->setCurrentFrame(frame);

	//	We can build the world view matrix from the camera position, target and an up vector.
	//	A scene is composed of nodes. There are 3 types of nodes:
	//	- MeshNodes :
	//		references a mesh in the getMesh().
	//		These nodes are at the beginning of of the Nodes array.
	//		And there are nNumMeshNode number of them.
	//		This way the .pod format can instantiate several times the same mesh
	//		with different attributes.
	//	- lights
	//	- cameras
	//	To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.

	pvr::float32 fov;
	glm::vec3 cameraPos, cameraTarget, cameraUp;
	scene->getCameraProperties(0, fov, cameraPos, cameraTarget, cameraUp);
	viewMtx = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	// update the ubo
	// only update the current swapchain ubo
	std::vector<UboPerMeshData> tempMtx(scene->getNumMeshNodes());
	for (pvr::uint32 i = 0; i < scene->getNumMeshNodes(); ++i)
	{
		tempMtx[i].mvpMtx = viewMtx * scene->getWorldMatrix(i);
		tempMtx[i].worldViewIT = glm::inverseTranspose(tempMtx[i].mvpMtx);
		tempMtx[i].mvpMtx = projMtx * tempMtx[i].mvpMtx;
		updateBuffer(uboDescriptorDynamic[getPlatformContext().getSwapChainIndex()].buffer,
		             0, perMeshUboSizePerItem * static_cast<pvr::uint32>(tempMtx.size()), tempMtx.data());
	}

	pvr::uint32 swapchainindex = getPlatformContext().getSwapChainIndex();
	submit_command_buffers(handles.graphicsQueue, handles.context.device, &commandBuffer[swapchainindex].handle, 1,
	                       &handles.semaphoreCanBeginRendering[swapchainindex], handles.semaphoreCanBeginRendering[swapchainindex] != 0,
	                       &handles.semaphoreFinishedRendering[swapchainindex], handles.semaphoreFinishedRendering[swapchainindex] != 0,
	                       handles.fenceRender[swapchainindex]);
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief	Pre-record the rendering commands
***********************************************************************************************************************/
void VulkanIntroducingPVRAssets::recordCommandBuffer()
{
	commandBuffer.resize(getPlatformContext().getSwapChainLength());
	// create the commandbuffer
	VkCommandBufferAllocateInfo sAllocateInfo = {};

	sAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	sAllocateInfo.pNext = NULL;
	sAllocateInfo.commandPool = getPlatformContext().getNativePlatformHandles().commandPool;
	sAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	sAllocateInfo.commandBufferCount = 1;

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
	for (uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		VkCommandBuffer& cmdBuffer = commandBuffer[i].handle;
		vkSuccessOrExit(vk::AllocateCommandBuffers(getDevice(), &sAllocateInfo, &cmdBuffer), "");
		vkSuccessOrExit(vk::BeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo),
		                "Failed to begin commandbuffer");

		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = NULL;
		renderPassBeginInfo.renderPass = renderPass.handle;
		renderPassBeginInfo.framebuffer = fboOnScreen[i].handle;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = { getWidth(), getHeight() };
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = &clearVals[0];
		vk::CmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vk::CmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);

		// A scene is composed of nodes. There are 3 types of nodes:
		// - MeshNodes :
		// references a mesh in the getMesh().
		// These nodes are at the beginning of of the Nodes array.
		// And there are nNumMeshNode number of them.
		// This way the .pod format can instantiate several times the same mesh
		// with different attributes.
		// - lights
		// - cameras
		// To draw a scene, you must go through all the MeshNodes and draw the referenced meshes.
		pvr::uint32 uboOffset = 0;
		VkDeviceSize vertexBufferOffset = 0;
		vk::CmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.handle, 2,
		                          1, &uboDescriptorStatic.descriptor.handle, 0, 0);
		for (unsigned int j = 0; j < scene->getNumMeshNodes(); ++j)
		{
			const pvr::assets::Model::Node* pNode = &scene->getMeshNode(j);
			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &scene->getMesh(pNode->getObjectId());
			uboOffset = perMeshUboSizePerItem * j;
			vk::CmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.handle, 0,
			                          1, &diffuseTextures[pNode->getMaterialIndex()].descriptor.handle, 0, 0);

			vk::CmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.handle, 1,
			                          1, &uboDescriptorDynamic[i].descriptor.handle, 1, &uboOffset);

			vk::CmdBindVertexBuffers(cmdBuffer, 0, 1, &vbos[pNode->getObjectId()].buffer, &vertexBufferOffset);
			vk::CmdBindIndexBuffer(cmdBuffer, ibos[pNode->getObjectId()].buffer, 0,
			                       (pMesh->getFaces().getDataTypeSize() == 16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32));

			//Now that the model-view matrix is set and the materials ready,
			//call another function to actually draw the mesh.
			vk::CmdDrawIndexed(cmdBuffer, pMesh->getNumFaces() * 3, 1, 0, 0, 0);
		}
		vk::CmdEndRenderPass(cmdBuffer);
		vk::EndCommandBuffer(cmdBuffer);
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

void VulkanIntroducingPVRAssets::createPipeline()
{
	VkShaderModule vertexShaderModule = {};
	loadShader(getAssetStream(VertShaderFileName), vertexShaderModule);
	VkShaderModule fragmentShaderModule = {};
	loadShader(getAssetStream(FragShaderFileName), fragmentShaderModule);
	// create the texture descriptor layout
	createDescriptorLayout(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	                       texLayout);

	createDescriptorLayout(VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	                       uboLayoutDynamic);

	createDescriptorLayout(VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	                       uboLayoutStatic);

	//The various CreateInfos needed for a graphics pipeline
	GraphicsPipelineCreate pipeCreate;

	//These arrays is pointed to by the vertexInput create struct:
	VkVertexInputAttributeDescription attributes[16];
	VkVertexInputBindingDescription bindings[16];

	pipeCreate.vi.pVertexAttributeDescriptions = attributes;
	pipeCreate.vi.pVertexBindingDescriptions = bindings;

	//This array is pointed to by the cb create struct
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

	//Set up the pipeline state
	pipeCreate.vkPipeInfo.pNext = NULL;

	//CreateInfos for the SetLayouts and PipelineLayouts
	VkPipelineLayoutCreateInfo sPipelineLayoutCreateInfo = {};
	sPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	sPipelineLayoutCreateInfo.pNext = NULL;
	sPipelineLayoutCreateInfo.flags = 0;
	sPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	sPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	VkDescriptorSetLayout descLayouts[] =
	{
		texLayout.handle,
		uboLayoutDynamic.handle,
		uboLayoutStatic
	};
	sPipelineLayoutCreateInfo.setLayoutCount = sizeof(descLayouts) / sizeof(descLayouts[0]);
	sPipelineLayoutCreateInfo.pSetLayouts = descLayouts;
	vk::CreatePipelineLayout(getDevice(), &sPipelineLayoutCreateInfo, NULL, &pipelineLayout.handle);

	static VkSampleMask sampleMask = 0xffffffff;
	pipeCreate.ms.pSampleMask = &sampleMask;
	initColorBlendAttachmentState(attachments[0]);
	setupVertexAttribs(bindings, attributes, pipeCreate.vi);

	VkRect2D scissors[1];
	VkViewport viewports[1];

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

	pipeCreate.vkPipeInfo.layout = pipelineLayout;
	pipeCreate.vkPipeInfo.renderPass = renderPass.handle;
	pipeCreate.vkPipeInfo.subpass = 0;
	pipeCreate.shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipeCreate.shaderStages[0].module = vertexShaderModule;
	pipeCreate.shaderStages[0].pName = "main";
	pipeCreate.shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipeCreate.shaderStages[1].module = fragmentShaderModule;
	pipeCreate.shaderStages[1].pName = "main";

	pipeCreate.vkPipeInfo.flags |= VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
	pipeCreate.vkPipeInfo.basePipelineIndex = -1;

	vkSuccessOrExit(vk::CreateGraphicsPipelines(getDevice(), VK_NULL_HANDLE, 1, &pipeCreate.vkPipeInfo, NULL, &pipeline.handle), "Failed to create the pipeline");

	// destroy the shader module not required anymore
	vk::DestroyShaderModule(getDevice(), vertexShaderModule, NULL);
	vk::DestroyShaderModule(getDevice(), fragmentShaderModule, NULL);
}

MultiFbo VulkanIntroducingPVRAssets::createOnScreenFbo(native::HRenderPass_ & renderPass)
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
    native::HTexture_ & outTexHandle, native::HImageView_ & outImageView)
{
	pvr::assets::Texture tempTexture;
	pvr::Result result;
	pvr::Stream::ptr_type assetStream = this->getAssetStream(filename);
	pvr::types::ImageAreaSize imageSize;
	if (!assetStream.get())
	{
		pvr::Log(pvr::Log.Error, "AssetStore.loadTexture error for filename %s : File not found", filename.c_str());
		return pvr::Result::NotFound;
	}
	result = pvr::assets::textureLoad(assetStream, pvr::assets::TextureFileFormat::PVR, tempTexture);
	PixelFormat pixelFormat;
	bool isDecompressed;
	if (result == pvr::Result::Success)
	{
		result = pvr::utils::textureUpload(getPlatformContext(), tempTexture, outTexHandle, imageSize, pixelFormat, isDecompressed);
	}
	if (result != pvr::Result::Success)
	{
		pvr::Log(pvr::Log.Error, "AssetStore.loadTexture error for filename %s : Failed to load texture with code %s.",
		         filename.c_str(), pvr::Log.getResultCodeString(result));
		return result;
	}

	// create the imageView
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = outTexHandle.image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	VkFormat viewFormat = pvr::api::ConvertToVk::pixelFormat(tempTexture.getPixelFormat(), tempTexture.getColorSpace(), tempTexture.getChannelType());
	if (isDecompressed)
	{
		viewFormat = pvr::api::ConvertToVk::pixelFormat(pixelFormat, tempTexture.getColorSpace(), tempTexture.getChannelType());
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
\brief	Create combined texture and sampler descriptor set for the materials in the scene
\return	Return true on success
***********************************************************************************************************************/
bool VulkanIntroducingPVRAssets::initDescriptors()
{
	//	create the sampler
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
		vkSuccessOrExit(vk::CreateSampler(getDevice(), &samplerInfo, NULL, &sampler.handle),
		                "failed to create the sampler");
	}

	pvr::uint32 numMaterials = scene->getNumMaterials();
	pvr::uint32 i = 0;
	while (i < scene->getNumMaterials() && scene->getMaterial(i).getDiffuseTextureIndex() != -1)
	{
		const pvr::assets::Model::Material& material = scene->getMaterial(i);

		MaterialDescSet matDescSet;
		// Load the diffuse texture map
		if (loadTexturePVR(scene->getTexture(material.getDiffuseTextureIndex()).getName(),
		                   matDescSet.texture, matDescSet.view) != pvr::Result::Success)
		{
			pvr::Log("Failed to load texture %s", scene->getTexture(material.getDiffuseTextureIndex()).getName().c_str());
			return false;
		}
		createCombinedImageSampler(matDescSet.view, sampler,
		                           texLayout, matDescSet.descriptor);
		diffuseTextures.push_back(matDescSet);
		++i;
	}

	// create the ubo
	uboDescriptorDynamic.resize(getPlatformContext().getSwapChainLength());
	for (uint32 i = 0; i < getPlatformContext().getSwapChainLength(); ++i)
	{
		{
			// create the dynamic descriptor
			if (!pvr::utils::createBuffer(getPlatformContext(), types::BufferBindingUse::UniformBuffer,
			                              perMeshUboSizePerItem * scene->getNumMeshNodes(), true,
			                              uboDescriptorDynamic[i].buffer))
			{
				return false;
			}
			createUbo(uboDescriptorDynamic[i].buffer, perMeshUboSizePerItem, uboLayoutDynamic, true, uboDescriptorDynamic[i].descriptor);
		}
	}

	// create the static ubo
	if (!pvr::utils::createBuffer(getPlatformContext(), types::BufferBindingUse::UniformBuffer, perMeshUboSizePerItem,
	                              true, uboDescriptorStatic.buffer))
	{
		return false;
	}
	createUbo(uboDescriptorStatic.buffer, perMeshUboSizePerItem, uboLayoutStatic, false, uboDescriptorStatic.descriptor);
	return true;
}

void VulkanIntroducingPVRAssets::createUbo(native::HBuffer_& buffers, pvr::uint32 range,
    native::HDescriptorSetLayout_ & descSetLayout,
    bool isDynamic, native::HDescriptorSet_ & outDescSet)
{
	VkDescriptorSetAllocateInfo	descAllocInfo = {};
	descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descAllocInfo.pNext = NULL;
	descAllocInfo.descriptorSetCount = 1;
	descAllocInfo.pSetLayouts = &descSetLayout.handle;
	descAllocInfo.descriptorPool = descriptorPool;
	vkSuccessOrExit(vk::AllocateDescriptorSets(getDevice(),
	                &descAllocInfo, &outDescSet.handle), "Failed to allocate descriptor set");

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
	vbos.resize(scene->getNumMeshes());
	ibos.resize(scene->getNumMeshes());
	// Load vertex data of all meshes in the scene into VBOs
	// The meshes have been exported with the "Interleave Vectors" option,
	// so all data is interleaved in the buffer at pMesh->pInterleaved.
	// Interleaving data improves the memory access pattern and cache efficiency,
	// thus it can be read faster by the hardware.
	for (pvr::uint32 i = 0; i < scene->getNumMeshes(); ++i)
	{
		// Load vertex data into buffer object
		const pvr::assets::Mesh& mesh = scene->getMesh(i);
		size_t size = mesh.getDataSize(0);
		VkMappedMemoryRange range = {};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.offset = 0;
		range.size = size;
		pvr::utils::createBuffer(getPlatformContext(), types::BufferBindingUse::VertexBuffer, static_cast<pvr::uint32>(size), true, vbos[i]);
		range.memory = vbos[i].memory;
		vk::MapMemory(getDevice(), vbos[i].memory, 0, size, 0, (void**)&ptr);
		memcpy(ptr, mesh.getData(0), size);
		vk::FlushMappedMemoryRanges(getDevice(), 1, &range);
		vk::UnmapMemory(getDevice(), vbos[i].memory);

		// Load index data into buffer object if available
		if (mesh.getFaces().getData())
		{
			size = mesh.getFaces().getDataSize();
			pvr::utils::createBuffer(getPlatformContext(), types::BufferBindingUse::IndexBuffer, static_cast<pvr::uint32>(size), true, ibos[i]);
			vk::MapMemory(getDevice(), ibos[i].memory, 0, size, 0, (void**)&ptr);
			memcpy(ptr, mesh.getFaces().getData(), size);
			range.memory = ibos[i].memory;
			range.offset = 0;
			range.size = size;
			vk::FlushMappedMemoryRanges(getDevice(), 1, &range);
			vk::UnmapMemory(getDevice(), ibos[i].memory);
		}
	}
}

void VulkanIntroducingPVRAssets::updateBuffer(native::HBuffer_ & buffer, pvr::uint32 offset, pvr::uint32 size, void* data)
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
	vkSuccessOrExit(vk::FlushMappedMemoryRanges(getDevice(), 1, &memRange), "Failed to flush mapped memory");
	vk::UnmapMemory(getDevice(), buffer.memory);
}

void VulkanIntroducingPVRAssets::createCombinedImageSampler(native::HImageView_ & images,
    native::HSampler_ sampler, native::HDescriptorSetLayout_ & descSetLayout,
    native::HDescriptorSet_ & outDescSet)
{
	VkDescriptorSetAllocateInfo descSetAlloc = {};
	descSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descSetAlloc.descriptorSetCount = 1;
	descSetAlloc.pNext = NULL;
	descSetAlloc.descriptorPool = descriptorPool;
	descSetAlloc.pSetLayouts = &descSetLayout.handle;
	// create the descriptor sets
	vkSuccessOrExit(vk::AllocateDescriptorSets(getDevice(),
	                &descSetAlloc, &outDescSet.handle), "Failed to allocate descriptor set");

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.sampler = sampler;
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
    native::HDescriptorSetLayout_& outLayout)
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
\brief	This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.
\return Return an auto ptr to the demo supplied by the user
***********************************************************************************************************************/
std::auto_ptr<pvr::Shell> pvr::newDemo() { return std::auto_ptr<pvr::Shell>(new VulkanIntroducingPVRAssets()); }
