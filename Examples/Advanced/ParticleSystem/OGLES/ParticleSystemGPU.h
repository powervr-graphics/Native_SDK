/*!*********************************************************************************************************************
\File         ParticleSystemGPU.h
\Title        ParticleSystemGPU
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description  Particle system implemented using direct manipulation of the VBOs in order to implement zero-copy operations on the GPU.
***********************************************************************************************************************/
#pragma once

#include "PVRShell/PVRShell.h"
#include "PVRApi/PVRApi.h"
#include "PVREngineUtils/PVREngineUtils.h"

const pvr::uint32 NumBuffers(2);

//The particle structure will be kept packed. We will have to be careful with strides
struct Particle
{
	glm::vec3 vPosition; //vec3
	float _padding;
	glm::vec3 vVelocity;	//vec4.xyz
	float fTimeToLive;	//vec4/w
};//SIZE:32 bytes

//All the following will all be used in uniforms/ssbos, so we will mimic the alignment of std140 glsl
//layout spec in order to make their use simpler
struct Sphere
{
	glm::vec3 vPosition; //vec4: xyz
	float fRadius;      //vec4: w
	Sphere(const glm::vec3& pos, float radius) : vPosition(pos), fRadius(radius) {}
};

struct Emitter
{
	glm::mat4	mTransformation;	//mat4
	float		fHeight;			//float
	float		fRadius;			//float
	Emitter(const glm::mat4& trans, float height, float radius) : mTransformation(trans), fHeight(height), fRadius(radius) {}
	Emitter() {}
};


struct ParticleConfig
{
	Emitter		emitter;		//Emitter will need 2 floats padding to be a			//18 floats
	float		_padding1[2];	//multiple of 16 (vec4 size)							//20 floats
	glm::vec3	vG;				//vec3													//23 floats
	float		fDt;			//simple float											//24 floats
	float		fTotalTime;		//simple float											//25 floats
	float		_padding2[3];	//std140 dictates that the size of the ubo will be		//28 floats
	//size of the 1st element of an array, so we must upload
	//enough data for it to be a multiple of vec4(i.e. 4floats/16 bytes : 25->28)
	ParticleConfig() {}
};


class ParticleSystemGPU
{
public:
	ParticleSystemGPU(pvr::Shell& assetLoader);
	virtual ~ParticleSystemGPU();

	bool init(std::string& ErrorStr);

	void updateUniforms(float step);
	void setNumberOfParticles(unsigned int numParticles);
	void setParticleVboBuffers(const pvr::api::Buffer* particleVbos);
	pvr::uint32 getNumberOfParticles() const { return particleArrayData.size(); }
	void setEmitter(const Emitter& emitter);
	void setGravity(const glm::vec3& g);
	void SetWorkGroupSize(unsigned int wgsize);
	unsigned int getWorkGroupSize(void) const { return workgroupSize; }
	const Particle* getParticleArray(void)const { return particleArrayData.data(); }
	bool setCollisionSpheres(const Sphere* spheres, pvr::uint32 numSpheres);
	void createDescriptorSet();

	void setPiplineConfigs();
	void recordCommandBuffer(pvr::api::CommandBufferBase cmdBuffer, pvr::uint8 idx);
protected:
	bool createComputePipeline(std::string& errorStr);

	enum BufferBindingPoint
	{
		SPHERES_UBO_BINDING_INDEX = 1,
		PARTICLE_CONFIG_UBO_BINDING_INDEX = 2,
		PARTICLES_SSBO_BINDING_INDEX_IN = 3,
		PARTICLES_SSBO_BINDING_INDEX_OUT = 4,
	};


	//CONTEXT
	pvr::GraphicsContext context;

	//SHADERS
	const char* computeShaderSrcFile;
	pvr::api::ComputePipeline pipe;

	//SIMULATION DATA
	glm::vec3 gravity;
	pvr::uint32 numParticles;
	pvr::uint32 maxWorkgroupSize;
	pvr::uint32 workgroupSize;
	pvr::uint32 numSpheres;

	ParticleConfig particleConfigData;
	std::vector<Particle> particleArrayData;

	//OPENGL BUFFER OBJECTS
	pvr::api::BufferView particleBufferViewSsbos[NumBuffers];
	pvr::api::BufferView  particleConfigUbo, collisonSpheresUbo;
	pvr::IAssetProvider& assetProvider;
	pvr::api::DescriptorSet  descSets[NumBuffers];

private:
	//DISABLE COPY CONSTRUCT AND ASSIGN
	ParticleSystemGPU(ParticleSystemGPU&);
	ParticleSystemGPU& operator=(ParticleSystemGPU&);
};