/*!****************************************************************************
@File          ParticleSystemGPU.cpp

@Title         ParticleSystemGPU

@Copyright     Copyright (C) by Imagination Technologies Limited.

@Platform      Independent

@Description   Particle system implemented using GPU Compute SHaders.
               Uses direct access to the VBOs as SSBOs
******************************************************************************/

#include "ParticleSystemGPU.h"

/*!****************************************************************************
 @Function		Constructor
 @Input         pvrContext         The PVR context that will be used for
				extension loading
 @Return        The newly constructed object
 @Description	Initializes default values for class members, leaving a class
				that is safe to Init or destroy. NO INITIALIZATION, see
				Init(...) for that.
******************************************************************************/
ParticleSystemGPU::ParticleSystemGPU(SPVRTContext& pvrContext)
	:
/*CONTEXT*/ m_PVRTContext(pvrContext),
/*SHADERS*/
	c_szComputeShaderBinFile("ParticleSolver.csc"),	c_szComputeShaderSrcFile("ParticleSolver.csh"),
	m_glProgram(0), m_glComputeShader(0),
	m_ui32MaxWorkgroupSize(256), m_ui32WorkgroupSize(256),

/*SIMULATION DATA*/
	m_ui32NumParticles(0), m_pParticleArrayData(0), m_ui32NumSpheres(0),

/*OPENGL BUFFER OBJECTS*/
	m_ParticleConfigUbo(0),	m_ParticleArrayVboSsbo(0), m_SpheresUbo(0)
{
	ParticleConfig tmp = {{PVRTMat4::Identity()}};
	m_ParticleConfigData = tmp;
}


/*!****************************************************************************
 @Function		Destructor
 @Description	Cleans up any resources that were constructed by this class
******************************************************************************/
ParticleSystemGPU::~ParticleSystemGPU()
{
	if (m_glProgram != 0) { glDeleteProgram(m_glProgram); }
	if (m_glComputeShader != 0) { glDeleteShader(m_glComputeShader); }
	if (m_ParticleConfigUbo) { glDeleteBuffers(1, &m_ParticleConfigUbo); }
	if (m_SpheresUbo) { glDeleteBuffers(1, &m_SpheresUbo); }

	delete [] m_pParticleArrayData;
}


/*!****************************************************************************
 @Function		Init
 @Input			ErrorStr	In case of error, any error messages will be stored
				in this string
 @Return		true if successfully completed, false otherwise.
 @Description	Initializes any state that needs to be created by the class itself.
				Init WILL NOT initialize objects that need to be set by a caller,
				most importantly:
				* The vertex buffer that will contain the actual particles
				* The actual Spheres
				* The actual number of particles (SetNumberOfParticles actually
				allocates memory for the particles)
******************************************************************************/
bool ParticleSystemGPU::Init(CPVRTString& errorStr)
{
	if (!CompileComputeShader(errorStr)) { return false; }

	if (m_ParticleConfigUbo == 0) { glGenBuffers(1, &m_ParticleConfigUbo); }
	if (m_SpheresUbo == 0) { glGenBuffers(1, &m_SpheresUbo); }

	glBindBuffer(GL_UNIFORM_BUFFER, m_SpheresUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_ParticleConfigUbo);

	glBufferData(GL_UNIFORM_BUFFER, sizeof(ParticleConfig), &m_ParticleConfigData, GL_STREAM_DRAW);

	glBindBufferBase(GL_UNIFORM_BUFFER, SPHERES_UBO_BINDING_INDEX, m_SpheresUbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, PARTICLE_CONFIG_UBO_BINDING_INDEX, m_ParticleConfigUbo);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return true;
}

