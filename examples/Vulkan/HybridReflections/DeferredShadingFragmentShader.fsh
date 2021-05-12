#version 450

#define PI 3.1415926535897932384626433832795
#define ONE_OVER_PI (1.0 / PI)

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

layout(set = 1, binding = 0) uniform sampler2D gbufferAlbedo;
layout(set = 1, binding = 1) uniform sampler2D gbufferNormal_Roughness;
layout(set = 1, binding = 2) uniform sampler2D gbufferWorldPosition_Metallic;
layout(set = 1, binding = 3) uniform sampler2D reflectionsImage;

layout(set = 2, binding = 0) uniform samplerCube skyboxImage;
layout(set = 2, binding = 1) uniform samplerCube prefilteredImage;
layout(set = 2, binding = 2) uniform samplerCube irradianceImage;
layout(set = 2, binding = 3) uniform sampler2D brdfLUT;

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inRayDir;

layout(location = 0) out vec4 outFragColor;

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

mediump vec3 directLighting(mediump vec3 V, mediump vec3 N, mediump vec3 L, mediump vec3 F0, mediump vec3 albedo, mediump float metallic, mediump float roughness, mediump float visibility)
{
	// half vector
	mediump vec3 H = normalize(V + L);

	mediump float dotNL = clamp(dot(N, L), 0.0, 1.0);

	mediump vec3 color = vec3(0.0);

	// light contributed only if the angle between the normal and light direction is less than equal to 90 degree.
	if (dotNL > 0.0)
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

		///-------- DIFFUSE + SPEC ------
		color += visibility * (diff + spec) * lightData.vLightColor.rgb * dotNL;// scale the final colour based on the angle between the light and the surface normal.
	}

	return color;
}

mediump vec3 indirectLighting(mediump vec3 N, mediump vec3 V, mediump vec3 R, mediump vec3 albedo, mediump vec3 F0, mediump float metallic, mediump float roughness, mediump vec3 specularIR)
{
	mediump vec2 brdf = texture(brdfLUT, vec2(clamp(dot(N, V), 0.0, 1.0), roughness)).rg;

	mediump vec3 F = f_schlickR(max(dot(N, V), 0.0), F0, roughness);

	// Indirect Diffuse 
	mediump vec3 diffIR = texture(irradianceImage, N).rgb;
	mediump vec3 kD  = (vec3(1.0) - F) * (1.0 - metallic);   

	return albedo * kD  * diffIR + specularIR * (F * brdf.x + brdf.y);
}

void main()
{
	// Unpack the values stored in the G-Buffer
	vec4 gbuffer0 = texture(gbufferAlbedo, inUV);
	vec4 gbuffer1 = texture(gbufferNormal_Roughness, inUV);
	vec4 gbuffer2 = texture(gbufferWorldPosition_Metallic, inUV);
	float foregroundFlag = gbuffer0.w;

	vec3 Li = vec3(0.0f);

	// If the foregroundFlag is 0.0f, the fragment belongs to the skybox so we can sample the cubemap
	if (foregroundFlag != 0.0f)
	{
		vec3 P = gbuffer2.xyz;
		vec3 N = gbuffer1.xyz;	
		vec3 V = normalize(vCameraPosition.xyz - P);
		vec3 R = -normalize(reflect(V, N));
		vec3 L = normalize(lightData.vLightPosition.xyz - P);

		vec3 albedo = gbuffer0.xyz;
		float roughness = gbuffer1.w;
		float metallic = gbuffer2.w;

		vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);

		vec4 reflectionAndVisiblity = texture(reflectionsImage, inUV);
	
		// Direct Lighting
		Li += directLighting(V, N, L, F0, albedo, metallic, roughness, reflectionAndVisiblity.w);

		// Indirect Lighting
		Li += indirectLighting(N, V, R, albedo, F0 , metallic, roughness, reflectionAndVisiblity.xyz);
	}
	else
		Li = texture(skyboxImage, inRayDir).rgb;

    outFragColor = vec4(Li, 1.0f);
}