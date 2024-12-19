/*!*********************************************************************************************************************
\File         VulkanShadows.cpp
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Shows how to generate dynamic shadows in real-time.
***********************************************************************************************************************/
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRPfx/RenderManagerVk.h"
#include "PVRCore/pfx/PFXParser.h"
#include "PVRUtils/Vulkan/HelperVk.h"
#include "PVRCore/cameras/TPSCamera.h"

#define _PI 3.14159

// Configuration
const float g_FOV = 65.0f;
const int g_ShadowMapSize = 256;
const float g_PCFBias = 0.008f;
const uint32_t g_PoissonDiskSampleCount = 8;
const float g_PoissonSamplingRadius = 9.0f;
const float g_VSMBias = 0.0005f;
const float g_EVSM2Bias = 0.03f;
const float g_EVSM4Bias = 0.03f;
const float g_VSMLightBleedReduction = 0.1f;
const float g_EVSM2LightBleedReduction = 0.0001f;
const float g_EVSM4LightBleedReduction = 0.0001f;

const uint32_t g_LocalWorkGroupSize = 8;

// Shaders
const char MeshVertShaderFileName[] = "MeshVertShader.vsh.spv";
const char MeshNoShadowsFragShaderFileName[] = "MeshFragShader.fsh.spv";
const char MeshHardShadowsFragShaderFileName[] = "MeshFragShaderHard.fsh.spv";
const char MeshPCFPoissonDiskShadowsFragShaderFileName[] = "MeshFragShaderPCFPoissonDisk.fsh.spv";
const char MeshPCFOptimised2x2ShadowsFragShaderFileName[] = "MeshFragShaderPCFOptimised2x2.fsh.spv";
const char MeshPCFOptimised3x3ShadowsFragShaderFileName[] = "MeshFragShaderPCFOptimised3x3.fsh.spv";
const char MeshPCFOptimised5x5ShadowsFragShaderFileName[] = "MeshFragShaderPCFOptimised5x5.fsh.spv";
const char MeshPCFOptimised7x7ShadowsFragShaderFileName[] = "MeshFragShaderPCFOptimised7x7.fsh.spv";
const char MeshVSMShadowsFragShaderFileName[] = "MeshFragShaderVSM.fsh.spv";
const char MeshEVSM2ShadowsFragShaderFileName[] = "MeshFragShaderEVSM2.fsh.spv";
const char MeshEVSM4ShadowsFragShaderFileName[] = "MeshFragShaderEVSM4.fsh.spv";
const char ShadowVertShaderFileName[] = "ShadowVertShader.vsh.spv";
const char ShadowFragShaderFileName[] = "ShadowFragShader.fsh.spv";
const char TriangleVertShaderFileName[] = "TriangleVertShader.vsh.spv";
const char LightingFragShaderFileName[] = "LightingFragShader.fsh.spv";
const char AmbientFragShaderFileName[] = "AmbientFragShader.fsh.spv";
const char GaussianBlurHorizontalVSMFragShaderFileName[] = "GaussianBlurHorizontalVSMFragShader.fsh.spv";
const char GaussianBlurHorizontalEVSM2FragShaderFileName[] = "GaussianBlurHorizontalEVSM2FragShader.fsh.spv";
const char GaussianBlurHorizontalEVSM4FragShaderFileName[] = "GaussianBlurHorizontalEVSM4FragShader.fsh.spv";
const char GaussianBlurVerticalFragShaderFileName[] = "GaussianBlurVerticalFragShader.fsh.spv";
const char GaussianBlurVSMCompShaderFileName[] = "GaussianBlurVSMCompShader.csh.spv";
const char GaussianBlurEVSM2CompShaderFileName[] = "GaussianBlurEVSM2CompShader.csh.spv";
const char GaussianBlurEVSM4CompShaderFileName[] = "GaussianBlurEVSM4CompShader.csh.spv";

enum class ShadowType : uint32_t
{
	None,
	ShadowMapHard,
	ShadowMapPCFPoissonDisk,
	ShadowMapPCFOptimised2x2,
	ShadowMapPCFOptimised3x3,
	ShadowMapPCFOptimised5x5,
	ShadowMapPCFOptimised7x7,
	ShadowMapVSM,
	ShadowMapEVSM2,
	ShadowMapEVSM4,
	ShadowMapVSMCompute,
	ShadowMapEVSM2Compute,
	ShadowMapEVSM4Compute,
	Count
};

// Scenes
const char ModelFileName[] = "GnomeToy.pod";

// Constants
const char* ShadowTypeNames[] = { "None", "Hard", "PCF Poisson Disk", "PCF Optimised 2x2", "PCF Optimised 3x3", "PCF Optimised 5x5", "PCF Optimised 7x7", "VSM", "EVSM2", "EVSM4",
	"VSM Compute", "EVSM2 Compute", "EVSM4 Compute" };
const char* SceneTypeNames[] = { "Single Teapot", "256 Teapots" };
const pvr::utils::VertexBindings_Name VertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" } };

struct ShadowMapPass;
struct NoShadowsSample;
struct PCFShadowsSample;
struct VSMShadowsSample;
struct GaussianBlurFragmentPass;
struct GaussianBlurComputePass;

struct UBO
{
	pvr::utils::StructuredBufferView view;
	pvrvk::Buffer buffer;
};

struct Material
{
	pvrvk::ImageView diffuseImageView;
	pvrvk::DescriptorSet materialDescriptorSet;
};

// Put all API managed objects in a struct so that we can one-line free them...
struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;

	std::vector<pvrvk::CommandPool> commandPool;
	pvrvk::Swapchain swapchain;
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::Queue queue[2];
	pvrvk::PipelineCache pipelineCache;
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;
	std::vector<Material> materials;

	// Passes
	std::shared_ptr<ShadowMapPass> shadowMapPass;
	std::shared_ptr<NoShadowsSample> noShadowsSample;
	std::shared_ptr<PCFShadowsSample> hardShadowsSample;
	std::shared_ptr<PCFShadowsSample> pcfPoissonDiskShadowsSample;
	std::shared_ptr<PCFShadowsSample> pcfOptimised2x2ShadowsSample;
	std::shared_ptr<PCFShadowsSample> pcfOptimised3x3ShadowsSample;
	std::shared_ptr<PCFShadowsSample> pcfOptimised5x5ShadowsSample;
	std::shared_ptr<PCFShadowsSample> pcfOptimised7x7ShadowsSample;
	std::shared_ptr<VSMShadowsSample> vsmFragmentShadowsSample;
	std::shared_ptr<VSMShadowsSample> evsm2FragmentShadowsSample;
	std::shared_ptr<VSMShadowsSample> evsm4FragmentShadowsSample;
	std::shared_ptr<VSMShadowsSample> vsmComputeShadowsSample;
	std::shared_ptr<VSMShadowsSample> evsm2ComputeShadowsSample;
	std::shared_ptr<VSMShadowsSample> evsm4ComputeShadowsSample;

	// Fragment Gaussian Blurs
	std::shared_ptr<GaussianBlurFragmentPass> gaussianBlurVSMFragmentPass;
	std::shared_ptr<GaussianBlurFragmentPass> gaussianBlurEVSM2FragmentPass;
	std::shared_ptr<GaussianBlurFragmentPass> gaussianBlurEVSM4FragmentPass;

	// Compute Gaussian Blurs
	std::shared_ptr<GaussianBlurComputePass> gaussianBlurVSMComputePass;
	std::shared_ptr<GaussianBlurComputePass> gaussianBlurEVSM2ComputePass;
	std::shared_ptr<GaussianBlurComputePass> gaussianBlurEVSM4ComputePass;

	pvrvk::Sampler samplerBilinear;
	pvrvk::Sampler samplerTrilinear;
	pvrvk::Sampler samplerNearestShadow;

	pvrvk::DescriptorSetLayout dsLayoutShadowMap;
	pvrvk::DescriptorSetLayout dsLayoutGlobal;
	pvrvk::DescriptorSetLayout dsLayoutMaterial;

	pvrvk::DescriptorSet dsGlobal;

	pvr::utils::vma::Allocator vmaAllocator;

	pvrvk::Surface surface;

	// Asset loader
	std::vector<pvrvk::CommandBuffer> cmdBuffers;

	std::vector<pvrvk::Framebuffer> onScreenFramebuffer;
	std::vector<pvrvk::ImageView> depthStencilImages;

	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;
	std::vector<pvrvk::Semaphore> presentationSemaphores;
	std::vector<pvrvk::Fence> perFrameResourcesFences;

	UBO globalUBO;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;
	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			}
		}
	}
};

struct ShadowMapPass
{
	ShadowMapPass() {}
	~ShadowMapPass() {}

	void init(const pvr::assets::ModelHandle& scene, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		createRenderPasses(deviceResources);
		createImages(deviceResources);
		createImageViews(deviceResources);
		createFramebuffers(deviceResources);
		createShaderModules(deviceResources, shell);
		createPipelineLayouts(deviceResources);
		createPipelines(scene, deviceResources);
	}

	void render(const pvr::assets::ModelHandle& scene, DeviceResources* deviceResources, uint32_t frameIndex, uint32_t queueIndex, uint32_t dynamicOffset)
	{
		// Setup clear color.
		const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(1.f, 0) };

		auto cmdBuffer = deviceResources->cmdBuffers[frameIndex];

		// Start render pass.
		cmdBuffer->beginRenderPass(_fbo[queueIndex], pvrvk::Rect2D(0, 0, g_ShadowMapSize, g_ShadowMapSize), true, clearValues, ARRAY_SIZE(clearValues));

