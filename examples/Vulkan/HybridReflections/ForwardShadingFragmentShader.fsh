#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

#define NUM_REFLECTION_RAYS 2
#define MAX_REFLECTION_DEPTH (NUM_REFLECTION_RAYS - 1)
#define MAX_REFLECTION_SHADOW_RAY_DEPTH 0
#define PI 3.1415926535897932384626433832795
#define ONE_OVER_PI (1.0 / PI)
#define NUMBER_OF_MESHES 4

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in highp vec3 vNormal;
layout(location = 2) in highp vec3 vWorldPosition;

layout(location = 0) out mediump vec4 oColor;

struct Vertex
{
  vec3 pos;
  vec3 nrm;
  vec2 texCoord;
  vec3 tangent;
};

struct Material
{
	ivec4 textureIndices;
	vec4 baseColor;
	vec4 metallicRoughness;
};

struct LightData
{
	highp vec4 vLightColor;
	highp vec4 vLightPosition;
	highp vec4 vAmbientColor;
};

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp mat4 mInvVPMatrix;
	highp vec4 vCameraPosition;
};

layout(set = 0, binding = 1) uniform LightDataUBO
{
	LightData lightData;
};

layout(set = 0, binding = 2) buffer MateralDataBufferBuffer { Material materials[]; };
layout(set = 0, binding = 3) buffer MatIndexColorBuffer { int i[]; } matIndex[NUMBER_OF_MESHES];
layout(set = 0, binding = 4) uniform sampler2D textureSamplers[4];
layout(set = 0, binding = 5) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 6, scalar) buffer Vertices { Vertex v[]; } vertices[NUMBER_OF_MESHES];
layout(set = 0, binding = 7) buffer Indices { uint i[]; } indices[NUMBER_OF_MESHES];
layout(set = 0, binding = 8, std140) uniform UboMeshTransforms { highp mat4 worldMatrix[NUMBER_OF_MESHES]; };

layout(set = 1, binding = 0) uniform samplerCube skyboxImage;
layout(set = 1, binding = 1) uniform samplerCube prefilteredImage;
layout(set = 1, binding = 2) uniform samplerCube irradianceImage;
layout(set = 1, binding = 3) uniform sampler2D brdfLUT;

layout(push_constant) uniform PushConsts {
	layout(offset = 4) uint materialID;
};

// Normal Distribution function
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
mediump float d_GGX(mediump float dotNH, mediump float roughness)
{
	mediump float alpha = roughness * roughness;
	mediump float alpha2 = alpha * alpha;
	mediump float x = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return alpha2 / max(PI * x * x, 0.0001);
}

mediump float g1(mediump float dotAB, mediump float k)
{
	return dotAB / max(dotAB * (1.0 - k) + k, 0.0001);
}

// Geometric Shadowing function
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
mediump float g_schlicksmithGGX(mediump float dotNL, mediump float dotNV, mediump float roughness)
{
	mediump float k = (roughness + 1.0);
	k = (k * k) / 8.;
	return g1(dotNL, k) * g1(dotNV, k);
}

// Fresnel function (Shlicks)
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
mediump vec3 f_schlick(mediump float cosTheta, mediump vec3 F0)
{
	return F0 + (vec3(1.0) - F0) * pow(2.0, (-5.55473 * cosTheta - 6.98316) * cosTheta);
}

mediump vec3 f_schlickR(mediump float cosTheta, mediump vec3 F0, mediump float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

bool traceRay(vec3 origin, vec3 direction, out vec3 hitPos, out vec3 normal, out vec2 texCoord, out int matID)
{
	float tMin     = 0.001;
    float tMax     = 10000.0;

	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsOpaqueEXT, 0xFF, origin, tMin, direction, tMax);
	
	// Start traversal: return false if traversal is complete
	while(rayQueryProceedEXT(rayQuery))
	{
	}
	
	// Returns type of committed (true) intersection
	if(rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT)
	{
		const int instanceId = rayQueryGetIntersectionInstanceIdEXT(rayQuery, true);
		const int primitiveId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true);
		const vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);

		// Indices of the triangle
		ivec3 ind = ivec3(indices[nonuniformEXT(instanceId)].i[3 * primitiveId + 0], 
						  indices[nonuniformEXT(instanceId)].i[3 * primitiveId + 1], 
						  indices[nonuniformEXT(instanceId)].i[3 * primitiveId + 2]);

		// Vertex of the triangle
		Vertex v0 = vertices[nonuniformEXT(instanceId)].v[ind.x];
		Vertex v1 = vertices[nonuniformEXT(instanceId)].v[ind.y];
		Vertex v2 = vertices[nonuniformEXT(instanceId)].v[ind.z];

		// Material of the object
		matID = matIndex[nonuniformEXT(instanceId)].i[primitiveId];
	
		// Get the world matrix for this instance 
		mat4 worldMat = worldMatrix[nonuniformEXT(instanceId)];

		const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	
		// Computing the normal at hit position
		normal = v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z;
		
		// Transform normal to world space 
		normal = mat3(worldMat) * normal;

		// Computing the coordinates of the hit position
		hitPos = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
		
		// Transform hitPos to world space 
		hitPos = (worldMat * vec4(hitPos, 1.0f)).xyz;

		// Computing the coordinates of the hit position
		texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;

		return true;
	}
	
	return false;
}

