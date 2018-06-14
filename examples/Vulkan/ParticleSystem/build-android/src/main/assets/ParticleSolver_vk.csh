#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//Please #define WORKGROUP_SIZE from the code side
#define WORKGROUP_SIZE 32
//Minimum lifetime of particle
#define MIN_TTL .5
//Variance of particle lifetime
#define MAX_ADDITIONAL_TTL 1.

//Binding points
#define SPHERES_UBO_BINDING 0
#define CONFIG_UNIFORM_BINDING 1
#define PARTICLES_SSBO_BINDING_IN 2
#define PARTICLES_SSBO_BINDING_OUT 3
//Arbitrary factors to make physics work in a visually pleasing way
#define TIME_FACTOR .1
#define VELOCITY_FACTOR 4.

#define numSpheres 8u

// Struct declarations to have a nice representation of the data layout.
// Please be aware of the alignment restrictions (e.g. sizeof(vec3) == sizeof(vec4)) that are defined in the GLSL specification.
// Because of that we need to be very careful, and upload the data in a controllable way from the CPU (e.g. using std140 packing,
// packing manually float values in quads and reading as vectors et.c.).

//Padding could be calculated instead of being explicitly added with std149, but it helps catch errors easier.
struct Particle
{	
	highp vec3 vPosition;			
	highp float _padding_particle_0;
	highp vec3 vVelocity;
	highp float fLife;
};

struct Emitter
{
    highp mat4  mTransformation;
	highp float    fHeight;
	highp float    fRadius;
};

layout(std140, binding=SPHERES_UBO_BINDING) uniform UboSpheres
{
	highp vec4 aSpheres[8];
};

layout(std140, binding=CONFIG_UNIFORM_BINDING) uniform UboData
{
    Emitter emitter;
	highp vec3 vG;
	highp float fDt;
	highp float fTotalTime;
};

layout(std140, binding = PARTICLES_SSBO_BINDING_IN)buffer SsboParticlesInOut
{
    Particle aParticlesInOut[];
};


int last_random;
void seed_random(int seed)
{
	last_random = seed;
}

float randf()
{
	//This is not random at all, and was chosen for looking nice and nothing else.
	//The last *100 is cheating - we want the pattern to discretize a bit to create the "magnetic field" lines
	//So, we effectively discard a few decimal precision points by "fract" that way...
	//If you want more uniform distribution, remove *100 or (or even replace it by .1)

	last_random = abs(1103515245 * abs(last_random) + 12345);

	return float(last_random) / float(2147483647);
}

// Emits the aParticles[gid] at a random position within the emitter params
void EmitParticle(mat4  inTransformation,
                  float	inHeight,
				  float	inRadius,
				  out vec3 outPosition,
				  out vec3 outVelocity,
				  out float outLife)
{
	// Generate random parameters for spawn position calculations
	highp float u = randf() * inHeight;
	highp float r = randf() * inRadius;

	//We need pos to be a vec4 so we can transform it using the emitter's transformation
	vec4 emittedDirection = vec4(r, u, r, 1);


	// Transform according to emitter orientation
	emittedDirection = inTransformation * emittedDirection;


	// Give it random speed
	outVelocity = emittedDirection.xyz * vec3(7., 15., 7.) + vec3(0., 15., 0.);

	//Start from the origin (this could have been randomised as well.
	outPosition = vec3(0, 0, 0);
}

// Main kernel entry point. Simulates the aParticles[gid] movement and collides it against collider geometry (fixed ground plane at y=0).
layout (local_size_x = 32) in;
void main()
{
	uint gid = gl_GlobalInvocationID.x;
	seed_random(int(fTotalTime * 13579846.) + int(gid + 10000u) * 145);
	
	/*We remove the following bounds check because our code has made sure that the particles are a multiple of our workgroup*/
	//const uint numParticles = 131072; //or numParticles passed as a uniform, or as a compile-time constant
	//if (gid >= numParticles) return; 

	//Load a particle's values in a register.

    highp float life = aParticlesInOut[gid].fLife;

	//Reduce it's lifetime ( 0 < life < in the order of 1)
	life -= fDt * TIME_FACTOR;
		
	//Respawn if particle is dead 
	if (life < .0)		
	{
	    EmitParticle(emitter.mTransformation, emitter.fHeight, emitter.fRadius,
		            aParticlesInOut[gid].vPosition,aParticlesInOut[gid].vVelocity,aParticlesInOut[gid].fLife);
		life = randf() * MAX_ADDITIONAL_TTL + MIN_TTL;
	}
	else
	{
	    highp vec3 v = aParticlesInOut[gid].vVelocity;

		//Simulate particle movement: 
		//Semi-implicit euler is the best of the "completely basic", forward integration methods - it has better
		//behaviour than explicit Euler basically for free. Error is of the same order as Explicit Euler, but it 
		//is much more stable as it normally retains energy.
		//Explicit Euler is		: x1 = x0 + v0.dt, then: v1 = v0 + a0*dt
		//Semi - Implicit euler is 
		//This integration is	: v1 = v0 + a0*dt, then: x1 = x0 + v1*dt (we will use this)
		//OR					: x1 = x0 + v0*dt, then: v1 = v0 + a1*dt

		//a0 = g. Update velocity due to a. 
		v = (vG * fDt) + v;

		//Do the last part of the semi-implicit Euler : 
		aParticlesInOut[gid].vPosition = v * fDt + aParticlesInOut[gid].vPosition;

		//Then, collide against the spheres (very fake, but looks convincing)
		for (uint i=0u; i < numSpheres; i++)
		{
			// Fetch sphere attributes (position=sphereDef.xyz, radius=sphereDef.w)
			vec4 sphereDef = aSpheres[i];
						
			//Check whether particle penetrated the sphere. Reuse calculations as much as possible.
			//Normally, we would just go ahead and use length(), but at this point we will probably
			//be using a lot of the intermediates as well.
			vec3 sphere_to_particle = aParticlesInOut[gid].vPosition - sphereDef.xyz;
			float distanceSq = dot(sphere_to_particle, sphere_to_particle);

			//If inside the sphere
			if (distanceSq < sphereDef.w * sphereDef.w)
			{	
				float rDist = inversesqrt(distanceSq);
				//Project it back to sphere surface and do a very simple and fast velocity inversion
				vec3 dir_to_circle = sphere_to_particle * rDist;
				aParticlesInOut[gid].vPosition = dir_to_circle * sphereDef.w + sphereDef.xyz; //Move it ON the sphere
				v = reflect(v, dir_to_circle) * .7; //Velocity pointing outwards of the sphere
				break;
			}			
		}
		
		// Collide against ground plane
		if (aParticlesInOut[gid].vPosition.y < 0.0f)
		{
		//	//x & z coeffs represent a sort of friction, y represents reflection -- As always, we can
		//	//use a model that is as physically correct or incorrect as is appropriate for our use
			const vec3 reflectCoeffs = vec3(0.4f, -0.3f, 0.4f);
			aParticlesInOut[gid].vPosition.y = -aParticlesInOut[gid].vPosition.y;
			v *= reflectCoeffs;
		}

		//Pack velocity back
		aParticlesInOut[gid].vVelocity = v;
	}
	//Pack life back
	aParticlesInOut[gid].fLife = life;
	//We do not synchronize as there is absolutely no interaction between particles in that case
}
