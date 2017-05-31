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
using namespace pvr;
const char* const ComputeShaderFileName = "ParticleSolver_vk.csh.spv";
typedef std::pair<pvr::StringHash, pvr::types::GpuDatatypes::Enum> BufferViewMapping;
const pvr::uint32 NumBuffers(2);
const pvr::uint32 MaxSwapChains = 8;
//The particle structure will be kept packed. We will have to be careful with strides
struct Particle
{
	glm::vec3 vPosition;	//vec3
	pvr::float32 _pad;
	glm::vec3 vVelocity;	//vec4.xyz
	pvr::float32 fTimeToLive;		//vec4/w
};//SIZE:32 bytes

const BufferViewMapping ParticleViewMapping[] =
{
	BufferViewMapping("vPosition", pvr::types::GpuDatatypes::vec3),
	BufferViewMapping("vVelocity", pvr::types::GpuDatatypes::vec3),
	BufferViewMapping("fTimeToLive", pvr::types::GpuDatatypes::float32),
};

namespace ParticleViewElements {
enum Enum {Position, Velocity, TimeToLive};
}

//All the following will all be used in uniforms/ssbos, so we will mimic the alignment of std140 glsl
//layout spec in order to make their use simpler
struct Sphere
{
	glm::vec3 vPosition; //vec4: xyz
	float fRadius;      //vec4: w
	Sphere(const glm::vec3& pos, float radius) : vPosition(pos), fRadius(radius) {}
};

const BufferViewMapping SphereViewMapping[] =
{
	BufferViewMapping("aSpheres", pvr::types::GpuDatatypes::vec4),
	BufferViewMapping("aSpheres", pvr::types::GpuDatatypes::vec4),
	BufferViewMapping("aSpheres", pvr::types::GpuDatatypes::vec4),
	BufferViewMapping("aSpheres", pvr::types::GpuDatatypes::vec4),
	BufferViewMapping("aSpheres", pvr::types::GpuDatatypes::vec4),
	BufferViewMapping("aSpheres", pvr::types::GpuDatatypes::vec4),
	BufferViewMapping("aSpheres", pvr::types::GpuDatatypes::vec4),
	BufferViewMapping("aSpheres", pvr::types::GpuDatatypes::vec4),
};

namespace SphereViewElements {
enum Enum { PositionRadius };
}

struct Emitter
{
	glm::mat4	mTransformation;	//mat4
	float		fHeight;			//float
	float		fRadius;			//float
	Emitter(const glm::mat4& trans, float height, float radius) :
		mTransformation(trans), fHeight(height), fRadius(radius) {}
	Emitter() {}
};


const BufferViewMapping ParticleConfigViewMapping[] =
{
	//Emitter
	BufferViewMapping("mTransformation", pvr::types::GpuDatatypes::mat4x4),
	BufferViewMapping("fHeight", pvr::types::GpuDatatypes::float32),
	BufferViewMapping("fRadius", pvr::types::GpuDatatypes::float32),

	BufferViewMapping("vG", pvr::types::GpuDatatypes::vec3),
	BufferViewMapping("fDt", pvr::types::GpuDatatypes::float32),
	BufferViewMapping("fTotalTime", pvr::types::GpuDatatypes::float32),
};

namespace ParticleConfigViewElements {
enum Enum { EmitterTransform, EmitterHeight, EmitterRadius, Gravity, DeltaTime, TotalTime	};
}

struct ParticleConfig
{
	Emitter		emitter;
	glm::vec3	vG;
	float		fDt;
	float		fTotalTime;
	//size of the 1st element of an array, so we must upload
	//enough data for it to be a multiple of vec4(i.e. 4floats/16 bytes : 25->28)
	ParticleConfig() {}

	void updateBufferView(pvr::utils::StructuredMemoryView& view, uint32 swapidx)
	{
		view.map(swapidx, pvr::types::MapBufferFlags::Write);
		view.setValue(ParticleConfigViewElements::EmitterTransform, emitter.mTransformation);
		view.setValue(ParticleConfigViewElements::EmitterHeight, emitter.fHeight);
		view.setValue(ParticleConfigViewElements::EmitterRadius, emitter.fRadius);
		view.setValue(ParticleConfigViewElements::Gravity, vG);
		view.setValue(ParticleConfigViewElements::DeltaTime, fDt);
		view.setValue(ParticleConfigViewElements::TotalTime, fTotalTime);
		view.unmap(swapidx);
	}
};


class ParticleSystemGPU
{
public:
	ParticleSystemGPU(pvr::Shell& assetLoader);
	virtual ~ParticleSystemGPU();

	bool init(pvr::uint32 maxParticles, const Sphere* sphere, pvr::uint32 numSpheres, std::string& ErrorStr);

	void updateUniforms(pvr::uint32 swapchain, pvr::float32 dt);
	void setNumberOfParticles(pvr::uint32 numParticles);
	pvr::uint32 getNumberOfParticles() const { return numParticles; }
	void setEmitter(const Emitter& emitter);
	void setGravity(const glm::vec3& g);
	unsigned int getWorkGroupSize(void) const { return workgroupSize; }
	bool setCollisionSpheres(const Sphere* spheres, pvr::uint32 numSpheres);
	void createDescriptorSet();
	pvr::api::Buffer getParticleBufferView()const {	return particleBufferViewSsbos; 	}
	void setPiplineConfigs();
	void recordCommandBuffer(pvr::uint8 swapchain);
	pvr::api::Semaphore& getWaitSemaphore(pvr::uint32 swapchain) { return signalSemaphore[swapchain]; }
	void renderFrame(pvr::uint32 swapchain)
	{
		if (cmdBufferFence->isSignalled())
		{
			cmdBufferFence->reset();
		}
		cmdBuffer[swapchain]->submitStartOfFrame(signalSemaphore[swapchain]);
	}
protected:
	bool createComputePipeline(std::string& errorStr);

	enum BufferBindingPoint
	{
		SPHERES_UBO_BINDING_INDEX = 0,
		PARTICLE_CONFIG_UBO_BINDING_INDEX = 1,
		PARTICLES_SSBO_BINDING_INDEX_IN_OUT = 2
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

	//OPENGL BUFFER OBJECTS
	utils::StructuredMemoryView collisonSpheresUbo;
	pvr::IAssetProvider& assetProvider;

	pvr::api::Buffer particleBufferViewSsbos;
	struct MultiBuffer
	{
		pvr::utils::StructuredMemoryView particleConfigUbo;
		pvr::api::DescriptorSet descSets[MaxSwapChains];
	};
	pvr::api::CommandBuffer cmdStaging;
	pvr::api::Semaphore		signalSemaphore[MaxSwapChains];
	MultiBuffer multiBuffer;
	pvr::api::CommandBuffer cmdBuffer[MaxSwapChains];

	pvr::api::Fence cmdBufferFence;
private:
	//DISABLE COPY CONSTRUCT AND ASSIGN
	ParticleSystemGPU(ParticleSystemGPU&);
	ParticleSystemGPU& operator=(ParticleSystemGPU&);
};