/*!*********************************************************************************************************************
\File         VulkanParticleSystem.cpp
\Title        ParticleSystem
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Particle animation system using Compute Shaders. Requires the PVRShell.
***********************************************************************************************************************/
#include "ParticleSystemGPU.h"
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

namespace Files {
// Asset files
const char SphereModelFile[] = "sphere.pod";
const char FragShaderSrcFile[] = "FragShader.fsh.spv";
const char VertShaderSrcFile[] = "VertShader.vsh.spv";
const char FloorVertShaderSrcFile[] = "FloorVertShader.vsh.spv";
const char ParticleShaderFragSrcFile[] = "ParticleFragShader.fsh.spv";
const char ParticleShaderVertSrcFile[] = "ParticleVertShader.vsh.spv";
} // namespace Files

namespace Configuration {
enum
{
	MinNoParticles = 128,
	InitialNoParticles = 32768,
	MaxNoParticles = 32768 * 8,
	NumberOfSpheres = 8,
};

const float CameraNear = .1f;
const float CameraFar = 1000.0f;
const glm::vec3 LightPosition(0.0f, 10.0f, 0.0f);

const Sphere Spheres[] = {
	Sphere(glm::vec3(-20.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(-20.0f, 6.0f, 0.0f), 5.f),
	Sphere(glm::vec3(-20.0f, 6.0f, 20.0f), 5.f),
	Sphere(glm::vec3(0.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(0.0f, 6.0f, 20.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f, 0.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f, 20.0f), 5.f),
};

const pvr::utils::StructuredMemoryDescription SpherePipeUboMapping("SpherePipelineUbo", 1,
	{
		{ "uModelViewMatrix", pvr::GpuDatatypes::mat4x4 },
		{ "uModelViewProjectionMatrix", pvr::GpuDatatypes::mat4x4 },
		{ "uModelViewITMatrix", pvr::GpuDatatypes::mat3x3 },
	});

namespace SpherePipeDynamicUboElements {
enum Enum
{
	ModelViewMatrix,
	ModelViewProjectionMatrix,
	ModelViewITMatrix,
	Count
};
}

const pvr::utils::StructuredMemoryDescription FloorPipeUboMapping("FloorPipelineUbo", 1,
	{
		{ "uModelViewMatrix", pvr::GpuDatatypes::mat4x4 },
		{ "uModelViewProjectionMatrix", pvr::GpuDatatypes::mat4x4 },
		{ "uModelViewITMatrix", pvr::GpuDatatypes::mat3x3 },
		{ "uLightPos", pvr::GpuDatatypes::vec3 },
	});

namespace FloorPipeDynamicUboElements {
enum Enum
{
	ModelViewMatrix,
	ModelViewProjectionMatrix,
	ModelViewITMatrix,
	LightPos,
	Count
};
}
} // namespace Configuration

// Index to bind the attributes to vertex shaders
namespace Attributes {
enum Enum
{
	ParticlePositionArray = 0,
	ParticleLifespanArray = 1,
	VertexArray = 0,
	NormalArray = 1,
	TexCoordArray = 2,
	BindingIndex0 = 0
};
}

struct PassSphere
{
	pvr::utils::StructuredBufferView uboPerModelBufferView;
	pvrvk::Buffer uboPerModel;
	pvr::utils::StructuredBufferView uboLightPropBufferView;
	pvrvk::Buffer uboLightProp;
	pvrvk::DescriptorSet descriptoruboPerModel[MaxSwapChains]; // per swapchains
	pvrvk::DescriptorSet descriptorLighProp[MaxSwapChains];
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::Buffer vbo;
	pvrvk::Buffer ibo;
};

struct PassParticles
{
	pvr::utils::StructuredBufferView uboMvpBufferView;
	pvrvk::Buffer uboMvp;
	pvrvk::DescriptorSet descriptorMvp[MaxSwapChains]; // per swapchains
	pvrvk::GraphicsPipeline pipeline;
};

struct PassFloor
{
	pvr::utils::StructuredBufferView uboPerModelBufferView;
	pvrvk::Buffer uboPerModel;
	pvrvk::DescriptorSet descriptorUbo[MaxSwapChains]; // per swapchains
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::Buffer vbo;
};

/*!*********************************************************************************************************************
Class implementing the PVRShell functions.
***********************************************************************************************************************/
class VulkanParticleSystem : public pvr::Shell
{
private:
	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
		pvrvk::Device device;
		pvrvk::Surface surface;
		pvrvk::Swapchain swapchain;
		pvrvk::Queue queue;
		pvrvk::CommandPool commandPool;
		pvrvk::DescriptorPool descriptorPool;
		ParticleSystemGPU particleSystemGPU;

		pvr::utils::vma::Allocator vmaAllocator;

		pvrvk::CommandBuffer mainCommandBuffers[MaxSwapChains];
		pvrvk::SecondaryCommandBuffer graphicsCommandBuffers[MaxSwapChains];
		pvrvk::SecondaryCommandBuffer uiRendererCommandBuffers[MaxSwapChains];
		pvr::Multi<pvrvk::ImageView> depthStencilImages;
		pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

		PassSphere passSphere;
		PassParticles passParticles;
		PassFloor passFloor;
		pvrvk::DescriptorSetLayout descLayoutUboPerModel;
		pvrvk::DescriptorSetLayout descLayoutUbo;

		pvrvk::PipelineCache pipelineCache;

		pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
		pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources(VulkanParticleSystem& thisApp) : particleSystemGPU(thisApp) {}
		~DeviceResources()
		{
			if (device)
			{
				device->waitIdle();
				uint32_t l = swapchain->getSwapchainLength();
				for (uint32_t i = 0; i < l; ++i)
				{
					if (perFrameResourcesFences[i])
						perFrameResourcesFences[i]->wait();
				}
			}
		}
	};

	std::unique_ptr<DeviceResources> _deviceResources;

	pvr::assets::ModelHandle _scene;
	bool _isCameraPaused;

	// View matrix
	glm::mat4 _viewMtx, _projMtx, _viewProjMtx;
	glm::mat3 _viewIT;
	glm::mat4 _mLightView, _mBiasMatrix;
	glm::vec3 _lightPos;
	uint32_t _frameId;

public:
	VulkanParticleSystem();

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	virtual void eventMappedInput(pvr::SimplifiedInput key);

	void createBuffers();
	void createPipelines();
	void recordCommandBuffers();
	void recordCommandBuffer(uint8_t idx);
	void recordCmdDrawFloor(uint8_t idx);
	void recordCmdDrawParticles(uint8_t idx);
	void recordCmdDrawSphere(uint32_t sphereId, uint8_t idx);
	void createDescriptors();
	void updateFloor();
	void updateSpheres();
	void updateParticleUniforms();
};

/*!*********************************************************************************************************************
\brief  Handles user input and updates live variables accordingly.
\param key Input key to handle
***********************************************************************************************************************/
void VulkanParticleSystem::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
	case pvr::SimplifiedInput::Left:
	{
		_deviceResources->queue->waitIdle(); // wait for the queue to finish and update all the compute commandbuffers
		uint32_t numParticles = _deviceResources->particleSystemGPU.getNumberOfParticles();
		if (numParticles / 2 >= Configuration::MinNoParticles)
		{
			_deviceResources->particleSystemGPU.setNumberOfParticles(numParticles / 2, _deviceResources->queue, _deviceResources->vmaAllocator);
			_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", numParticles / 2));
			_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
			recordCommandBuffers();
		}
	}
	break;
	case pvr::SimplifiedInput::Right:
	{
		_deviceResources->queue->waitIdle(); // wait for the queue to finish and to update all the compute commandbuffers
		uint32_t numParticles = _deviceResources->particleSystemGPU.getNumberOfParticles();
		if (numParticles * 2 <= Configuration::MaxNoParticles)
		{
			_deviceResources->particleSystemGPU.setNumberOfParticles(numParticles * 2, _deviceResources->queue, _deviceResources->vmaAllocator);
			_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", numParticles * 2));
			_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
			recordCommandBuffers();
		}
	}
	break;
	case pvr::SimplifiedInput::Action1:
		_isCameraPaused = !_isCameraPaused;
		break;
	case pvr::SimplifiedInput::ActionClose:
		exitShell();
		break;
	default:
		break;
	}
}

