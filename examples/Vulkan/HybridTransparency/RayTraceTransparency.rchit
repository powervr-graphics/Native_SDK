#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

#define TRANSPARENCY_HIT_OFFSET 0
#define TRANSPARENCY_MISS_INDEX 0
#define RAY_RANGE_MIN 0.001
#define RAY_RANGE_MAX 10000.0
#define OIT_WEIGHT_MAX_DISTANCE 300.0

struct TransparencyRayPayload
{
	vec4 transparencyColor;         // Where to accumulate merged samples of transparent scene elements following the weighted blended order-independent transparency implementation
	float reveal;                   // Used to weight the contributions from all the transparent scene elements and the opaque scene element intersected in the final color
	bool isShadowRay;               // True means shadow ray casted from every texel in the GBuffer corresponding to an opaque scene element in the direction of the emitter. False means the ray is casted from the camera towards the direction of a transparent scene element in the GBuffer, to see through it
	int numTransparentIntersection; // Flag to count how many intersections with transparent scene elements happened
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
	mediump vec4 baseColor;              // Base color in case no texture is available to sample
	mediump int reflectanceTextureIndex; // Reflectance texture index
	mediump int isTransparent;           // Flag to know if the material is transparent or opaque
	mediump float transparency;          // 0.0 means totally transparent surface, 1.0 means totally opaque (but not light blocking) surface
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

struct Global
{
	highp mat4 viewMatrix;
	highp mat4 projectionMatrix;
	highp mat4 inverseViewProjectionMatrix;
	highp vec4 cameraPosition;
};

layout(location = 0) rayPayloadInEXT TransparencyRayPayload rayPayload;
layout(location = 2) rayPayloadEXT bool visiblityRayPayload;

hitAttributeEXT vec2 attribs;

layout(set = 2, binding = 0) uniform GlobalUBO { Global global; };
layout(set = 2, binding = 1) uniform LightDataUBO { LightData lightData; };
layout(set = 2, binding = 2, scalar) buffer MateralDataBufferBuffer { Material m[]; } materials;
layout(set = 2, binding = 3) buffer MatIndexColorBuffer { int i[]; } matIndex[];
layout(set = 2, binding = 4) uniform sampler2D textureSamplers[];
layout(set = 2, binding = 5) uniform accelerationStructureEXT topLevelAS;
layout(set = 2, binding = 6, scalar) buffer Vertices { Vertex v[]; } vertices[];
layout(set = 2, binding = 7) buffer Indices { uint i[]; } indices[];
layout(set = 2, binding = 8, scalar) buffer ScnDesc { sceneDesc i[]; } scnDesc;

highp vec3 evaluateDiffuseMaterial(highp vec3 worldPosition, highp vec3 normal, highp vec3 reflectance, highp vec3 lightColor)
{
	highp vec3 originToLightPosition = lightData.lightPosition.xyz - worldPosition;
	highp float distSquared          = dot(originToLightPosition, originToLightPosition);
	originToLightPosition            = normalize(originToLightPosition);
	highp float thetaLight           = clamp(0.0, 1.0, dot(normal, originToLightPosition));
	highp vec3 Li                    = ((lightData.ambientColorIntensity.w * lightColor * thetaLight * reflectance) / distSquared);

	return Li;
}

void traceRay(vec3 rayDirection, vec3 rayOrigin, float rayMaxRange)
{
	traceRayEXT(topLevelAS,  	 // acceleration structure
		gl_RayFlagsOpaqueEXT,    // rayFlags
		0xFF,                    // cullMask
		TRANSPARENCY_HIT_OFFSET, // sbtRecordOffset
		0,                       // sbtRecordStride
		TRANSPARENCY_MISS_INDEX, // missIndex
		rayOrigin,               // ray origin
		RAY_RANGE_MIN,           // ray min range
		rayDirection,            // ray direction
		rayMaxRange,             // ray max range
		0                        // payload (location = 0)
	);
}

void addTransparentContribution(float transparency, vec3 reflectance, vec3 hitWorldPos, vec3 hitWorldNormal)
{
	// Weighted blended order-independent transparency
	float transparencyDifference      = rayPayload.transparencyColor.w - transparency;
	float weight                      = transparencyDifference <= 0.0 ? rayPayload.transparencyColor.w : transparencyDifference;
	weight                            = clamp(weight, 0.0, 1.0);
	rayPayload.transparencyColor.xyz += vec3(reflectance * transparency * weight);
	rayPayload.transparencyColor.w    = weight;
	rayPayload.reveal                 = rayPayload.reveal * (1.0 - transparency);

	vec3 rayDirection                 = normalize(gl_WorldRayDirectionEXT);
	vec3 rayOrigin                    = hitWorldPos - hitWorldNormal * 0.2;
	float rayMaxRange                 = rayPayload.isShadowRay ? length(lightData.lightPosition.xyz - rayOrigin) : RAY_RANGE_MAX;

	// The ray hit another transparent object, trace a new ray
	traceRay(rayDirection, rayOrigin, rayMaxRange);
}

// If the ray hits anything we will fetch the material properties and shade this point as usual.
// Image Based Lighting is used here so we can get some reflections within reflections without having to fire more rays.
void main()
{
	// Triangle indices
	ivec3 ind = ivec3(indices[nonuniformEXT(gl_InstanceID)].i[3 * gl_PrimitiveID + 0],
					  indices[nonuniformEXT(gl_InstanceID)].i[3 * gl_PrimitiveID + 1],
					  indices[nonuniformEXT(gl_InstanceID)].i[3 * gl_PrimitiveID + 2]);

	// Triangle vertices
	Vertex v0 = vertices[nonuniformEXT(gl_InstanceID)].v[ind.x];
	Vertex v1 = vertices[nonuniformEXT(gl_InstanceID)].v[ind.y];
	Vertex v2 = vertices[nonuniformEXT(gl_InstanceID)].v[ind.z];

	vec3 barycentrics  = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	vec2 texCoord      = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;

	// Compute the coordinates of the hit position
	vec3 hitWorldPos   = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
	hitWorldPos        = vec3(scnDesc.i[gl_InstanceID].transform * vec4(hitWorldPos, 1.0));

	// Retrieve material properties (reflectance, transparency)
	int matID          = matIndex[nonuniformEXT(gl_InstanceID)].i[gl_PrimitiveID];
	Material mat       = materials.m[nonuniformEXT(matID)];
	vec3 reflectance   = mat.baseColor.xyz;
	bool isTransparent = (mat.isTransparent == 1);
	
	if (mat.reflectanceTextureIndex != -1)
	{
		reflectance = texture(textureSamplers[nonuniformEXT(mat.reflectanceTextureIndex)], texCoord).xyz;
	}

	// Computing the normal at hit position
	vec3 hitWorldNormal        = normalize(v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z);
	mat4 transform             = scnDesc.i[gl_InstanceID].transform;
	mat3 transformNormalMatrix = transpose(inverse(mat3(transform)));
	hitWorldNormal             = normalize(transformNormalMatrix * hitWorldNormal);

	if(isTransparent)
	{
		// Came across a transparent object during a shadow ray or
		// during a ray cast from the camera, the ray came across another transparent surface: Add contribution
		rayPayload.numTransparentIntersection++;

		addTransparentContribution(mat.transparency, reflectance, hitWorldPos, hitWorldNormal);
	}
	else
	{
		if(rayPayload.isShadowRay)
		{
			// Came across an opaque object during shadow ray
			rayPayload.transparencyColor          = vec4(0.0);
			rayPayload.numTransparentIntersection = -1;
		}
		else
		{
			// During a ray cast from the camera, the ray came across an opaque surface: Resolve the whole opaque + transparent contribution
			// For the current scene, assuming that opaque surface intersected is lit, computing its lighting with a Phong shading is enough. A complete approach would trace a shadow ray first
			reflectance                      = evaluateDiffuseMaterial(hitWorldPos, hitWorldNormal, reflectance, lightData.lightColor.xyz);
			vec3 transparencyFinal           = rayPayload.transparencyColor.xyz;
			vec3 finalColor                  = reflectance.xyz * rayPayload.reveal + transparencyFinal * (1.0 - rayPayload.reveal);
			rayPayload.transparencyColor.xyz = finalColor; // Use this field to hold the final color
		}
	}
}
