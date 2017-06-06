/*!*********************************************************************************************************************
\File         ParticleSystemGPU.cpp
\Title        ParticleSystemGPU
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Particle system implemented using GPU Compute SHaders. Uses direct access to the VBOs as SSBOs
***********************************************************************************************************************/
#include "ParticleSystemGPU.h"
#include "PVRApi/ApiObjects/ComputePipeline.h"
#include "PVRAssets/Shader.h"

using namespace pvr::types;
using namespace pvr;
#define SPHERES_UBO_BINDING 0
#define CONFIG_UNIFORM_BINDING 1
#define PARTICLES_SSBO_BINDING_IN_OUT 2
/*!*********************************************************************************************************************
\brief ctor
***********************************************************************************************************************/
ParticleSystemGPU::ParticleSystemGPU(pvr::Shell& assetLoader)	:
	context(assetLoader.getGraphicsContext()), assetProvider(assetLoader),
	computeShaderSrcFile("ParticleSolver.csh"), workgroupSize(32),
	/*SIMULATION DATA*/
	numParticles(0), numSpheres(0)
{
	memset(&particleConfigData, 0, sizeof(ParticleConfig));
}

/*!*********************************************************************************************************************
\brief	Cleans up any resources that were constructed by this class
***********************************************************************************************************************/
ParticleSystemGPU::~ParticleSystemGPU()
{
}

/*!*********************************************************************************************************************
\param[in]	ErrorStr	In case of error, any error messages will be stored in this string
\return	true if successfully completed, false otherwise.
\brief	Initializes any state that needs to be created by the class itself.
		Init WILL NOT initialize objects that need to be set by a caller, most importantly:
		- The vertex buffer that will contain the actual particles
		- The actual Spheres
		- The actual number of particles (SetNumberOfParticles actually allocates memory for the particles)
***********************************************************************************************************************/
bool ParticleSystemGPU::init(pvr::uint32 maxParticles, const Sphere* spheres, pvr::uint32 numSpheres, pvr::string& errorStr)
{
	if (!createComputePipeline(errorStr))
	{
		pvr::Log("Failed to create compute pipleline");
		return false;
	}
	setCollisionSpheres(spheres, numSpheres);
	//create the ssbo buffer
	particleBufferViewSsbos = context->createBuffer(sizeof(Particle) * maxParticles,
	                          BufferBindingUse(BufferBindingUse::StorageBuffer | BufferBindingUse::TransferDest));
	if (!particleBufferViewSsbos.isValid()) { pvr::Log("Failed to create the uniform buffer"); return false;}

	multiBuffer.particleConfigUbo.addEntriesPacked(ParticleConfigViewMapping,
	    sizeof(ParticleConfigViewMapping) / sizeof(ParticleConfigViewMapping[0]));
	multiBuffer.particleConfigUbo.finalize(context, 1, pvr::types::BufferBindingUse::UniformBuffer, false, false);

	for (pvr::uint8 i = 0; i < context->getSwapChainLength(); ++i)
	{
		multiBuffer.particleConfigUbo.connectWithBuffer(i,
		    context->createBufferAndView(multiBuffer.particleConfigUbo.getAlignedTotalSize(),
		                                 BufferBindingUse::UniformBuffer, true));

		multiBuffer.descSets[i] =
		  context->createDescriptorSetOnDefaultPool(pipe->getPipelineLayout()->getDescriptorSetLayout(0));
		if (!multiBuffer.descSets[i].isValid()) { pvr::Log("Failed to create Descriptor Set"); return false; }
		//update
		multiBuffer.descSets[i]->update(pvr::api::DescriptorSetUpdate()
		                                .setUbo(BufferBindingPoint::SPHERES_UBO_BINDING_INDEX, collisonSpheresUbo.getConnectedBuffer(0))
		                                .setUbo(BufferBindingPoint::PARTICLE_CONFIG_UBO_BINDING_INDEX,
		                                        multiBuffer.particleConfigUbo.getConnectedBuffer(i))
		                                .setSsbo(BufferBindingPoint::PARTICLES_SSBO_BINDING_INDEX_IN_OUT,
		                                    context->createBufferView(particleBufferViewSsbos, 0, particleBufferViewSsbos->getSize())));

		signalSemaphore[i] = context->createSemaphore();
	}
	cmdBufferFence = context->createFence();

	return true;
}

/*!*********************************************************************************************************************
\brief	create the compute pipeline used for this example
\return	true if success
\param[in]	pvr::string & errorStr
***********************************************************************************************************************/
bool ParticleSystemGPU::createComputePipeline(pvr::string& errorStr)
{
	pvr::api::DescriptorSetLayoutCreateParam descSetLayoutInfo;
	pvr::api::PipelineLayoutCreateParam pipeLayoutInfo;
	descSetLayoutInfo
	.setBinding(SPHERES_UBO_BINDING, DescriptorType::UniformBuffer, 1, ShaderStageFlags::Compute)
	.setBinding(CONFIG_UNIFORM_BINDING, DescriptorType::UniformBuffer, 1, ShaderStageFlags::Compute)
	.setBinding(PARTICLES_SSBO_BINDING_IN_OUT, DescriptorType::StorageBuffer, 1, ShaderStageFlags::Compute);
	pipeLayoutInfo.setDescSetLayout(0, context->createDescriptorSetLayout(descSetLayoutInfo));

	pvr::api::ComputePipelineCreateParam pipeCreateInfo;
	pvr::api::Shader shader = context->createShader(*assetProvider.getAssetStream(ComputeShaderFileName), ShaderType::ComputeShader);
	if (shader.isNull())
	{
		pvr::Log("Failed to load Compute shader");
		return false;
	}
	pipeCreateInfo.computeShader.setShader(shader);
	pipeCreateInfo.pipelineLayout = context->createPipelineLayout(pipeLayoutInfo);
	pipe = context->createComputePipeline(pipeCreateInfo);
	return pipe.isValid();
}