/*!*********************************************************************************************************************
\brief ctor
***********************************************************************************************************************/
VulkanParticleSystem::VulkanParticleSystem() : _isCameraPaused(0) {}

/*!*********************************************************************************************************************
\brief  Loads the mesh data required for this training course into vertex buffer objects
\return Return true on success
***********************************************************************************************************************/
void VulkanParticleSystem::createBuffers()
{
	// create the spheres vertex and index buffers
	_deviceResources->mainCommandBuffers[0]->begin();
	bool requiresCommandBufferSubmission = false;
	pvr::utils::createSingleBuffersFromMesh(_deviceResources->device, _scene->getMesh(0), _deviceResources->passSphere.vbo, _deviceResources->passSphere.ibo,
		_deviceResources->mainCommandBuffers[0], requiresCommandBufferSubmission, &_deviceResources->vmaAllocator);

	_deviceResources->mainCommandBuffers[0]->end();

	if (requiresCommandBufferSubmission)
	{
		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &_deviceResources->mainCommandBuffers[0];
		submitInfo.numCommandBuffers = 1;

		// submit the queue and wait for it to become idle
		_deviceResources->queue->submit(&submitInfo, 1);
		_deviceResources->queue->waitIdle();
	}

	// Initialize the vertex buffer data for the floor: 3*Position data, 3* normal data
	const glm::vec2 maxCorner(40, 40);
	const float afVertexBufferData[] = { -maxCorner.x, 0.0f, -maxCorner.y, 0.0f, 1.0f, 0.0f, -maxCorner.x, 0.0f, maxCorner.y, 0.0f, 1.0f, 0.0f, maxCorner.x, 0.0f, -maxCorner.y,
		0.0f, 1.0f, 0.0f, maxCorner.x, 0.0f, maxCorner.y, 0.0f, 1.0f, 0.0f };

	_deviceResources->passFloor.vbo =
		pvr::utils::createBuffer(_deviceResources->device, sizeof(afVertexBufferData), pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			&_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	pvr::utils::updateHostVisibleBuffer(_deviceResources->passFloor.vbo, afVertexBufferData, 0, sizeof(afVertexBufferData), true);
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occured
\brief  Loads and compiles the shaders and links the shader programs required for this training course
***********************************************************************************************************************/
void VulkanParticleSystem::createPipelines()
{
	pvrvk::ShaderModule fragShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::FragShaderSrcFile)->readToEnd<uint32_t>()));
	// Sphere Pipeline
	{
		pvrvk::ShaderModule vertShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::VertShaderSrcFile)->readToEnd<uint32_t>()));
		const pvr::utils::VertexBindings attributes[] = { { "POSITION", 0 }, { "NORMAL", 1 } };
		pvrvk::GraphicsPipelineCreateInfo pipeCreateInfo;
		pipeCreateInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()),
				static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
			pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));

		pipeCreateInfo.vertexShader.setShader(vertShader);
		pipeCreateInfo.fragmentShader.setShader(fragShader);

		pipeCreateInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipeCreateInfo.depthStencil.enableDepthWrite(true).enableDepthTest(true);
		pipeCreateInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		pipeCreateInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

		pvr::utils::populateInputAssemblyFromMesh(
			_scene->getMesh(0), attributes, sizeof(attributes) / sizeof(attributes[0]), pipeCreateInfo.vertexInput, pipeCreateInfo.inputAssembler);

		pipeCreateInfo.pipelineLayout = _deviceResources->device->createPipelineLayout(
			pvrvk::PipelineLayoutCreateInfo().addDescSetLayout(_deviceResources->descLayoutUboPerModel).addDescSetLayout(_deviceResources->descLayoutUbo));

		_deviceResources->passSphere.pipeline = _deviceResources->device->createGraphicsPipeline(pipeCreateInfo, _deviceResources->pipelineCache);
	}

	//  Floor Pipeline
	{
		pvrvk::ShaderModule vertShader =
			_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::FloorVertShaderSrcFile)->readToEnd<uint32_t>()));
		const pvrvk::VertexInputAttributeDescription attributes[] = { pvrvk::VertexInputAttributeDescription(0, 0, pvrvk::Format::e_R32G32B32_SFLOAT, 0),
			pvrvk::VertexInputAttributeDescription(1, 0, pvrvk::Format::e_R32G32B32_SFLOAT, sizeof(float) * 3) };
		pvrvk::GraphicsPipelineCreateInfo pipeCreateInfo;
		pipeCreateInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()),
				static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
			pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));
		pipeCreateInfo.vertexShader.setShader(vertShader);
		pipeCreateInfo.fragmentShader.setShader(fragShader);

		pipeCreateInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipeCreateInfo.depthStencil.enableDepthWrite(true).enableDepthTest(true);
		pipeCreateInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipeCreateInfo.depthStencil.enableDepthWrite(true).enableDepthTest(true);
		pipeCreateInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		pipeCreateInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);
		pipeCreateInfo.vertexInput.addInputAttributes(attributes, sizeof(attributes) / sizeof(attributes[0])).addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(float_t) * 6));

		pipeCreateInfo.pipelineLayout = _deviceResources->device->createPipelineLayout(pvrvk::PipelineLayoutCreateInfo().addDescSetLayout(_deviceResources->descLayoutUbo));

		pipeCreateInfo.subpass = 0;

		_deviceResources->passFloor.pipeline = _deviceResources->device->createGraphicsPipeline(pipeCreateInfo, _deviceResources->pipelineCache);
	}

	//  Particle Pipeline
	{
		const pvrvk::VertexInputAttributeDescription attributes[] = { pvrvk::VertexInputAttributeDescription(Attributes::ParticlePositionArray, 0, pvrvk::Format::e_R32G32B32_SFLOAT, 0),
			pvrvk::VertexInputAttributeDescription(Attributes::ParticleLifespanArray, 0, pvrvk::Format::e_R32_SFLOAT,
				static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4) + pvr::getSize(pvr::GpuDatatypes::vec3))) };

		pvrvk::GraphicsPipelineCreateInfo pipeCreateInfo;

		pipeCreateInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()),
				static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
			pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));

		pipeCreateInfo.colorBlend.setAttachmentState(0,
			pvrvk::PipelineColorBlendAttachmentState(
				true, pvrvk::BlendFactor::e_SRC_ALPHA, pvrvk::BlendFactor::e_ONE, pvrvk::BlendOp::e_ADD, pvrvk::BlendFactor::e_ZERO, pvrvk::BlendFactor::e_ONE));

		pipeCreateInfo.depthStencil.enableDepthWrite(true).enableDepthTest(true);

		pipeCreateInfo.vertexShader.setShader(
			_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::ParticleShaderVertSrcFile)->readToEnd<uint32_t>())));

		pipeCreateInfo.fragmentShader.setShader(
			_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::ParticleShaderFragSrcFile)->readToEnd<uint32_t>())));

		pipeCreateInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		pipeCreateInfo.vertexInput.addInputAttributes(attributes, sizeof(attributes) / sizeof(attributes[0]));
		pipeCreateInfo.vertexInput.addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(Particle)));

		pipeCreateInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_POINT_LIST);
		pipeCreateInfo.pipelineLayout = _deviceResources->device->createPipelineLayout(pvrvk::PipelineLayoutCreateInfo().addDescSetLayout(_deviceResources->descLayoutUbo));
		_deviceResources->passParticles.pipeline = _deviceResources->device->createGraphicsPipeline(pipeCreateInfo, _deviceResources->pipelineCache);
	}
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initApplication() will be called by the Shell once per run, before the rendering context is created.
	Used to initialize variables that are not dependent on it  (e.g. external modules, loading meshes, etc.)
	If the rendering context is lost, InitApplication() will not be called again.
