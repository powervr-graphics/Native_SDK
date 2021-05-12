#version 450

// maximum number of light sources supported
#define MAXIMUM_LIGHTS 3

//// Constants ////
const highp float PI = 3.14159265359;

struct Material
{
	ivec4 textureIndices;
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

layout(set = 1, binding = 0) uniform sampler2D gbufferAlbedo_Metallic;
layout(set = 1, binding = 1) uniform sampler2D gbufferNormal_Reflectivity;
layout(set = 1, binding = 2) uniform sampler2D gbufferWorldPosition_F90;
layout(set = 1, binding = 3) uniform sampler2D gbufferF0_Roughness;
layout(set = 1, binding = 4) uniform sampler2D shadowMask;

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 oColor;

// Unpack the image from the 2-component octahedral packing into a 3-component direction vector
vec3 unpackNormal(vec2 e)
{
	vec3 v = vec3(e, 1.0 - abs(e.x) - abs(e.y));
    if (v.z < 0.0)
        v.xy = (1.0 - abs(v.yx)) * (step(0.0, v.xy) * 2.0 - vec2(1.0));
    return normalize(v);
}

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

void main()
{
	// Unpack the values stored in the G-Buffer
	vec4 gbuffer0 = texture(gbufferAlbedo_Metallic, vTexCoord);
	vec4 gbuffer1 = texture(gbufferNormal_Reflectivity, vTexCoord);
	vec4 gbuffer2 = texture(gbufferWorldPosition_F90, vTexCoord);
	vec4 gbuffer3 = texture(gbufferF0_Roughness, vTexCoord);

	vec3 normal = normalize(gbuffer1.xyz);
	vec3 worldPosition = gbuffer2.xyz;
	
	highp vec3 directionFromEye = normalize(worldPosition - vCameraPosition.xyz);
	highp vec3 eyeDirection = -directionFromEye;

	// Output black for any fragments belonging to the background 
	if ((normal.x + normal.y + normal.z) == 0.0f)
	{
		oColor = vec4(0.0);
		return;
	}

	highp vec3 baseColor = gbuffer0.xyz;
	highp float reflectivity = gbuffer1.w;
	highp float roughness = gbuffer3.w;
	highp float metallicity = gbuffer0.w;
	highp float f90 = gbuffer2.w;
	highp vec3 f0 = gbuffer3.xyz;

	// the reflective color at normal incidence only includes albedo for metals
	f0 = metallicity >= 1.0 ? f0 * baseColor.rgb : f0;

	// Colored ambient term
	highp float coloredAmbient = reflectivity * (metallicity - 1.0) + 1.0;
	// Specular, non-metallicity ambient term
	highp float whiteAmbient = reflectivity * (1.0 - metallicity);
	highp vec4 ambientColor = roughness * vec4((whiteAmbient * vAmbientLightColor.rgb) + (coloredAmbient * baseColor.rgb * vAmbientLightColor.rgb), 1.0);

	// Sample the shadow mask texture to determine the visiblity for the currently shaded fragment
    vec4 visibility = texture(shadowMask, vTexCoord);

	// diffuse base color for non-metallicity objects and fresnel reflectance at normal incidence for metallicity objects
	baseColor = metallicity >= 1.0 ? vec3(f0) : baseColor;

	// outgoing radiance towards the camera
	vec3 Lo = vec3(0.0f);

	for(uint i = 0u; i < uNumLights; i++)
	{
		PerLightData currentLight = lightData[i];

		// calculate the direction to the current light source
		highp vec3 directionToLight = currentLight.lightPosition.xyz - worldPosition;
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
						
		Lo += visibility[i] * (currentLight.lightIntensity * currentLight.lightColor.rgb) * (diffuseLightColor + specularLightColor);
	}

	// Lo += ambientColor.xyz;
	Lo += baseColor.xyz * 0.01f;

    // Reinhard tone mapping
    Lo = Lo / (1.0 + Lo);

    oColor = vec4(Lo, 1.0f);
}