/*!*********************************************************************************************************************
\File         ParticleSystemGPU.cpp
\Title        ParticleSystemGPU
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief      Particle system implemented using GPU Compute SHaders. Uses direct access to the VBOs as SSBOs
***********************************************************************************************************************/
#include "ParticleSystemGPU.h"
#include "PVRVk/ComputePipelineVk.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRAssets/Shader.h"
#include "PVRVk/QueueVk.h"

#define SPHERES_UBO_BINDING 0
#define CONFIG_UNIFORM_BINDING 1
#define PARTICLES_SSBO_BINDING_IN_OUT 2
/*!*********************************************************************************************************************
\brief ctor
***********************************************************************************************************************/
ParticleSystemGPU::ParticleSystemGPU(pvr::Shell& assetLoader) :
	assetProvider(assetLoader), computeShaderSrcFile("ParticleSolver.csh"), workgroupSize(32),
	/*SIMULATION DATA*/
	numParticles(0), numSpheres(0)
{
	memset(&particleConfigData, 0, sizeof(ParticleConfig));
}

/*!*********************************************************************************************************************
\brief  Cleans up any resources that were constructed by this class
***********************************************************************************************************************/
ParticleSystemGPU::~ParticleSystemGPU()
{
}

/*!*********************************************************************************************************************
\param[in]  ErrorStr  In case of error, any error messages will be stored in this std::string
\return true if successfully completed, false otherwise.
\brief  Initializes any state that needs to be created by the class itself.
    Init WILL NOT initialize objects that need to be set by a caller, most importantly:
    - The vertex buffer that will contain the actual particles
    - The actual Spheres
    - The actual number of particles (SetNumberOfParticles actually allocates memory for the particles)
***********************************************************************************************************************/
bool ParticleSystemGPU::init(uint32_t maxParticles, const Sphere* spheres, uint32_t numSpheres,
                             pvrvk::Device& device, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
                             uint32_t numSwapchains)
{
	this->commandPool = commandPool;
	this->device = device;
	swapchainLength = numSwapchains;

	if (!createComputePipeline())
	{
		Log("Failed to create compute pipleline");
		return false;
	}
	setCollisionSpheres(spheres, numSpheres);
	//create the ssbo buffer
	particleBufferViewSsbos = pvr::utils::createBuffer(device,sizeof(Particle) * maxParticles,
	                          VkBufferUsageFlags::e_STORAGE_BUFFER_BIT | VkBufferUsageFlags::e_TRANSFER_DST_BIT,
	                          VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);
	if (!particleBufferViewSsbos.isValid())
	{
		Log("Failed to create the uniform buffer");
		return false;
	}

	particleConfigUboBufferView.initDynamic(ParticleConfigViewMapping, swapchainLength,
	                                        pvr::BufferUsageFlags::UniformBuffer, static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment));
	particleConfigUbo = pvr::utils::createBuffer(device,particleConfigUboBufferView.getSize(),
	                    VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);

	pvrvk::WriteDescriptorSet writeDescSets[pvrvk::FrameworkCaps::MaxSwapChains * 3];
	for (uint8_t i = 0; i < swapchainLength; ++i)
	{
		multiBuffer.descSets[i] = descriptorPool->allocateDescriptorSet(
		                            pipe->getPipelineLayout()->getDescriptorSetLayout(0));
		if (!multiBuffer.descSets[i].isValid())
		{
			Log("Failed to create Descriptor Set"); return false;
		}
		//update
		writeDescSets[i * 3]
		.set(VkDescriptorType::e_UNIFORM_BUFFER,
		     multiBuffer.descSets[i], BufferBindingPoint::SPHERES_UBO_BINDING_INDEX)
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(collisonSpheresUbo, 0,
		               collisonSpheresUboBufferView.getDynamicSliceSize()));

		writeDescSets[i * 3 + 1]
		.set(VkDescriptorType::e_UNIFORM_BUFFER,
		     multiBuffer.descSets[i], BufferBindingPoint::PARTICLE_CONFIG_UBO_BINDING_INDEX)
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(particleConfigUbo, 0,
		               particleConfigUboBufferView.getDynamicSliceSize()));

		writeDescSets[i * 3 + 2]
		.set(VkDescriptorType::e_STORAGE_BUFFER,
		     multiBuffer.descSets[i], BufferBindingPoint::PARTICLES_SSBO_BINDING_INDEX_IN_OUT)
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(particleBufferViewSsbos, 0, particleBufferViewSsbos->getSize()));
	}
	device->updateDescriptorSets(writeDescSets, swapchainLength * 3, nullptr, 0);
	return true;
}