		// Insert a debug label.
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("Shadow Map Pass - Swapchain (%i)", frameIndex)));

		uint32_t offsets[1];
		offsets[0] = dynamicOffset;

		// Bind descriptor set containing global UBO.
		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipelineLayout, 0, deviceResources->dsGlobal, offsets, 1);

		// Render all mesh nodes.
		for (uint32_t i = 0; i < scene->getNumMeshNodes(); i++)
		{
			const pvr::assets::Model::Node* pNode = &scene->getMeshNode(i);
			const uint32_t meshId = pNode->getObjectId();

			cmdBuffer->bindPipeline(_pipelines[meshId]);

			glm::mat4 transform = scene->getWorldMatrix(i);
			cmdBuffer->pushConstants(
				_pipelines[meshId]->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)), &transform);

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &scene->getMesh(meshId);

			// bind the vertex and index buffer
			cmdBuffer->bindVertexBuffer(deviceResources->vbos[meshId], 0, 0);
			cmdBuffer->bindIndexBuffer(
				deviceResources->ibos[meshId], 0, pMesh->getFaces().getDataType() == pvr::IndexType::IndexType16Bit ? pvrvk::IndexType::e_UINT16 : pvrvk::IndexType::e_UINT32);

			cmdBuffer->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
		}

		// End debug label region.
		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);

		cmdBuffer->endRenderPass();
	}

	void createImages(DeviceResources* deviceResources)
	{
		pvrvk::Extent3D texExtents = pvrvk::Extent3D(g_ShadowMapSize, g_ShadowMapSize, 1);

		for (int i = 0; i < 2; i++)
		{
			_image[i] = pvr::utils::createImage(deviceResources->device,
				pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, pvrvk::Format::e_D32_SFLOAT, texExtents,
					pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, deviceResources->vmaAllocator);
		}
	}

	void createImageViews(DeviceResources* deviceResources)
	{
		for (int i = 0; i < 2; i++) _imageView[i] = deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(_image[i]));
	}

	void createFramebuffers(DeviceResources* deviceResources)
	{
		for (int i = 0; i < 2; i++)
			_fbo[i] = deviceResources->device->createFramebuffer(pvrvk::FramebufferCreateInfo(g_ShadowMapSize, g_ShadowMapSize, 1, _renderPass, 1, &_imageView[i]));
	}

	void createRenderPasses(DeviceResources* deviceResources)
	{
		pvrvk::AttachmentDescription depthAttachment = pvrvk::AttachmentDescription::createDepthStencilDescription(pvrvk::Format::e_D32_SFLOAT, pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_DEPTH_STENCIL_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::AttachmentLoadOp::e_DONT_CARE,
			pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::SampleCountFlags::e_1_BIT);

		pvrvk::AttachmentReference depthAttachmentRef = pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		pvrvk::SubpassDescription subpassDesc = pvrvk::SubpassDescription().setDepthStencilAttachmentReference(depthAttachmentRef);

		pvrvk::SubpassDependency dependency[2];

		dependency[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
		dependency[0].setDstSubpass(0);
		dependency[0].setSrcStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
		dependency[0].setDstStageMask(pvrvk::PipelineStageFlags::e_EARLY_FRAGMENT_TESTS_BIT);
		dependency[0].setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
		dependency[0].setDstAccessMask(pvrvk::AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
		dependency[0].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

		dependency[1].setSrcSubpass(0);
		dependency[1].setDstSubpass(VK_SUBPASS_EXTERNAL);
		dependency[1].setSrcStageMask(pvrvk::PipelineStageFlags::e_LATE_FRAGMENT_TESTS_BIT);
		dependency[1].setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
		dependency[1].setSrcAccessMask(pvrvk::AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
		dependency[1].setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
		dependency[1].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

		pvrvk::RenderPassCreateInfo renderPassCreateInfo =
			pvrvk::RenderPassCreateInfo().setAttachmentDescription(0, depthAttachment).setSubpass(0, subpassDesc).addSubpassDependencies(dependency, 2);

		_renderPass = deviceResources->device->createRenderPass(renderPassCreateInfo);
		_renderPass->setObjectName("ShadowMapRenderPass");
	}

	void createShaderModules(DeviceResources* deviceResources, pvr::Shell& shell)
	{
		_vs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(ShadowVertShaderFileName)->readToEnd<uint32_t>()));
		_fs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(ShadowFragShaderFileName)->readToEnd<uint32_t>()));
	}

	void createPipelines(const pvr::assets::ModelHandle& scene, DeviceResources* deviceResources)
	{
		_pipelines.resize(scene->getNumMeshes());

		for (uint32_t i = 0; i < scene->getNumMeshes(); i++)
		{
			pvrvk::GraphicsPipelineCreateInfo renderShadowPipelineCreateInfo;

			renderShadowPipelineCreateInfo.viewport.setViewportAndScissor(
				0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(g_ShadowMapSize), static_cast<float>(g_ShadowMapSize)), pvrvk::Rect2D(0, 0, g_ShadowMapSize, g_ShadowMapSize));

			// enable front face culling
			renderShadowPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);

			// set counter clockwise winding order for front faces
			renderShadowPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
			// renderShadowPipelineCreateInfo.rasterizer.setDepthClip = false;

			// enable depth testing
			renderShadowPipelineCreateInfo.depthStencil.enableDepthTest(true);
			renderShadowPipelineCreateInfo.depthStencil.enableDepthWrite(true);

			// load and create appropriate shaders
			renderShadowPipelineCreateInfo.vertexShader.setShader(_vs);
			renderShadowPipelineCreateInfo.fragmentShader.setShader(_fs);

			// setup vertex inputs
			pvr::utils::populateInputAssemblyFromMesh(
				scene->getMesh(i), VertexBindings, ARRAY_SIZE(VertexBindings), renderShadowPipelineCreateInfo.vertexInput, renderShadowPipelineCreateInfo.inputAssembler);

			// renderpass/subpass
			renderShadowPipelineCreateInfo.renderPass = _renderPass;

			// enable stencil testing
			pvrvk::StencilOpState stencilState;

			// only replace stencil buffer when the depth test passes
			stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
			stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
			stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
			stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

			pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;

			colorAttachmentState.setBlendEnable(false);
			renderShadowPipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

			// set stencil reference to 1
			stencilState.setReference(1);

			// disable stencil writing
			stencilState.setWriteMask(0);

			// enable the stencil tests
			renderShadowPipelineCreateInfo.depthStencil.enableStencilTest(false);
			// set stencil states
			renderShadowPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
			renderShadowPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

			renderShadowPipelineCreateInfo.pipelineLayout = _pipelineLayout;

			_pipelines[i] = deviceResources->device->createGraphicsPipeline(renderShadowPipelineCreateInfo, deviceResources->pipelineCache);
			_pipelines[i]->setObjectName("ShadowMapPass" + std::to_string(i) + "GraphicsPipeline");
			_pipelines[i]->setObjectName("Mesh" + std::to_string(i) + "ShadowMapGraphicsPipeline");
		}
	}

	void createPipelineLayouts(DeviceResources* deviceResources)
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutGlobal);

		pipeLayoutInfo.setPushConstantRange(0, pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4))));

		_pipelineLayout = deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	pvrvk::ShaderModule _vs;
	pvrvk::ShaderModule _fs;
	std::vector<pvrvk::GraphicsPipeline> _pipelines;
	pvrvk::PipelineLayout _pipelineLayout;
	pvrvk::RenderPass _renderPass;
	pvrvk::Image _image[2];
	pvrvk::ImageView _imageView[2];
	pvrvk::Framebuffer _fbo[2];
};

struct NoShadowsSample
{
	NoShadowsSample() {}
	~NoShadowsSample() {}

	void init(const pvr::assets::ModelHandle& scene, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		createShaderModules(shell, deviceResources);
		createPipelineLayouts(deviceResources);
		createPipelines(scene, shell, deviceResources);
	}

	void render(const pvr::assets::ModelHandle& scene, DeviceResources* deviceResources, pvr::Shell& shell, uint32_t frameIndex)
	{
		// Setup clear color.
		const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(0.0f, 0.40f, .39f, 1.0f), pvrvk::ClearValue(1.f, 0) };

		auto cmdBuffer = deviceResources->cmdBuffers[frameIndex];
		auto fbo = deviceResources->onScreenFramebuffer[frameIndex];

		// Start render pass.
		cmdBuffer->beginRenderPass(fbo, pvrvk::Rect2D(0, 0, shell.getWidth(), shell.getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		// Insert a debug label.
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("(No Shadows) Main Scene Render Pass - Swapchain (%i)", frameIndex)));

		uint32_t offsets[1];
		offsets[0] = deviceResources->globalUBO.view.getDynamicSliceOffset(frameIndex);

		// Render all mesh nodes.
		for (uint32_t i = 0; i < scene->getNumMeshNodes(); i++)
		{
			const pvr::assets::Model::Node* pNode = &scene->getMeshNode(i);
			const uint32_t meshId = pNode->getObjectId();

			cmdBuffer->bindPipeline(_pipelines[meshId]);

			pvrvk::DescriptorSet arrayDS[] = { deviceResources->dsGlobal, deviceResources->materials[pNode->getMaterialIndex()].materialDescriptorSet };

			// Bind descriptor set containing global UBO.
			cmdBuffer->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipelineLayout, 0, arrayDS, 2, offsets, 1);

