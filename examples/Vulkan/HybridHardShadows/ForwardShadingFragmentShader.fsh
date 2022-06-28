#version 460

#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in highp vec3 vNormal;
layout(location = 2) in highp vec3 vWorldPosition;

layout(location = 0) out mediump vec4 oColor;

// maximum number of light sources supported
#define MAXIMUM_LIGHTS 3

//// Constants ////
const highp float PI = 3.14159265359;

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
};

layout(set = 0, binding = 0) uniform GlobalUBO
{
	highp mat4 mViewMatrix;
	highp mat4 mProjectionMatrix;
	highp vec4 vAmbientLightColor;
	highp vec4 vCameraPosition;
	uint uNumLights;
};

layout(set = 0, binding = 1) uniform LightDataUBO
{
	PerLightData lightData[MAXIMUM_LIGHTS];
};

layout(set = 0, binding = 2) buffer MateralDataBufferBuffer { Material materials[]; } ;
layout(set = 0, binding = 3) uniform accelerationStructureEXT topLevelAS;

layout(push_constant) uniform PushConsts {
	layout(offset = 4) uint materialID;
};

// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"].
highp float D_GGX(in highp float roughness, in highp float NdotH)
{
	float m2 = roughness * roughness;
	float d = (NdotH * m2 - NdotH) * NdotH + 1.0;
	return m2 / (d * d);
	// note that the divide by PI is applied at a later stage
}

// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
// [Lagarde 2012, "Spherical Gaussian approximation for Blinn-Phong, Phong and Fresnel"]
highp vec3 F_Schlick(in highp vec3 f0, in highp float f90, in highp float u)
{
	return f0 + (vec3(f90) - f0) * pow(1.0 - u, 5.0);
}

// Frostbite optimized GGX Correlated
highp float V_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
	float alphaG2 = alphaG * alphaG;
	float Lambda_GGXV = NdotL * sqrt(( - NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
	float Lambda_GGXL = NdotV * sqrt(( - NdotL * alphaG2 + NdotL) * NdotL + alphaG2);
	return 0.5 / (Lambda_GGXV + Lambda_GGXL);
}

float queryVisiblity(vec3 origin, vec3 direction)
{
	float visibility = 1.0f;
    float tMin     = 1.0f;
    float tMax     = 10000.0;

	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tMin, direction, tMax);
	
	// Start traversal: return false if traversal is complete
	while(rayQueryProceedEXT(rayQuery))
	{
	}
	
	// Returns type of committed (true) intersection
	if(rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT)
	{
	  // Got an intersection == Shadow
	  visibility = 0.0f;
	}

	return visibility;
}

void main()
{
	highp vec3 normal = normalize(vNormal);	
	highp vec3 directionFromEye = normalize(vWorldPosition - vCameraPosition.xyz);
	highp vec3 eyeDirection = -directionFromEye;

	// Retrieve material properties		
	const Material mat = materials[materialID];

	highp vec3 baseColor = mat.baseColor.rgb;

	highp float reflectivity = mat.metallicRoughnessReflectivity.b;
	highp float roughness = mat.metallicRoughnessReflectivity.g;
	highp float metallicity = mat.metallicRoughnessReflectivity.r;
	highp float f90 = mat.f0f90.a;
	highp vec3 f0 = mat.f0f90.rgb;

	// the reflective color at normal incidence only includes albedo for metals
	f0 = metallicity >= 1.0 ? f0 * baseColor.rgb : f0;

	// Colored ambient term
	highp float coloredAmbient = reflectivity * (metallicity - 1.0) + 1.0;
	// Specular, non-metallicity ambient term
	highp float whiteAmbient = reflectivity * (1.0 - metallicity);
	highp vec4 ambientColor = roughness * vec4((whiteAmbient * vAmbientLightColor.rgb) + (coloredAmbient * baseColor.rgb * vAmbientLightColor.rgb), 1.0);

	// diffuse base color for non-metallicity objects and fresnel reflectance at normal incidence for metallicity objects
	baseColor = metallicity >= 1.0 ? vec3(f0) : baseColor;

	// outgoing radiance towards the camera
	vec3 Lo = vec3(0.0f);

	for(uint i = 0u; i < uNumLights; i++)
	{
		PerLightData currentLight = lightData[i];

		// calculate the direction to the current light source
		highp vec3 directionToLight = currentLight.lightPosition.xyz - vWorldPosition;
		highp vec3 normalisedDirectionToLight = normalize(directionToLight);

		highp vec3 halfVector = normalize(normalisedDirectionToLight + eyeDirection);
		highp float NdotL = clamp(dot(normal, normalisedDirectionToLight), 0.0, 1.0);
		highp float LdotH = clamp(dot(normalisedDirectionToLight, halfVector), 0.0, 1.0);
		highp float NdotH = clamp(dot(normal, halfVector), 0.0, 1.0);
		highp float NdotV = abs(dot(normal, eyeDirection)) + 1e-1f; // add an episilon value to avoid artifacts at grazing angles

		// F - Fresnel Function using Shlick
		// The Fresnel equation describes the ratio of surface reflection at different surface angles
		highp vec3 F = F_Schlick(f0, f90, LdotH);

		// D - Microfacet Distribution Function using GGX
		// The distribution function approximates the amount the surface's microfacets are aligned to the halfway vector influenced by the roughness of the surface; this is the primary function approximating the microfacets
		highp float D = D_GGX(roughness, NdotH);

		// G - Geometry Function using Sclick/Smith
		// describes the self-shadowing property of the microfacets. When a surface is relatively rough the surface's microfacets can overshadow other microfacets thereby reducing the light the surface reflects.
		highp float G = V_SmithGGXCorrelated(NdotL, NdotV, roughness);

		// Specular Color
		highp vec3 Fr = (F * D * G) / PI;
		highp vec3 specularLightColor = NdotL * Fr;

		// Diffuse Color
		highp float Fd = clamp(NdotL / PI, 0.0, 1.0);
		highp vec3 diffuseLightColor = Fd * baseColor.rgb;
					
		// Is this fragment in shadow?
		float visibility = queryVisiblity(vWorldPosition + normal * 0.1f, normalisedDirectionToLight);

		Lo += visibility * (currentLight.lightIntensity * currentLight.lightColor.rgb) * (diffuseLightColor + specularLightColor);
	}

	// Lo += ambientColor.xyz;
	Lo += baseColor.xyz * 0.01f;

    // Reinhard tone mapping
    Lo = Lo / (1.0 + Lo);

    oColor = vec4(Lo, 1.0);
}
