/*!*********************************************************************************************************************
\File         ParticleSystemGPU.h
\Title        ParticleSystemGPU
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description  Particle system implemented using direct manipulation of the VBOs in order to implement zero-copy operations on the GPU.
***********************************************************************************************************************/
#pragma once

#include "PVRShell/PVRShell.h"
#include "PVRCore/Strings/StringHash.h"
#include "PVRUtils/PVRUtilsVk.h"
const char* const ComputeShaderFileName = "ParticleSolver_vk.csh.spv";
const uint32_t NumBuffers(2);
const uint32_t MaxSwapChains = uint32_t(pvrvk::FrameworkCaps::MaxSwapChains);
// The particle structure will be kept packed. We will have to be careful with strides
struct Particle
{
	glm::vec3 vPosition; // vec3
	float _pad;
	glm::vec3 vVelocity; // vec4.xyz
	float fTimeToLive; // vec4/w
}; // SIZE:32 bytes

const pvr::utils::StructuredMemoryDescription ParticleViewMapping(
	"ParticlesBuffer", 1, { { "vPosition", pvr::GpuDatatypes::vec3 }, { "vVelocity", pvr::GpuDatatypes::vec3 }, { "fTimeToLive", pvr::GpuDatatypes::Float } });

namespace ParticleViewElements {
enum Enum
{
	Position,
	Velocity,
	TimeToLive
};
}

// All the following will all be used in uniforms/ssbos, so we will mimic the alignment of std140 glsl
// layout spec in order to make their use simpler
struct Sphere
{
	glm::vec3 vPosition; // vec4: xyz
	float fRadius; // vec4: w
	Sphere(const glm::vec3& pos, float radius) : vPosition(pos), fRadius(radius) {}
};

const pvr::utils::StructuredMemoryDescription SphereViewMapping("SphereBuffer", 1, { { "SphereArray", 8, pvr::GpuDatatypes::vec4 } });

namespace SphereViewElements {
enum Enum
{
	PositionRadius
};
}

struct Emitter
{
	glm::mat4 mTransformation; // mat4
	float fHeight; // float
	float fRadius; // float
	Emitter(const glm::mat4& trans, float height, float radius) : mTransformation(trans), fHeight(height), fRadius(radius) {}
	Emitter() {}
};

const pvr::utils::StructuredMemoryDescription ParticleConfigViewMapping("ParticleConfig", 1,
	{
		// Emitter
		{ "mTransformation", pvr::GpuDatatypes::mat4x4 },
		{ "fHeight", pvr::GpuDatatypes::Float },
		{ "fRadius", pvr::GpuDatatypes::Float },
		{ "vG", pvr::GpuDatatypes::vec3 },
		{ "fDt", pvr::GpuDatatypes::Float },
		{ "fTotalTime", pvr::GpuDatatypes::Float },
	});

namespace ParticleConfigViewElements {
enum Enum
{
	EmitterTransform,
	EmitterHeight,
	EmitterRadius,
	Gravity,
	DeltaTime,
	TotalTime
};
}

struct ParticleConfig
{
	Emitter emitter;
	glm::vec3 vG;
	float fDt;
	float fTotalTime;
	// size of the 1st element of an array, so we must upload
	// enough data for it to be a multiple of vec4(i.e. 4floats/16 bytes : 25->28)
	ParticleConfig() {}

	void updateBufferView(pvr::utils::StructuredBufferView& view, pvrvk::Buffer& buffer, uint32_t swapidx)
	{
		view.getElement(ParticleConfigViewElements::EmitterTransform, 0, swapidx).setValue(emitter.mTransformation);
		view.getElement(ParticleConfigViewElements::EmitterHeight, 0, swapidx).setValue(emitter.fHeight);
		view.getElement(ParticleConfigViewElements::EmitterRadius, 0, swapidx).setValue(emitter.fRadius);
		view.getElement(ParticleConfigViewElements::Gravity, 0, swapidx).setValue(vG);
		view.getElement(ParticleConfigViewElements::DeltaTime, 0, swapidx).setValue(fDt);
		view.getElement(ParticleConfigViewElements::TotalTime, 0, swapidx).setValue(fTotalTime);

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			buffer->getDeviceMemory()->flushRange(view.getDynamicSliceOffset(swapidx), view.getDynamicSliceSize());
		}
	}
};

class ParticleSystemGPU
{
public:
	ParticleSystemGPU(pvr::Shell& assetLoader);
	~ParticleSystemGPU();

	void init(uint32_t maxParticles, const Sphere* spheres, uint32_t numSpheres, pvrvk::Device& device, pvrvk::CommandPool& commandPool, pvrvk::DescriptorPool& descriptorPool,
		uint32_t numSwapchains, pvr::utils::vma::Allocator allocator);

	void updateUniforms(uint32_t swapchain, float dt);
	void setNumberOfParticles(uint32_t numParticles, pvrvk::Queue& queue, pvr::utils::vma::Allocator allocator);
	uint32_t getNumberOfParticles() const
	{
		return numParticles;
	}
	void setEmitter(const Emitter& emitter);
	void setGravity(const glm::vec3& g);
	unsigned int getWorkGroupSize(void) const
	{
		return workgroupSize;
	}
	void setCollisionSpheres(const Sphere* spheres, uint32_t numSpheres, pvr::utils::vma::Allocator allocator);
	pvrvk::Buffer getParticleBufferView() const
	{
		return particleBufferViewSsbos;
	}
	void recordCommandBuffer(uint8_t swapchain);

	pvrvk::SecondaryCommandBuffer& getCommandBuffer(uint32_t swapIndex)
	{
		return commandBuffer[swapIndex];
	}

private:
	void createComputePipeline();

	enum BufferBindingPoint
	{
		SPHERES_UBO_BINDING_INDEX = 0,
		PARTICLE_CONFIG_UBO_BINDING_INDEX = 1,
		PARTICLES_SSBO_BINDING_INDEX_IN_OUT = 2
	};

	// SHADERS
	const char* computeShaderSrcFile;
	pvrvk::ComputePipeline pipe;

	// SIMULATION DATA
	glm::vec3 gravity;
	uint32_t numParticles;
	uint32_t maxWorkgroupSize;
	uint32_t workgroupSize;
	uint32_t numSpheres;
	uint32_t swapchainLength;
	ParticleConfig particleConfigData;

	// OPENGL BUFFER OBJECTS
	pvr::utils::StructuredBufferView collisonSpheresUboBufferView;
	pvrvk::Buffer collisonSpheresUbo;
	pvr::IAssetProvider& assetProvider;

	pvrvk::Buffer stagingBuffer;
	pvrvk::Buffer particleBufferViewSsbos;
	struct MultiBuffer
	{
		pvrvk::DescriptorSet descSets[MaxSwapChains];
	};

	pvr::utils::StructuredBufferView particleConfigUboBufferView;
	pvrvk::Buffer particleConfigUbo;

	pvrvk::CommandBuffer commandStaging;
	MultiBuffer multiBuffer;
	pvrvk::Fence stagingFence;
	pvrvk::SecondaryCommandBuffer commandBuffer[MaxSwapChains];
	pvrvk::CommandPool commandPool;
	pvrvk::Device device;

	// DISABLE COPY CONSTRUCT AND ASSIGN
	ParticleSystemGPU(ParticleSystemGPU&);
	ParticleSystemGPU& operator=(ParticleSystemGPU&);
};