/*!*********************************************************************************************************************
\brief  create the compute pipeline used for this example
\return true if success
\param[in]  std::string & errorStr
***********************************************************************************************************************/
bool ParticleSystemGPU::createComputePipeline()
{
	pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;

	descSetLayoutInfo
	.setBinding(SPHERES_UBO_BINDING, VkDescriptorType::e_UNIFORM_BUFFER, 1, VkShaderStageFlags::e_COMPUTE_BIT)
	.setBinding(CONFIG_UNIFORM_BINDING, VkDescriptorType::e_UNIFORM_BUFFER, 1, VkShaderStageFlags::e_COMPUTE_BIT)
	.setBinding(PARTICLES_SSBO_BINDING_IN_OUT, VkDescriptorType::e_STORAGE_BUFFER, 1, VkShaderStageFlags::e_COMPUTE_BIT);

	pipeLayoutInfo.setDescSetLayout(0, device->createDescriptorSetLayout(descSetLayoutInfo));

	pvrvk::ComputePipelineCreateInfo pipeCreateInfo;
	pvrvk::Shader shader = device->createShader(assetProvider.getAssetStream(ComputeShaderFileName)->readToEnd<uint32_t>());
	if (shader.isNull())
	{
		Log("Failed to load Compute shader");
		return false;
	}
	pipeCreateInfo.computeShader.setShader(shader);
	pipeCreateInfo.pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	pipe = device->createComputePipeline(pipeCreateInfo);
	return pipe.isValid();
}

/*!*********************************************************************************************************************
\brief      Updates the common uniforms (notably time)
\param    dt  Elapsed time from last iteration (frame)
\details  Advances the simulation by dt. Invalidates the following OpenGL state:Current program, GL_UNIFORM_BUFFER binding
***********************************************************************************************************************/
void ParticleSystemGPU::updateUniforms(uint32_t swapchain, float dt)
{
	if (dt == 0) { return; }
	dt *= 0.001f;
	particleConfigData.fDt = dt;
	particleConfigData.fTotalTime += dt;
	particleConfigData.updateBufferView(particleConfigUboBufferView, particleConfigUbo, swapchain);
}