***********************************************************************************************************************/
pvr::Result VulkanParticleSystem::initApplication()
{
	// Load the _scene
	_scene = pvr::assets::Model::createWithReader(pvr::assets::PODReader(getAssetStream(Files::SphereModelFile)));

	_frameId = 0;

	for (uint32_t i = 0; i < _scene->getNumMeshes(); ++i)
	{
		_scene->getMesh(i).setVertexAttributeIndex("POSITION0", Attributes::VertexArray);
		_scene->getMesh(i).setVertexAttributeIndex("NORMAL0", Attributes::NormalArray);
		_scene->getMesh(i).setVertexAttributeIndex("UV0", Attributes::TexCoordArray);
	}

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in quitApplication() will be called by the Shell once per run, just before exiting the program.
	  If the rendering context is lost, QuitApplication() will not be called.
***********************************************************************************************************************/
pvr::Result VulkanParticleSystem::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

void VulkanParticleSystem::createDescriptors()
{
	pvrvk::DescriptorSetLayoutCreateInfo descLayoutInfo;
	// create dynamic ubo descriptor set layout
	{
		descLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		_deviceResources->descLayoutUboPerModel = _deviceResources->device->createDescriptorSetLayout(descLayoutInfo);
	}
	// create static ubo descriptor set layout
	{
		descLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		_deviceResources->descLayoutUbo = _deviceResources->device->createDescriptorSetLayout(descLayoutInfo);
	}

	{
		_deviceResources->passSphere.uboPerModelBufferView.initDynamic(Configuration::SpherePipeUboMapping,
			Configuration::NumberOfSpheres * _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->passSphere.uboPerModel = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->passSphere.uboPerModelBufferView.getSize(),
			pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			&_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->passSphere.uboPerModelBufferView.pointToMappedMemory(_deviceResources->passSphere.uboPerModel->getDeviceMemory()->getMappedData());
	}

	{
		_deviceResources->passFloor.uboPerModelBufferView.initDynamic(Configuration::FloorPipeUboMapping, _deviceResources->swapchain->getSwapchainLength(),
			pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->passFloor.uboPerModel = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->passFloor.uboPerModelBufferView.getSize(),
			pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			&_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->passFloor.uboPerModelBufferView.pointToMappedMemory(_deviceResources->passFloor.uboPerModel->getDeviceMemory()->getMappedData());
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("uLightPosition", pvr::GpuDatatypes::vec3);

		_deviceResources->passSphere.uboLightPropBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->passSphere.uboLightProp = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->passSphere.uboLightPropBufferView.getSize(),
			pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			&_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->passSphere.uboLightPropBufferView.pointToMappedMemory(_deviceResources->passSphere.uboLightProp->getDeviceMemory()->getMappedData());
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("uModelViewProjectionMatrix", pvr::GpuDatatypes::mat4x4);

		_deviceResources->passParticles.uboMvpBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->passParticles.uboMvp = pvr::utils::createBuffer(_deviceResources->device, _deviceResources->passParticles.uboMvpBufferView.getSize(),
			pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT, pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			&_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->passParticles.uboMvpBufferView.pointToMappedMemory(_deviceResources->passParticles.uboMvp->getDeviceMemory()->getMappedData());
	}

	pvrvk::WriteDescriptorSet descSetWrites[pvrvk::FrameworkCaps::MaxSwapChains * 4];
	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();
	for (uint32_t i = 0; i < swapchainLength; ++i)
	{
		// sphere descriptors
		{
			// create the ubo dynamic descriptor set
			_deviceResources->passSphere.descriptoruboPerModel[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descLayoutUboPerModel);

			descSetWrites[i * 4]
				.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->passSphere.descriptoruboPerModel[i], 0)
				.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->passSphere.uboPerModel, 0, _deviceResources->passSphere.uboPerModelBufferView.getDynamicSliceSize()));

			// create the ubo static descriptor set
			_deviceResources->passSphere.descriptorLighProp[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descLayoutUbo);

			descSetWrites[i * 4 + 1]
				.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->passSphere.descriptorLighProp[i], 0)
				.setBufferInfo(0,
					pvrvk::DescriptorBufferInfo(_deviceResources->passSphere.uboLightProp, _deviceResources->passSphere.uboLightPropBufferView.getDynamicSliceOffset(i),
						_deviceResources->passSphere.uboLightPropBufferView.getDynamicSliceSize()));
		}

		// particle descriptor
		{
			_deviceResources->passParticles.descriptorMvp[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descLayoutUbo);

			descSetWrites[i * 4 + 2]
				.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->passParticles.descriptorMvp[i], 0)
				.setBufferInfo(0,
					pvrvk::DescriptorBufferInfo(_deviceResources->passParticles.uboMvp, _deviceResources->passParticles.uboMvpBufferView.getDynamicSliceOffset(i),
						_deviceResources->passParticles.uboMvpBufferView.getDynamicSliceSize()));
		}

		// floor descriptors
		{
			// create the ubo dynamic descriptor set
			_deviceResources->passFloor.descriptorUbo[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descLayoutUbo);
			descSetWrites[i * 4 + 3]
				.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->passFloor.descriptorUbo[i], 0)
				.setBufferInfo(0,
					pvrvk::DescriptorBufferInfo(_deviceResources->passFloor.uboPerModel, _deviceResources->passFloor.uboPerModelBufferView.getDynamicSliceOffset(i),
						_deviceResources->passFloor.uboPerModelBufferView.getDynamicSliceSize()));
		}
	}
	_deviceResources->device->updateDescriptorSets(descSetWrites, swapchainLength * 4, nullptr, 0);
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in initView() will be called by the Shell upon initialization or after a change in the rendering context.
	Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.)
