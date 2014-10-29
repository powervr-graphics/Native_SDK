/*!****************************************************************************
 @File          ParticleSystemGPU.h

 @Title         ParticleSystemGPU

 @Copyright     Copyright (C) by Imagination Technologies Limited.

 @Platform      Independent

 @Description   Particle system implemented using direct manipulation of
				the VBOs in order to implement zero-copy operations on the GPU.
******************************************************************************/

#ifndef _PARTICLESYSTEMGPU__H_
#define _PARTICLESYSTEMGPU__H_

#include "OGLES31Tools.h"

/******************************************************************************
 Structure definitions
******************************************************************************/

//The particle structure will be kept packed. We will have to be careful with strides
struct Particle
{
	PVRTVec3 vPosition; //vec3
	float _padding;
	PVRTVec3 vVelocity;	//vec4.xyz
	float fTimeToLive;	//vec4/w
};//SIZE:32 bytes

//All the following will all be used in uniforms/ssbos, so we will mimic the alignment of std140 glsl
//layout spec in order to make their use simpler
struct Sphere
{
	PVRTVec3 vPosition; //vec4: xyz
	float fRadius;      //vec4: w
};

struct Emitter
{
	PVRTMat4	mTransformation;	//mat4
	float		fHeight;			//float
	float		fRadius;			//float
};


struct ParticleConfig
{
	Emitter		emitter;		//Emitter will need 2 floats padding to be a			//18 floats
	float		_padding1[2];	//multiple of 16 (vec4 size)							//20 floats
	PVRTVec3	vG;				//vec3													//23 floats
	float		fDt;			//simple float											//24 floats
	float		fTotalTime;		//simple float											//25 floats
	float		_padding2[3];	//std140 dictates that the size of the ubo will be		//28 floats
	//size of the 1st element of an array, so we must upload
	//enough data for it to be a multiple of vec4(i.e. 4floats/16 bytes : 25->28)
};


class ParticleSystemGPU
{
public:
	ParticleSystemGPU(SPVRTContext& pvrContext);
	virtual ~ParticleSystemGPU();

	bool Init(CPVRTString& ErrorStr);

	void Update(float step);
	bool SetNumberOfParticles(unsigned int numParticles);
	void SetParticleVbo(GLuint particleVbo)
	{
		m_ParticleArrayVboSsbo = particleVbo;
	}
	unsigned int GetNumberOfParticles() const { return m_ui32NumParticles; }
	void SetEmitter(const Emitter& emitter);
	void SetGravity(const PVRTVec3& g);
	void SetWorkGroupSize(unsigned int wgsize);
	unsigned int GetWorkGroupSize(void) const { return m_ui32WorkgroupSize; }
	const Particle* GetParticleArray(void) { return m_pParticleArrayData; }
	bool SetCollisionSpheres(const CPVRTArray<Sphere>& spheres);

protected:
	bool CompileComputeShader(CPVRTString& errorStr);

	enum BufferBindingPoint
	{
	  PARTICLES_SSBO_BINDING_INDEX = 1,
	  SPHERES_UBO_BINDING_INDEX = 2,
	  PARTICLE_CONFIG_UBO_BINDING_INDEX = 3
	};


	//CONTEXT
	SPVRTContext& m_PVRTContext;

	//SHADERS
	const char* c_szComputeShaderSrcFile;
	const char* c_szComputeShaderBinFile;
	GLuint m_glProgram;
	GLuint m_glComputeShader;

	//SIMULATION DATA
	PVRTVec3 m_afGravity;
	PVRTuint32 m_ui32NumParticles;
	PVRTuint32 m_ui32MaxWorkgroupSize;
	PVRTuint32 m_ui32WorkgroupSize;
	PVRTuint32 m_ui32NumSpheres;

	ParticleConfig m_ParticleConfigData;
	Particle* m_pParticleArrayData;

	//OPENGL BUFFER OBJECTS
	GLuint m_ParticleArrayVboSsbo;
	GLuint m_ParticleConfigUbo;
	GLuint m_SpheresUbo;

private:
	//DISABLE COPY CONSTRUCT AND ASSIGN
	ParticleSystemGPU(ParticleSystemGPU&);
	ParticleSystemGPU& operator=(ParticleSystemGPU&);
};

#endif // _ParticleSystemGPU__H_


/******************************************************************************
 End of file (ParticleSystemGPU.h)
******************************************************************************/
