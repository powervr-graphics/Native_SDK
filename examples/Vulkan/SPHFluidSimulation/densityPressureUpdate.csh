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

// Each particle's simulation process is done in two steps:
//     + Density and pressure computation.
//     + Pressure force and viscous force computation.
// Following SPH approach, density, pressure, pressure force and viscous force are computed with specific kernels.

// This shader implements the density and pressure computations for the Smoothed Particle Hydrodynamics (SPH) thesis 
// "Accelerating Fluids Simulation Using SPH and Implementation on GPU" (http://uu.diva-portal.org/smash/get/diva2:885188/FULLTEXT01.pdf) [1]

// The density computation is implemented following the Polynomial Kernel suggested by M. Müller, D. Charypar, and M. Gross in
// "Particle-based fluid simulation for interactive applications" (Proceedings of 2003 ACM SIGGRAPH Symposium on Computer Animation)
// also present in eq 2.12 in [1]

// The pressure is computed following the same equation suggested, where p = k * (Rho - Rho0), being k "a gas constant that depends on the temperature",
// and being Rho and Rho0 the fluid density and the fluid rest density, with fluid rest density defined as "The density of a small portion of a fluid in a Lorentz 
// frame in which that portion of the fluid is at rest.".

shared float arrayDensityAccumulatedValue[NUM_THREAD_PER_PARTICLE]; // One per workgroup thread

void main()
{
	int i = int(gl_GlobalInvocationID.x);
	int localThreadIndex = int(gl_LocalInvocationID.y);

	float densityAccumulatedValue = 0.0;
	float coreRadiusPow2 = pow(coreRadius, 2.0);

	for (int j = (localThreadIndex * NUM_NEIGHBOUR_PARTICLE_PER_THREAD); j < (localThreadIndex * NUM_NEIGHBOUR_PARTICLE_PER_THREAD + NUM_NEIGHBOUR_PARTICLE_PER_THREAD); ++j)
	{
		float particleDistanceSq = dot(particles[i].positionDensity - particles[j].positionDensity, particles[i].positionDensity - particles[j].positionDensity);

		if (particleDistanceSq < coreRadiusPow2)
		{
			// Note: The original equation is commented below, using some precomputed values to lower computational cost
			//densityAccumulatedValue += pow(pow(coreRadius, 2.0) - pow(particleDistance, 2.0), 3.0);
			densityAccumulatedValue += pow(coreRadiusPow2 - particleDistanceSq, 3.0);
		}
	}

	arrayDensityAccumulatedValue[localThreadIndex] = densityAccumulatedValue;

	// Add all the values each thread computed and stored in the shared variable arrayDensityAccumulatedValue
	barrier();

	if(mod(float(localThreadIndex), 2.0) == 0.0)
	{
		arrayDensityAccumulatedValue[localThreadIndex] += arrayDensityAccumulatedValue[localThreadIndex + 1];
	}

	barrier();

	if(mod(float(localThreadIndex), 4.0) == 0.0)
	{
		arrayDensityAccumulatedValue[localThreadIndex] += arrayDensityAccumulatedValue[localThreadIndex + 2];
	}

	barrier();

	if (localThreadIndex == 0)
	{
		arrayDensityAccumulatedValue[localThreadIndex] += arrayDensityAccumulatedValue[localThreadIndex + 4];

		// The Polynomial Kernel suggested by [1] is the default kernel used for the calculation of density
		float densityResult = (mass * 315.0 / (64.0 * PI * pow(coreRadius, 9.0))) * arrayDensityAccumulatedValue[localThreadIndex];
		
		particles[i].positionDensity.w = densityResult; // Particle density is stored in the w field of the positionDensity struct field
		particles[i].velocityPressure.w = max(fluidStiffness * (densityResult - fluidRestDensity), 0.0); // Particle pressure is stored in the w field of the velocityPressure struct field
	}
}