***********************************************************************************************************************/
pvr::Result VulkanParticleSystem::initView()
{
	_deviceResources = std::unique_ptr<DeviceResources>(new DeviceResources(*this));

	// Create instance and retrieve compatible physical devices
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName());

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	_deviceResources->surface = pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	const pvr::utils::QueuePopulateInfo queueFlags[] = {
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT | pvrvk::QueueFlags::e_COMPUTE_BIT, _deviceResources->surface } // request for graphics, compute and presentation
	};

	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queueFlags, ARRAY_SIZE(queueFlags), &queueAccessInfo);

	// get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// create the commandpool
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(queueAccessInfo.familyId, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	// create the descriptor pool
	pvrvk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
		.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
		.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 16)
		.addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_BUFFER, 16)
		.setMaxDescriptorSets(16);

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(poolInfo);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(_deviceResources->surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}

	// create the swapchain
	pvr::utils::createSwapchainAndDepthStencilImageAndViews(_deviceResources->device, _deviceResources->surface, getDisplayAttributes(), _deviceResources->swapchain,
		_deviceResources->depthStencilImages, swapchainImageUsage, pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT,
		&_deviceResources->vmaAllocator);

	// create the on screen framebuffer
	pvr::utils::createOnscreenFramebufferAndRenderPass(_deviceResources->swapchain, &_deviceResources->depthStencilImages[0], _deviceResources->onScreenFramebuffer);

	// create the per swpapchain commandbuffers, semaphores and fence.
	// trasform the swachain & depthstencil image initial layout.
	for (uint8_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->mainCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->graphicsCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->uiRendererCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	// Initialize UIRenderer textures
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	std::string errorStr;
	_deviceResources->particleSystemGPU.init(Configuration::MaxNoParticles, Configuration::Spheres, Configuration::NumberOfSpheres, _deviceResources->device,
		_deviceResources->commandPool, _deviceResources->descriptorPool, _deviceResources->swapchain->getSwapchainLength(), _deviceResources->vmaAllocator,
		_deviceResources->pipelineCache);

	//  Create the Buffers
	createBuffers();

	createDescriptors();

	//  Load and compile the shaders & link programs
	createPipelines();

	// Create view matrices
	_mLightView = glm::lookAt(glm::vec3(0.0f, 80.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

	// Creates the projection matrix.
	_projMtx = pvr::math::perspectiveFov(
		pvr::Api::Vulkan, glm::pi<float>() / 3.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), Configuration::CameraNear, Configuration::CameraFar);

	// Create a bias matrix
	_mBiasMatrix = glm::mat4(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f);

	_deviceResources->particleSystemGPU.setGravity(glm::vec3(0.f, -9.81f, 0.f));
	_deviceResources->particleSystemGPU.setNumberOfParticles(Configuration::InitialNoParticles, _deviceResources->queue, _deviceResources->vmaAllocator);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("ParticleSystem");
	_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", Configuration::InitialNoParticles));
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action1: Pause rotation\nLeft: Decrease particles\n"
															   "Right: Increase particles");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	recordCommandBuffers();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Code in releaseView() will be called by pvr::Shell when the application quits or before a change in the rendering context.
***********************************************************************************************************************/
pvr::Result VulkanParticleSystem::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\return Return pvr::Result::Success if no error occurred
\brief  Main rendering loop function of the program. The shell will call this function every frame.
***********************************************************************************************************************/
pvr::Result VulkanParticleSystem::renderFrame()
{
	// wait & reset the fence for acquiring new image
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	if (!_isCameraPaused)
	{
		static float angle = 0;
		angle += getFrameTime() / 5000.0f;
		glm::vec3 vFrom = glm::vec3(sinf(angle) * 50.0f, 30.0f, cosf(angle) * 50.0f);

		_viewMtx = glm::lookAt(vFrom, glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		_viewIT = glm::inverseTranspose(glm::mat3(_viewMtx));
		_lightPos = glm::vec3(_viewMtx * glm::vec4(Configuration::LightPosition, 1.0f));
		_viewProjMtx = _projMtx * _viewMtx;
	}

	// Render floor
	updateParticleUniforms();
	updateFloor();
	updateSpheres();

	// submit the compute
	// the compute need to wait on the particle update
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->mainCommandBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			&_deviceResources->vmaAllocator, &_deviceResources->vmaAllocator);
	}

	// present
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->queue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/*!*********************************************************************************************************************
\brief  Updates the memory from where the command buffer will read the values to update the uniforms for the spheres
\param[in] proj projection matrix
\param[in] view view matrix
***********************************************************************************************************************/
void VulkanParticleSystem::updateSpheres()
{
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	pvr::utils::StructuredBufferView& bufferView = _deviceResources->passSphere.uboPerModelBufferView;

	{
		glm::mat4 modelView;
		for (uint32_t i = 0; i < Configuration::NumberOfSpheres; ++i)
		{
			uint32_t dynamicSlice = i + swapchainIndex * Configuration::NumberOfSpheres;

			const glm::vec3& position = Configuration::Spheres[i].vPosition;
			float radius = Configuration::Spheres[i].fRadius;
			modelView = _viewMtx * glm::translate(position) * glm::scale(glm::vec3(radius));
			bufferView.getElement(Configuration::SpherePipeDynamicUboElements::ModelViewMatrix, 0, dynamicSlice).setValue(modelView);
			bufferView.getElement(Configuration::SpherePipeDynamicUboElements::ModelViewProjectionMatrix, 0, dynamicSlice).setValue(_projMtx * modelView);
			bufferView.getElement(Configuration::SpherePipeDynamicUboElements::ModelViewITMatrix, 0, dynamicSlice).setValue(glm::mat3x4(glm::inverseTranspose(glm::mat3(modelView))));
		}

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->passSphere.uboPerModel->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->passSphere.uboPerModel->getDeviceMemory()->flushRange(
				bufferView.getDynamicSliceOffset(swapchainIndex * Configuration::NumberOfSpheres), bufferView.getDynamicSliceSize() * Configuration::NumberOfSpheres);
		}
	}

	_deviceResources->passSphere.uboLightPropBufferView.getElement(0, 0, swapchainIndex).setValue(_lightPos);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->passSphere.uboLightProp->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->passSphere.uboLightProp->getDeviceMemory()->flushRange(
			_deviceResources->passSphere.uboLightPropBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->passSphere.uboLightPropBufferView.getDynamicSliceSize());
	}
}

