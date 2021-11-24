#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

#define REFLECTIONS_HIT_OFFSET 0
#define REFLECTIONS_MISS_INDEX 0
#define SHADOW_HIT_OFFSET 1
#define SHADOW_MISS_INDEX 1
#define RAY_RANGE_MIN 0.001
#define RAY_RANGE_MAX 10000.0
#define MAX_RAY_RECURSION 4

struct ReflectionRayPayload
{
	vec3 Li; // Incident radiance
	uint depth;
	bool inside;
	float indexOfRefraction;
};

struct Vertex
{
	vec3 pos;
	vec3 nrm;
	vec2 texCoord;
	vec3 tangent;
};

struct Material
{
	vec4 baseColor;
	int reflectanceTextureIndex;
	float indexOfRefraction;
	float attenuationCoefficient;
};

struct sceneDesc
{
	int objId;
	mat4 transform;
	mat4 transformIT;
};

struct LightData
{
	highp vec4 lightColor;
	highp vec4 lightPosition;
	highp vec4 ambientColorIntensity;
};

layout(location = 0) rayPayloadInEXT ReflectionRayPayload reflectionRayPayload;
layout(location = 2) rayPayloadEXT bool visiblityRayPayload;

hitAttributeEXT vec2 attribs;

layout(set = 2, binding = 1) uniform LightDataUBO { LightData lightData; };
layout(set = 2, binding = 2, scalar) buffer MateralDataBufferBuffer { Material m[]; } materials;
layout(set = 2, binding = 3) buffer MatIndexColorBuffer { int i[]; } matIndex[];
layout(set = 2, binding = 4) uniform sampler2D textureSamplers[];
layout(set = 2, binding = 5) uniform accelerationStructureEXT topLevelAS;
layout(set = 2, binding = 6, scalar) buffer Vertices { Vertex v[]; } vertices[];
layout(set = 2, binding = 7) buffer Indices { uint i[]; } indices[];
layout(set = 2, binding = 8, scalar) buffer ScnDesc { sceneDesc i[]; } scnDesc;

struct RayHit
{
	vec3 hitWorldNormal;
	vec3 hitWorldPos;
	vec3 wi;
	vec3 reflectance;
	bool isDiffuseMaterial;
	float indexOfRefraction;
	float attenuationCoefficient;
	int currentDepth;
};

RayHit rayHit;

void getRayHitInfo()
{
	// Triangle indices
	ivec3 ind = ivec3(indices[nonuniformEXT(gl_InstanceID)].i[3 * gl_PrimitiveID + 0],
		              indices[nonuniformEXT(gl_InstanceID)].i[3 * gl_PrimitiveID + 1],
		              indices[nonuniformEXT(gl_InstanceID)].i[3 * gl_PrimitiveID + 2]);

	// Triangle vertices
	Vertex v0 = vertices[nonuniformEXT(gl_InstanceID)].v[ind.x];
	Vertex v1 = vertices[nonuniformEXT(gl_InstanceID)].v[ind.y];
	Vertex v2 = vertices[nonuniformEXT(gl_InstanceID)].v[ind.z];

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;

	// Computing the normal at hit position
	rayHit.hitWorldNormal      = normalize(v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z);
	mat4 transform             = scnDesc.i[gl_InstanceID].transform;
	mat3 transformNormalMatrix = transpose(inverse(mat3(transform)));
	rayHit.hitWorldNormal      = normalize(transformNormalMatrix * rayHit.hitWorldNormal);

	// Computing the coordinates of the hit position
	rayHit.hitWorldPos = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
	rayHit.hitWorldPos = vec3(scnDesc.i[gl_InstanceID].transform * vec4(rayHit.hitWorldPos, 1.0));

	rayHit.wi = normalize(-1.0 * gl_WorldRayDirectionEXT);

	rayHit.currentDepth = int(reflectionRayPayload.depth);

	// Retrieve material properties (reflectance, index of refraction, attenuation coefficient)
	int matID           = matIndex[nonuniformEXT(gl_InstanceID)].i[gl_PrimitiveID];
	Material mat        = materials.m[nonuniformEXT(matID)];
	rayHit.reflectance  = mat.baseColor.xyz;

	if (mat.reflectanceTextureIndex != -1)
	{
		rayHit.reflectance = texture(textureSamplers[nonuniformEXT(mat.reflectanceTextureIndex)], texCoord).xyz;
	}

	rayHit.indexOfRefraction = mat.indexOfRefraction;
	rayHit.isDiffuseMaterial = false;

	if ((matID == 2) || (matID == 3) || (matID == 4))
	{
		rayHit.isDiffuseMaterial = true;
	}

	rayHit.attenuationCoefficient = mat.attenuationCoefficient;
}