			glm::mat4 transform = scene->getWorldMatrix(i);
			cmdBuffer->pushConstants(
				_pipelines[meshId]->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)), &transform);

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &scene->getMesh(meshId);

			// bind the vertex and index buffer
			cmdBuffer->bindVertexBuffer(deviceResources->vbos[meshId], 0, 0);
			cmdBuffer->bindIndexBuffer(
				deviceResources->ibos[meshId], 0, pMesh->getFaces().getDataType() == pvr::IndexType::IndexType16Bit ? pvrvk::IndexType::e_UINT16 : pvrvk::IndexType::e_UINT32);

			cmdBuffer->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
		}

		// End debug label region.
		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
	}

	void createShaderModules(pvr::Shell& shell, DeviceResources* deviceResources)
	{
		_vs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(MeshVertShaderFileName)->readToEnd<uint32_t>()));
		_fs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(MeshNoShadowsFragShaderFileName)->readToEnd<uint32_t>()));
	}

	void createPipelines(const pvr::assets::ModelHandle& scene, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		_pipelines.resize(scene->getNumMeshes());

		for (uint32_t i = 0; i < scene->getNumMeshes(); i++)
		{
			pvrvk::GraphicsPipelineCreateInfo renderShadowPipelineCreateInfo;

			renderShadowPipelineCreateInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(shell.getWidth()), static_cast<float>(shell.getHeight())), pvrvk::Rect2D(0, 0, shell.getWidth(), shell.getHeight()));

			// enable front face culling
			renderShadowPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

			// set counter clockwise winding order for front faces
			renderShadowPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

			// enable depth testing
			renderShadowPipelineCreateInfo.depthStencil.enableDepthTest(true);
			renderShadowPipelineCreateInfo.depthStencil.enableDepthWrite(true);

			// load and create appropriate shaders
			renderShadowPipelineCreateInfo.vertexShader.setShader(_vs);
			renderShadowPipelineCreateInfo.fragmentShader.setShader(_fs);

			// setup vertex inputs
			pvr::utils::populateInputAssemblyFromMesh(
				scene->getMesh(i), VertexBindings, ARRAY_SIZE(VertexBindings), renderShadowPipelineCreateInfo.vertexInput, renderShadowPipelineCreateInfo.inputAssembler);

			// renderpass/subpass
			renderShadowPipelineCreateInfo.renderPass = deviceResources->onScreenFramebuffer[0]->getRenderPass();

			// enable stencil testing
			pvrvk::StencilOpState stencilState;

			// only replace stencil buffer when the depth test passes
			stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
			stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
			stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
			stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

			pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;

			colorAttachmentState.setBlendEnable(false);
			renderShadowPipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

			// set stencil reference to 1
			stencilState.setReference(1);

			// disable stencil writing
			stencilState.setWriteMask(0);

			// enable the stencil tests
			renderShadowPipelineCreateInfo.depthStencil.enableStencilTest(false);
			// set stencil states
			renderShadowPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
			renderShadowPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

			renderShadowPipelineCreateInfo.pipelineLayout = _pipelineLayout;

			_pipelines[i] = deviceResources->device->createGraphicsPipeline(renderShadowPipelineCreateInfo, deviceResources->pipelineCache);
			_pipelines[i]->setObjectName("Mesh" + std::to_string(i) + "NoShadowsGraphicsPipeline");
		}
	}

	void createPipelineLayouts(DeviceResources* deviceResources)
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutGlobal);
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutMaterial);

		pipeLayoutInfo.setPushConstantRange(0, pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4))));
		pipeLayoutInfo.setPushConstantRange(1,
			pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)),
				static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4))));

		_pipelineLayout = deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	pvrvk::ShaderModule _vs;
	pvrvk::ShaderModule _fs;
	std::vector<pvrvk::GraphicsPipeline> _pipelines;
	pvrvk::PipelineLayout _pipelineLayout;
};

struct PCFShadowsSample
{
	PCFShadowsSample() {}
	~PCFShadowsSample() {}

	void init(const char* fragmentShaderPath, const pvr::assets::ModelHandle& scene, pvr::Shell& shell, DeviceResources* deviceResources, ShadowMapPass* shadowMapPass)
	{
		_shadowMapPass = shadowMapPass;

		createShaderModules(fragmentShaderPath, shell, deviceResources);
		createDescriptorSets(deviceResources);
		createPipelineLayouts(deviceResources);
		createPipelines(scene, shell, deviceResources);
	}

	void render(const pvr::assets::ModelHandle& scene, DeviceResources* deviceResources, pvr::Shell& shell, uint32_t frameIndex, uint32_t queueIndex, glm::vec4 shadowParams)
	{
		auto cmdBuffer = deviceResources->cmdBuffers[frameIndex];
		auto fbo = deviceResources->onScreenFramebuffer[frameIndex];

		// Render shadow map.
		_shadowMapPass->render(scene, deviceResources, frameIndex, queueIndex, deviceResources->globalUBO.view.getDynamicSliceOffset(frameIndex));

		// Setup clear color.
		const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(0.0f, 0.40f, .39f, 1.0f), pvrvk::ClearValue(1.f, 0) };

		// Start render pass.
		cmdBuffer->beginRenderPass(fbo, pvrvk::Rect2D(0, 0, shell.getWidth(), shell.getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		// Insert a debug label.
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("(No Shadows) Main Scene Render Pass - Swapchain (%i)", frameIndex)));

		// Pass push constants containing shadow filtering parameters.
		cmdBuffer->pushConstants(_pipelineLayoutFinalScene, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, sizeof(glm::mat4), sizeof(glm::vec4), &shadowParams);

		// Render all mesh nodes.
		for (uint32_t i = 0; i < scene->getNumMeshNodes(); i++)
		{
			const pvr::assets::Model::Node* pNode = &scene->getMeshNode(i);
			const uint32_t meshId = pNode->getObjectId();

			cmdBuffer->bindPipeline(_pipelines[meshId]);

			uint32_t offsets[1];
			offsets[0] = deviceResources->globalUBO.view.getDynamicSliceOffset(frameIndex);

			pvrvk::DescriptorSet arrayDS[] = { deviceResources->dsGlobal, deviceResources->materials[pNode->getMaterialIndex()].materialDescriptorSet, _dsFinalScene[queueIndex] };

			// Bind descriptor set containing global UBO.
			cmdBuffer->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipelineLayoutFinalScene, 0, arrayDS, 3, offsets, 1);

			glm::mat4 transform = scene->getWorldMatrix(i);
			cmdBuffer->pushConstants(
				_pipelines[meshId]->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)), &transform);

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &scene->getMesh(meshId);

			// bind the vertex and index buffer
			cmdBuffer->bindVertexBuffer(deviceResources->vbos[meshId], 0, 0);
			cmdBuffer->bindIndexBuffer(
				deviceResources->ibos[meshId], 0, pMesh->getFaces().getDataType() == pvr::IndexType::IndexType16Bit ? pvrvk::IndexType::e_UINT16 : pvrvk::IndexType::e_UINT32);

			cmdBuffer->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
		}

		// End debug label region.
		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
	}

	void createShaderModules(const char* fragmentShaderPath, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		_vsFinalScene = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(MeshVertShaderFileName)->readToEnd<uint32_t>()));
		_fsFinalScene = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(fragmentShaderPath)->readToEnd<uint32_t>()));
	}

	void createPipelines(const pvr::assets::ModelHandle& scene, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		_pipelines.resize(scene->getNumMeshes());

		for (uint32_t i = 0; i < scene->getNumMeshes(); i++)
		{
			pvrvk::GraphicsPipelineCreateInfo renderShadowPipelineCreateInfo;

			renderShadowPipelineCreateInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(shell.getWidth()), static_cast<float>(shell.getHeight())), pvrvk::Rect2D(0, 0, shell.getWidth(), shell.getHeight()));

			// enable front face culling
			renderShadowPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

			// set counter clockwise winding order for front faces
			renderShadowPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

			// enable depth testing
			renderShadowPipelineCreateInfo.depthStencil.enableDepthTest(true);
			renderShadowPipelineCreateInfo.depthStencil.enableDepthWrite(true);

			// load and create appropriate shaders
			renderShadowPipelineCreateInfo.vertexShader.setShader(_vsFinalScene);
			renderShadowPipelineCreateInfo.fragmentShader.setShader(_fsFinalScene);

			// setup vertex inputs
			pvr::utils::populateInputAssemblyFromMesh(
				scene->getMesh(i), VertexBindings, ARRAY_SIZE(VertexBindings), renderShadowPipelineCreateInfo.vertexInput, renderShadowPipelineCreateInfo.inputAssembler);

			// renderpass/subpass
			renderShadowPipelineCreateInfo.renderPass = deviceResources->onScreenFramebuffer[0]->getRenderPass();

			// enable stencil testing
			pvrvk::StencilOpState stencilState;

			// only replace stencil buffer when the depth test passes
			stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
			stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
			stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
			stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

			pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;

			colorAttachmentState.setBlendEnable(false);
			renderShadowPipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

			// set stencil reference to 1
			stencilState.setReference(1);

			// disable stencil writing
			stencilState.setWriteMask(0);

			// enable the stencil tests
			renderShadowPipelineCreateInfo.depthStencil.enableStencilTest(false);
			// set stencil states
			renderShadowPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
			renderShadowPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

			renderShadowPipelineCreateInfo.pipelineLayout = _pipelineLayoutFinalScene;

			_pipelines[i] = deviceResources->device->createGraphicsPipeline(renderShadowPipelineCreateInfo, deviceResources->pipelineCache);
			_pipelines[i]->setObjectName("Mesh" + std::to_string(i) + "PCFShadowsGraphicsPipeline");
		}
	}

	void createDescriptorSets(DeviceResources* deviceResources)
	{
		for (int i = 0; i < 2; i++)
		{
			_dsFinalScene[i] = deviceResources->descriptorPool->allocateDescriptorSet(deviceResources->dsLayoutShadowMap);
			_dsFinalScene[i]->setObjectName("PCFShadowsIndex" + std::to_string(i) + "DescriptorSet");

			// Update descriptor sets
			std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _dsFinalScene[i], 0));
			writeDescSets.back().setImageInfo(
				0, pvrvk::DescriptorImageInfo(_shadowMapPass->_imageView[i], deviceResources->samplerNearestShadow, pvrvk::ImageLayout::e_DEPTH_STENCIL_READ_ONLY_OPTIMAL));

			deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
		}
	}

	void createPipelineLayouts(DeviceResources* deviceResources)
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutGlobal);
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutMaterial);
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutShadowMap);

		pipeLayoutInfo.setPushConstantRange(0, pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4))));
		pipeLayoutInfo.setPushConstantRange(1,
			pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)),
				static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4))));

		_pipelineLayoutFinalScene = deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	ShadowMapPass* _shadowMapPass;

	// Final scene pass resources
	pvrvk::ShaderModule _vsFinalScene;
	pvrvk::ShaderModule _fsFinalScene;
	std::vector<pvrvk::GraphicsPipeline> _pipelines;
	pvrvk::PipelineLayout _pipelineLayoutFinalScene;
	pvrvk::DescriptorSet _dsFinalScene[2];
};

