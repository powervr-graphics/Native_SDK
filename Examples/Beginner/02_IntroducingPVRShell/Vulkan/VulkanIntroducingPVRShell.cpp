/*
Copyright (c) 2015 Imagination Technologies Ltd.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and/or associated documentation files (the
"Materials"), to deal in the Materials without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Materials, and to
permit persons to whom the Materials are furnished to do so, subject to
the following conditions:

The above copyright notice(s) and this permission notice shall be included
in all copies or substantial portions of the Materials.


The Materials are Confidential Information as defined by the
Khronos Membership Agreement until designated non-confidential by Khronos,
at which point this condition clause shall be removed.


THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#if defined(ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif
#define VK_PROTOTYPES
#include <algorithm>
#include <cstdlib>
#include "PVRShell/PVRShell.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/Vulkan/PlatformHandlesVulkanGlue.h"
#include "PVRNativeApi/Vulkan/BufferUtilsVk.h"
void vulkanSuccessOnDie(VkResult result, const char* msg)
{
	if (result != VK_SUCCESS)
	{
		pvr::Log("%s Vulkan Raised an error", msg);
		exit(0);
	}
}
const char* VertShaderName = "VertShader_vk.spv";
const char* FragShaderName = "FragShader_vk.spv";
class App;
typedef std::vector<VkFramebuffer> MultiFbo;
using namespace pvr;

struct GraphicsPipelineCreate
{
	enum ShaderStage
	{
		Vertex, Fragment
	};
	VkShaderModule fs;
	VkShaderModule vs;
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
		ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		ds.front.compareMask = 0xff;
		ds.front.compareOp = VK_COMPARE_OP_ALWAYS;
		ds.front.depthFailOp = ds.front.passOp = ds.front.failOp = VK_STENCIL_OP_KEEP;
		ds.back = ds.front;
		return *this;
	}
};

class App : public pvr::Shell
{
	struct Vertex
	{
		float x, y, z, w;
	};

	VkRenderPass		renderPass;
	VkCommandBuffer		cmdBuffer[8];
	MultiFbo			framebuffer;
	VkPipelineLayout	emptyPipelayout;
	VkPipeline			opaquePipeline;

	native::HBuffer_	vertexBuffer;

	pvr::IPlatformContext* platformContext;
	VkCommandPool cmdPool;

	pvr::Result initApplication() {	return pvr::Result::Success;	}

	MultiFbo createOnScreenFbo(VkRenderPass& renderPass);

	void initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state);

	VkRenderPass createOnScreenRenderPass(VkAttachmentLoadOp colorLoad = VK_ATTACHMENT_LOAD_OP_CLEAR,
	                                      VkAttachmentStoreOp colorStore = VK_ATTACHMENT_STORE_OP_STORE,
	                                      VkAttachmentLoadOp dsLoad = VK_ATTACHMENT_LOAD_OP_CLEAR,
	                                      VkAttachmentStoreOp dsStore = VK_ATTACHMENT_STORE_OP_DONT_CARE);

	VkDevice& getDevice() { return platformContext->getNativePlatformHandles().context.device; }

	pvr::Result initView();

	pvr::Result releaseView();

	pvr::Result quitApplication() {	return pvr::Result::Success;	}

	pvr::Result renderFrame();

	bool loadShader(pvr::Stream::ptr_type stream, VkShaderModule& outShader);

	void recordCommandBuffer();

	void createPipeline();

	void writeVertexBuffer();

	void setupVertexAttribs(VkVertexInputBindingDescription* bindings, VkVertexInputAttributeDescription* attributes,
	                        VkPipelineVertexInputStateCreateInfo& createInfo);

	bool createBuffer(pvr::uint32 size, types::BufferBindingUse usage, native::HBuffer_& outBuffer);
};

pvr::Result App::initView()
{
	platformContext = &getPlatformContext();
	vk::initVk(platformContext->getNativePlatformHandles().context.instance,
	           platformContext->getNativePlatformHandles().context.device);
	{
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.pNext = NULL;
		cmdPoolCreateInfo.queueFamilyIndex = platformContext->getNativePlatformHandles().universalQueueIndex;
		vk::CreateCommandPool(platformContext->getNativePlatformHandles().context.device, &cmdPoolCreateInfo, NULL, &cmdPool);
	}
	// create the renderpass and framebuffer
	renderPass = createOnScreenRenderPass();
	framebuffer = createOnScreenFbo(renderPass);

	createPipeline();
	createBuffer(4096, types::BufferBindingUse::VertexBuffer, vertexBuffer);
	writeVertexBuffer();
	recordCommandBuffer();
	return pvr::Result::Success;
}

pvr::Result App::releaseView()
{
	auto& handles = platformContext->getNativePlatformHandles();
	vk::QueueWaitIdle(handles.mainQueue());
	for (uint32 i = 0; i < getSwapChainLength(); ++i)
	{
		vk::DestroyFramebuffer(getDevice(), framebuffer[i], NULL);
	}
	vk::DestroyRenderPass(getDevice(), renderPass, NULL);
	vk::DestroyPipelineLayout(getDevice(), emptyPipelayout, NULL);
	vk::DestroyPipeline(getDevice(), opaquePipeline, NULL);
	vk::DestroyBuffer(getDevice(), vertexBuffer.buffer, NULL);
	vk::FreeMemory(getDevice(), vertexBuffer.memory, NULL);
	vk::FreeCommandBuffers(getDevice(), cmdPool, platformContext->getSwapChainLength(), cmdBuffer);
	vk::DestroyCommandPool(getDevice(), cmdPool, NULL);
	return pvr::Result::Success;
}

inline static void submit_command_buffers(
  VkQueue queue, VkCommandBuffer* cmdBuffs,
    pvr::uint32 numCmdBuffs = 1, VkSemaphore* waitSems = NULL, pvr::uint32 numWaitSems = 0,
    VkSemaphore* signalSems = NULL, pvr::uint32 numSignalSems = 0, VkFence fence = VK_NULL_HANDLE)
{
	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkSubmitInfo nfo={};
	nfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	nfo.waitSemaphoreCount = numWaitSems;
	nfo.pWaitSemaphores = waitSems;
	nfo.pWaitDstStageMask = &pipeStageFlags;
	nfo.pCommandBuffers = cmdBuffs;
	nfo.commandBufferCount = numCmdBuffs;
	nfo.pSignalSemaphores = signalSems;
	nfo.signalSemaphoreCount = numSignalSems;
	vulkanSuccessOnDie(vk::QueueSubmit(queue, 1, &nfo, fence), "CommandBufferBase::submitCommandBuffers failed");
}

pvr::Result App::renderFrame()
{
	auto& handles = platformContext->getNativePlatformHandles();
	pvr::uint32 swapchainindex = getPlatformContext().getSwapChainIndex();
	submit_command_buffers(handles.mainQueue(), &cmdBuffer[swapchainindex], 1,
	                       &handles.semaphoreCanBeginRendering[swapchainindex], handles.semaphoreCanBeginRendering[swapchainindex] != 0,
	                       &handles.semaphoreFinishedRendering[swapchainindex], handles.semaphoreFinishedRendering[swapchainindex] != 0,
	                       handles.fenceRender[swapchainindex]);
	return pvr::Result::Success;
}

bool App::loadShader(pvr::Stream::ptr_type stream, VkShaderModule& outShader)
{
	pvr::assertion(stream.get() != NULL && "Invalid Shader source");
	VkShaderModuleCreateInfo shaderInfo = {};
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	std::vector<pvr::uint32> readData(stream->getSize());
	size_t read;
	stream->read(stream->getSize(), 1, readData.data(), read);
	shaderInfo.codeSize = stream->getSize();
	shaderInfo.pCode = readData.data();
	vulkanSuccessOnDie(vk::CreateShaderModule(getDevice(), &shaderInfo, NULL, &outShader),
	                   "Failed to create the shader");
	return true;
}

void App::recordCommandBuffer()
{
	uint32_t dynamicOffset = 0;
	VkCommandBufferAllocateInfo sAllocateInfo;

	sAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	sAllocateInfo.pNext = NULL;
	sAllocateInfo.commandPool = cmdPool;
	sAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	sAllocateInfo.commandBufferCount = 1;

	VkCommandBufferBeginInfo cmdBufferBeginInfo;
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = NULL;
	cmdBufferBeginInfo.flags = 0;
	cmdBufferBeginInfo.pInheritanceInfo = NULL;

	VkRenderPassBeginInfo renderPassBeginInfo;
	VkClearValue clearVals[2] = { 0 };
	clearVals[0].color.float32[0] = 0.00f;
	clearVals[0].color.float32[1] = 0.70f;
	clearVals[0].color.float32[2] = .67f;
	clearVals[0].color.float32[3] = 1.0f;
	clearVals[1].depthStencil.depth = 1.0f;
	clearVals[1].depthStencil.stencil = 0xFF;
	for (pvr::uint32 i = 0; i < platformContext->getSwapChainLength(); ++i)
	{
		vk::AllocateCommandBuffers(getDevice(), &sAllocateInfo, &cmdBuffer[i]);
		VkCommandBuffer cmd = cmdBuffer[i];
		vk::BeginCommandBuffer(cmd, &cmdBufferBeginInfo);

		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = NULL;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffer[i];
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = { getWidth(), getHeight() };
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = &clearVals[0];
		vk::CmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vk::CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, opaquePipeline);
		VkDeviceSize vertexOffset = 0;
		vk::CmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer.buffer, &vertexOffset);
		vk::CmdDraw(cmd, 3, 1, 0, 0);
		vk::CmdEndRenderPass(cmd);
		vk::EndCommandBuffer(cmd);
	}
}

void App::createPipeline()
{
	//The various CreateInfos needed for a graphics pipeline
	GraphicsPipelineCreate pipeCreate;

	//These arrays is pointed to by the vertexInput create struct:
	VkVertexInputAttributeDescription attributes[16];
	VkVertexInputBindingDescription bindings[16];

	pipeCreate.vi.pVertexAttributeDescriptions = attributes;
	pipeCreate.vi.pVertexBindingDescriptions = bindings;

	//This array is pointed to by the cb create struct
	VkPipelineColorBlendAttachmentState attachments[1];

	pipeCreate.cb.pAttachments = attachments;

	//Set up the pipeline state
	pipeCreate.vkPipeInfo.pNext = NULL;

	//CreateInfos for the SetLayouts and PipelineLayouts
	VkPipelineLayoutCreateInfo sPipelineLayoutCreateInfo;
	sPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	sPipelineLayoutCreateInfo.pNext = NULL;
	sPipelineLayoutCreateInfo.flags = 0;
	sPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	sPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	sPipelineLayoutCreateInfo.setLayoutCount = 0;
	sPipelineLayoutCreateInfo.pSetLayouts = NULL;
	vk::CreatePipelineLayout(getDevice(), &sPipelineLayoutCreateInfo, NULL, &emptyPipelayout);

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
	//These are only required to create the graphics pipeline, so we create and destroy them locally
	VkShaderModule vertexShaderModule; loadShader(getAssetStream(VertShaderName), vertexShaderModule);
	VkShaderModule fragmentShaderModule; loadShader(getAssetStream(FragShaderName), fragmentShaderModule);

	pipeCreate.vkPipeInfo.layout = emptyPipelayout;
	pipeCreate.vkPipeInfo.renderPass = renderPass;
	pipeCreate.vkPipeInfo.subpass = 0;
	pipeCreate.shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipeCreate.shaderStages[0].module = vertexShaderModule;
	pipeCreate.shaderStages[0].pName = "main";
	pipeCreate.shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipeCreate.shaderStages[1].module = fragmentShaderModule;
	pipeCreate.shaderStages[1].pName = "main";
	attachments[0].blendEnable = VK_FALSE;
	vulkanSuccessOnDie(vk::CreateGraphicsPipelines(getDevice(), VK_NULL_HANDLE, 1, &pipeCreate.vkPipeInfo, NULL, &opaquePipeline), "Failed to create the pipeline");
	vk::DestroyShaderModule(getDevice(), vertexShaderModule, NULL);
	vk::DestroyShaderModule(getDevice(), fragmentShaderModule, NULL);
}

void App::writeVertexBuffer()
{
	Vertex* ptr = 0;

	vk::MapMemory(getDevice(), vertexBuffer.memory, 0, 4096, 0, (void**)&ptr);

	// triangle
	ptr->x = -.4; ptr->y = .4; ptr->z = 0; ptr->w = 1;
	ptr++;

	ptr->x = .4; ptr->y = .4; ptr->z = 0; ptr->w = 1;
	ptr++;

	ptr->x = 0; ptr->y = -.4; ptr->z = 0; ptr->w = 1;
	ptr++;
	vk::UnmapMemory(getDevice(), vertexBuffer.memory);
}


void App::setupVertexAttribs(VkVertexInputBindingDescription* bindings, VkVertexInputAttributeDescription* attributes, VkPipelineVertexInputStateCreateInfo& createInfo)
{
	VkFormat sAttributeFormat;

	sAttributeFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	bindings[0].binding = 0;
	bindings[0].stride = sizeof(Vertex);
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	attributes[0].location = 0;
	attributes[0].binding = 0;
	attributes[0].offset = 0 * 4 * 4;
	attributes[0].format = sAttributeFormat;
	createInfo.vertexBindingDescriptionCount = 1;
	createInfo.vertexAttributeDescriptionCount = 1;
}

bool App::createBuffer(pvr::uint32 size, types::BufferBindingUse usage, native::HBuffer_& outBuffer)
{
	return pvr::utils::vulkan::createBufferAndMemory(getDevice(), getPlatformContext().getNativePlatformHandles().deviceMemProperties,
	       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, usage, size, outBuffer, NULL);
}


MultiFbo App::createOnScreenFbo(VkRenderPass& renderPass)
{
	MultiFbo outFbo(platformContext->getSwapChainLength());
	VkFramebufferCreateInfo fboInfo = {};
	fboInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fboInfo.flags = 0;
	fboInfo.width = platformContext->getNativeDisplayHandle().displayExtent.width;
	fboInfo.height = platformContext->getNativeDisplayHandle().displayExtent.height;
	fboInfo.layers = 1;
	fboInfo.renderPass = renderPass;
	fboInfo.attachmentCount = 2;
	for (pvr::uint32 i = 0; i < platformContext->getSwapChainLength(); ++i)
	{
		VkImageView imageViews[] =
		{
			platformContext->getNativeDisplayHandle().onscreenFbo.colorImageViews[i],
			platformContext->getNativeDisplayHandle().onscreenFbo.depthStencilImageView[i]
		};
		fboInfo.pAttachments = imageViews;
		vulkanSuccessOnDie(vk::CreateFramebuffer(getDevice(), &fboInfo, NULL, &outFbo[i]), "Failed to create the fbo");
	}
	return outFbo;
}

VkRenderPass App::createOnScreenRenderPass(VkAttachmentLoadOp colorLoad, VkAttachmentStoreOp colorStore,
    VkAttachmentLoadOp dsLoad, VkAttachmentStoreOp dsStore)
{
	VkRenderPassCreateInfo renderPassInfo = {};
	VkAttachmentDescription attachmentDesc[2] = {0};
	VkSubpassDescription subpass = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachmentDesc;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.subpassCount = 1;

	attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc[0].format = platformContext->getNativeDisplayHandle().onscreenFbo.colorFormat;
	attachmentDesc[0].loadOp = colorLoad;
	attachmentDesc[0].storeOp = colorStore;
	attachmentDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	attachmentDesc[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc[1].format = platformContext->getNativeDisplayHandle().onscreenFbo.depthStencilFormat;
	attachmentDesc[1].loadOp = dsLoad;
	attachmentDesc[1].storeOp = dsStore;
	attachmentDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc[1].stencilStoreOp =  VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkAttachmentReference attachmentRef[2] =
	{
		{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
		{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
	};

	// setup subpass descriptio
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = attachmentRef;
	subpass.pDepthStencilAttachment = &attachmentRef[1];
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	VkRenderPass outRenderpass;
	vulkanSuccessOnDie(vk::CreateRenderPass(platformContext->getNativePlatformHandles().context.device, &renderPassInfo, NULL, &outRenderpass), "Failed to create renderpass");
	return outRenderpass;
}



void App::initColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& state)
{
	state.blendEnable = VK_TRUE;
	state.colorWriteMask = 0xf;

	state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	state.colorBlendOp = VK_BLEND_OP_ADD;

	state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	state.alphaBlendOp = VK_BLEND_OP_ADD;
}

pvr::GraphicsContextStrongReference pvr::createGraphicsContext()
{
	return pvr::GraphicsContextStrongReference();
}
std::auto_ptr<pvr::Shell> pvr::newDemo()
{
	return std::auto_ptr<pvr::Shell>(new App);
}