/*!*********************************************************************************************************************
\brief  Updates the memory from where the commandbuffer will read the values to update the uniforms for the floor
***********************************************************************************************************************/
void VulkanParticleSystem::updateFloor()
{
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	pvr::utils::StructuredBufferView& uboView = _deviceResources->passFloor.uboPerModelBufferView;

	uboView.getElement(Configuration::FloorPipeDynamicUboElements::ModelViewMatrix, 0, swapchainIndex).setValue(_viewMtx);
	uboView.getElement(Configuration::FloorPipeDynamicUboElements::ModelViewProjectionMatrix, 0, swapchainIndex).setValue(_viewProjMtx);
	uboView.getElement(Configuration::FloorPipeDynamicUboElements::ModelViewITMatrix, 0, swapchainIndex).setValue(_viewIT);
	uboView.getElement(Configuration::FloorPipeDynamicUboElements::LightPos, 0, swapchainIndex).setValue(_lightPos);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->passFloor.uboPerModel->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->passFloor.uboPerModel->getDeviceMemory()->flushRange(uboView.getDynamicSliceOffset(swapchainIndex), uboView.getDynamicSliceSize());
	}
}

/*!*********************************************************************************************************************
\brief  Updates particle positions and attributes, e.g. lifespan, position, velocity etc.
	Will update the buffer that was "just used" as the Input, as output, so that we can exploit more GPU parallelization.
************************************************************************************************************************/
void VulkanParticleSystem::updateParticleUniforms()
{
	uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	float dt = static_cast<float>(getFrameTime());

	static float rot_angle = 0.0f;
	rot_angle += dt / 500.0f;
	float el_angle = (sinf(rot_angle / 4.0f) + 1.0f) * 0.2f + 0.2f;

	glm::mat4 rot = glm::rotate(rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 skew = glm::rotate(el_angle, glm::vec3(0.0f, 0.0f, 1.0f));

	Emitter sEmitter(rot * skew, 1.3f, 1.0f);

	_deviceResources->particleSystemGPU.setEmitter(sEmitter);
	_deviceResources->particleSystemGPU.updateUniforms(swapchainIndex, dt);

	_deviceResources->passParticles.uboMvpBufferView.getElement(0, 0, swapchainIndex).setValue(_viewProjMtx);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->passParticles.uboMvp->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->passParticles.uboMvp->getDeviceMemory()->flushRange(
			_deviceResources->passParticles.uboMvpBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->passParticles.uboMvpBufferView.getDynamicSliceSize());
	}
}