struct GaussianBlurPass
{
	virtual void render(uint32_t frameIndex, uint32_t queueIndex, uint32_t familyIndex, uint32_t dynamicOffset, pvrvk::CommandBuffer& cmdBuffer, pvrvk::DescriptorSet inputImage) = 0;
	virtual pvrvk::DescriptorSet samplingDS(uint32_t queueIndex) = 0;
	void computeBlurFactors()
	{
		const int n = _blurSize;
		assertion(n < 8, "blur size n > 7 not supported unless more gaussianFactors are allocated (currently 4 vec4s)");
		const float standardDeviation = sqrt(static_cast<float>(n) / 2.0f);
		const float factor1D = sqrt(1.0f / (2.0f * _PI * standardDeviation * standardDeviation));
		const float factorExp = 1.0f / (2.0f * standardDeviation * standardDeviation);

		float factorSum = 0.0f;
		size_t gaussianIndex = 0;
		for (int x = -n; x <= n; x++)
		{
			float factor = factor1D * exp(-(x * x) * factorExp);
			_gaussianFactors[gaussianIndex] = factor;
			factorSum += factor;
			gaussianIndex++;
		}

		// double check scaling - factors should add up to 1!
		for (size_t g = 0; g < gaussianIndex; g++) _gaussianFactors[g] /= factorSum;

		// fill remaining spaces
		while (gaussianIndex < _gaussianFactors.size())
		{
			_gaussianFactors[gaussianIndex] = 0.0f;
			gaussianIndex++;
		}
	}

	std::array<float, 16> _gaussianFactors;
	uint32_t _blurSize = 2;
};

struct GaussianBlurFragmentPass : public GaussianBlurPass
{
	void init(const char* fragmentShaderPath, pvrvk::Format imageFormat, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		computeBlurFactors();
		createShaderModules(fragmentShaderPath, shell, deviceResources);
		createImages(imageFormat, deviceResources);
		createImageViews(deviceResources);
		createRenderPasses(imageFormat, deviceResources);
		createFramebuffers(deviceResources);
		createDescriptorSets(deviceResources);
		createPipelineLayouts(deviceResources);
		createPipelines(deviceResources);
	}

	void render(uint32_t frameIndex, uint32_t queueIndex, uint32_t familyIndex, uint32_t dynamicOffset, pvrvk::CommandBuffer& cmdBuffer, pvrvk::DescriptorSet inputImage) override
	{
		blur("Fragment Gaussian Blur - Horizontal", frameIndex, queueIndex, dynamicOffset, cmdBuffer, _fbos[queueIndex][0], _pipelineHorizontal, inputImage);
		blur("Fragment Gaussian Blur - Vertical", frameIndex, queueIndex, dynamicOffset, cmdBuffer, _fbos[queueIndex][1], _pipelineVertical, _descriptorSets[queueIndex][0]);
	}

	void blur(std::string debugLabel, uint32_t frameIndex, uint32_t queueIndex, uint32_t dynamicOffset, pvrvk::CommandBuffer& cmdBuffer, pvrvk::Framebuffer fbo,
		pvrvk::GraphicsPipeline pipeline, pvrvk::DescriptorSet inputImage)
	{
		const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 0.0f), pvrvk::ClearValue(1.f, 0) };

		cmdBuffer->beginRenderPass(fbo, pvrvk::Rect2D(0, 0, g_ShadowMapSize, g_ShadowMapSize), true, clearValues, ARRAY_SIZE(clearValues));

		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(debugLabel));

		cmdBuffer->bindPipeline(pipeline);

		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipelineLayout, 0, inputImage, nullptr, 0);

		cmdBuffer->pushConstants(_pipelineLayout, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)), &_gaussianFactors[0]);

		glm::uvec2 blurSizeShadowMapSize = glm::uvec2(_blurSize, g_ShadowMapSize);

		cmdBuffer->pushConstants(_pipelineLayout, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)),
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uvec2)), &blurSizeShadowMapSize);

		cmdBuffer->draw(0, 3);

		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);

		cmdBuffer->endRenderPass();
	}

	void createImages(pvrvk::Format imageFormat, DeviceResources* deviceResources)
	{
		pvrvk::Extent3D texExtents = pvrvk::Extent3D(g_ShadowMapSize, g_ShadowMapSize, 1);

		for (int queueIndex = 0; queueIndex < 2; queueIndex++)
		{
			for (int i = 0; i < 2; i++)
			{
				_images[queueIndex][i] = pvr::utils::createImage(deviceResources->device,
					pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, imageFormat, texExtents, pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
					pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, deviceResources->vmaAllocator);
			}
		}
	}

	void createImageViews(DeviceResources* deviceResources)
	{
		for (int queueIndex = 0; queueIndex < 2; queueIndex++)
		{
			for (int i = 0; i < 2; i++) _imageViews[queueIndex][i] = deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(_images[queueIndex][i]));
		}
	}

	void createFramebuffers(DeviceResources* deviceResources)
	{
		for (int queueIndex = 0; queueIndex < 2; queueIndex++)
		{
			for (int i = 0; i < 2; i++)
				_fbos[queueIndex][i] =
					deviceResources->device->createFramebuffer(pvrvk::FramebufferCreateInfo(g_ShadowMapSize, g_ShadowMapSize, 1, _renderPass, 1, &_imageViews[queueIndex][i]));
		}
	}

	void createRenderPasses(pvrvk::Format imageFormat, DeviceResources* deviceResources)
	{
		pvrvk::AttachmentDescription colorAttachment = pvrvk::AttachmentDescription::createColorDescription(imageFormat, pvrvk::ImageLayout::e_UNDEFINED,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE, pvrvk::SampleCountFlags::e_1_BIT);

		pvrvk::AttachmentReference colorAttachmentRef = pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);

		pvrvk::SubpassDescription subpassDesc = pvrvk::SubpassDescription().setColorAttachmentReference(0, colorAttachmentRef);

		pvrvk::SubpassDependency dependency[2];

		dependency[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
		dependency[0].setDstSubpass(0);
		dependency[0].setSrcStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
		dependency[0].setDstStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
		dependency[0].setSrcAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
		dependency[0].setDstAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
		dependency[0].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

		dependency[1].setSrcSubpass(0);
		dependency[1].setDstSubpass(VK_SUBPASS_EXTERNAL);
		dependency[1].setSrcStageMask(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT);
		dependency[1].setDstStageMask(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT);
		dependency[1].setSrcAccessMask(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT);
		dependency[1].setDstAccessMask(pvrvk::AccessFlags::e_SHADER_READ_BIT);
		dependency[1].setDependencyFlags(pvrvk::DependencyFlags::e_BY_REGION_BIT);

		pvrvk::RenderPassCreateInfo renderPassCreateInfo =
			pvrvk::RenderPassCreateInfo().setAttachmentDescription(0, colorAttachment).setSubpass(0, subpassDesc).addSubpassDependencies(dependency, 2);

		_renderPass = deviceResources->device->createRenderPass(renderPassCreateInfo);
		_renderPass->setObjectName("GaussianBlurRenderPass");
	}

	void createShaderModules(const char* horizontalFragmentShaderPath, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		_vs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(TriangleVertShaderFileName)->readToEnd<uint32_t>()));
		_fsHorizontal = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(horizontalFragmentShaderPath)->readToEnd<uint32_t>()));
		_fsVertical = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(GaussianBlurVerticalFragShaderFileName)->readToEnd<uint32_t>()));
	}

	void createDescriptorSets(DeviceResources* deviceResources)
	{
		for (int queueIndex = 0; queueIndex < 2; queueIndex++)
		{
			for (int i = 0; i < 2; i++)
			{
				_descriptorSets[queueIndex][i] = deviceResources->descriptorPool->allocateDescriptorSet(deviceResources->dsLayoutShadowMap);
				_descriptorSets[queueIndex][i]->setObjectName("GaussianBlurShadowMap" + std::to_string(queueIndex) + "Index" + std::to_string(i) + "DescriptorSet");

				// Update descriptor sets
				std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _descriptorSets[queueIndex][i], 0));
				writeDescSets.back().setImageInfo(
					0, pvrvk::DescriptorImageInfo(_imageViews[queueIndex][i], deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

				deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
			}
		}
	}

	void createPipelineLayouts(DeviceResources* deviceResources)
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutShadowMap);

		pipeLayoutInfo.setPushConstantRange(0,
			pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0,
				static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)) + static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uvec2))));

		_pipelineLayout = deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	void createPipelines(DeviceResources* deviceResources)
	{
		pvrvk::GraphicsPipelineCreateInfo renderShadowPipelineCreateInfo;

		renderShadowPipelineCreateInfo.viewport.setViewportAndScissor(
			0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(g_ShadowMapSize), static_cast<float>(g_ShadowMapSize)), pvrvk::Rect2D(0, 0, g_ShadowMapSize, g_ShadowMapSize));

		// enable front face culling
		renderShadowPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);

		// set counter clockwise winding order for front faces
		renderShadowPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		// enable stencil testing
		pvrvk::StencilOpState stencilState;

		// only replace stencil buffer when the depth test passes
		stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
		stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
		stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
		stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

		// set stencil reference to 1
		stencilState.setReference(1);

		// disable stencil writing
		stencilState.setWriteMask(0);

		// blend state
		pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;

		colorAttachmentState.setBlendEnable(false);
		renderShadowPipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

		// enable the stencil tests
		renderShadowPipelineCreateInfo.depthStencil.enableStencilTest(false);
		// set stencil states
		renderShadowPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
		renderShadowPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

		// enable depth testing
		renderShadowPipelineCreateInfo.pipelineLayout = _pipelineLayout;
		renderShadowPipelineCreateInfo.depthStencil.enableDepthTest(false);
		renderShadowPipelineCreateInfo.depthStencil.enableDepthWrite(false);

		// setup vertex inputs
		renderShadowPipelineCreateInfo.vertexInput.clear();
		renderShadowPipelineCreateInfo.inputAssembler = pvrvk::PipelineInputAssemblerStateCreateInfo();

		// renderpass/subpass
		renderShadowPipelineCreateInfo.renderPass = _renderPass;

		// load and create appropriate shaders
		renderShadowPipelineCreateInfo.vertexShader.setShader(_vs);
		renderShadowPipelineCreateInfo.fragmentShader.setShader(_fsHorizontal);

		_pipelineHorizontal = deviceResources->device->createGraphicsPipeline(renderShadowPipelineCreateInfo, deviceResources->pipelineCache);
		_pipelineHorizontal->setObjectName("GaussianBlurHorizontalPassGraphicsPipeline");

		renderShadowPipelineCreateInfo.fragmentShader.setShader(_fsVertical);

		_pipelineVertical = deviceResources->device->createGraphicsPipeline(renderShadowPipelineCreateInfo, deviceResources->pipelineCache);
		_pipelineVertical->setObjectName("GaussianBlurVerticalPassGraphicsPipeline");
	}

	pvrvk::DescriptorSet samplingDS(uint32_t queueIndex) override { return _descriptorSets[queueIndex][1]; }

	pvrvk::ShaderModule _vs;
	pvrvk::ShaderModule _fsHorizontal;
	pvrvk::ShaderModule _fsVertical;
	pvrvk::GraphicsPipeline _pipelineHorizontal;
	pvrvk::GraphicsPipeline _pipelineVertical;
	pvrvk::Framebuffer _fbos[2][2];
	pvrvk::RenderPass _renderPass;
	pvrvk::PipelineLayout _pipelineLayout;
	pvrvk::DescriptorSet _descriptorSets[2][2];
	pvrvk::Image _images[2][2];
	pvrvk::ImageView _imageViews[2][2];
};