/*!*********************************************************************************************************************
\param[in]  numParticles  Set the number of particles to this number
\brief  Allocates memory for the actual particles and configures the simulation accordingly.
        Invalidates the following OpenGL state:
    GL_SHADER_STORAGE_BUFFER binding. Call this at least once to create storage for the particles.
***********************************************************************************************************************/
void ParticleSystemGPU::setNumberOfParticles(uint32_t numParticles, pvrvk::Queue& queue)
{
	// create and fill the staging buffer
	this->numParticles = numParticles;
	pvrvk::Buffer copyBuffer = pvr::utils::createBuffer(device,
	                             particleBufferViewSsbos->getSize(), VkBufferUsageFlags::e_TRANSFER_SRC_BIT,
	                             VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);

	void* memory;
	copyBuffer->getDeviceMemory()->map(&memory, 0, copyBuffer->getSize());
	Particle* tmpData = (Particle*)memory;
	for (uint32_t i = 0; i < numParticles; ++i, ++tmpData)
	{
		tmpData->vPosition = glm::vec3(((float)rand() / RAND_MAX) * 50.f - 25.f,
		                               ((float)rand() / RAND_MAX) * 50.f, ((float)rand() / RAND_MAX) * 50.f - 25.f);
		tmpData->vVelocity = tmpData->vPosition * .2f;
		tmpData->fTimeToLive = ((float)rand() / RAND_MAX);
	}
	memset(tmpData, 0, copyBuffer->getSize() - (sizeof(Particle) * numParticles));// zero out the remaining entries
	copyBuffer->getDeviceMemory()->flushRange(0, copyBuffer->getSize());
	copyBuffer->getDeviceMemory()->unmap();

	// copy it to the destination buffer
	if (commandStaging.isNull())
	{ commandStaging = commandPool->allocateCommandBuffer(); }

	commandStaging->begin();
	commandStaging->copyBuffer(copyBuffer, particleBufferViewSsbos, 0, 0, static_cast<uint32_t>(copyBuffer->getSize()));
	commandStaging->end();

	if (stagingFence.isNull())
	{ stagingFence = device->createFence(VkFenceCreateFlags(0)); }
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &commandStaging;
	submitInfo.numCommandBuffers = 1;
	queue->submit(&submitInfo, 1, stagingFence);
	stagingFence->wait();
	stagingFence->reset();
	// if (stagingFence->isSignalled())
	{
		for (uint32_t i = 0; i < swapchainLength; ++i)
		{
			recordCommandBuffer(i);
		}
	}
}

/*!*********************************************************************************************************************
\param[in]  emitter Set Emitter state to this
\brief  Sets the transformation, height and radius of the active emitter to this state
***********************************************************************************************************************/
void ParticleSystemGPU::setEmitter(const Emitter& emitter) {  particleConfigData.emitter = emitter;}

/*!*********************************************************************************************************************
 \param[in] g A 3D vector that is the gravity of the simulation (m*sec^-2)
***********************************************************************************************************************/
void ParticleSystemGPU::setGravity(const glm::vec3& g) {   particleConfigData.vG = g;}

/*!*********************************************************************************************************************
\param[in]      pSpheres  Pointer to an array of Sphere structs
\param[in]      uiNumSpheres  The number of spheres
\brief  Sets the physical model of the collision spheres and initializes them.
***********************************************************************************************************************/
bool ParticleSystemGPU::setCollisionSpheres(const Sphere* spheres, uint32_t numSpheres)
{
	if (numSpheres)
	{
		collisonSpheresUboBufferView.init(SphereViewMapping);

		collisonSpheresUbo = pvr::utils::createBuffer(device,collisonSpheresUboBufferView.getSize(),
		                     VkBufferUsageFlags::e_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT | VkMemoryPropertyFlags::e_HOST_COHERENT_BIT);

		void* memory;
		collisonSpheresUbo->getDeviceMemory()->map(&memory);
		collisonSpheresUboBufferView.pointToMappedMemory(memory);
		for (uint32_t i = 0; i < numSpheres; ++i)
		{
			collisonSpheresUboBufferView.getElement(SphereViewElements::PositionRadius, i).setValue(glm::vec4(spheres[i].vPosition, spheres[i].fRadius));
		}
		collisonSpheresUbo->getDeviceMemory()->unmap();
	}
	return true;
}

/*!*********************************************************************************************************************
\brief  record compute commands
\param  pvr::api::CommandBufferBase commandBuffer
\param  uint8_t swapchain
***********************************************************************************************************************/
void ParticleSystemGPU::recordCommandBuffer(uint8_t swapchain)
{
	if (!commandBuffer[swapchain].isValid())
	{
		commandBuffer[swapchain] = commandPool->allocateSecondaryCommandBuffer();
	}
	commandBuffer[swapchain]->begin();
	commandBuffer[swapchain]->bindPipeline(pipe);
	commandBuffer[swapchain]->bindDescriptorSets(VkPipelineBindPoint::e_COMPUTE,
	    pipe->getPipelineLayout(), 0, &multiBuffer.descSets[swapchain], 1);
	commandBuffer[swapchain]->dispatch(numParticles / workgroupSize, 1, 1);
	commandBuffer[swapchain]->end();
}