/*!*********************************************************************************************************************
\brief  Pre record the rendering commands
***********************************************************************************************************************/
void VulkanParticleSystem::recordCommandBuffers()
{
	for (uint8_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		recordCommandBuffer(i);
	}
}

/*!*********************************************************************************************************************
\brief  Record the commands buffer
\param  idx Commandbuffer index
***********************************************************************************************************************/
void VulkanParticleSystem::recordCommandBuffer(uint8_t swapchain)
{
	_deviceResources->graphicsCommandBuffers[swapchain]->begin(_deviceResources->onScreenFramebuffer[swapchain], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	// Render floor
	recordCmdDrawFloor(swapchain);
	// render the spheres
	_deviceResources->graphicsCommandBuffers[swapchain]->bindPipeline(_deviceResources->passSphere.pipeline);
	_deviceResources->graphicsCommandBuffers[swapchain]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->passSphere.pipeline->getPipelineLayout(), 1, _deviceResources->passSphere.descriptorLighProp[0]);
	for (uint32_t i = 0; i < Configuration::NumberOfSpheres; i++)
	{
		recordCmdDrawSphere(i, swapchain);
	}

	// Render particles
	recordCmdDrawParticles(swapchain);

	_deviceResources->graphicsCommandBuffers[swapchain]->end();

	_deviceResources->uiRendererCommandBuffers[swapchain]->begin(_deviceResources->onScreenFramebuffer[swapchain], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
	_deviceResources->uiRenderer.beginRendering(_deviceResources->uiRendererCommandBuffers[swapchain], _deviceResources->onScreenFramebuffer[swapchain], true);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->uiRendererCommandBuffers[swapchain]->end();

	_deviceResources->mainCommandBuffers[swapchain]->begin();
	_deviceResources->mainCommandBuffers[swapchain]->executeCommands(_deviceResources->particleSystemGPU.getCommandBuffer(swapchain));

	pvrvk::MemoryBarrierSet memBarrierSet;
	memBarrierSet.addBarrier(pvrvk::BufferMemoryBarrier(pvrvk::AccessFlags::e_SHADER_WRITE_BIT, pvrvk::AccessFlags::e_VERTEX_ATTRIBUTE_READ_BIT,
		_deviceResources->particleSystemGPU.getParticleBufferView(), 0, static_cast<uint32_t>(_deviceResources->particleSystemGPU.getParticleBufferView()->getSize())));
	_deviceResources->mainCommandBuffers[swapchain]->pipelineBarrier(
		pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_VERTEX_INPUT_BIT, memBarrierSet, false);

	const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.0f), pvrvk::ClearValue::createDefaultDepthStencilClearValue() };
	_deviceResources->mainCommandBuffers[swapchain]->beginRenderPass(
		_deviceResources->onScreenFramebuffer[swapchain], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, clearValues, ARRAY_SIZE(clearValues));

	_deviceResources->mainCommandBuffers[swapchain]->executeCommands(_deviceResources->graphicsCommandBuffers[swapchain]);

	_deviceResources->mainCommandBuffers[swapchain]->executeCommands(_deviceResources->uiRendererCommandBuffers[swapchain]);

	_deviceResources->mainCommandBuffers[swapchain]->endRenderPass();
	_deviceResources->mainCommandBuffers[swapchain]->end();
}

