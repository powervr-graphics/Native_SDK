#version 320 es

// Uniform buffer to set the ambient occlusion parameters
layout(set = 0, binding = 0) uniform computeSimulationSettingsBuffer
{
	int numberParticles;    // Number of particles in the simulation
	float deltaTime;        // Delta time each frame in seconds
	float lowerBoundX;      // Bounds in the x dimension for the volume where the particle simulation is being performed
	float upperBoundX;      // Bounds in the x dimension for the volume where the particle simulation is being performed
	float lowerBoundY;      // Bounds in the y dimension for the volume where the particle simulation is being performed
	float upperBoundY;      // Bounds in the y dimension for the volume where the particle simulation is being performed
	float lowerBoundZ;      // Bounds in the z dimension for the volume where the particle simulation is being performed
	float upperBoundZ;      // Bounds in the z dimension for the volume where the particle simulation is being performed
	float radius;           // Particle radius
	float coreRadius;       // Particle core radius
	float mass;             // Particle mass
	float fluidRestDensity; // Particle rest density
	float fluidStiffness;   // Fluid stiffness
	float fluidViscosity;   // Fluid viscosity
	float speedDecay;       // Particle speed decay
	vec3 gravity;           // Particle gravity
};

struct ParticleStruct
{
	vec4 positionDensity;  // Particle position in xyz fields, particle density in w field
	vec4 velocityPressure; // Particle velocity in xyz fields, particle pressure in w field
};

layout(std430, set = 0, binding = 1) buffer Particle
{
	ParticleStruct particles[];
};

#define PI 3.141592653589793
#define NUM_NEIGHBOUR_PARTICLE_PER_THREAD 512
#define NUM_THREAD_PER_PARTICLE 8

layout(local_size_x = 1, local_size_y = NUM_THREAD_PER_PARTICLE, local_size_z = 1) in;

// This shader computes the pressure force and viscosity force. For more information see Algorithm 1 or equations 2.8 and 2.11 in
// "Accelerating Fluids Simulation Using SPH and Implementation on GPU" (http://uu.diva-portal.org/smash/get/diva2:885188/FULLTEXT01.pdf).
// To compute pressure force the gradient of the spiky smoothing kernel is applied.
// To compute viscosity force the Laplacian of the spiky smoothing kernel is applied.

shared vec3 arrayPressureForceAccumulated[NUM_THREAD_PER_PARTICLE]; // One per workgroup thread
shared vec3 arrayViscosityForceAccumulated[NUM_THREAD_PER_PARTICLE]; // One per workgroup thread

void main()
{
	int i = int(gl_GlobalInvocationID.x);
	int localThreadIndex = int(gl_LocalInvocationID.y);

	float particleIDensity = particles[i].positionDensity.w;
	float particleIPressure = particles[i].velocityPressure.w;
	
	float coreRadiusPow2 = pow(coreRadius, 2.0);
	vec3 pressureForceAccumulated = vec3(0.0);
	vec3 viscosityForceAccumulated = vec3(0.0);

	for (int j = (localThreadIndex * NUM_NEIGHBOUR_PARTICLE_PER_THREAD); j < (localThreadIndex * NUM_NEIGHBOUR_PARTICLE_PER_THREAD + NUM_NEIGHBOUR_PARTICLE_PER_THREAD); ++j)
	{
		if (i == j)
		{
			continue;
		}

		float particleDistanceSq = dot(particles[i].positionDensity.xyz - particles[j].positionDensity.xyz, particles[i].positionDensity.xyz - particles[j].positionDensity.xyz);
		if (particleDistanceSq < coreRadiusPow2)
		{
			float particleDistance = sqrt(particleDistanceSq);
			float particleJDensity = particles[j].positionDensity.w;
			float particleJPressure = particles[j].velocityPressure.w;
			float particleIJDensity = particleIDensity * particleJDensity;

			vec3 particleDirectionJToI = particles[i].positionDensity.xyz - particles[j].positionDensity.xyz;
			pressureForceAccumulated += ((particleIPressure + particleJPressure) / (2.0 * particleJDensity)) * normalize(particleDirectionJToI) * pow(coreRadius - particleDistance, 2.0);

			vec3 velocityVectorItoJ = particles[j].velocityPressure.xyz - particles[i].velocityPressure.xyz;
			viscosityForceAccumulated += (velocityVectorItoJ / particleJDensity) * (coreRadius - particleDistance);
		}
	}

	if(any(isnan(pressureForceAccumulated)))
	{
		pressureForceAccumulated = vec3(0.0);
	}
	if(any(isnan(viscosityForceAccumulated)))
	{
		viscosityForceAccumulated = vec3(0.0);
	}

	float kernelLastPart = mass * (45.0 / (PI * pow(coreRadius, 6.0)));
	arrayPressureForceAccumulated[localThreadIndex] = pressureForceAccumulated * kernelLastPart;
	arrayViscosityForceAccumulated[localThreadIndex] = viscosityForceAccumulated * kernelLastPart;

	// Add all the values each thread computed and stored in the shared variables arrayPressureForceAccumulated and arrayViscosityForceAccumulated
	barrier();

	if(mod(float(localThreadIndex), 2.0) == 0.0)
	{
		arrayPressureForceAccumulated[localThreadIndex] += arrayPressureForceAccumulated[localThreadIndex + 1];
		arrayViscosityForceAccumulated[localThreadIndex] += arrayViscosityForceAccumulated[localThreadIndex + 1];
	}

	barrier();

	if(mod(float(localThreadIndex), 4.0) == 0.0)
	{
		arrayPressureForceAccumulated[localThreadIndex] += arrayPressureForceAccumulated[localThreadIndex + 2];
		arrayViscosityForceAccumulated[localThreadIndex] += arrayViscosityForceAccumulated[localThreadIndex + 2];
	}

	barrier();

	if (localThreadIndex == 0)
	{
		arrayPressureForceAccumulated[localThreadIndex] += arrayPressureForceAccumulated[localThreadIndex + 4];
		arrayViscosityForceAccumulated[localThreadIndex] += arrayViscosityForceAccumulated[localThreadIndex + 4];

		vec3 acc = arrayPressureForceAccumulated[localThreadIndex] + (fluidViscosity * arrayViscosityForceAccumulated[localThreadIndex]) + gravity;
		vec3 velocity = particles[i].velocityPressure.xyz + deltaTime * acc;
		particles[i].velocityPressure.xyz = velocity;
	}
}
