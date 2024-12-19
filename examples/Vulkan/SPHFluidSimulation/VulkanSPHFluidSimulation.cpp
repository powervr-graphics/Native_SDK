/*!
\brief SPH fluid simulation on the GPU
\file VulkanSPHFluidSimulation.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

/// <summary>Index to bind the attributes to vertex shaders.</summary>
namespace Attributes
{
	enum Enum
	{
		VertexArray = 0,
		NormalArray = 1,
		TexCoordArray = 2,
	};
}

/// <summary>Vertex shader used by the graphics part for drawing the particles.</summary>
const char FragShaderSphereSrcFile[] = "FragShaderSphere.fsh.spv";

/// <summary>Fragment shader used by the graphics part for drawing the particles.</summary>
const char VertShaderSphereSrcFile[] = "VertShaderSphere.vsh.spv";

/// <summary>One of the three compute shaders used for the particle simulation.</summary>
const char ComputeDensityPressureUpdate[] = "densityPressureUpdate.csh.spv";

/// <summary>One of the three compute shaders used for the particle simulation.</summary>
const char AccelerationUpdate[] = "accelerationUpdate.csh.spv";

/// <summary>One of the three compute shaders used for the particle simulation.</summary>
const char PositionUpdate[] = "positionUpdate.csh.spv";

/// <summary>POD scene file with the sphere model used to draw the particles.</summary>
const char SphereModelFile[] = "sphere.pod";

/// <summary>The particles are displayed on a tri-dimensional grid, and this value represents the number of particles for the x dimension.</summary>
const uint32_t numberParticlesPerDimensionX = 32;

/// <summary>The particles are displayed on a tri-dimensional grid, and this value represents the number of particles for the y dimension.</summary>
const uint32_t numberParticlesPerDimensionY = 8;

/// <summary>The particles are displayed on a tri-dimensional grid, and this value represents the number of particles for the z dimension.</summary>
const uint32_t numberParticlesPerDimensionZ = 16;

/// <summary>The total number of particles is the amount of particles per dimension to the power of three.</summary>
const uint32_t numberParticles = numberParticlesPerDimensionX * numberParticlesPerDimensionY * numberParticlesPerDimensionZ;

/// <summary>Binding index used in the instanced rendering of the particles for the vertex data buffer.</summary>
const uint32_t vertexBufferBindID = 0;

/// <summary>Binding index used in the instanced rendering of the particles for the instance data buffer, which is the buffer used to store the particle simulation data.</summary>
const uint32_t instanceBufferBindID = 1;

/// <summary>Parameter used in the particle simulation for particle radius.</summary>
const float particleSimulationRadius = 0.04f;

/// <summary>Parameter used in the particle simulation for particle core radius.</summary>
const float particleSimulationCoreRadius = particleSimulationRadius * 10.0f;

/// <summary>Parameter used in the particle simulation for particle mass.</summary>
const float particleSimulationMass = 100.0f;

/// <summary>Parameter used in the particle simulation for the fluid rest density ("The density of a small portion of a fluid in a Lorentz frame in which that portion of the fluid is at rest").</summary>
const float particleSimulationFluidRestDensity = 1.0f;

/// <summary>Parameter used in the particle simulation for particle stiffness.</summary>
const float particleSimulationFluidStiffness = 0.0025f;

/// <summary>Parameter used in the particle simulation for particle viscosity.</summary>
const float particleSimulationFluidViscosity = 0.1f;

/// <summary>Parameter used in the particle simulation for particle gravity.</summary>
const glm::vec3 particleSimulationGravity = glm::vec3(0.0f, -9.8f, 0.0f);

/// <summary>Parameter used in the particle simulation for particle speed decay.</summary>
const float particleSimulationSpeedDecay = 0.8f;

/// <summary>How many second until the particle simulation bounding x limit starts to be dynamically change to animate the scene.</summary>
const float simulationTimeUntilChangingBoundingX = 10.0f;

/// <summary>How many second each animation of the bounding X limit takes.</summary>
const float boundingXAnimationTime = 3.0f;

/// <summary>Struct with all the information needed to perform the particle simulation for eacxh particle.</summary>
struct Particle
{
	glm::vec4 positionDensity; // Particle position in xyz field, particle density in w field
	glm::vec4 velocityPressure; // Particle velocity in xyz field, particle pressure in the w field
};

/// <summary>Size in bytes of the particle buffer.</summary>
const uint32_t particleSystemBufferSize = sizeof(Particle) * numberParticles;

/// <summary>Struct containing all the Vulkan objects used in the sample.</summary>
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

	/// <summary>Graphics queue where to submit commands.</summary>
	pvrvk::Queue graphicsQueue;

	/// <summary>Compute queue where to submit commands.</summary>
	pvrvk::Queue computeQueue;

	/// <summary>vma memory allocator used to build some buffers.</summary>
	pvr::utils::vma::Allocator vmaAllocator;

	/// <summary>Semaphore signaled when the next swap chain image has been acquired.</summary>
	std::vector<pvrvk::Semaphore> imageAcquiredSemaphores;

	/// <summary>Semaphore signaled when the graphics command buffers of the scene to draw the particles.</summary>
	std::vector<pvrvk::Semaphore> presentationSemaphores;

	/// <summary>Semaphore signaled when the compute command buffer with the particle simulation dispatchs have finished.</summary>
	std::vector<pvrvk::Semaphore> computeSemaphores;

	/// <summary>Fence to wait in the host for the compute command buffers to complete execution.</summary>
	std::vector<pvrvk::Fence> computeFences;

	/// <summary>Fence to wait in the host for the graphics command buffers to complete execution.</summary>
	std::vector<pvrvk::Fence> graphicsFences;

	/// <summary>Command buffers where to record graphics commands.</summary>
	std::vector<pvrvk::CommandBuffer> graphicsCommandBuffers;

	/// <summary>Framebuffers to draw the scene used to present in the screen, one per swapchain image available.</summary>
	std::vector<pvrvk::Framebuffer> onScreenFramebuffer;

	/// <summary>Pipeline cache where to generate the graphics and compute pipelines used in the sample.</summary>
	pvrvk::PipelineCache pipelineCache;

	/// <summary>UIRenderer used to display text.</summary>
	pvr::ui::UIRenderer uiRenderer;

	/// <summary>Descriptor set layout for the compute shaders doing the particle fluid simulation.</summary>
	pvrvk::DescriptorSetLayout computeDescriptorSetLayout;

	/// <summary>Vector with all descriptor sets, recording one per swapchain, for the compute shaders doing the particle fluid simulation.</summary>
	std::vector<pvrvk::DescriptorSet> vectorComputeDescriptorSet;

	/// <summary>Descriptor pool where to get descriptor sets allocated from.</summary>
	pvrvk::DescriptorPool descriptorPool;

	/// <summary>Pipeline layout for the compute shaders doing the particle fluid simulation.</summary>
	pvrvk::PipelineLayout computePipelinelayout;

	/// <summary>Compute pipeline for one of the three shaders (density and pressure computations) used in the particle fluid simulation.</summary>
	pvrvk::ComputePipeline computePipelineDensityPressureUpdate;

	/// <summary>Compute ipeline for one of the three shaders (acceleration computations) used in the particle fluid simulation.</summary>
	pvrvk::ComputePipeline computePipelineAccelerationUpdate;

	/// <summary>Compute pipeline for one of the three shaders (position computations) used in the particle fluid simulation.</summary>
	pvrvk::ComputePipeline computePipelinePositionUpdate;

	/// <summary>Device only memory buffer where to store all the per particle information in the particle fluid simulation.</summary>
	pvrvk::Buffer particleBuffer;

	/// <summary>Buffer used in the buffer view where all the particle compute simulation scene settings are specified.</summary>
	pvrvk::Buffer computeSimulationSettingsBuffer;

	/// <summary>Buffer view where all the particle compute simulation scene settings are specified.</summary>
	pvr::utils::StructuredBufferView computeSimulationSettingsBufferView;

	/// <summary>Compute command buffers where all the commands for the compute shaders performing the particle simulation are recorded, one per swapchain.</summary>
	std::vector<pvrvk::CommandBuffer> computeCommandBuffers;

	/// <summary>Sphere 3D model vertex buffer object.</summary>
	pvrvk::Buffer sphereVertexBufferObject;

	/// <summary>Sphere 3D model index buffer object.</summary>
	pvrvk::Buffer sphereIndexBufferObject;

	/// <summary>Sphere graphics pipeline used for drawing the particles in the scene.</summary>
	pvrvk::GraphicsPipeline sphereDrawingPipeline;

	/// <summary>Descriptor set layout for the graphics part used to draw the particles in the scene.</summary>
	pvrvk::DescriptorSetLayout graphicsDescriptorSetLayout;

	/// <summary>Buffer used by the buffer view in the graphical commands used for scene particle drawing.</summary>
	pvrvk::Buffer sceneSettingsBuffer;

	/// <summary>Buffer view used in the graphical commands used for scene particle drawing.</summary>
	pvr::utils::StructuredBufferView sceneSettingsBufferView;

	/// <summary>Despriptor set used in the graphical commands for scene particle drawing.</summary>
	std::vector<pvrvk::DescriptorSet> vectorGraphicsDescriptorSet;

	/// <summary>Piepline layou used in the graphical commands for scene particle drawing.</summary>
	pvrvk::PipelineLayout graphicsPipelineLayout;

	/// <summary>Struct destructor.</summary>
	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
		}

		if (swapchain)
		{
			uint32_t l = swapchain->getSwapchainLength();

			for (uint32_t i = 0; i < l; ++i)
			{
				if (graphicsFences[i]) { graphicsFences[i]->wait(); }
				if (computeFences[i]) { computeFences[i]->wait(); }
			}
		}
	}
};

/// <summary>Class implementing the Shell functions.</summary>
class VulkanSPHFluidSimulation: public pvr::Shell
{
	/// <summary>Scene with the 3D sphere model representing a particle used for drawing the particles.</summary>
	pvr::assets::ModelHandle _sphereScene;

	/// <summary>View projection matrix.</summary>
	glm::mat4 _viewProj;

	/// <summary>Frame counter module the amount of swapchain images, to pick the correct index command buffers.</summary>
	uint32_t _frameId;

	/// <summary>Vulkan objects.</summary>
	std::unique_ptr<DeviceResources> _deviceResources;

	/// <summary>Initial value for the bounds in the x dimension for the square volume where the particle simulation is being performed.</summary>
	float _initialBoundingX;

	/// <summary>Bounds in the x dimension for the volume where the particle simulation is being performed.</summary>
	float _lowerBoundX;

	/// <summary>Bounds in the x dimension for the volume where the particle simulation is being performed.</summary>
	float _upperBoundX;

	/// <summary>Bounds in the y dimension for the volume where the particle simulation is being performed.</summary>
	float _lowerBoundY;

	/// <summary>Bounds in the y dimension for the volume where the particle simulation is being performed.</summary>
	float _upperBoundY;

	/// <summary>Bounds in the z dimension for the volume where the particle simulation is being performed.</summary>
	float _lowerBoundZ;

	/// <summary>Bounds in the z dimension for the volume where the particle simulation is being performed.</summary>
	float _upperBoundZ;

	/// <summary>Number of swapchain images when running the sample on a specific device.</summary>
	uint32_t _swapchainLength;

	/// <summary>Flag for the logic of the bounding x animation.</summary>
	bool _boundingXGoingToLowerValue;

	/// <summary>Helper variable for the logic of the bounding x animation.</summary>
	float _boundingXAnimationAccumulatedTime;

	/// <summary>Accumulayed time since the first call to updateSettingsBufferViews.</summary>
	float _accumulatedExecutionTime;
	
public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void buildDescriptorPool();
	void buildComputeDescriptorSetLayout();
	void buildSphereParticleDescriptorSetLayout();
	void buildComputePipelines();
	void buildParticleBuffer(pvrvk::CommandBuffer commandBuffer);
	void buildParticleSimulationSettingsBuffer();
	void buildParticleDrawingBuffer();
	void updateComputeDescriptorSet();
	void updateGraphicsDescriptorSet();
	void buildSphereVertexIndexBuffer(pvrvk::CommandBuffer commandBuffer);
	void buildSphereDrawingPipeline();
	void buildGraphicsPipelineLayout();
	void recordGraphicsCommandBuffer();
	void recordComputeCommandBuffer();
	void recordDrawMeshCommandBuffer(pvrvk::CommandBuffer commandBuffer, uint32_t swapchainIndex);
	void animateSimulationBoundingX(float deltaTime);
	void updateSettingsBufferViews();
};

/// <summary>Build the descriptor pool used to allocate descriptor sets.</summary>
void VulkanSPHFluidSimulation::buildDescriptorPool()
{
	pvrvk::DescriptorPoolCreateInfo descPoolCreateInfo;
	descPoolCreateInfo.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, static_cast<uint16_t>(_swapchainLength * 2));
	descPoolCreateInfo.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, static_cast<uint16_t>(_swapchainLength * 2));
	descPoolCreateInfo.addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_BUFFER, static_cast<uint16_t>(_swapchainLength * 2));
	descPoolCreateInfo.setMaxDescriptorSets(static_cast<uint16_t>(_swapchainLength * 2));
	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(descPoolCreateInfo);
	_deviceResources->descriptorPool->setObjectName("DescriptorPool");
}

/// <summary>Build the descriptor set layout used in the compute pipeline and pipeline for the particle simulation.</summary>
void VulkanSPHFluidSimulation::buildComputeDescriptorSetLayout()
{
	pvrvk::DescriptorSetLayoutCreateInfo layoutCreateInfo;

	layoutCreateInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT); // Binding 0 will have computeSimulationSettingsBuffer buffer
	layoutCreateInfo.setBinding(1, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT); // Binding 1 will have particleBuffer buffer
	_deviceResources->computeDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(layoutCreateInfo);
}

/// <summary>Build the descriptor set layout used in the graphics pipeline to draw the particles.</summary>
void VulkanSPHFluidSimulation::buildSphereParticleDescriptorSetLayout()
{
	pvrvk::DescriptorSetLayoutCreateInfo layoutCreateInfo;

	layoutCreateInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); // Binding 0 will have sceneSettingsBuffer buffer
	_deviceResources->graphicsDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(layoutCreateInfo);
}

/// <summary>Build the three compute pipelines needed for the particle simulation, which share the same pipeline layout.</summary>
void VulkanSPHFluidSimulation::buildComputePipelines()
{
	{
		pvrvk::PipelineLayoutCreateInfo createInfo;
		createInfo.addDescSetLayout(_deviceResources->computeDescriptorSetLayout);
		_deviceResources->computePipelinelayout = _deviceResources->device->createPipelineLayout(createInfo);
	}

	{
		pvrvk::ShaderModule computeShaderDensityPressureUpdate =
			_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(ComputeDensityPressureUpdate)->readToEnd<uint32_t>()));

		pvrvk::ComputePipelineCreateInfo createInfo;
		createInfo.computeShader.setShader(computeShaderDensityPressureUpdate);
		createInfo.pipelineLayout = _deviceResources->computePipelinelayout;
		_deviceResources->computePipelineDensityPressureUpdate = _deviceResources->device->createComputePipeline(createInfo, _deviceResources->pipelineCache);		
		_deviceResources->computePipelineDensityPressureUpdate->setObjectName("DensityPressureUpdateComputePipeline");
	}

	{
		pvrvk::ShaderModule computeAccelerationUpdate =
			_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(AccelerationUpdate)->readToEnd<uint32_t>()));

		pvrvk::ComputePipelineCreateInfo createInfo;
		createInfo.computeShader.setShader(computeAccelerationUpdate);
		createInfo.pipelineLayout = _deviceResources->computePipelinelayout;
		_deviceResources->computePipelineAccelerationUpdate = _deviceResources->device->createComputePipeline(createInfo, _deviceResources->pipelineCache);
		_deviceResources->computePipelineAccelerationUpdate->setObjectName("AccelerationUpdateComputePipeline");
	}

	{
		pvrvk::ShaderModule computePositionUpdate = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(PositionUpdate)->readToEnd<uint32_t>()));

		pvrvk::ComputePipelineCreateInfo createInfo;
		createInfo.computeShader.setShader(computePositionUpdate);
		createInfo.pipelineLayout = _deviceResources->computePipelinelayout;
		_deviceResources->computePipelinePositionUpdate = _deviceResources->device->createComputePipeline(createInfo, _deviceResources->pipelineCache);
		_deviceResources->computePipelinePositionUpdate->setObjectName("PositionUpdateComputePipeline");
	}
}

/// <summary>Build a storage buffer which will contain all the particle information and initialise it.</summary>
/// <param name="commandBuffer">command buffer to record commands required to build the buffer</param>
void VulkanSPHFluidSimulation::buildParticleBuffer(pvrvk::CommandBuffer commandBuffer)
{
	_deviceResources->particleBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(particleSystemBufferSize),
			pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT |
				pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, nullptr, pvr::utils::vma::AllocationCreateFlags::e_NONE);
	_deviceResources->particleBuffer->setObjectName("ParticleSBO");

	std::vector<Particle> vectorParticleInitial(numberParticles);
	int halfNumberParticlesPerDimensionX = numberParticlesPerDimensionX / 2;
	int halfNumberParticlesPerDimensionY = numberParticlesPerDimensionY / 2;
	int halfNumberParticlesPerDimensionZ = numberParticlesPerDimensionZ / 2;
	float particleOffset = 4.0f * (2.0f * particleSimulationRadius);
	uint32_t counter = 0;

	float minX =  1.0f * float(numberParticlesPerDimensionX) * particleOffset;
	float minY =  1.0f * float(numberParticlesPerDimensionX) * particleOffset;
	float minZ =  1.0f * float(numberParticlesPerDimensionY) * particleOffset;
	float maxX = -1.0f * float(numberParticlesPerDimensionY) * particleOffset;
	float maxY = -1.0f * float(numberParticlesPerDimensionZ) * particleOffset;
	float maxZ = -1.0f * float(numberParticlesPerDimensionZ) * particleOffset;

	for (int i = -halfNumberParticlesPerDimensionX; i < halfNumberParticlesPerDimensionX; ++i)
	{
		for (int j = -halfNumberParticlesPerDimensionY; j < halfNumberParticlesPerDimensionY; ++j)
		{
			for (int k = -halfNumberParticlesPerDimensionZ; k < halfNumberParticlesPerDimensionZ; ++k)
			{
				glm::vec4 position = glm::vec4(particleOffset * i + particleOffset * 0.5f, particleOffset * j + particleOffset * 0.5f, particleOffset * k + particleOffset * 0.5f, 1.0f);
				vectorParticleInitial[counter].positionDensity = position;
				vectorParticleInitial[counter].velocityPressure = glm::vec4(0.0f);
				counter++;

				minX = glm::min(minX, position.x);
				minY = glm::min(minY, position.y);
				minZ = glm::min(minZ, position.z);

				maxX = glm::max(maxX, position.x);
				maxY = glm::max(maxY, position.y);
				maxZ = glm::max(maxZ, position.z);
			}
		}
	}

	_lowerBoundX = minX;
	_upperBoundX = maxX;
	_lowerBoundY = -5.0f;
	_upperBoundY = maxY;
	_lowerBoundZ = minZ;
	_upperBoundZ = maxZ;
	_initialBoundingX = _upperBoundX;

	pvr::utils::updateBufferUsingStagingBuffer(_deviceResources->device, _deviceResources->particleBuffer, commandBuffer, vectorParticleInitial.data(), 0, particleSystemBufferSize, nullptr);
}

/// <summary>Build a buffer with the particle simulation settings, together with a structured view of it to change values from the host.</summary>
void VulkanSPHFluidSimulation::buildParticleSimulationSettingsBuffer()
{
	// Uniform buffer with the scene settings
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("numberParticles", pvr::GpuDatatypes::Integer);
	desc.addElement("deltaTime", pvr::GpuDatatypes::Float);
	desc.addElement("lowerBoundX", pvr::GpuDatatypes::Float);
	desc.addElement("upperBoundX", pvr::GpuDatatypes::Float);
	desc.addElement("lowerBoundY", pvr::GpuDatatypes::Float);
	desc.addElement("upperBoundY", pvr::GpuDatatypes::Float);
	desc.addElement("lowerBoundZ", pvr::GpuDatatypes::Float);
	desc.addElement("upperBoundZ", pvr::GpuDatatypes::Float);
	desc.addElement("radius", pvr::GpuDatatypes::Float);
	desc.addElement("coreRadius", pvr::GpuDatatypes::Float);
	desc.addElement("mass", pvr::GpuDatatypes::Float);
	desc.addElement("fluidRestDensity", pvr::GpuDatatypes::Float);
	desc.addElement("fluidStiffness", pvr::GpuDatatypes::Float);
	desc.addElement("fluidViscosity", pvr::GpuDatatypes::Float);
	desc.addElement("speedDecay", pvr::GpuDatatypes::Float);
	desc.addElement("gravity", pvr::GpuDatatypes::vec3);
	
	_deviceResources->computeSimulationSettingsBufferView.initDynamic(desc, _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->computeSimulationSettingsBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->computeSimulationSettingsBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator,
		pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->computeSimulationSettingsBuffer->setObjectName("ComputeSimulationSettingsUBO");

	_deviceResources->computeSimulationSettingsBufferView.pointToMappedMemory(_deviceResources->computeSimulationSettingsBuffer->getDeviceMemory()->getMappedData());

	// Update those values which remain constant during the particle simulation
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("numberParticles", 0, i).setValue(numberParticles);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("lowerBoundX", 0, i).setValue(_lowerBoundX);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("upperBoundX", 0, i).setValue(_upperBoundX);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("lowerBoundY", 0, i).setValue(_lowerBoundY);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("upperBoundY", 0, i).setValue(_upperBoundY);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("lowerBoundZ", 0, i).setValue(_lowerBoundZ);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("upperBoundZ", 0, i).setValue(_upperBoundZ);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("radius", 0, i).setValue(particleSimulationRadius);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("coreRadius", 0, i).setValue(particleSimulationCoreRadius);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("mass", 0, i).setValue(particleSimulationMass);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("fluidRestDensity", 0, i).setValue(particleSimulationFluidRestDensity);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("fluidStiffness", 0, i).setValue(particleSimulationFluidStiffness);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("fluidViscosity", 0, i).setValue(particleSimulationFluidViscosity);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("speedDecay", 0, i).setValue(particleSimulationSpeedDecay);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("gravity", 0, i).setValue(particleSimulationGravity);

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->computeSimulationSettingsBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->computeSimulationSettingsBuffer->getDeviceMemory()->flushRange(_deviceResources->computeSimulationSettingsBufferView.getDynamicSliceOffset(i),
				_deviceResources->computeSimulationSettingsBufferView.getDynamicSliceSize());
		}
	}
}

/// <summary>Build a buffer with the particle drawing settings, together with a structured view of it to change values from the host.</summary>
void VulkanSPHFluidSimulation::buildParticleDrawingBuffer()
{
	pvr::utils::StructuredMemoryDescription desc;
	desc.addElement("viewProjectionMatrix", pvr::GpuDatatypes::mat4x4);
	desc.addElement("modelMatrix", pvr::GpuDatatypes::mat4x4);

	_deviceResources->sceneSettingsBufferView.initDynamic(desc, _swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
		static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

	_deviceResources->sceneSettingsBuffer = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(_deviceResources->sceneSettingsBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	_deviceResources->sceneSettingsBuffer->setObjectName("SceneSettingsUBO");

	_deviceResources->sceneSettingsBufferView.pointToMappedMemory(_deviceResources->sceneSettingsBuffer->getDeviceMemory()->getMappedData());

	// Update the buffer with those values which stay constant
	glm::mat4 modelMatrix = glm::scale(glm::vec3(0.175f));

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->sceneSettingsBufferView.getElementByName("modelMatrix", 0, i).setValue(modelMatrix);

		// if the memory property flags used by the buffers' device memory does not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->sceneSettingsBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->sceneSettingsBuffer->getDeviceMemory()->flushRange(
				_deviceResources->sceneSettingsBufferView.getDynamicSliceOffset(i), _deviceResources->sceneSettingsBufferView.getDynamicSliceSize());
		}
	}
}

/// <summary>Update the compute descriptor sets (one per swapchain). The same descriptor set is used for the three compute dispatches for the particle simulation.</summary>
void VulkanSPHFluidSimulation::updateComputeDescriptorSet()
{
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	_deviceResources->vectorComputeDescriptorSet.resize(_swapchainLength);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorComputeDescriptorSet[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->computeDescriptorSetLayout);
		_deviceResources->vectorComputeDescriptorSet[i]->setObjectName("ComputeSpwachain" + std::to_string(i) + "DescriptorSet");

		// Binding 0: computeSimulationSettingsBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoUniform = pvrvk::DescriptorBufferInfo(_deviceResources->computeSimulationSettingsBuffer, 0, _deviceResources->computeSimulationSettingsBufferView.getDynamicSliceSize());
		pvrvk::WriteDescriptorSet writeDescriptorSetUniform = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->vectorComputeDescriptorSet[i], 0);
		writeDescriptorSetUniform.setBufferInfo(0, descriptorBufferInfoUniform);

		// Binding 1: particleBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoStorage = pvrvk::DescriptorBufferInfo(_deviceResources->particleBuffer, 0, particleSystemBufferSize);
		pvrvk::WriteDescriptorSet writeDescriptorSetStorage = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, _deviceResources->vectorComputeDescriptorSet[i], 1);
		writeDescriptorSetStorage.setBufferInfo(0, descriptorBufferInfoStorage);

		writeDescSets.push_back(writeDescriptorSetUniform);
		writeDescSets.push_back(writeDescriptorSetStorage);
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Update the graphics descriptor sets (one per swapchain image).</summary>
void VulkanSPHFluidSimulation::updateGraphicsDescriptorSet()
{
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	_deviceResources->vectorGraphicsDescriptorSet.resize(_swapchainLength);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->vectorGraphicsDescriptorSet[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->graphicsDescriptorSetLayout);
		_deviceResources->vectorGraphicsDescriptorSet[i]->setObjectName("GraphicsSpwachain" + std::to_string(i) + "DescriptorSet");

		// Binding 0: sceneSettingsBuffer
		pvrvk::DescriptorBufferInfo descriptorBufferInfoUniform = pvrvk::DescriptorBufferInfo(_deviceResources->sceneSettingsBuffer, 0, _deviceResources->sceneSettingsBufferView.getDynamicSliceSize());
		pvrvk::WriteDescriptorSet writeDescriptorSetUniform = pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->vectorGraphicsDescriptorSet[i], 0);
		writeDescriptorSetUniform.setBufferInfo(0, descriptorBufferInfoUniform);

		writeDescSets.push_back(writeDescriptorSetUniform);
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Build the vertex and index buffers for the sphere 3D model used to draw the particles.</summary>
/// <param name="commandBuffer">command buffer to record commands required to build the vertex and index buffers</param>
void VulkanSPHFluidSimulation::buildSphereVertexIndexBuffer(pvrvk::CommandBuffer commandBuffer)
{
	bool requiresCommandBufferSubmission = false;
	pvr::utils::createSingleBuffersFromMesh(_deviceResources->device, _sphereScene->getMesh(0), _deviceResources->sphereVertexBufferObject,
		_deviceResources->sphereIndexBufferObject,
		commandBuffer,
		requiresCommandBufferSubmission, _deviceResources->vmaAllocator);
}

/// <summary>Build the graphics pipeline used to draw the particles in the scene, using instanced rendering.</summary>
void VulkanSPHFluidSimulation::buildSphereDrawingPipeline()
{
	pvrvk::PipelineColorBlendAttachmentState colorAttachmentState;
	pvrvk::GraphicsPipelineCreateInfo pipeInfo;
	colorAttachmentState.setBlendEnable(false);

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_deviceResources->graphicsDescriptorSetLayout);
	_deviceResources->graphicsPipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);

	const pvrvk::Rect2D rect(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight());
	pipeInfo.viewport.setViewportAndScissor(0,
		pvrvk::Viewport(static_cast<float>(rect.getOffset().getX()), static_cast<float>(rect.getOffset().getY()), static_cast<float>(rect.getExtent().getWidth()),
			static_cast<float>(rect.getExtent().getHeight())),
		rect);
	pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	pipeInfo.colorBlend.setAttachmentState(0, colorAttachmentState);

	pipeInfo.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(VertShaderSphereSrcFile)->readToEnd<uint32_t>())));
	pipeInfo.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(FragShaderSphereSrcFile)->readToEnd<uint32_t>())));

	const pvr::assets::Mesh& mesh = _sphereScene->getMesh(0);
	pipeInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToPVRVk(mesh.getPrimitiveType()));
	pipeInfo.pipelineLayout = _deviceResources->graphicsPipelineLayout;
	pipeInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pipeInfo.subpass = 0;

	// shader attributes
	const pvr::utils::VertexBindings sphereVertexAttribBindings[] =
	{
		{ "POSITION", 0 },
		{ "NORMAL", 1 },
	};

	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.enableDepthTest(true);
	pipeInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
	pipeInfo.depthStencil.enableDepthWrite(true);
	pvr::utils::populateInputAssemblyFromMesh(
		mesh, sphereVertexAttribBindings, sizeof(sphereVertexAttribBindings) / sizeof(sphereVertexAttribBindings[0]),
		pipeInfo.vertexInput, pipeInfo.inputAssembler);

	// For instance rendering two input bindings are defined. The first corresponds to the vertex attrributes from the mesh to be drawn,
	// a sphere, while the second corresponds to the per-instance information buffer, which is the Particle::currPos field of the Particle
	// struct from the particleBuffer buffer where the particle simulation is being performed (this explains the stride value, as there are
	// other fields in the Particle struct which are ignored)
	pvrvk::PipelineVertexInputStateCreateInfo vertexInput; //!< Vertex Input creation info
	pvrvk::VertexInputBindingDescription vertexInputBindingDescription;
	vertexInputBindingDescription.setBinding(0);
	vertexInputBindingDescription.setStride(24); // Stride corresponds to the vertex and normal per-vertex information, with a total of 6 floats
	vertexInputBindingDescription.setInputRate(pvrvk::VertexInputRate::e_VERTEX);

	pvrvk::VertexInputBindingDescription instanceInputBindingDescription;
	instanceInputBindingDescription.setBinding(1);
	instanceInputBindingDescription.setStride(sizeof(Particle)); // Stride corresponds to the size of the Particle struct
	instanceInputBindingDescription.setInputRate(pvrvk::VertexInputRate::e_INSTANCE);

	vertexInput.addInputBinding(vertexInputBindingDescription);
	vertexInput.addInputBinding(instanceInputBindingDescription);

	// Input attributes at index 0 are vertex and normal, each requiring three floats
	vertexInput.addInputAttribute(pvrvk::VertexInputAttributeDescription(0, vertexBufferBindID, pvrvk::Format::e_R32G32B32_SFLOAT, 0));
	vertexInput.addInputAttribute(pvrvk::VertexInputAttributeDescription(1, vertexBufferBindID, pvrvk::Format::e_R32G32B32_SFLOAT, 12));

	// Input attributes at index 1 is a position, requiring three floats
	vertexInput.addInputAttribute(pvrvk::VertexInputAttributeDescription(2, instanceBufferBindID, pvrvk::Format::e_R32G32B32A32_SFLOAT, 0));

	pipeInfo.vertexInput = vertexInput;
	pipeInfo.inputAssembler = pvrvk::PipelineInputAssemblerStateCreateInfo(); // Default parameters from the constructor

	_deviceResources->sphereDrawingPipeline = _deviceResources->device->createGraphicsPipeline(pipeInfo, _deviceResources->pipelineCache);
	_deviceResources->sphereDrawingPipeline->setObjectName("SphereMeshGraphicsPipeline");
}

/// <summary>Record for each element in DeviceResources::computeCommandBuffers (equal to the number of swapchain images) the command buffers corresponding
/// to the compute commands used to perform the particle simulation. A barrier is added between commands to guarantee the updates from previous compute dispatches
/// will be visible for the new dispatch </summary>
void VulkanSPHFluidSimulation::recordComputeCommandBuffer()
{
	// When using the VK_KHR_synchronization2 extension for memory barriers, the usual fields are present at the VkBufferMemoryBarrier memory barrier struct (source and
	// destination access mask, source and destination queue familiy index), but VkBufferMemoryBarrier2KHR also incorporates source and destination pipeline stage, which is
	// individually specified per buffer memory barrier struct (VkBufferMemoryBarrier2KHR), and per image memory barrier struct (VkImageMemoryBarrier2KHR)
	pvrvk::MemoryBarrierSet2 barrier2;
	barrier2.addBarrier(pvrvk::BufferMemoryBarrier2(
		pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR, // srcStageMask
		pvrvk::PipelineStageFlagBits2KHR::e_2_COMPUTE_SHADER_BIT_KHR, // dstStageMask
		pvrvk::AccessFlagBits2KHR::e_2_SHADER_WRITE_BIT_KHR, // srcAccessMask
		pvrvk::AccessFlagBits2KHR::e_2_SHADER_READ_BIT_KHR, // dstAccessMask
		_deviceResources->computeQueue->getFamilyIndex(), // srcQueueFamilyIndexParam
		_deviceResources->computeQueue->getFamilyIndex(), // dstQueueFamilyIndexParam
		_deviceResources->particleBuffer, // buffer
		0, // buffer offset
		static_cast<uint32_t>(_deviceResources->particleBuffer->getSize()))); // buffer size to apply the barrier to

	uint32_t offset = 0;

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		_deviceResources->computeCommandBuffers[i]->begin();
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->computeCommandBuffers[i], pvrvk::DebugUtilsLabel("Compute Commands"));

		offset = _deviceResources->computeSimulationSettingsBufferView.getDynamicSliceOffset(i);
		_deviceResources->computeCommandBuffers[i]->bindPipeline(_deviceResources->computePipelineDensityPressureUpdate);
		_deviceResources->computeCommandBuffers[i]->bindDescriptorSets(pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->computePipelinelayout, 0, &_deviceResources->vectorComputeDescriptorSet[i], 1, &offset, 1);
		_deviceResources->computeCommandBuffers[i]->dispatch(numberParticles, 1, 1);

		// Add a barrier so the buffer writes from the previous compute dispatch are visible to the next compute dispatch inside the same command buffer
		_deviceResources->computeCommandBuffers[i]->pipelineBarrier2(barrier2);
		
		_deviceResources->computeCommandBuffers[i]->bindPipeline(_deviceResources->computePipelineAccelerationUpdate);
		_deviceResources->computeCommandBuffers[i]->bindDescriptorSets(pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->computePipelinelayout, 0, &_deviceResources->vectorComputeDescriptorSet[i], 1, &offset, 1);
		_deviceResources->computeCommandBuffers[i]->dispatch(numberParticles, 1, 1);

		// Add a barrier so the buffer writes from the previous compute dispatch are visible to the next compute dispatch inside the same command buffer
		_deviceResources->computeCommandBuffers[i]->pipelineBarrier2(barrier2);

		_deviceResources->computeCommandBuffers[i]->bindPipeline(_deviceResources->computePipelinePositionUpdate);
		_deviceResources->computeCommandBuffers[i]->bindDescriptorSets(pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->computePipelinelayout, 0, &_deviceResources->vectorComputeDescriptorSet[i], 1, &offset, 1);
		_deviceResources->computeCommandBuffers[i]->dispatch(numberParticles, 1, 1);
		
		pvr::utils::endCommandBufferDebugLabel(_deviceResources->computeCommandBuffers[i]);

		// end recording commands for the current command buffer
		_deviceResources->computeCommandBuffers[i]->end();
	}
}

/// <summary>Record the graphics command buffers used to draw the particles in the scene using instanced rendering.</summary>
/// <param name="commandBuffer">command buffer to be submitted to the graphics queue to record commands</param>
/// <param name="swapchainIndex">current swap chain index</param>
void VulkanSPHFluidSimulation::recordDrawMeshCommandBuffer(pvrvk::CommandBuffer commandBuffer, uint32_t swapchainIndex)
{
	pvr::utils::beginCommandBufferDebugLabel(commandBuffer, pvrvk::DebugUtilsLabel("Sphere"));

	// calculate the dynamic offset to use
	// enqueue the static states which wont be changed through out the frame
	commandBuffer->bindPipeline(_deviceResources->sphereDrawingPipeline);
	commandBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->graphicsPipelineLayout, 0, _deviceResources->vectorGraphicsDescriptorSet[swapchainIndex], nullptr);

	const uint32_t meshId = _sphereScene->getNode(0).getObjectId();
	const pvr::assets::Mesh& mesh = _sphereScene->getMesh(meshId);

	commandBuffer->bindVertexBuffer(_deviceResources->sphereVertexBufferObject, 0, vertexBufferBindID);
	commandBuffer->bindVertexBuffer(_deviceResources->particleBuffer, 0, instanceBufferBindID);
	commandBuffer->bindIndexBuffer(_deviceResources->sphereIndexBufferObject, 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
	commandBuffer->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, numberParticles);

	pvr::utils::endCommandBufferDebugLabel(commandBuffer);
}

/// <summary>Once the particle simulation time reaches the value in seconds simulationTimeUntilChangingBoundingX the bounding x limit for the simulation will be animated to animate the scene.</summary>
/// <param name="deltaTime">delta time in seconds</param>
void VulkanSPHFluidSimulation::animateSimulationBoundingX(float deltaTime)
{
	float boundingXLowerLimit = 0.0f;
	float boundingXUpperLimit = _initialBoundingX;
	_boundingXAnimationAccumulatedTime += deltaTime;

	if (_boundingXAnimationAccumulatedTime >= boundingXAnimationTime)
	{
		_boundingXGoingToLowerValue = !_boundingXGoingToLowerValue;
		_boundingXAnimationAccumulatedTime = 0.0f;
	}

	float factor = _boundingXAnimationAccumulatedTime / boundingXAnimationTime;
	factor = glm::clamp(factor, 0.0f, 1.0f);

	if (_boundingXGoingToLowerValue)
	{
		_upperBoundX = factor * boundingXLowerLimit + (1.0f - factor) * boundingXUpperLimit;
	}
	else
	{
		_upperBoundX = factor * boundingXUpperLimit + (1.0f - factor) * boundingXLowerLimit;
	}
}

/// <summary>Update for the current swapchain index those compute and scene settings that change per frame.</summary>
void VulkanSPHFluidSimulation::updateSettingsBufferViews()
{
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	// Update the compute simulation settings buffer
	float frameTime = float(getFrameTime());
	_accumulatedExecutionTime += frameTime * 0.001f;

	// Clamp max delta value to avoid large values which could drive the physical simulation wrong due to long loading times or update times in low end devices
	frameTime = glm::clamp(frameTime, 0.0f, 25.0f);

	// Delta time in the simulation has seconds as unit
	float deltaTime = frameTime * 0.001f;

	if (_accumulatedExecutionTime >= simulationTimeUntilChangingBoundingX)
	{
		animateSimulationBoundingX(deltaTime);
		_deviceResources->computeSimulationSettingsBufferView.getElementByName("upperBoundX", 0, swapchainIndex).setValue(_upperBoundX);
	}

	_deviceResources->computeSimulationSettingsBufferView.getElementByName("deltaTime", 0, swapchainIndex).setValue(deltaTime);

	// if the memory property flags used by the buffers' device memory does not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->computeSimulationSettingsBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->computeSimulationSettingsBuffer->getDeviceMemory()->flushRange(_deviceResources->computeSimulationSettingsBufferView.getDynamicSliceOffset(swapchainIndex),
			_deviceResources->computeSimulationSettingsBufferView.getDynamicSliceSize());
	}

	// Update the scene settings buffer
	_deviceResources->sceneSettingsBufferView.getElementByName("viewProjectionMatrix", 0, swapchainIndex).setValue(_viewProj);

	// if the memory property flags used by the buffers' device memory does not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->sceneSettingsBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->sceneSettingsBuffer->getDeviceMemory()->flushRange(
			_deviceResources->sceneSettingsBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->sceneSettingsBufferView.getDynamicSliceSize());
	}
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanSPHFluidSimulation::initApplication()
{
	_frameId = 0;
	_boundingXGoingToLowerValue = true;
	_boundingXAnimationAccumulatedTime = 0.0f;
	_accumulatedExecutionTime = 0.0f;

	// Load the scene
	_sphereScene = pvr::assets::loadModel(*this, SphereModelFile);

	for (uint32_t i = 0; i < _sphereScene->getNumMeshes(); ++i)
	{
		_sphereScene->getMesh(i).setVertexAttributeIndex("POSITION0", Attributes::VertexArray);
		_sphereScene->getMesh(i).setVertexAttributeIndex("NORMAL0", Attributes::NormalArray);
		_sphereScene->getMesh(i).setVertexAttributeIndex("UV0", Attributes::TexCoordArray);
	}

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanSPHFluidSimulation::quitApplication()
{
	_sphereScene.reset();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanSPHFluidSimulation::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create Vulkan 1.0 instance and retrieve compatible physical devices
	pvr::utils::VulkanVersion VulkanVersion(1, 0, 0);
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName(), VulkanVersion, pvr::utils::InstanceExtensions(VulkanVersion));

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	std::vector<std::string> vectorExtensionNames{ VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME };

	std::vector<int> vectorPhysicalDevicesIndex = pvr::utils::validatePhysicalDeviceExtensions(_deviceResources->instance, vectorExtensionNames);

	if (vectorPhysicalDevicesIndex.size() == 0)
	{
		throw pvrvk::ErrorExtensionNotPresent(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
	}

	// Create the surface
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	//const pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueuePopulateInfo queueCreateInfos[] =
	{
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT, surface }, // Queue 0 for Graphics
		{ pvrvk::QueueFlags::e_COMPUTE_BIT } // Queue 1 For Compute
	};

	pvr::utils::DeviceExtensions deviceExtensions = pvr::utils::DeviceExtensions();
	for (const std::string& extensionName : vectorExtensionNames)
	{
		deviceExtensions.addExtension(extensionName);
	}

	// Get the physical device features for all of the raytracing extensions through a continual pNext chain
	VkPhysicalDeviceFeatures2 deviceFeatures{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_FEATURES_2) };

	// Raytracing Pipeline Features
	VkPhysicalDeviceSynchronization2FeaturesKHR physicalDeviceSynchronization2FeaturesKHR{ static_cast<VkStructureType>(pvrvk::StructureType::e_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES) };
	deviceFeatures.pNext = &physicalDeviceSynchronization2FeaturesKHR;

	// Fill in all of these device features with one call
	_deviceResources->instance->getVkBindings().vkGetPhysicalDeviceFeatures2(_deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0])->getVkHandle(), &deviceFeatures);

	// Add these device features to the physical device, since they're all connected by a pNext chain, we only need to explicitly attach the top feature
	deviceExtensions.addExtensionFeatureVk<VkPhysicalDeviceSynchronization2FeaturesKHR>(&physicalDeviceSynchronization2FeaturesKHR);

	// create device and queues
	pvr::utils::QueueAccessInfo queueAccessInfos[2];
	_deviceResources->device =
		pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(vectorPhysicalDevicesIndex[0]), queueCreateInfos, 2, queueAccessInfos, deviceExtensions);

	_deviceResources->graphicsQueue = _deviceResources->device->getQueue(queueAccessInfos[0].familyId, queueAccessInfos[0].queueId);
	_deviceResources->graphicsQueue->setObjectName("GraphicsQueue");

	if (queueAccessInfos[1].familyId != static_cast<uint32_t>(-1) && queueAccessInfos[1].queueId != static_cast<uint32_t>(-1))
	{
		Log(LogLevel::Information, "Multiple queues support e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI. These queues will be used to ping-pong work each frame");

		_deviceResources->computeQueue = _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);
		_deviceResources->computeQueue->setObjectName("ComputeQueue");
	}
	else
	{
		Log(LogLevel::Information, "Only a single queue supports e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI");

		_deviceResources->computeQueue = _deviceResources->graphicsQueue;
	}

	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("initView"));

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;

	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{
		swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT;
	}
	
	// Create the swapchain, on screen framebuffers and renderpass
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));
	_deviceResources->swapchain = swapChainCreateOutput.swapchain;

	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	_swapchainLength = _deviceResources->swapchain->getSwapchainLength();

	// Create the command pool and descriptor set pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(_deviceResources->graphicsQueue->getFamilyIndex()));
	_deviceResources->commandPool->setObjectName("Main Command Pool");

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// Allocate all the Vulkan resources related resources (command buffers, semaphores and fences)

	_deviceResources->imageAcquiredSemaphores.resize(_swapchainLength);
	_deviceResources->presentationSemaphores.resize(_swapchainLength);
	_deviceResources->computeSemaphores.resize(_swapchainLength);
	_deviceResources->computeFences.resize(_swapchainLength);
	_deviceResources->graphicsFences.resize(_swapchainLength);
	_deviceResources->graphicsCommandBuffers.resize(_swapchainLength);
	_deviceResources->computeCommandBuffers.resize(_swapchainLength);

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// Per swapchain command buffers
		_deviceResources->graphicsCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->computeCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->graphicsCommandBuffers[i]->setObjectName("MainCommandBufferSwapchain" + std::to_string(i));
		_deviceResources->computeCommandBuffers[i]->setObjectName("ComputeCommandBufferSwapchain" + std::to_string(i));

		_deviceResources->computeCommandBuffers[i]->setVKSynchronization2IsSupported(true);

		// Per swapchain semaphores
		_deviceResources->computeSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();

		_deviceResources->computeSemaphores[i]->setObjectName("ComputeSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->presentationSemaphores[i]->setObjectName("PresentationSemaphoreSwapchain" + std::to_string(i));
		_deviceResources->imageAcquiredSemaphores[i]->setObjectName("ImageAcquiredSemaphoreSwapchain" + std::to_string(i));

		// Per swapchain fences
		_deviceResources->graphicsFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->computeFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->graphicsFences[i]->setObjectName(std::string("Per Frame Command Buffer Fence [") + std::to_string(i) + "]");
		_deviceResources->computeFences[i]->setObjectName(std::string("Per Frame Compute Command Buffer Fence [") + std::to_string(i) + "]");
	}

	// Create a single time submit command buffer for uploading resources
	pvrvk::CommandBuffer uploadBuffer = _deviceResources->commandPool->allocateCommandBuffer();
	uploadBuffer->setObjectName("InitView : Upload Command Buffer");
	uploadBuffer->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);

	// Build all the resources used in the sample
	buildDescriptorPool();
	buildComputeDescriptorSetLayout();
	buildSphereParticleDescriptorSetLayout();
	buildComputePipelines();
	buildParticleBuffer(uploadBuffer);
	buildParticleSimulationSettingsBuffer();
	buildParticleDrawingBuffer();
	updateComputeDescriptorSet();
	updateGraphicsDescriptorSet();
	buildSphereVertexIndexBuffer(uploadBuffer);
	buildSphereDrawingPipeline();

	// create the image samplers
	uploadBuffer->end();

	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("Batching Application Resource Upload"));

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &uploadBuffer;
	submitInfo.numCommandBuffers = 1;
	_deviceResources->graphicsQueue->submit(&submitInfo, 1);
	_deviceResources->graphicsQueue->waitIdle();

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);

	//  Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->graphicsQueue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("SPH fluid simulation");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	// Is the screen rotated
	const bool bRotate = this->isScreenRotated();
	
	float fov = (40.0f * glm::pi<float>()) / 180.0f; // Angle in radians
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;	

	_viewProj = (bRotate ? pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, static_cast<float>(this->getHeight()), static_cast<float>(this->getWidth()), nearPlane, farPlane, 0.0f)
						 : pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()), nearPlane, farPlane));

	glm::vec3 from = glm::vec3(0.0f, 2.5f, 10.0f);
	glm::vec3 to = glm::vec3(0.0f, -4.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	_viewProj = _viewProj * glm::lookAt(from, to, up);

	// record the command buffers for both graphics and compute
	recordGraphicsCommandBuffer();
	recordComputeCommandBuffer();

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanSPHFluidSimulation::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanSPHFluidSimulation::renderFrame()
{
	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("renderFrame"));

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->graphicsFences[swapchainIndex]->wait();
	_deviceResources->graphicsFences[swapchainIndex]->reset();

	updateSettingsBufferViews();

	// Wait for any previous compute command buffer pending to complete execution
	_deviceResources->computeFences[swapchainIndex]->wait();
	_deviceResources->computeFences[swapchainIndex]->reset();

	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("Submitting per frame command buffers"));

	// Submit compute command buffers to the compute queue. We will use a semaphore in the compute queue (computeSemaphores) that will be waited for
	// in the graphics queue so the graphic commands part has the information for drawing updated when executed
	pvrvk::SubmitInfo submitInfoCompute;
	pvrvk::PipelineStageFlags pipeWaitStageFlagsCompute = pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT;
	submitInfoCompute.commandBuffers = &_deviceResources->computeCommandBuffers[swapchainIndex];
	submitInfoCompute.numCommandBuffers = 1;
	submitInfoCompute.waitSemaphores = nullptr; // Do not wait for any semaphores, synchronize later with the image acquire part
	submitInfoCompute.numWaitSemaphores = 0;
	submitInfoCompute.signalSemaphores = &_deviceResources->computeSemaphores[_frameId];
	submitInfoCompute.numSignalSemaphores = 1;
	submitInfoCompute.waitDstStageMask = &pipeWaitStageFlagsCompute;
	_deviceResources->computeQueue->submit(&submitInfoCompute, 1, _deviceResources->computeFences[swapchainIndex]);

	// For the graphics queue command buffer, wait for both the compute semaphore and the swapchain image acquire semaphore to be signaled 
	// before doing any vertex shader work, and on completion signal the presentationSemaphores semaphore
	pvrvk::SubmitInfo submitInfo;
	const pvrvk::PipelineStageFlags pipeWaitStageFlags[] = { pvrvk::PipelineStageFlags::e_VERTEX_SHADER_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT };
	
	std::vector<pvrvk::Semaphore> vectorSemaphore = { _deviceResources->computeSemaphores[_frameId], _deviceResources->imageAcquiredSemaphores[_frameId] };
	submitInfo.waitSemaphores = vectorSemaphore.data();
	submitInfo.numWaitSemaphores = 2;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags[0];
	submitInfo.numCommandBuffers = 1;
	submitInfo.commandBuffers = &_deviceResources->graphicsCommandBuffers[swapchainIndex];
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	_deviceResources->graphicsQueue->submit(&submitInfo, 1, _deviceResources->graphicsFences[swapchainIndex]);

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->graphicsQueue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// Once the presentationSemaphores semaphore has been signalled, submit the present command to show the contents in the backbuffer on the screen
	pvr::utils::beginQueueDebugLabel(_deviceResources->graphicsQueue, pvrvk::DebugUtilsLabel("Presenting swapchain image to the screen"));

	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->graphicsQueue->present(presentInfo);

	pvr::utils::endQueueDebugLabel(_deviceResources->graphicsQueue);

	_frameId = (_frameId + 1) % _swapchainLength;

	return pvr::Result::Success;
}

/// <summary>Record graphics command buffers per swapchain image.</summary>
void VulkanSPHFluidSimulation::recordGraphicsCommandBuffer()
{
	pvrvk::ClearValue clearValues[2] = { pvrvk::ClearValue(0.0f, 0.45f, 0.41f, 1.f), pvrvk::ClearValue(1.f, 0u) };

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// begin recording commands for the current swap chain command buffer
		_deviceResources->graphicsCommandBuffers[i]->begin();
		pvr::utils::beginCommandBufferDebugLabel(_deviceResources->graphicsCommandBuffers[i], pvrvk::DebugUtilsLabel("Render Frame Commands"));

		// begin the render pass
		_deviceResources->graphicsCommandBuffers[i]->beginRenderPass(
			_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));

		recordDrawMeshCommandBuffer(_deviceResources->graphicsCommandBuffers[i], i);

		// record the ui renderer commands
		_deviceResources->uiRenderer.beginRendering(_deviceResources->graphicsCommandBuffers[i]);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();

		// end the renderpass
		_deviceResources->graphicsCommandBuffers[i]->endRenderPass();

		pvr::utils::endCommandBufferDebugLabel(_deviceResources->graphicsCommandBuffers[i]);

		// end recording commands for the current command buffer
		_deviceResources->graphicsCommandBuffers[i]->end();
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanSPHFluidSimulation>(); }