struct GaussianBlurComputePass : public GaussianBlurPass
{
	void init(const char* computeShaderPath, pvrvk::Format imageFormat, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		computeBlurFactors();
		createShaderModules(computeShaderPath, shell, deviceResources);
		createImages(imageFormat, deviceResources);
		createImageViews(deviceResources);
		createDescriptorSetLayout(deviceResources);
		createDescriptorSets(deviceResources);
		createPipelineLayouts(deviceResources);
		createPipeline(deviceResources);
	}

	void render(uint32_t frameIndex, uint32_t queueIndex, uint32_t familyIndex, uint32_t dynamicOffset, pvrvk::CommandBuffer& cmdBuffer, pvrvk::DescriptorSet inputImage) override
	{
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("Compute Gaussian Blur"));

		{
			pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_UNDEFINED;
			pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_GENERAL;

			pvrvk::MemoryBarrierSet layoutTransitions;
			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_READ_BIT, pvrvk::AccessFlags::e_SHADER_WRITE_BIT, _images[queueIndex],
				pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout, familyIndex, familyIndex));

			cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, layoutTransitions);
		}

		cmdBuffer->bindPipeline(_pipeline);

		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, _pipelineLayout, 0, inputImage, nullptr, 0);
		cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, _pipelineLayout, 1, _dsOutput[queueIndex], nullptr, 0);

		cmdBuffer->pushConstants(_pipelineLayout, pvrvk::ShaderStageFlags::e_COMPUTE_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)), &_gaussianFactors[0]);

		glm::uvec2 blurSizeShadowMapSize = glm::uvec2(_blurSize, g_ShadowMapSize);

		cmdBuffer->pushConstants(_pipelineLayout, pvrvk::ShaderStageFlags::e_COMPUTE_BIT, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)),
			static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uvec2)), &blurSizeShadowMapSize);

		int dispatchSize = (g_ShadowMapSize + g_LocalWorkGroupSize - 1) / g_LocalWorkGroupSize;

		cmdBuffer->dispatch(dispatchSize, dispatchSize, 1);

		{
			pvrvk::ImageLayout sourceImageLayout = pvrvk::ImageLayout::e_GENERAL;
			pvrvk::ImageLayout destinationImageLayout = pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL;

			pvrvk::MemoryBarrierSet layoutTransitions;
			layoutTransitions.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_SHADER_READ_BIT, _images[queueIndex],
				pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), sourceImageLayout, destinationImageLayout, familyIndex, familyIndex));

			cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, layoutTransitions);
		}

		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
	}

	void createImages(pvrvk::Format imageFormat, DeviceResources* deviceResources)
	{
		pvrvk::Extent3D texExtents = pvrvk::Extent3D(g_ShadowMapSize, g_ShadowMapSize, 1);

		for (int queueIndex = 0; queueIndex < 2; queueIndex++)
			_images[queueIndex] = pvr::utils::createImage(deviceResources->device,
				pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, imageFormat, texExtents, pvrvk::ImageUsageFlags::e_STORAGE_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_NONE, deviceResources->vmaAllocator);
	}

	void createImageViews(DeviceResources* deviceResources)
	{
		for (int queueIndex = 0; queueIndex < 2; queueIndex++) _imageViews[queueIndex] = deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(_images[queueIndex]));
	}

	void createShaderModules(const char* computeShaderPath, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		_cs = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(computeShaderPath)->readToEnd<uint32_t>()));
	}

	void createDescriptorSetLayout(DeviceResources* deviceResources)
	{
		pvrvk::DescriptorSetLayoutCreateInfo shadowMapDescSetInfo;

		shadowMapDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1u, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

		_dsLayoutOutput = deviceResources->device->createDescriptorSetLayout(shadowMapDescSetInfo);
	}

	void createDescriptorSets(DeviceResources* deviceResources)
	{
		for (int queueIndex = 0; queueIndex < 2; queueIndex++)
		{
			{
				_dsSampling[queueIndex] = deviceResources->descriptorPool->allocateDescriptorSet(deviceResources->dsLayoutShadowMap);
				_dsSampling[queueIndex]->setObjectName("GaussianBlurComputeQueueIndex" + std::to_string(queueIndex) + "DescriptorSet");

				// Update descriptor sets
				std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _dsSampling[queueIndex], 0));
				writeDescSets.back().setImageInfo(
					0, pvrvk::DescriptorImageInfo(_imageViews[queueIndex], deviceResources->samplerBilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

				deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
			}

			{
				_dsOutput[queueIndex] = deviceResources->descriptorPool->allocateDescriptorSet(_dsLayoutOutput);
				_dsOutput[queueIndex]->setObjectName("GaussianBlurOutputComputePassQueueIndex" + std::to_string(queueIndex) + "DescriptorSet");

				// Update descriptor sets
				std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

				writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _dsOutput[queueIndex], 0));
				writeDescSets.back().setImageInfo(0, pvrvk::DescriptorImageInfo(_imageViews[queueIndex], pvrvk::ImageLayout::e_GENERAL));

				deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
			}
		}
	}

	void createPipelineLayouts(DeviceResources* deviceResources)
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutShadowMap);
		pipeLayoutInfo.addDescSetLayout(_dsLayoutOutput);

		pipeLayoutInfo.setPushConstantRange(0,
			pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_COMPUTE_BIT, 0,
				static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)) + static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::uvec2))));

		_pipelineLayout = deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	void createPipeline(DeviceResources* deviceResources)
	{
		pvrvk::ComputePipelineCreateInfo pipelineCreateInfo;

		pipelineCreateInfo.computeShader.setShader(_cs);
		pipelineCreateInfo.computeShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &_blurSize, sizeof(uint32_t)));
		pipelineCreateInfo.pipelineLayout = _pipelineLayout;

		_pipeline = deviceResources->device->createComputePipeline(pipelineCreateInfo, deviceResources->pipelineCache);
		_pipeline->setObjectName("GaussianBlurComputePipeline");
	}

	pvrvk::DescriptorSet samplingDS(uint32_t queueIndex) override { return _dsSampling[queueIndex]; }

	pvrvk::ShaderModule _cs;
	pvrvk::ComputePipeline _pipeline;
	pvrvk::PipelineLayout _pipelineLayout;
	pvrvk::DescriptorSet _dsSampling[2];
	pvrvk::DescriptorSetLayout _dsLayoutOutput;
	pvrvk::DescriptorSet _dsOutput[2];
	pvrvk::Image _images[2];
	pvrvk::ImageView _imageViews[2];
};

struct VSMShadowsSample
{
	VSMShadowsSample() {}
	~VSMShadowsSample() {}

	void init(const char* fragmentShaderPath, const pvr::assets::ModelHandle& scene, pvr::Shell& shell, DeviceResources* deviceResources, ShadowMapPass* shadowMapPass,
		GaussianBlurPass* blurPass)
	{
		_shadowMapPass = shadowMapPass;
		_blurPass = blurPass;

		createShaderModules(fragmentShaderPath, shell, deviceResources);
		createPipelineLayouts(deviceResources);
		createPipelines(scene, shell, deviceResources);
		createDescriptorSets(deviceResources);
	}

	void render(const pvr::assets::ModelHandle& scene, DeviceResources* deviceResources, pvr::Shell& shell, uint32_t frameIndex, uint32_t queueIndex, glm::vec4 shadowParams)
	{
		auto cmdBuffer = deviceResources->cmdBuffers[frameIndex];
		auto fbo = deviceResources->onScreenFramebuffer[frameIndex];

		// Render shadow map.
		_shadowMapPass->render(scene, deviceResources, frameIndex, queueIndex, deviceResources->globalUBO.view.getDynamicSliceOffset(frameIndex));

		// Perform blur
		_blurPass->render(frameIndex, queueIndex, deviceResources->queue[0]->getFamilyIndex(), deviceResources->globalUBO.view.getDynamicSliceOffset(frameIndex), cmdBuffer,
			_dsDepthMap[queueIndex]);

		// Setup clear color.
		const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(0.0f, 0.40f, .39f, 1.0f), pvrvk::ClearValue(1.f, 0) };

