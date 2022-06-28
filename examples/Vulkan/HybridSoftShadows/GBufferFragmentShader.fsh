#version 460

#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

#define M_PI 3.14159
#define NUMBER_OF_RANDOM_ROTATIONS (32 * 32)
#define NUMBER_OF_MESHES 4

highp float constantAttenuation = 1.0;
highp float linearAttenuation = 0.00002;
highp float quadraticAttenuation = 0.000002;
highp float depthModifier = 0.000001;
highp float lightDistanceModifier = 0.0001;

struct Material
{
	ivec4 textureIndices;
	vec4 baseColor;
	vec4 shininess;
};

layout(set = 0, binding = 0) uniform StaticPerScene
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp mat4 mInvViewProjectionMatrix;
	highp vec4 vEyePosition;
	highp vec4 vClipPlanes;
	uint uFrameIdx;
};

layout(set = 0, binding = 1) uniform LightUBO
{
	highp vec4 vLightColor;
	highp vec4 vLightPosition;
	highp vec4 vAmbientColor;
	highp vec4 vLightDirection;
	highp float penumbraAngle;
	highp float lightRadius;
	highp float innerConeAngle;
	highp float outerConeAngle;
	int numShadowRays;
};

layout(set = 0, binding = 2) buffer MateralDataBufferBuffer { Material materials[]; } ;
layout(set = 0, binding = 3) uniform sampler2D textureSamplers[5];
layout(set = 0, binding = 4) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 5, std140) uniform UboPoissonRotation { highp vec4 poissonRotation[NUMBER_OF_RANDOM_ROTATIONS]; };
layout(set = 0, binding = 6, std140) uniform UboMeshTransforms { highp mat4 worldMatrix[NUMBER_OF_MESHES]; };

layout(push_constant) uniform PushConsts {
	layout(offset = 4) uint materialID;
};

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in highp vec3 vNormal;
layout(location = 2) in highp vec3 vWorldPosition;

layout(location = 0) out highp vec4 oAlbedo_Shininess;
layout(location = 1) out highp vec4 oNormal_Visibility_HitDistance;

highp vec3 hash33(highp vec3 p)
{
	float n = sin(dot(p, vec3(7.0, 157.0, 113.0)));
	return fract(vec3(2097152.0, 262144.0, 32768.0) * n);
}

float queryVisiblity(vec3 origin, vec3 direction, out float tHit)
{
	float visibility = 1.0f;
	float tMin = 0.001;
	float tMax = 10000.0;

	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tMin, direction, tMax);

	// Start traversal: return false if traversal is complete
	while (rayQueryProceedEXT(rayQuery)) {}

	// Returns type of committed (true) intersection
	if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT)
	{
		// Got an intersection == Shadow
		visibility = 0.0f;
		tHit = rayQueryGetIntersectionTEXT(rayQuery, true);
	}
	else
		tHit = 0.0f;

	return visibility;
}

vec2 softShadowVisibility(vec3 worldPos, vec3 normal)
{
	vec3 toLight = vLightPosition.xyz - worldPos;
	vec3 lightDir = normalize(toLight);
	float lightDistance = length(toLight);
	float distLightRadius = lightRadius / lightDistance;

	vec3 lightTangent = normalize(cross(lightDir, vec3(0.0f, 1.0f, 0.0f)));
	vec3 lightBitangent = normalize(cross(lightTangent, lightDir));
	
	// Compute spot light intensity so that we can only trace rays within the spot light cone.
	highp float NdotL = dot(normal, lightDir);
	
	highp float currentCosAngle = dot(-lightDir, vLightDirection.xyz);

	// minConeAngle and maxConeAngle are expressed as cos(radians(angle))
	highp float spotIntensity = smoothstep(outerConeAngle, innerConeAngle, currentCosAngle);

	// attenuate based on the distance from the light source
	// distance to light source
	// attenuate based on the distance from the light source
	// distance to light source
	highp float attenuation = 1.0 / (constantAttenuation + linearAttenuation * lightDistance + quadraticAttenuation * lightDistance * lightDistance);

	highp float spotLightIntensity = spotIntensity * attenuation;

	/*
	nDotL > 0 will produce hard shadow edges where we want slightly smoother results
		instead we use smoothstep to create a smooth transition between 0 and 1 when nDotL lies
		between [0,...,0.1] and store in l.
	The random rotation value is then used in conjunction with l in another smoothstep.
		if l lies between [randomRotation.x,...,1.0] it will be interpolated between 0.0 and 1.0
		depending on how similar it is to randomRotation.x and 1.0
	*/
	highp vec4 randomRotation = poissonRotation[int(abs(vWorldPosition.xyz) * 999999.0) % NUMBER_OF_RANDOM_ROTATIONS];
	highp float l = clamp(smoothstep(0.0, 0.12, NdotL), 0.0, 1.0);
	highp float t = smoothstep(randomRotation.x * 0.4, 1.0f, l);

	spotLightIntensity *= (NdotL * t);

	float tHit = 0.0f;
	float visibility = 0.0f; 

	for (int i = 0; i < numShadowRays; i++)
	{
		vec2 rndSample = hash33(worldPos.xyz + vec3(i)).xy;

		// calculate disk point
		float pointRadius = distLightRadius * sqrt(rndSample.x);
		float pointAngle = rndSample.y * 2.0f * M_PI;
		vec2 diskPoint = vec2(pointRadius * cos(pointAngle), pointRadius * sin(pointAngle));

		vec3 rayDir = normalize(lightDir + diskPoint.x * lightTangent + diskPoint.y * lightBitangent);
		
		if (spotLightIntensity > 0.0f)
		{
			float hitDistance = 0.0f;
			visibility += queryVisiblity(worldPos + normal * 0.1f, rayDir, hitDistance) * spotLightIntensity;
			tHit += hitDistance;
		}
	}
	
	return vec2(visibility, tHit) / numShadowRays;
}

// Pack the 3-component direction vector into a 2-component octahedral packing
vec2 packNormal(vec3 normal)
{
	vec2 p = normal.xy * (1.0 / dot(abs(normal), vec3(1.0)));
    return normal.z > 0.0 ? p : (1.0 - abs(p.yx)) * (step(0.0, p) * 2.0 - vec2(1.0));
}

void main()
{
	const Material mat = materials[materialID];

	vec3 vAlbedo;

	// If a valid texture index does not exist, use the albedo color stored in the Material structure
	if (mat.textureIndices.x == -1)
		vAlbedo = mat.baseColor.rgb;
	else // If a valid texture index exists, use it to index into the image sampler array and sample the texture
		vAlbedo = texture(textureSamplers[mat.textureIndices.x], vTexCoord).rgb;

	vec3 vNormalisedNormal = normalize(vNormal);
	vec2 vVisibilityHitDistance = softShadowVisibility(vWorldPosition, vNormalisedNormal);

	oAlbedo_Shininess = vec4(vAlbedo, mat.shininess.x / 255.0f);
	oNormal_Visibility_HitDistance = vec4(packNormal(vNormalisedNormal), vVisibilityHitDistance);
}