bool traceShadowRay(vec3 origin, vec3 direction)
{
	float tMin     = 0.001;
    float tMax     = 10000.0;

	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tMin, direction, tMax);
	
	// Start traversal: return false if traversal is complete
	while(rayQueryProceedEXT(rayQuery))
	{
	}
	
	// Returns type of committed (true) intersection
	if(rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT)
		return true;

	return false;
}

mediump vec3 directLighting(mediump vec3 P, highp vec3 V, mediump vec3 N, highp vec3 L, mediump vec3 F0, mediump vec3 albedo, mediump float metallic, mediump float roughness, int rayDepth)
{
	// half vector
	mediump vec3 H = normalize(V + L);

	mediump float dotNL = clamp(dot(N, L), 0.0, 1.0);

	mediump vec3 color = vec3(0.0);

	// light contributed only if the angle between the normal and light direction is less than equal to 90 degree.
	if(dotNL > 0.0)
	{
		mediump float dotLH = clamp(dot(L, H), 0.0, 1.0);
		mediump float dotNH = clamp(dot(N, H), 0.0, 1.0);
		mediump float dotNV = clamp(dot(N, V), 0.0, 1.0);
		
		///-------- Specular BRDF: COOK-TORRANCE ---------
		// D = Microfacet Normal distribution.
		mediump float D = d_GGX(dotNH, roughness);

		// G = Geometric Occlusion
		mediump float G = g_schlicksmithGGX(dotNL, dotNV, roughness);

		// F = Surface Reflection
		mediump vec3 F = f_schlick(dotLH, F0);

		mediump vec3 spec = F * ((D * G) / (4.0 * dotNL * dotNV + 0.001/* avoid divide by 0 */));

		///-------- DIFFUSE BRDF ----------
		// kD factor out the lambertian diffuse based on the material's metallicity and fresenal.
		// e.g If the material is fully metallic than it wouldn't have diffuse.
		mediump vec3 kD =  (vec3(1.0) - F) * (1.0 - metallic);
		mediump vec3 diff = kD * albedo * ONE_OVER_PI;

		mediump float visibility = 1.0f;

		if (rayDepth <= MAX_REFLECTION_SHADOW_RAY_DEPTH)
			visibility = traceShadowRay(P + N * 0.1f, L) ? 0.0f : 1.0f;

		///-------- DIFFUSE + SPEC ------
		color += visibility * (diff + spec) * lightData.vLightColor.rgb * dotNL;// scale the final colour based on the angle between the light and the surface normal.
	}

	return color;
}

mediump vec3 prefilteredReflection(mediump float roughness, mediump vec3 R)
{
	// We need to detect where we need to sample from.
	const mediump float maxmip = float(3);

	mediump float cutoff = 1. / maxmip;

	if(roughness <= cutoff)
	{
		mediump float lod = roughness * maxmip;
		return mix(textureLod(skyboxImage, R, 0.0f).rgb, textureLod(prefilteredImage, R, 0.).rgb, 0.0f);
	}
	else
	{
		mediump float lod = (roughness - cutoff) * maxmip / (1. - cutoff); // Remap to 0..1 on rest of mimpmaps
		return textureLod(prefilteredImage, R, lod).rgb;
	}
}

mediump vec3 indirectLightingIBL(mediump vec3 N, mediump vec3 V, mediump vec3 R, mediump vec3 albedo, mediump vec3 F0, mediump float metallic, mediump float roughness)
{
	mediump vec3 specularIR = prefilteredReflection(roughness, R);
	mediump vec2 brdf = texture(brdfLUT, vec2(clamp(dot(N, V), 0.0, 1.0), roughness)).rg;

	mediump vec3 F = f_schlickR(max(dot(N, V), 0.0), F0, roughness);

	mediump vec3 diffIR = textureLod(irradianceImage, N, 0.0f).rgb;
	mediump vec3 kD  = (vec3(1.0) - F) * (1.0 - metallic);// Diffuse factor   

	return albedo * kD  * diffIR + specularIR * (F * brdf.x + brdf.y);
}