		// Start render pass.
		cmdBuffer->beginRenderPass(fbo, pvrvk::Rect2D(0, 0, shell.getWidth(), shell.getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		// Insert a debug label.
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel(pvr::strings::createFormatted("(No Shadows) Main Scene Render Pass - Swapchain (%i)", frameIndex)));

		// Pass push constants containing shadow filtering parameters.
		cmdBuffer->pushConstants(_pipelineLayoutFinalScene, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, sizeof(glm::mat4), sizeof(glm::vec4), &shadowParams);

		// Render all mesh nodes.
		for (uint32_t i = 0; i < scene->getNumMeshNodes(); i++)
		{
			const pvr::assets::Model::Node* pNode = &scene->getMeshNode(i);
			const uint32_t meshId = pNode->getObjectId();

			cmdBuffer->bindPipeline(_pipelines[meshId]);

			uint32_t offsets[1];
			offsets[0] = deviceResources->globalUBO.view.getDynamicSliceOffset(frameIndex);

			pvrvk::DescriptorSet arrayDS[] = { deviceResources->dsGlobal, deviceResources->materials[pNode->getMaterialIndex()].materialDescriptorSet,
				_blurPass->samplingDS(queueIndex) };

			// Bind descriptor set containing global UBO.
			cmdBuffer->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipelineLayoutFinalScene, 0, arrayDS, 3, offsets, 1);

			glm::mat4 transform = scene->getWorldMatrix(i);
			cmdBuffer->pushConstants(
				_pipelines[meshId]->getPipelineLayout(), pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)), &transform);

			// Gets pMesh referenced by the pNode
			const pvr::assets::Mesh* pMesh = &scene->getMesh(meshId);

			// bind the vertex and index buffer
			cmdBuffer->bindVertexBuffer(deviceResources->vbos[meshId], 0, 0);
			cmdBuffer->bindIndexBuffer(
				deviceResources->ibos[meshId], 0, pMesh->getFaces().getDataType() == pvr::IndexType::IndexType16Bit ? pvrvk::IndexType::e_UINT16 : pvrvk::IndexType::e_UINT32);

			cmdBuffer->drawIndexed(0, pMesh->getNumFaces() * 3, 0, 0, 1);
		}

		// End debug label region.
		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
	}

	void createShaderModules(const char* fragmentShaderPath, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		_vsFinalScene = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(MeshVertShaderFileName)->readToEnd<uint32_t>()));
		_fsFinalScene = deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(fragmentShaderPath)->readToEnd<uint32_t>()));
	}

	void createPipelines(const pvr::assets::ModelHandle& scene, pvr::Shell& shell, DeviceResources* deviceResources)
	{
		_pipelines.resize(scene->getNumMeshes());

		for (uint32_t i = 0; i < scene->getNumMeshes(); i++)
		{
			pvrvk::GraphicsPipelineCreateInfo renderShadowPipelineCreateInfo;

			renderShadowPipelineCreateInfo.viewport.setViewportAndScissor(0,
				pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(shell.getWidth()), static_cast<float>(shell.getHeight())), pvrvk::Rect2D(0, 0, shell.getWidth(), shell.getHeight()));

			// enable front face culling
			renderShadowPipelineCreateInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

			// set counter clockwise winding order for front faces
			renderShadowPipelineCreateInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

			// enable depth testing
			renderShadowPipelineCreateInfo.depthStencil.enableDepthTest(true);
			renderShadowPipelineCreateInfo.depthStencil.enableDepthWrite(true);

			// load and create appropriate shaders
			renderShadowPipelineCreateInfo.vertexShader.setShader(_vsFinalScene);
			renderShadowPipelineCreateInfo.fragmentShader.setShader(_fsFinalScene);

			// setup vertex inputs
			pvr::utils::populateInputAssemblyFromMesh(
				scene->getMesh(i), VertexBindings, ARRAY_SIZE(VertexBindings), renderShadowPipelineCreateInfo.vertexInput, renderShadowPipelineCreateInfo.inputAssembler);

			// renderpass/subpass
			renderShadowPipelineCreateInfo.renderPass = deviceResources->onScreenFramebuffer[0]->getRenderPass();

			// enable stencil testing
			pvrvk::StencilOpState stencilState;

			// only replace stencil buffer when the depth test passes
			stencilState.setFailOp(pvrvk::StencilOp::e_KEEP);
			stencilState.setDepthFailOp(pvrvk::StencilOp::e_KEEP);
			stencilState.setPassOp(pvrvk::StencilOp::e_REPLACE);
			stencilState.setCompareOp(pvrvk::CompareOp::e_ALWAYS);

			pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;

			colorAttachmentState.setBlendEnable(false);
			renderShadowPipelineCreateInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

			// set stencil reference to 1
			stencilState.setReference(1);

			// disable stencil writing
			stencilState.setWriteMask(0);

			// enable the stencil tests
			renderShadowPipelineCreateInfo.depthStencil.enableStencilTest(false);
			// set stencil states
			renderShadowPipelineCreateInfo.depthStencil.setStencilFront(stencilState);
			renderShadowPipelineCreateInfo.depthStencil.setStencilBack(stencilState);

			renderShadowPipelineCreateInfo.pipelineLayout = _pipelineLayoutFinalScene;

			_pipelines[i] = deviceResources->device->createGraphicsPipeline(renderShadowPipelineCreateInfo, deviceResources->pipelineCache);
			_pipelines[i]->setObjectName("Mesh" + std::to_string(i) + "VSMShadowsGraphicsPipeline");
		}
	}

	void createPipelineLayouts(DeviceResources* deviceResources)
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutGlobal);
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutMaterial);
		pipeLayoutInfo.addDescSetLayout(deviceResources->dsLayoutShadowMap);

		pipeLayoutInfo.setPushConstantRange(0, pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_VERTEX_BIT, 0, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4))));
		pipeLayoutInfo.setPushConstantRange(1,
			pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::mat4x4)),
				static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4))));

		_pipelineLayoutFinalScene = deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}

	void createDescriptorSets(DeviceResources* deviceResources)
	{
		for (int i = 0; i < 2; i++)
		{
			_dsDepthMap[i] = deviceResources->descriptorPool->allocateDescriptorSet(deviceResources->dsLayoutShadowMap);
			_dsDepthMap[i]->setObjectName("VSMShadowsIndex" + std::to_string(i) + "DescriptorSet");

			// Update descriptor sets
			std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _dsDepthMap[i], 0));
			writeDescSets.back().setImageInfo(
				0, pvrvk::DescriptorImageInfo(_shadowMapPass->_imageView[i], deviceResources->samplerBilinear, pvrvk::ImageLayout::e_DEPTH_STENCIL_READ_ONLY_OPTIMAL));

			deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
		}
	}

	ShadowMapPass* _shadowMapPass;
	GaussianBlurPass* _blurPass;

	// Final scene pass resources
	pvrvk::ShaderModule _vsFinalScene;
	pvrvk::ShaderModule _fsFinalScene;
	std::vector<pvrvk::GraphicsPipeline> _pipelines;
	pvrvk::PipelineLayout _pipelineLayoutFinalScene;
	pvrvk::DescriptorSet _dsDepthMap[2];
};

/*!*********************************************************************************************************************
Class implementing the Shell functions.
***********************************************************************************************************************/
class VulkanShadows : public pvr::Shell
{
	std::unique_ptr<DeviceResources> _deviceResources;

	uint32_t _frameId;
	float _frame = 0.0f;
	uint32_t _queueIndex;
	bool _isPaused;

	// Variables to handle the animation in a time-based manner
	float _currentFrame;

	glm::mat4 _projMtx;
	pvr::TPSOrbitCamera _camera;
	pvr::assets::ModelHandle _scene;
	int32_t _selectedShadowTypeIdx = (int)ShadowType::ShadowMapPCFPoissonDisk;
	glm::vec3 _lightDir;
	float _rotation = 75.0f;
	bool _rotate = false;

	/// <summary>Flag to know whether astc iss upported by the physical device.</summary>
	bool _astcSupported;

	/// <summary>How many swapchain images are available.</summary>
	uint32_t _swapchainLength;

public:
	VulkanShadows() : _isPaused(false), _currentFrame(0) {}

	pvr::Result initApplication();
	pvr::Result initView();
	pvr::Result releaseView();
	pvr::Result quitApplication();
	pvr::Result renderFrame();

	void createUbos();
	void createResources();
	void createPasses();
	void updateControlsUI();
	void updateUbo(uint32_t swapIndex);
	void eventMappedInput(pvr::SimplifiedInput action);
};

