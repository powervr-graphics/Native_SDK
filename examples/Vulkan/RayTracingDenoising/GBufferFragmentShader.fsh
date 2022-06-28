#version 460

#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

#define M_PI 3.14159265359
#define SOFT_SHADOWS

struct RNG
{
    uvec2 s;
};

// xoroshiro64* random number generator.
// http://prng.di.unimi.it/xoroshiro64star.c
uint rngRotl(uint x, uint k)
{
    return (x << k) | (x >> (32 - k));
}

// Xoroshiro64* RNG
uint rngNext(inout RNG rng)
{
    uint result = rng.s.x * 0x9e3779bb;

    rng.s.y ^= rng.s.x;
    rng.s.x = rngRotl(rng.s.x, 26) ^ rng.s.y ^ (rng.s.y << 9);
    rng.s.y = rngRotl(rng.s.y, 13);

    return result;
}

// Thomas Wang 32-bit hash.
// http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
uint rngHash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

RNG rngInit(uvec2 id, uint frameIndex)
{
    uint s0 = (id.x << 16) | id.y;
    uint s1 = frameIndex;

    RNG rng;
    rng.s.x = rngHash(s0);
    rng.s.y = rngHash(s1);
    rngNext(rng);
    return rng;
}

float nextFloat(inout RNG rng)
{
    uint u = 0x3f800000 | (rngNext(rng) >> 9);
    return uintBitsToFloat(u) - 1.0;
}

vec2 nextVec2(inout RNG rng)
{
    return vec2(nextFloat(rng), nextFloat(rng));
}

struct Material
{
	vec4 baseColor;
	vec4 metallicRoughnessReflectivity;
	vec4 f0f90;
};

struct PerLightData
{
	highp vec4 lightPosition;
	highp vec4 lightColor;
	highp float lightIntensity;
	highp float lightRadius;
};

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp mat4 mViewProjInverseMatrix;
	highp vec4 vAmbientLightColor;
	highp vec4 vCameraPosition;
};

layout(set = 0, binding = 1) uniform LightDataUBO
{
	PerLightData lightData;
};

layout(set = 0, binding = 2) buffer MateralDataBufferBuffer { Material materials[]; } ;
layout(set = 0, binding = 4) uniform accelerationStructureEXT topLevelAS;

layout(push_constant) uniform PushConsts {
	layout(offset = 4) uint materialID;
	layout(offset = 8) uint frameIdx;
};

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in mediump vec3 vNormal;
layout(location = 2) in highp vec3 vWorldPosition;
layout(location = 3) in highp vec4 vClipPosition;
layout(location = 4) in highp vec4 vPrevClipPosition;

layout(location = 0) out highp vec4 oAlbedo_Metallic;
layout(location = 1) out highp vec4 oNormal_Reflectivity;
layout(location = 2) out highp vec4 oMotionVector_F90;
layout(location = 3) out highp vec4 oF0_Rougness;
layout(location = 4) out highp float oShadowMask;

float queryVisiblity(vec3 origin, vec3 direction)
{
	float tMin = 0.001;
	float tMax = 10000.0;

	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tMin, direction, tMax);

	// Start traversal: return false if traversal is complete
	while (rayQueryProceedEXT(rayQuery)) {}

	// Returns type of committed (true) intersection
	if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT)
		return 0.0f;
	else 
		return 1.0f;
}

float softShadows()
{
	RNG rng = rngInit(uvec2(gl_FragCoord.xy), frameIdx);

	// Offset origin in the direction of the normal to prevent self-intersection
	vec3 rayOrigin = vWorldPosition + normalize(vNormal) * 0.1f;

	// calculate the direction to the current light source
	highp vec3 directionToLight = normalize(lightData.lightPosition.xyz - vWorldPosition);
	highp vec3 normalisedDirectionToLight = normalize(directionToLight);
    highp float lightDistance = length(directionToLight);

#if defined(SOFT_SHADOWS)
	highp vec2 random = nextVec2(rng);

    highp vec3 lightTangent   = normalize(cross(normalisedDirectionToLight, vec3(0.0f, 1.0f, 0.0f)));
    highp vec3 lightBitangent = normalize(cross(lightTangent, normalisedDirectionToLight));  
    
    // calculate disk point
    float currentLightRadius = lightData.lightRadius / lightDistance;  
    float pointRadius = currentLightRadius * sqrt(random.x);
    float pointAngle  = random.y * 2.0f * M_PI;
    vec2  diskPoint   = vec2(pointRadius * cos(pointAngle), pointRadius * sin(pointAngle));    
    normalisedDirectionToLight = normalize(normalisedDirectionToLight + diskPoint.x * lightTangent + diskPoint.y * lightBitangent);
#endif

	return queryVisiblity(rayOrigin, normalisedDirectionToLight);
}

vec2 computeMotionVector(vec4 prevPos, vec4 currentPos)
{
    // Perspective division, covert clip space positions to NDC.
    vec2 current = (currentPos.xy / currentPos.w);
    vec2 prev    = (prevPos.xy / prevPos.w);

    // Remap to [0, 1] range
    current = current * 0.5 + 0.5;
    prev    = prev * 0.5 + 0.5;

    // Calculate velocity (current -> prev)
    return (prev - current);
}

void main()
{
	const Material mat = materials[materialID];

	vec3 albedo = mat.baseColor.rgb;
	float roughness = mat.metallicRoughnessReflectivity.y;
	float metallic = mat.metallicRoughnessReflectivity.x;
	float reflectivity = mat.metallicRoughnessReflectivity.z;
	vec3 F0 = mat.f0f90.xyz;
	float F90 = mat.f0f90.w;

	oAlbedo_Metallic = vec4(albedo, metallic);
	oNormal_Reflectivity = vec4(normalize(vNormal), reflectivity);
	oMotionVector_F90 = vec4(computeMotionVector(vPrevClipPosition, vClipPosition), F90, gl_FragCoord.z / gl_FragCoord.w);
	oF0_Rougness = vec4(F0, roughness);
	oShadowMask = softShadows();
}