/*!*********************************************************************************************************************
\brief  Record the draw particles commands
\param  idx Commandbuffer index
***********************************************************************************************************************/
void VulkanParticleSystem::recordCmdDrawParticles(uint8_t idx)
{
	_deviceResources->graphicsCommandBuffers[idx]->bindPipeline(_deviceResources->passParticles.pipeline);
	_deviceResources->graphicsCommandBuffers[idx]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->passParticles.pipeline->getPipelineLayout(), 0, _deviceResources->passParticles.descriptorMvp[idx]);
	_deviceResources->graphicsCommandBuffers[idx]->bindVertexBuffer(_deviceResources->particleSystemGPU.getParticleBufferView(), 0, 0);
	_deviceResources->graphicsCommandBuffers[idx]->draw(0, _deviceResources->particleSystemGPU.getNumberOfParticles(), 0, 1);
}

/*!*********************************************************************************************************************
\brief  Renders a sphere at the specified position.
\param[in] passSphere Sphere draw pass
\param[in] idx Commandbuffer index
***********************************************************************************************************************/
void VulkanParticleSystem::recordCmdDrawSphere(uint32_t sphereId, uint8_t swapchain)
{
	static const pvr::assets::Mesh& mesh = _scene->getMesh(0);
	uint32_t offset = _deviceResources->passSphere.uboPerModelBufferView.getDynamicSliceOffset(sphereId + swapchain * Configuration::NumberOfSpheres);
	_deviceResources->graphicsCommandBuffers[swapchain]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->passSphere.pipeline->getPipelineLayout(), 0,
		_deviceResources->passSphere.descriptoruboPerModel[swapchain], &offset, 1);

	_deviceResources->graphicsCommandBuffers[swapchain]->bindVertexBuffer(_deviceResources->passSphere.vbo, 0, 0);
	_deviceResources->graphicsCommandBuffers[swapchain]->bindIndexBuffer(_deviceResources->passSphere.ibo, 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
	// Indexed Triangle list
	_deviceResources->graphicsCommandBuffers[swapchain]->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
}