bool ParticleSystemGPU::CompileComputeShader(CPVRTString& errorStr)
{
	CPVRTArray<char> defines("WORKGROUP_SIZE               ", 30);
	sprintf(&defines[15], "%d", m_ui32WorkgroupSize);
	char* defines_buffer = &defines[0];

	if (m_glComputeShader) { glDeleteShader(m_glComputeShader); m_glComputeShader = 0; }
	if (m_glProgram) { glDeleteProgram(m_glProgram); m_glProgram = 0; }

	if (PVRTShaderLoadFromFile(c_szComputeShaderBinFile, c_szComputeShaderSrcFile, GL_COMPUTE_SHADER, GL_SGX_BINARY_IMG, &m_glComputeShader, &errorStr, &m_PVRTContext, &defines_buffer, 1) != PVR_SUCCESS)
	{ errorStr = "Particle Compute Shader : " + errorStr; return false; }

	m_glProgram = glCreateProgram();
	glAttachShader(m_glProgram, m_glComputeShader);
	glLinkProgram(m_glProgram);
	GLint Linked;
	glGetProgramiv(m_glProgram, GL_LINK_STATUS, &Linked);

	if (!Linked)
	{
		int i32InfoLogLength, i32CharsWritten;
		glGetProgramiv(m_glProgram, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetProgramInfoLog(m_glProgram, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		errorStr = CPVRTString("Failed to link compute shader:: ") + pszInfoLog + "\n";
		delete [] pszInfoLog;
		return false;
	}
	return true;
}

/*!****************************************************************************
 @Function		Update
 @Input			dt	Elapsed time from last iteration (frame)
 @Description	Advances the simulation by dt. Invalidates the following OpenGL
				state:Current program, GL_UNIFORM_BUFFER binding
******************************************************************************/
void ParticleSystemGPU::Update(float dt)
{
	if (dt == 0) { return; }
	dt *= 0.001f;
	GLuint numGroups = m_ui32NumParticles / m_ui32WorkgroupSize;

	m_ParticleConfigData.fDt = dt;
	m_ParticleConfigData.fTotalTime += dt;

	glBindBuffer(GL_UNIFORM_BUFFER, m_ParticleConfigUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_ParticleConfigData), &m_ParticleConfigData, GL_STREAM_DRAW);

	glUseProgram(m_glProgram);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(numGroups, 1, 1);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}


/*!****************************************************************************
 @Function		SetNumberOfParticles
 @Input			numParticles	Set the number of particles to this number
 @Description	Allocates memory for the actual particles and configures the
				simulation accordingly. Invalidates the following OpenGL state:
				GL_SHADER_STORAGE_BUFFER binding. Call this at least once to
				create storage for the particles.
******************************************************************************/
bool ParticleSystemGPU::SetNumberOfParticles(unsigned int numParticles)
{
	m_ui32NumParticles = numParticles;

	delete m_pParticleArrayData;
	m_pParticleArrayData = new Particle[numParticles];

	for (unsigned int i = 0; i < numParticles; i++)
	{
		m_pParticleArrayData[i].fTimeToLive = ((float)rand() / RAND_MAX);
		m_pParticleArrayData[i].vPosition.x = ((float)rand() / RAND_MAX) * 50.f - 25.f;
		m_pParticleArrayData[i].vPosition.y = ((float)rand() / RAND_MAX) * 50.f;
		m_pParticleArrayData[i].vPosition.z = ((float)rand() / RAND_MAX) * 50.f - 25.f;
		m_pParticleArrayData[i].vVelocity = m_pParticleArrayData[i].vPosition * .2;
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ParticleArrayVboSsbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle)*numParticles, m_pParticleArrayData, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, PARTICLES_SSBO_BINDING_INDEX, m_ParticleArrayVboSsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return true;
}


/*!****************************************************************************
 @Function		SetEmitter
 @Input			emitter	Set Emitter state to this
 @Description	Sets the transformation, height and radius of the active emitter
				to this state
******************************************************************************/
void ParticleSystemGPU::SetEmitter(const Emitter& emitter)
{
	m_ParticleConfigData.emitter = emitter;
}


/*!****************************************************************************
 @Function		SetGravity
 @Input			g	A 3D vector that is the gravity of the simulation (m*sec^-2)
 ******************************************************************************/
void ParticleSystemGPU::SetGravity(const PVRTVec3& g)
{
	m_ParticleConfigData.vG = g;
}


/*!****************************************************************************
 @Function		SetCollisionSpheres
 @Input			pSpheres	Pointer to an array of Sphere structs
 @Input			uiNumSpheres	The number of spheres
 @Description	Sets the physical model of the collision spheres and initializes them.
 ******************************************************************************/
bool ParticleSystemGPU::SetCollisionSpheres(const CPVRTArray<Sphere>& spheres)
{
	size_t size;
	if (size = spheres.GetSize())
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_SpheresUbo);
		glBufferData(GL_UNIFORM_BUFFER, size * sizeof(Sphere), &spheres[0], GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, SPHERES_UBO_BINDING_INDEX, m_SpheresUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	return true;
}