/*!*********************************************************************************************************************
\brief  handle the input event
\param  action input actions to handle
***********************************************************************************************************************/
void VulkanShadows::eventMappedInput(pvr::SimplifiedInput action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Action1: {
		_rotate = !_rotate;
		break;
	}
	case pvr::SimplifiedInput::Action2: {
		_selectedShadowTypeIdx++;
		break;
	}
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}

	updateControlsUI();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by Shell once per run, before the rendering context is created.
Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
If the rendering context is lost, initApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanShadows::initApplication()
{
	this->setStencilBitsPerPixel(0);
	_frameId = 0;
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in quitApplication() will be called by Shell once per run, just before exiting the program.
If the rendering context is lost, quitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanShadows::quitApplication() { return pvr::Result::Success; }

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanShadows::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create vulkan instance and surface
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName());
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	pvrvk::PhysicalDevice physicalDevice = _deviceResources->instance->getPhysicalDevice(0);

	// Populate queue for rendering and transfer operation
	const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };

	// Create the device and queue
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(physicalDevice, &queuePopulateInfo, 1, &queueAccessInfo);

	// Get the queue
	_deviceResources->queue[0] = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);
	_deviceResources->queue[1] = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	_deviceResources->queue[0]->setObjectName("GraphicsQueue0");
	_deviceResources->queue[1]->setObjectName("GraphicsQueue1");

	// validate the supported swapchain image usage for source transfer option for capturing screenshots.
	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice->getSurfaceCapabilities(surface);
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT)) // Transfer operation supported.
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// initialise the vma allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// specify that we need stencil bits
	auto& dispAttrib = getDisplayAttributes();

	dispAttrib.depthBPP = 24;
	dispAttrib.stencilBPP = 8;

	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, dispAttrib,
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage).enableDepthBuffer(true));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;
	_deviceResources->depthStencilImages = swapChainCreateOutput.depthStencilImages;

	_swapchainLength = _deviceResources->swapchain->getSwapchainLength();

	_deviceResources->imageAcquiredSemaphores.resize(_swapchainLength);
	_deviceResources->presentationSemaphores.resize(_swapchainLength);
	_deviceResources->perFrameResourcesFences.resize(_swapchainLength);
	_deviceResources->commandPool.resize(_swapchainLength);
	_deviceResources->cmdBuffers.resize(_swapchainLength);

	_currentFrame = 0.;
	_queueIndex = 0;

	_astcSupported = pvr::utils::isSupportedFormat(_deviceResources->device->getPhysicalDevice(), pvrvk::Format::e_ASTC_4x4_UNORM_BLOCK);

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, static_cast<uint16_t>(32 * _swapchainLength))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, static_cast<uint16_t>(32 * _swapchainLength))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, static_cast<uint16_t>(32 * _swapchainLength))
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, static_cast<uint16_t>(32 * _swapchainLength))
																						  .setMaxDescriptorSets(static_cast<uint16_t>(32 * _swapchainLength)));

	_deviceResources->descriptorPool->setObjectName("DescriptorPool");

	// create the commandbuffers, semaphores & the fence
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->commandPool[i] = _deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(_deviceResources->queue[0]->getFamilyIndex()));

		_deviceResources->cmdBuffers[i] = _deviceResources->commandPool[i]->allocateCommandBuffer();
		_deviceResources->cmdBuffers[i]->setObjectName("MainCommandBufferSwapchain" + std::to_string(i));

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->presentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));

		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->perFrameResourcesFences[i]->setObjectName("FenceSwapchain" + std::to_string(i));
	}

	_deviceResources->cmdBuffers[0]->begin();

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool[0], _deviceResources->queue[0]);

	_deviceResources->cmdBuffers[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->perFrameResourcesFences[0]->reset();
	_deviceResources->queue[0]->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[0]);
	_deviceResources->perFrameResourcesFences[0]->wait();

	_deviceResources->commandPool[0]->reset(pvrvk::CommandPoolResetFlags::e_RELEASE_RESOURCES_BIT);

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Shadows");
	updateControlsUI();
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	_deviceResources->uiRenderer.getSdkLogo()->setColor(1.0f, 1.0f, 1.0f, 1.f);
	_deviceResources->uiRenderer.getSdkLogo()->commitUpdates();

	createUbos();
	createResources();
	createPasses();

	if (isScreenRotated())
		_projMtx = pvr::math::perspectiveFov(
			pvr::Api::Vulkan, glm::radians(g_FOV), static_cast<float>(this->getHeight()), static_cast<float>(this->getWidth()), 0.1f, 2000.f, glm::pi<float>() * .5f);
	else
		_projMtx = pvr::math::perspectiveFov(pvr::Api::Vulkan, glm::radians(g_FOV), static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()), 0.1f, 2000.f);

	// setup the camera
	_camera.setTargetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
	_camera.setDistanceFromTarget(150.0f);
	_camera.setInclination(25.f);

	_queueIndex = _queueIndex == 0 ? 1 : 0;

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in releaseView() will be called by Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanShadows::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanShadows::renderFrame()
{
	if (_rotate) _rotation += getFrameTime() * 0.05f;

	_camera.setAzimuth(_rotation);

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	pvr::assets::AnimationInstance& animInst = _scene->getAnimationInstance(0);

	//  Calculates the _frame number to animate in a time-based manner.
	//  get the time in milliseconds.
	_frame += static_cast<float>(getFrameTime()); // design-time target fps for animation

	if (_frame >= animInst.getTotalTimeInMs()) { _frame = 0; }

	// Sets the _scene animation to this _frame
	animInst.updateAnimation(_frame);

	updateUbo(swapchainIndex);

	_deviceResources->commandPool[swapchainIndex]->reset(pvrvk::CommandPoolResetFlags::e_RELEASE_RESOURCES_BIT);

	_deviceResources->cmdBuffers[swapchainIndex]->begin();

	pvr::utils::beginCommandBufferDebugLabel(_deviceResources->cmdBuffers[swapchainIndex], pvrvk::DebugUtilsLabel("MainRenderPassSwapchain" + std::to_string(swapchainIndex)));

	ShadowType type = (ShadowType)(_selectedShadowTypeIdx % (int32_t)ShadowType::Count);

	switch (type)
	{
	case ShadowType::None: {
		_deviceResources->noShadowsSample->render(_scene, _deviceResources.get(), *this, swapchainIndex);
		break;
	}
	case ShadowType::ShadowMapHard: {
		_deviceResources->hardShadowsSample->render(_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_PCFBias, 0.0f, 0.0f, g_ShadowMapSize));
		break;
	}
	case ShadowType::ShadowMapPCFPoissonDisk: {
		_deviceResources->pcfPoissonDiskShadowsSample->render(
			_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_PCFBias, g_PoissonSamplingRadius, g_PoissonDiskSampleCount, g_ShadowMapSize));
		break;
	}
	case ShadowType::ShadowMapPCFOptimised2x2: {
		_deviceResources->pcfOptimised2x2ShadowsSample->render(_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_PCFBias, 0.0f, 0.0f, g_ShadowMapSize));
		break;
	}
	case ShadowType::ShadowMapPCFOptimised3x3: {
		_deviceResources->pcfOptimised3x3ShadowsSample->render(_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_PCFBias, 0.0f, 0.0f, g_ShadowMapSize));
		break;
	}
	case ShadowType::ShadowMapPCFOptimised5x5: {
		_deviceResources->pcfOptimised5x5ShadowsSample->render(_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_PCFBias, 0.0f, 0.0f, g_ShadowMapSize));
		break;
	}
	case ShadowType::ShadowMapPCFOptimised7x7: {
		_deviceResources->pcfOptimised7x7ShadowsSample->render(_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_PCFBias, 0.0f, 0.0f, g_ShadowMapSize));
		break;
	}
	case ShadowType::ShadowMapVSM: {
		_deviceResources->vsmFragmentShadowsSample->render(
			_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_VSMBias, g_VSMLightBleedReduction, 0.0f, 0.0f));
		break;
	}
	case ShadowType::ShadowMapEVSM2: {
		_deviceResources->evsm2FragmentShadowsSample->render(
			_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_EVSM2Bias, g_EVSM2LightBleedReduction, 0.0f, 0.0f));
		break;
	}
	case ShadowType::ShadowMapEVSM4: {
		_deviceResources->evsm4FragmentShadowsSample->render(
			_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_EVSM4Bias, g_EVSM4LightBleedReduction, 0.0f, 0.0f));
		break;
	}
	case ShadowType::ShadowMapVSMCompute: {
		_deviceResources->vsmComputeShadowsSample->render(_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_VSMBias, g_VSMLightBleedReduction, 0.0f, 0.0f));
		break;
	}
	case ShadowType::ShadowMapEVSM2Compute: {
		_deviceResources->evsm2ComputeShadowsSample->render(
			_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_EVSM2Bias, g_EVSM2LightBleedReduction, 0.0f, 0.0f));
		break;
	}
	case ShadowType::ShadowMapEVSM4Compute: {
		_deviceResources->evsm4ComputeShadowsSample->render(
			_scene, _deviceResources.get(), *this, swapchainIndex, _queueIndex, glm::vec4(g_EVSM4Bias, g_EVSM4LightBleedReduction, 0.0f, 0.0f));
		break;
	}
	default: {
		break;
	}
	}

	_deviceResources->uiRenderer.beginRendering(_deviceResources->cmdBuffers[swapchainIndex]);
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.endRendering();

	_deviceResources->cmdBuffers[swapchainIndex]->endRenderPass();
	pvr::utils::endCommandBufferDebugLabel(_deviceResources->cmdBuffers[swapchainIndex]);
	_deviceResources->cmdBuffers[swapchainIndex]->end();

	// Update all the bones matrices
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags;

	_deviceResources->queue[_queueIndex]->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue[0], _deviceResources->commandPool[0], _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &swapchainIndex;

	_deviceResources->queue[0]->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();
	_queueIndex = _queueIndex == 0 ? 1 : 0;

	return pvr::Result::Success;
}

void VulkanShadows::createUbos()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("ViewProjMat", pvr::GpuDatatypes::mat4x4);
	desc.addElement("ProjMat", pvr::GpuDatatypes::mat4x4);
	desc.addElement("ViewMat", pvr::GpuDatatypes::mat4x4);
	desc.addElement("ShadowMat", pvr::GpuDatatypes::mat4x4);
	desc.addElement("LightDir", pvr::GpuDatatypes::vec4);
	desc.addElement("LightPosVS", pvr::GpuDatatypes::vec4);
	desc.addElement("LightDirVS", pvr::GpuDatatypes::vec4);

	_deviceResources->globalUBO.view.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	const pvrvk::DeviceSize size = _deviceResources->globalUBO.view.getSize();

	_deviceResources->globalUBO.buffer = pvr::utils::createBuffer(_deviceResources->device, pvrvk::BufferCreateInfo(size, pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
		_deviceResources->vmaAllocator);
	_deviceResources->globalUBO.buffer->setObjectName("GlobalUBO");

	_deviceResources->globalUBO.view.pointToMappedMemory(_deviceResources->globalUBO.buffer->getDeviceMemory()->getMappedData());
}