/*!*********************************************************************************************************************
\brief  Renders the floor as a quad.
\param idx Commandbuffer index
***********************************************************************************************************************/
void VulkanParticleSystem::recordCmdDrawFloor(uint8_t swapchain)
{
	// Enables depth testing
	// We need to calculate the texture projection matrix. This matrix takes the pixels from world space to previously rendered light projection space
	// where we can look up values from our saved depth buffer. The matrix is constructed from the light view and projection matrices as used for the previous render and
	// then multiplied by the inverse of the current view matrix.
	_deviceResources->graphicsCommandBuffers[swapchain]->bindPipeline(_deviceResources->passFloor.pipeline);
	_deviceResources->graphicsCommandBuffers[swapchain]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->passFloor.pipeline->getPipelineLayout(), 0, _deviceResources->passFloor.descriptorUbo[swapchain]);
	_deviceResources->graphicsCommandBuffers[swapchain]->bindVertexBuffer(_deviceResources->passFloor.vbo, 0, 0);
	// Draw the quad
	_deviceResources->graphicsCommandBuffers[swapchain]->draw(0, 4);
}

/*!*********************************************************************************************************************
\return Return a smart pointer to the application class.
\brief  This function must be implemented by the user of the shell. It should return the Application class (a class inheriting from pvr::Shell.
***********************************************************************************************************************/
std::unique_ptr<pvr::Shell> pvr::newDemo()
{
	return std::unique_ptr<pvr::Shell>(new VulkanParticleSystem());
}