float fresnel(vec3 wi, vec3 normal, float ior)
{
	float kr             = 0.0;
	float etaIncident    = 1.0;
	float etaTransmitted = ior;
	float cosIncident    = clamp(-1.0, 1.0, dot(wi, normal));
	
	if (cosIncident > 0)
	{
		float temp     = etaIncident;
		etaIncident    = etaTransmitted;
		etaTransmitted = temp;
	}

	float sinTransmitted = etaIncident / etaTransmitted * sqrt(max(0.0, 1.0 - cosIncident * cosIncident));

	if (sinTransmitted >= 1.0)
	{
		kr = 1.0; // Total internal reflection
	}
	else
	{
		float cosTransmitted = sqrt(max(0.0, 1.0 - sinTransmitted * sinTransmitted));
		cosIncident          = abs(cosIncident);
		float Rs             = ((etaTransmitted * cosIncident) - (etaIncident    * cosTransmitted)) / ((etaTransmitted * cosIncident) + (etaIncident    * cosTransmitted));
		float Rp             = ((etaIncident    * cosIncident) - (etaTransmitted * cosTransmitted)) / ((etaIncident    * cosIncident) + (etaTransmitted * cosTransmitted));
		kr                   = (Rs * Rs + Rp * Rp) / 2.0;
	}

	return kr; // Due to sonservation of energy, kr + kt = 1 -> kt = 1 - kr
}

void traceRay(vec3 rayOrigin, vec3 rayDirection, bool inside, float indexOfRefraction)
{
	reflectionRayPayload.Li                = vec3(0.0);
	reflectionRayPayload.depth             = rayHit.currentDepth + 1;
	reflectionRayPayload.inside            = inside;
	reflectionRayPayload.indexOfRefraction = indexOfRefraction;

	traceRayEXT(topLevelAS,     // acceleration structure
		gl_RayFlagsOpaqueEXT,   // rayFlags
		0xFF,                   // cullMask
		REFLECTIONS_HIT_OFFSET, // sbtRecordOffset
		0,                      // sbtRecordStride
		REFLECTIONS_MISS_INDEX, // missIndex
		rayOrigin,              // ray origin
		RAY_RANGE_MIN,          // ray min range
		rayDirection,           // ray direction
		RAY_RANGE_MAX,          // ray max range
		0                       // payload (location = 0)
	);
}

vec3 refractFunction(vec3 wi, vec3 normal, float eta)
{
	// Snell's law is used to compute the cosine
	float cosThetaIncident           = dot(normal, wi);
	float sinSquaredThetaIncident    = max(0.0, 1.0 - cosThetaIncident * cosThetaIncident);
	float sinSquaredThetaTransmitted = sinSquaredThetaIncident / sqrt(eta);

	// Handle total internal reflection case
	if (sinSquaredThetaTransmitted >= 1.0)
	{
		return vec3(0.0);
	}

	float cosThetaTransmitted = sqrt(1.0 - sinSquaredThetaTransmitted);

	vec3 wt = -wi / eta + (cosThetaIncident / eta - cosThetaTransmitted) * vec3(normal);

	return wt;
}

bool traceShadowRay(vec3 rayStart, vec3 rayEnd)
{
	visiblityRayPayload = false;
	vec3 rayDirection   = rayEnd - rayStart;

	traceRayEXT(topLevelAS,                                       // acceleration structure
		gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT, // rayFlags
		0xFF,                                                     // cullMask
		SHADOW_HIT_OFFSET,                                        // sbtRecordOffset
		0,                                                        // sbtRecordStride
		SHADOW_MISS_INDEX,                                        // missIndex
		rayStart,                                                 // ray origin
		0.001,                                                    // ray min range
		normalize(rayDirection),                                  // ray direction
		length(rayDirection),                                     // ray max range
		2                                                         // payload (location = 2)
	);

	return visiblityRayPayload;
}

vec3 evaluateDiffuseMaterial()
{
	bool visibility            = traceShadowRay(rayHit.hitWorldPos, lightData.lightPosition.xyz);
	vec3 originToLightPosition = lightData.lightPosition.xyz - rayHit.hitWorldPos;
	float distSquared          = dot(originToLightPosition, originToLightPosition);
	originToLightPosition      = normalize(originToLightPosition);
	float thetaLight           = clamp(0.0, 1.0, dot(rayHit.hitWorldNormal, originToLightPosition));
	vec3 Li                    = (float(visibility) * (lightData.ambientColorIntensity.w * lightData.lightColor.xyz * thetaLight * rayHit.reflectance) / distSquared) + lightData.ambientColorIntensity.xyz * rayHit.reflectance;

	return Li;
}

