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
/*!*********************************************************************************************************************
\brief ctor
***********************************************************************************************************************/
ParticleSystemGPU::ParticleSystemGPU(pvr::Shell& assetLoader)	:
	context(assetLoader.getGraphicsContext()), assetProvider(assetLoader),
	computeShaderSrcFile("ParticleSolver.csh"), workgroupSize(32),
	/*SIMULATION DATA*/
	numParticles(0), particleArrayData(0), numSpheres(0)
{
	memset(&particleConfigData, 0, sizeof(ParticleConfig));
}

/*!*********************************************************************************************************************
\brief	Cleans up any resources that were constructed by this class
***********************************************************************************************************************/
ParticleSystemGPU::~ParticleSystemGPU()
{
	for (int i = 0; i < NumBuffers; ++i)
	{
		descSets[i].reset();
		particleBufferViewSsbos[i].reset();
	}
	particleConfigUbo.reset();
	collisonSpheresUbo.reset();
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
bool ParticleSystemGPU::init(pvr::string& errorStr)
{
	if (!createComputePipeline(errorStr)) { return false; }
	pvr::api::Buffer buffer = context->createBuffer(sizeof(ParticleConfig), BufferBindingUse::UniformBuffer);
	particleConfigUbo = context->createBufferView(buffer, 0, sizeof(ParticleConfig));
	particleConfigUbo->update(&particleConfigData, 0, sizeof(ParticleConfig));
	for (pvr::uint8 i = 0; i < NumBuffers; ++i)
	{
		descSets[i] = context->createDescriptorSetOnDefaultPool(pipe->getPipelineLayout()->getDescriptorSetLayout(0));
	}
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
	.setBinding(1, DescriptorType::UniformBuffer, 1, ShaderStageFlags::Compute)
	.setBinding(2, DescriptorType::UniformBuffer, 1, ShaderStageFlags::Compute)
	.setBinding(3, DescriptorType::StorageBuffer, 1, ShaderStageFlags::Compute)
	.setBinding(4, DescriptorType::StorageBuffer, 1, ShaderStageFlags::Compute);
	pipeLayoutInfo.setDescSetLayout(0, context->createDescriptorSetLayout(descSetLayoutInfo));

	pvr::api::debugLogApiError("ComputeShaderProgramState::generate enter");
	pvr::api::ComputePipelineCreateParam pipeCreateInfo;
	std::string defines("WORKGROUP_SIZE               ", 30);
	sprintf(&defines[15], "%d", workgroupSize);
	const char* defines_buffer[1] = { &defines[0] };
	pvr::api::debugLogApiError("ComputeShaderProgramState::generate enter");
	pvr::assets::ShaderFile fileVersioning;
	fileVersioning.populateValidVersions(computeShaderSrcFile, assetProvider);
	pvr::api::Shader shader = context->createShader(*fileVersioning.getBestStreamForApi(context->getApiType()),
	                          ShaderType::ComputeShader, defines_buffer, 1);
	if (shader.isNull()) {	return false;	}
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
void ParticleSystemGPU::updateUniforms(pvr::float32 dt)
{
	if (dt == 0) { return; }
	dt *= 0.001f;
	particleConfigData.fDt = dt;
	particleConfigData.fTotalTime += dt;
	particleConfigUbo->update(&particleConfigData, 0, sizeof(particleConfigData));
}

/*!*********************************************************************************************************************
\param[in]	numParticles	Set the number of particles to this number
\brief	Allocates memory for the actual particles and configures the simulation accordingly. Invalidates the following OpenGL state:
		GL_SHADER_STORAGE_BUFFER binding. Call this at least once to create storage for the particles.
***********************************************************************************************************************/
void ParticleSystemGPU::setNumberOfParticles(unsigned int numParticles)
{
	particleArrayData.resize(numParticles);

	for (pvr::uint32 i = 0; i < numParticles; ++i)
	{
		particleArrayData[i].fTimeToLive = ((float)rand() / RAND_MAX);
		particleArrayData[i].vPosition.x = ((float)rand() / RAND_MAX) * 50.f - 25.f;
		particleArrayData[i].vPosition.y = ((float)rand() / RAND_MAX) * 50.f;
		particleArrayData[i].vPosition.z = ((float)rand() / RAND_MAX) * 50.f - 25.f;
		particleArrayData[i].vVelocity = particleArrayData[i].vPosition * .2f;
	}
	for (int i = 0; i < NumBuffers; ++i)
	{
		particleBufferViewSsbos[i]->update(particleArrayData.data(), 0, sizeof(Particle)*numParticles);
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
		collisonSpheresUbo = context->createBufferView(context->createBuffer(sizeof(Sphere) * numSpheres, BufferBindingUse::UniformBuffer), 0, sizeof(Sphere) * numSpheres);
		collisonSpheresUbo->update(spheres, 0, sizeof(Sphere)* numSpheres);
	}
	return true;
}


/*!*********************************************************************************************************************
\brief	record compute commands
\param	pvr::api::CommandBufferBase cmdBuffer
\param	pvr::uint8 idx
***********************************************************************************************************************/
void ParticleSystemGPU::recordCommandBuffer(pvr::api::CommandBufferBase cmdBuffer, pvr::uint8 idx)
{
	cmdBuffer->bindPipeline(pipe);
	cmdBuffer->bindDescriptorSets(PipelineBindPoint::Compute, pipe->getPipelineLayout(), 0, &descSets[idx], 1);
	cmdBuffer->dispatchCompute(particleArrayData.size() / workgroupSize, 1, 1);
}

/*!*********************************************************************************************************************
\brief	set the particle vertex buffer
\param	const pvr::api::Buffer * particleVbos
***********************************************************************************************************************/
void ParticleSystemGPU::setParticleVboBuffers(const pvr::api::Buffer* particleVbos)
{
	for (pvr::uint8 i = 0; i < NumBuffers; ++i)
	{
		particleBufferViewSsbos[i] = context->createBufferView(particleVbos[i], 0, particleVbos[i]->getSize());
	}
	for (pvr::uint8 i = 0; i < NumBuffers; ++i)
	{
		int id_in = (i ? i : NumBuffers);
		id_in = (id_in - 1) % NumBuffers;
		int id_out = (id_in + 1) % NumBuffers;
		pvr::api::DescriptorSetUpdate descSetInfo;
		descSetInfo.setUbo(PARTICLE_CONFIG_UBO_BINDING_INDEX, particleConfigUbo)
		.setUbo(SPHERES_UBO_BINDING_INDEX, collisonSpheresUbo)
		.setSsbo(PARTICLES_SSBO_BINDING_INDEX_IN, particleBufferViewSsbos[id_in])
		.setSsbo(PARTICLES_SSBO_BINDING_INDEX_OUT, particleBufferViewSsbos[id_out]);
		descSets[i]->update(descSetInfo);
	}
}
