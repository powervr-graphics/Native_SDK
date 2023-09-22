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

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main()
{
	int i = int(gl_GlobalInvocationID.x);

	vec3 velocity         = particles[i].velocityPressure.xyz;
	vec3 positionDensity  = particles[i].positionDensity.xyz + velocity * deltaTime;

	if (positionDensity.x < lowerBoundX)
	{
		positionDensity.x = lowerBoundX;
		velocity.x = -1.0 * velocity.x * speedDecay;
	}
	
	if (positionDensity.x > upperBoundX)
	{
		positionDensity.x = upperBoundX;
		velocity.x = -1.0 * velocity.x * speedDecay;
	}

	if (positionDensity.y < lowerBoundY)
	{
		positionDensity.y = lowerBoundY;
		velocity.y = -1.0 * velocity.y * speedDecay;
	}

	if (positionDensity.z < lowerBoundZ)
	{
		positionDensity.z = lowerBoundZ;
		velocity.z = -1.0 * velocity.z * speedDecay;
	}

	if (positionDensity.z > upperBoundZ)
	{
		positionDensity.z = upperBoundZ;
		velocity.z = -1.0 * velocity.z * speedDecay;
	}

	particles[i].positionDensity = vec4(positionDensity.xyz, 0.0);
	particles[i].velocityPressure.xyz = velocity;
}