void VulkanShadows::createResources()
{
	bool reqSubmission = false;

	_deviceResources->cmdBuffers[0]->begin();

	// Load the model from disk.
	_scene = pvr::assets::loadModel(*this, ModelFileName);

	// Insert Vertex and Index data from mesh into buffers.
	pvr::utils::appendSingleBuffersFromModel(
		_deviceResources->device, *_scene, _deviceResources->vbos, _deviceResources->ibos, _deviceResources->cmdBuffers[0], reqSubmission, _deviceResources->vmaAllocator);

	// Create the sampler object
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = pvrvk::SamplerAddressMode::e_REPEAT;
	_deviceResources->samplerTrilinear = _deviceResources->device->createSampler(samplerInfo);

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	_deviceResources->materials.resize(_scene->getNumMaterials());

	// Create descriptor set layout for materials
	pvrvk::DescriptorSetLayoutCreateInfo materialDescSetInfo;

	materialDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	_deviceResources->dsLayoutMaterial = _deviceResources->device->createDescriptorSetLayout(materialDescSetInfo);

	for (uint32_t i = 0; i < _scene->getNumMaterials(); ++i)
	{
		if (_scene->getMaterial(i).defaultSemantics().getDiffuseTextureIndex() == static_cast<uint32_t>(-1)) { continue; }

		_deviceResources->materials[i].materialDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->dsLayoutMaterial);
		_deviceResources->materials[i].materialDescriptorSet->setObjectName("Material" + std::to_string(i) + "DescriptorSet");

		const pvr::assets::Model::Material& material = _scene->getMaterial(i);

		// Load the diffuse texture map
		std::string fileName = _scene->getTexture(material.defaultSemantics().getDiffuseTextureIndex()).getName().c_str();
		pvr::assets::helper::getTextureNameWithExtension(fileName, _astcSupported);

		_deviceResources->materials[i].diffuseImageView = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, fileName.c_str(), true, _deviceResources->cmdBuffers[0],
			*this,
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);

		writeDescSets.push_back(pvrvk::WriteDescriptorSet());
		pvrvk::WriteDescriptorSet& writeDescSet = writeDescSets.back();
		writeDescSet.set(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->materials[i].materialDescriptorSet, 0);
		writeDescSet.setImageInfo(
			0, pvrvk::DescriptorImageInfo(_deviceResources->materials[i].diffuseImageView, _deviceResources->samplerTrilinear, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	}

	_deviceResources->cmdBuffers[0]->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[0];
	submitInfo.numCommandBuffers = 1;

	_deviceResources->perFrameResourcesFences[0]->reset();
	_deviceResources->queue[0]->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[0]);
	_deviceResources->perFrameResourcesFences[0]->wait();
	_deviceResources->commandPool[0]->reset(pvrvk::CommandPoolResetFlags::e_RELEASE_RESOURCES_BIT);

	pvrvk::SamplerCreateInfo samplerNearestInfo;
	samplerNearestInfo.minFilter = samplerNearestInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerNearestInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	samplerNearestInfo.wrapModeU = samplerNearestInfo.wrapModeV = samplerNearestInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_BORDER;
	samplerNearestInfo.borderColor = pvrvk::BorderColor::e_FLOAT_OPAQUE_WHITE;
	samplerNearestInfo.compareOp = pvrvk::CompareOp::e_LESS;
	samplerNearestInfo.compareOpEnable = true;

	_deviceResources->samplerNearestShadow = _deviceResources->device->createSampler(samplerNearestInfo);

	pvrvk::SamplerCreateInfo samplerBilinearInfo;
	samplerBilinearInfo.minFilter = samplerBilinearInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerBilinearInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	samplerBilinearInfo.wrapModeU = samplerBilinearInfo.wrapModeV = samplerBilinearInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerBilinearInfo.borderColor = pvrvk::BorderColor::e_FLOAT_OPAQUE_WHITE;

	_deviceResources->samplerBilinear = _deviceResources->device->createSampler(samplerBilinearInfo);

	// Create descriptor set layout for global UBO
	pvrvk::DescriptorSetLayoutCreateInfo globalDescSetInfo;

	globalDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1u,
		pvrvk::ShaderStageFlags::e_VERTEX_BIT | pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT | pvrvk::ShaderStageFlags::e_GEOMETRY_BIT);

	_deviceResources->dsLayoutGlobal = _deviceResources->device->createDescriptorSetLayout(globalDescSetInfo);

	// Allocate and update global descriptor set
	_deviceResources->dsGlobal = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->dsLayoutGlobal);
	_deviceResources->dsGlobal->setObjectName("GlobalDescriptorSet");

	// Update descriptor sets
	writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->dsGlobal, 0));
	writeDescSets.back().setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->globalUBO.buffer, 0, _deviceResources->globalUBO.view.getDynamicSliceSize()));

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);

	// Allocate descriptor set layout for shadow map binding
	pvrvk::DescriptorSetLayoutCreateInfo shadowMapDescSetInfo;

	shadowMapDescSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1u, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT | pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

	_deviceResources->dsLayoutShadowMap = _deviceResources->device->createDescriptorSetLayout(shadowMapDescSetInfo);
}

void VulkanShadows::createPasses()
{
	_deviceResources->noShadowsSample = std::make_shared<NoShadowsSample>();
	_deviceResources->noShadowsSample->init(_scene, *this, _deviceResources.get());

	_deviceResources->shadowMapPass = std::make_shared<ShadowMapPass>();
	_deviceResources->shadowMapPass->init(_scene, *this, _deviceResources.get());

	_deviceResources->hardShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->hardShadowsSample->init(MeshHardShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get());

	// PCF techniques
	_deviceResources->pcfPoissonDiskShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfPoissonDiskShadowsSample->init(MeshPCFPoissonDiskShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get());

	_deviceResources->pcfOptimised2x2ShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfOptimised2x2ShadowsSample->init(MeshPCFOptimised2x2ShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get());

	_deviceResources->pcfOptimised2x2ShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfOptimised2x2ShadowsSample->init(MeshPCFOptimised2x2ShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get());

	_deviceResources->pcfOptimised3x3ShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfOptimised3x3ShadowsSample->init(MeshPCFOptimised3x3ShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get());

	_deviceResources->pcfOptimised5x5ShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfOptimised5x5ShadowsSample->init(MeshPCFOptimised5x5ShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get());

	_deviceResources->pcfOptimised7x7ShadowsSample = std::make_shared<PCFShadowsSample>();
	_deviceResources->pcfOptimised7x7ShadowsSample->init(MeshPCFOptimised7x7ShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get());

	// Blur Passes
	_deviceResources->gaussianBlurVSMFragmentPass = std::make_shared<GaussianBlurFragmentPass>();
	_deviceResources->gaussianBlurVSMFragmentPass->init(GaussianBlurHorizontalVSMFragShaderFileName, pvrvk::Format::e_R16G16_SFLOAT, *this, _deviceResources.get());

	_deviceResources->gaussianBlurEVSM2FragmentPass = std::make_shared<GaussianBlurFragmentPass>();
	_deviceResources->gaussianBlurEVSM2FragmentPass->init(GaussianBlurHorizontalEVSM2FragShaderFileName, pvrvk::Format::e_R16G16_SFLOAT, *this, _deviceResources.get());

	_deviceResources->gaussianBlurEVSM4FragmentPass = std::make_shared<GaussianBlurFragmentPass>();
	_deviceResources->gaussianBlurEVSM4FragmentPass->init(GaussianBlurHorizontalEVSM4FragShaderFileName, pvrvk::Format::e_R16G16B16A16_SFLOAT, *this, _deviceResources.get());

	_deviceResources->gaussianBlurVSMComputePass = std::make_shared<GaussianBlurComputePass>();
	_deviceResources->gaussianBlurVSMComputePass->init(GaussianBlurVSMCompShaderFileName, pvrvk::Format::e_R16G16_SFLOAT, *this, _deviceResources.get());

	_deviceResources->gaussianBlurEVSM2ComputePass = std::make_shared<GaussianBlurComputePass>();
	_deviceResources->gaussianBlurEVSM2ComputePass->init(GaussianBlurEVSM2CompShaderFileName, pvrvk::Format::e_R16G16_SFLOAT, *this, _deviceResources.get());

	_deviceResources->gaussianBlurEVSM4ComputePass = std::make_shared<GaussianBlurComputePass>();
	_deviceResources->gaussianBlurEVSM4ComputePass->init(GaussianBlurEVSM4CompShaderFileName, pvrvk::Format::e_R16G16B16A16_SFLOAT, *this, _deviceResources.get());

	// VSM techniques
	_deviceResources->vsmFragmentShadowsSample = std::make_shared<VSMShadowsSample>();
	_deviceResources->vsmFragmentShadowsSample->init(
		MeshVSMShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get(), _deviceResources->gaussianBlurVSMFragmentPass.get());

	_deviceResources->evsm2FragmentShadowsSample = std::make_shared<VSMShadowsSample>();
	_deviceResources->evsm2FragmentShadowsSample->init(
		MeshEVSM2ShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get(), _deviceResources->gaussianBlurEVSM2FragmentPass.get());

	_deviceResources->evsm4FragmentShadowsSample = std::make_shared<VSMShadowsSample>();
	_deviceResources->evsm4FragmentShadowsSample->init(
		MeshEVSM4ShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get(), _deviceResources->gaussianBlurEVSM4FragmentPass.get());

	_deviceResources->vsmComputeShadowsSample = std::make_shared<VSMShadowsSample>();
	_deviceResources->vsmComputeShadowsSample->init(
		MeshVSMShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get(), _deviceResources->gaussianBlurVSMComputePass.get());

	_deviceResources->evsm2ComputeShadowsSample = std::make_shared<VSMShadowsSample>();
	_deviceResources->evsm2ComputeShadowsSample->init(
		MeshEVSM2ShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get(), _deviceResources->gaussianBlurEVSM2ComputePass.get());

	_deviceResources->evsm4ComputeShadowsSample = std::make_shared<VSMShadowsSample>();
	_deviceResources->evsm4ComputeShadowsSample->init(
		MeshEVSM4ShadowsFragShaderFileName, _scene, *this, _deviceResources.get(), _deviceResources->shadowMapPass.get(), _deviceResources->gaussianBlurEVSM4ComputePass.get());
}

void VulkanShadows::updateUbo(uint32_t swapIndex)
{
	_lightDir = glm::normalize(glm::vec3(sinf(getTime() * 0.001f), -1.0f, cosf(getTime() * 0.001f)));

	const glm::mat4 viewProj = _projMtx * _camera.getViewMatrix();

	float shadowMapSize = 90.0f;

	glm::mat4 shadowProjMat = glm::ortho(-shadowMapSize, shadowMapSize, -shadowMapSize, shadowMapSize, 10.0f, 500.0f);

	shadowProjMat[1] *= -1.f;

	glm::vec3 shadowCamTargetPos = glm::vec3(0.0f);
	glm::vec3 shadowCamPos = -_lightDir * 250.0f;

	glm::mat4 shadowViewMat = glm::lookAt(shadowCamPos, shadowCamTargetPos, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 shadowMat = shadowProjMat * shadowViewMat;

	auto& uboView = _deviceResources->globalUBO.view;

	uboView.getElement(0, 0, swapIndex).setValue(viewProj);
	uboView.getElement(1, 0, swapIndex).setValue(_projMtx);
	uboView.getElement(2, 0, swapIndex).setValue(_camera.getViewMatrix());
	uboView.getElement(3, 0, swapIndex).setValue(shadowMat);
	uboView.getElement(4, 0, swapIndex).setValue(glm::vec4(_lightDir, 0.0f));
	uboView.getElement(5, 0, swapIndex).setValue(_camera.getViewMatrix() * glm::vec4(shadowCamPos, 1.0f));
	uboView.getElement(6, 0, swapIndex).setValue(_camera.getViewMatrix() * glm::vec4(_lightDir, 0.0f));

	if (uint32_t(_deviceResources->globalUBO.buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->globalUBO.buffer->getDeviceMemory()->flushRange(uboView.getDynamicSliceOffset(swapIndex), uboView.getDynamicSliceSize());
	}
}

void VulkanShadows::updateControlsUI()
{
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action 1: Pause\n"
															   "Action 2: Change Technique (" +
		std::string(ShadowTypeNames[_selectedShadowTypeIdx % (int32_t)ShadowType::Count]) + ")\n");
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanShadows>(); }