// If the ray hits anything we will fetch the material properties and shade this point as usual.
// Image Based Lighting is used here so we can get some reflections within reflections without having to fire more rays.
void main()
{
	getRayHitInfo();

	if(rayHit.isDiffuseMaterial)
	{
		reflectionRayPayload.Li = evaluateDiffuseMaterial();
		return;
	}

	if (reflectionRayPayload.depth > MAX_RAY_RECURSION)
	{
		reflectionRayPayload.Li = vec3(0.0, 0.0, 0.0);
		return;
	}

	vec3 Li = vec3(0.0);

	if (reflectionRayPayload.inside)
	{
		float kr                = fresnel(rayHit.wi, -1.0 * rayHit.hitWorldNormal, rayHit.indexOfRefraction);
		vec3 refractedDirection = normalize(refractFunction(rayHit.wi, -1.0 * rayHit.hitWorldNormal, 1.0 / rayHit.indexOfRefraction));
		vec3 reflectedDirection = normalize(reflect(-1.0 * rayHit.wi, -1.0 * rayHit.hitWorldNormal)); // NOTE: In GLSL's implementation of reflect, incident direction points towards the sample and not outwards

		float rayDistance = distance(gl_WorldRayOriginEXT, rayHit.hitWorldPos);
		vec3 attenuation  = vec3(1.0 / exp(rayDistance * rayHit.attenuationCoefficient * rayHit.reflectance.x),
							     1.0 / exp(rayDistance * rayHit.attenuationCoefficient * rayHit.reflectance.y),
							     1.0 / exp(rayDistance * rayHit.attenuationCoefficient * rayHit.reflectance.z));

		if ((dot(refractedDirection, vec3(1.0)) == 0.0) || isnan(refractedDirection.x) || isnan(refractedDirection.y) || isnan(refractedDirection.z))
		{
			// Total Internal Reflection, avoid refracted ray and cast just one reflected ray inside the dielectic
			// IOR does not change, the ray stays inside the dielectric.
			traceRay(rayHit.hitWorldPos - rayHit.hitWorldNormal * 0.1, reflectedDirection, true, reflectionRayPayload.indexOfRefraction);
			Li += reflectionRayPayload.Li * attenuation;
		}
		else
		{
			// Trace a ray with the reflected direction
			// IOR does not change, the ray stays inside the dielectric
			traceRay(rayHit.hitWorldPos - rayHit.hitWorldNormal * 0.1, reflectedDirection, true, reflectionRayPayload.indexOfRefraction);
			Li += rayHit.reflectance * kr * reflectionRayPayload.Li * abs(clamp(0.0, 1.0, dot(rayHit.hitWorldNormal, rayHit.wi))) * attenuation;

			// Trace a ray with the refracted direction
			// IOR is air's now, the ray gets out of the dielectric
			traceRay(rayHit.hitWorldPos + rayHit.hitWorldNormal * 0.1, refractedDirection, false, 1.0);
			Li += rayHit.reflectance * (1.0 - kr) * reflectionRayPayload.Li * abs(clamp(0.0, 1.0, dot(rayHit.hitWorldNormal, refractedDirection))) * attenuation;
		}
	}
	else
	{
		float kr                = fresnel(rayHit.wi, rayHit.hitWorldNormal, 1.0 / rayHit.indexOfRefraction);
		vec3 refractedDirection = normalize(refractFunction(rayHit.wi, rayHit.hitWorldNormal, rayHit.indexOfRefraction));
		vec3 reflectedDirection = normalize(reflect(-1.0 * rayHit.wi, rayHit.hitWorldNormal)); // NOTE: In GLSL's implementation of reflect, incident direction points towards the sample and not outwards

		// Trace a reflected and a refracted ray, use Fresnel term to compute each's contribution to Li
		traceRay(rayHit.hitWorldPos + rayHit.hitWorldNormal * 0.1, reflectedDirection, false, 1.0);
		Li += rayHit.reflectance * kr * reflectionRayPayload.Li * abs(clamp(0.0, 1.0, dot(rayHit.hitWorldNormal, rayHit.wi)));

		// Use IOR of material used in new intersected mesh
		traceRay(rayHit.hitWorldPos - rayHit.hitWorldNormal * 0.1, refractedDirection, true, rayHit.indexOfRefraction);
		Li += rayHit.reflectance * (1.0 - kr) * reflectionRayPayload.Li * abs(clamp(0.0, 1.0, dot(rayHit.hitWorldNormal, refractedDirection)));
	}

	reflectionRayPayload.Li += Li;
}