/*!*********************************************************************************************************************
\brief      Updates the common uniforms (notably time)
\param		dt	Elapsed time from last iteration (frame)
\details	Advances the simulation by dt. Invalidates the following OpenGL state:Current program, GL_UNIFORM_BUFFER binding
***********************************************************************************************************************/
void ParticleSystemGPU::updateUniforms(pvr::uint32 swapchain, pvr::float32 dt)
{
	if (dt == 0) { return; }
	dt *= 0.001f;
	particleConfigData.fDt = dt;
	particleConfigData.fTotalTime += dt;
	particleConfigData.updateBufferView(multiBuffer.particleConfigUbo, swapchain);
}

/*!*********************************************************************************************************************
\param[in]	numParticles	Set the number of particles to this number
\brief	Allocates memory for the actual particles and configures the simulation accordingly. Invalidates the following OpenGL state:
		GL_SHADER_STORAGE_BUFFER binding. Call this at least once to create storage for the particles.
***********************************************************************************************************************/
void ParticleSystemGPU::setNumberOfParticles(pvr::uint32 numParticles)
{
	// create and fill the staging buffer
	this->numParticles = numParticles;
	api::Buffer copyBuffer = context->createBuffer(particleBufferViewSsbos->getSize(), BufferBindingUse::TransferSrc, true);
	Particle* tmpData = (Particle*)copyBuffer->map(MapBufferFlags::Write, 0, copyBuffer->getSize());
	for (pvr::uint32 i = 0; i < numParticles; ++i, ++tmpData)
	{
		tmpData->vPosition = glm::vec3(((float32)rand() / RAND_MAX) * 50.f - 25.f, ((float32)rand() / RAND_MAX) * 50.f, ((float32)rand() / RAND_MAX) * 50.f - 25.f);
		tmpData->vVelocity = tmpData->vPosition * .2f;
		tmpData->fTimeToLive = ((float32)rand() / RAND_MAX);
	}
	memset(tmpData, 0, copyBuffer->getSize() - (sizeof(Particle) * numParticles));// zero out the remaining entries
	copyBuffer->unmap();
	// copy it to the destination buffer
	cmdStaging = context->createCommandBufferOnDefaultPool();

	cmdStaging->beginRecording();
	cmdStaging->copyBuffer(copyBuffer, particleBufferViewSsbos, 0, 0, copyBuffer->getSize());
	cmdStaging->endRecording();

	api::Fence fence = context->createFence(false);
	cmdStaging->submit(pvr::api::Semaphore(), api::Semaphore(), fence);
	fence->wait(uint32(-1));
        if (fence->isSignalled())
	{
		cmdStaging.reset();
		for (pvr::uint32 i = 0; i < context->getSwapChainLength(); ++i)
		{
			recordCommandBuffer(i);
		}
	}
}

/*!*********************************************************************************************************************
\param[in]	emitter	Set Emitter state to this
\brief	Sets the transformation, height and radius of the active emitter to this state
***********************************************************************************************************************/
void ParticleSystemGPU::setEmitter(const Emitter& emitter) {  particleConfigData.emitter = emitter;}

/*!*********************************************************************************************************************
 \param[in]	g	A 3D vector that is the gravity of the simulation (m*sec^-2)
***********************************************************************************************************************/
void ParticleSystemGPU::setGravity(const glm::vec3& g) {   particleConfigData.vG = g;}

/*!*********************************************************************************************************************
\param[in]			pSpheres	Pointer to an array of Sphere structs
\param[in]			uiNumSpheres	The number of spheres
\brief	Sets the physical model of the collision spheres and initializes them.
***********************************************************************************************************************/
bool ParticleSystemGPU::setCollisionSpheres(const Sphere* spheres, pvr::uint32 numSpheres)
{
	if (numSpheres)
	{
		collisonSpheresUbo.addEntriesPacked(SphereViewMapping, sizeof(SphereViewMapping) / sizeof(SphereViewMapping[0]));
		collisonSpheresUbo.finalize(context, 1, pvr::types::BufferBindingUse::UniformBuffer, false, false);
		collisonSpheresUbo.connectWithBuffer(0, context->createBufferAndView(collisonSpheresUbo.getAlignedElementSize(),
		                                     BufferBindingUse::UniformBuffer, true));

		collisonSpheresUbo.map(0, MapBufferFlags::Write);
		for (pvr::uint32 i = 0; i < numSpheres; ++i)
		{
			collisonSpheresUbo.setValue(SphereViewElements::PositionRadius + i,
			                            glm::vec4(spheres[i].vPosition, spheres[i].fRadius));
		}
		collisonSpheresUbo.unmap(0);
	}
	return true;
}

/*!*********************************************************************************************************************
\brief	record compute commands
\param	pvr::api::CommandBufferBase cmdBuffer
\param	pvr::uint8 swapchain
***********************************************************************************************************************/
void ParticleSystemGPU::recordCommandBuffer(pvr::uint8 swapchain)
{
	if (!cmdBuffer[swapchain].isValid())
	{
		cmdBuffer[swapchain] = context->createCommandBufferOnDefaultPool();
	}
	cmdBuffer[swapchain]->beginRecording();
	cmdBuffer[swapchain]->bindPipeline(pipe);
	cmdBuffer[swapchain]->bindDescriptorSets(PipelineBindPoint::Compute, pipe->getPipelineLayout(), 0,
	    &multiBuffer.descSets[swapchain], 1);
	cmdBuffer[swapchain]->dispatchCompute(numParticles / workgroupSize, 1, 1);
	cmdBuffer[swapchain]->endRecording();
}