mediump vec3 indirectSpecular(highp vec3 N, mediump vec3 R)
{
	mediump vec3 Li = vec3(0.0f);
	mediump vec3 T = vec3(1.0f);

	mediump vec3 origin = vWorldPosition + N * 0.1f;
	highp vec3 direction = R;

	for (int i = 0; i < NUM_REFLECTION_RAYS; i++)
	{
		int matID;
		vec3 normal;
		vec3 hitPos;
		vec2 texCoord;
		
		if (traceRay(origin, direction, hitPos, normal, texCoord, matID))
		{
			const Material mat = materials[nonuniformEXT(matID)];
			
			N = normalize(normal);	
			vec3 V = normalize(-direction);
			R = normalize(reflect(-V, N));
			vec3 L = normalize(lightData.vLightPosition.xyz - hitPos);
			
			vec3 albedo;
			float roughness = max(mat.metallicRoughness.g, 0.01);
			float metallic = mat.metallicRoughness.r;
			
			// If a valid texture index does not exist, use the albedo color stored in the Material structure
			if (mat.textureIndices.x == -1)
				albedo = mat.baseColor.rgb;
			else  // If a valid texture index exists, use it to index into the image sampler array and sample the texture
				albedo = textureLod(textureSamplers[nonuniformEXT(mat.textureIndices.x)], texCoord, 0.0f).rgb;
			
			vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
			
			// Direct Lighting
			Li += T * directLighting(hitPos, V, N, L, F0, albedo, metallic, roughness, i);
		
			// Indirect Lighting
			if (i == MAX_REFLECTION_DEPTH)
				Li += T * indirectLightingIBL(N, V, R, albedo, F0 , metallic, roughness); 
			else
			{
				mediump vec2 brdf = texture(brdfLUT, vec2(clamp(dot(N, V), 0.0, 1.0), roughness)).rg;
				mediump vec3 F = f_schlickR(max(dot(N, V), 0.0), F0, roughness);

				mediump vec3 diffIR = texture(irradianceImage, N).rgb;
				mediump vec3 kD  = (vec3(1.0) - F) * (1.0 - metallic);

				// Indirect Diffuse
				Li += T * (albedo * kD  * diffIR);

				// Accumulate the throughput for the next specular bounce
				T *= (F * brdf.x + brdf.y);

				origin = hitPos + N * 0.1f;
				direction = R;
			}
		}
		else
		{
			Li += T * prefilteredReflection(0.0f, direction); 
			break;
		}
	}

	return Li;
}

mediump vec3 indirectLighting(mediump vec3 N, mediump vec3 V, mediump vec3 R, mediump vec3 albedo, mediump vec3 F0, mediump float metallic, mediump float roughness)
{
	mediump vec3 specularIR = indirectSpecular(N, R);
	mediump vec2 brdf = texture(brdfLUT, vec2(clamp(dot(N, V), 0.0, 1.0), roughness)).rg;

	mediump vec3 F = f_schlickR(max(dot(N, V), 0.0), F0, roughness);

	mediump vec3 diffIR = texture(irradianceImage, N).rgb;
	mediump vec3 kD  = (vec3(1.0) - F) * (1.0 - metallic);// Diffuse factor   

	return albedo * kD  * diffIR + specularIR * (F * brdf.x + brdf.y);
}

void main()
{
	const float ambientIntensity = 0.01f;
	
	const Material mat = materials[materialID];

	vec3 N = normalize(vNormal);	
	vec3 V = normalize(vCameraPosition.xyz - vWorldPosition);
	vec3 R = normalize(reflect(-V, N));
	vec3 L = normalize(lightData.vLightPosition.xyz - vWorldPosition);

	vec3 albedo;
	float roughness = max(mat.metallicRoughness.g, 0.01);
	float metallic = mat.metallicRoughness.r;

	// If a valid texture index does not exist, use the albedo color stored in the Material structure
	if (mat.textureIndices.x == -1)
		albedo = mat.baseColor.rgb;
	else // If a valid texture index exists, use it to index into the image sampler array and sample the texture
		albedo = texture(textureSamplers[mat.textureIndices.x], vTexCoord).rgb;

	vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);

	vec3 Li = vec3(0.0f);

	// Direct Lighting
	Li += directLighting(vWorldPosition, V, N, L, F0, albedo, metallic, roughness, 0);

	// Indirect Lighting
	Li += indirectLighting(N, V, R, albedo, F0 , metallic, roughness);

    oColor = vec4(Li, 1.0);
}
