#version 310 es

#define PI 3.1415926535897932384626433832795
const highp float ONE_OVER_PI = 1.0 / PI;
const mediump float ONE_OVER_EIGHT = 1.0 / 8.0;
const highp  float ExposureBias = 4.0;

const highp vec3 lightColor = vec3(1.0);

layout(location = 0) in highp vec3 inWorldPos;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec2 inTexCoord;
layout(location = 3) in mediump vec2 inMetallicRoughness;// r: metallic, g: roughness
layout(location = 4) in mediump vec3 rgb;
layout(location = 5) in mediump vec3 inTangent;
layout(location = 6) in mediump vec3 inBitTangent;

#ifdef MATERIAL_TEXTURES
layout(binding = 0) uniform mediump sampler2D albedoMap;
layout(binding = 1) uniform mediump sampler2D metallicRoughnessMap;
layout(binding = 2) uniform mediump sampler2D normalMap;
layout(binding = 3) uniform mediump sampler2D emissiveMap;
#endif

layout(binding = 4) uniform mediump samplerCube irradianceMap;
layout(binding = 5) uniform mediump samplerCube prefilteredMap;
layout(binding = 6) uniform mediump sampler2D brdfLUTmap;
layout(binding = 7) uniform mediump samplerCube prefilteredMipMap;

layout(location = 0) out mediump vec4 outColor;

layout(std140, binding = 0) uniform UboStatic
{
	highp vec3 lightDir;
	uint numPrefilteredMipLevels;
} uboStatic;

layout(std140, binding = 1) uniform UboDynamic
{
	highp vec3 camPos;
	highp mat4 VPMatrix;
} uboDynamic;

layout(std140, binding = 2) uniform UboPerModel
{
	highp mat4 ModelMatrix;
	mediump float emissiveScale;
} uboPerModel;

// Normal Distribution function
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
mediump float d_GGX(mediump float dotNH, mediump float roughness)
{
    mediump float alpha = roughness * roughness;
    mediump float alpha2 = alpha * alpha;
    mediump float x = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * x * x);
}

mediump float g1(mediump float dotAB, mediump float k)
{
	return dotAB / (dotAB * (1.0 - k) + k);
}

// Geometric Shadowing function
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
mediump float g_schlicksmithGGX(mediump float dotNL, mediump float dotNV, mediump float roughness)
{
	mediump float k = (roughness + 1.0);
	k = (k * k) * ONE_OVER_EIGHT;
	return g1(dotNL, k) * g1(dotNV, k);
}

// Fresnel function (Shlicks)
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
mediump vec3 f_schlick(mediump float cosTheta, mediump vec3 F0)
{
	// OPTIMIZED.
	return F0 + (vec3(1.0) - F0) * pow(2.0, (-5.55473 * cosTheta - 6.98316) * cosTheta);
}

mediump vec3 f_schlickR(mediump float cosTheta, mediump vec3 F0, mediump float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

mediump vec3 computeLight(mediump vec3 V, mediump vec3 N, mediump vec3 F0, mediump vec3 albedo, mediump float metallic, mediump float roughness)
{
	// Directional light
	// NOTE: we negate the light direction here because the direction was from the light source.
	mediump vec3 L = normalize(-uboStatic.lightDir);
	// half vector
	mediump vec3 H = normalize (V + L);

	mediump float dotNL = clamp(dot(N, L), 0.0, 1.0);
	highp vec3 color = vec3(0.0);

	// light contributed only if the angle between the normal and light direction is less than equal to 90 degree.
	if(dotNL > 0.0)
	{
		mediump float dotLH = clamp(dot(L, H), 0.0, 1.0);
		mediump float dotNH = clamp(dot(N, H), 0.0, 1.0);
		mediump float dotNV = clamp(dot(N, V), 0.0, 1.0);

		///--------  Specular BRDF: COOK-TORRANCE ---------
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
		color += (diff + spec) * lightColor * dotNL;// scale the final color based on the angle between the light and the surface normal.
	}
	return color;
}

mediump vec3 RGBMDecode(mediump vec4 rgbm) 
{
	return 6.0 * rgbm.rgb * rgbm.a;
}

mediump vec3 prefilteredReflection(mediump float roughness, mediump vec3 R)
{
	mediump float lod = roughness * float(uboStatic.numPrefilteredMipLevels - 1u);
	if(lod < 1.)
	{
		return mix(RGBMDecode(texture(prefilteredMipMap, R)), RGBMDecode(textureLod(prefilteredMap, R, lod)), lod);
	}
	else
	{
		return RGBMDecode(textureLod(prefilteredMap, R, lod));
	}
}

mediump vec3 computeEnvironmentLighting(mediump vec3 N, mediump vec3 V, mediump vec3 R, mediump vec3 albedo,
    mediump vec3 F0, mediump float metallic, mediump float roughness)
{
	mediump vec3 specularIR = prefilteredReflection(roughness, R);
	mediump vec2 brdf = texture(brdfLUTmap, vec2(clamp(dot(N, V), 0.0, 1.0), roughness)).rg;
    
	mediump vec3 F = f_schlickR(max(dot(N, V), 0.0), F0, roughness);

	mediump vec3 diffIR = RGBMDecode(texture(irradianceMap, N));
	mediump vec3 kD  = vec3(1.0 - F) * (1.0 - metallic);// Diffuse factor
	return albedo * kD  * diffIR + specularIR * (F * brdf.x + brdf.y);  
}

#ifdef NORMAL_MAP
mediump vec3 perturbNormal()
{
	// transform the tangent space normal into model space. 
	mediump vec3 tangentNormal = texture(normalMap, inTexCoord).xyz * 2.0 - 1.0;
	mediump vec3 n = normalize(inNormal);
	mediump vec3 t = normalize(inTangent);
	mediump vec3 b = normalize(inBitTangent);
	return normalize(mat3(t, b, n) * tangentNormal);
}
#endif

void main()
{
#ifdef NORMAL_MAP
	mediump vec3 N = perturbNormal();    
#else
	mediump vec3 N = normalize(inNormal);
#endif
	// calculate the view direction, the direction from the surface towards the camera in world space.
	mediump vec3 V = normalize(uboDynamic.camPos - inWorldPos);

	// calculate the reflection vector. The reflection vector is a reflected View vector on the surface.
	mediump vec3 R = -normalize(reflect(V, N));
	
	mediump vec2 metallicRoughness = inMetallicRoughness;

	mediump vec4 albedo = vec4(rgb, 1.0);

#ifdef MATERIAL_TEXTURES
	metallicRoughness *= texture(metallicRoughnessMap, inTexCoord).bg;

	mediump vec4 albedoTex = texture(albedoMap, inTexCoord);
	albedo.rgb *= albedoTex.rgb;
	albedo.a = albedoTex.a;

	mediump vec3 emissive = texture(emissiveMap, inTexCoord).rgb * uboPerModel.emissiveScale;
#endif

	// The base color has two different interpretations depending on the value of metalness.
	// When the material is a metal, the base color is the specific measured reflectance value at normal incidence (F0).
	// For a non-metal the base color represents the reflected diffuse color of the material.
	// In this model it is not possible to specify a F0 value for non-metals, and a linear value of 4% (0.04) is used.
	mediump vec3 F0 = mix(vec3(0.04), albedo.rgb, metallicRoughness.r);

	mediump vec3 color = vec3(0.0);

	// Compute the direction light diffuse and specular
	//mediump vec3 dirLightDiffuseSpec = computeLight(V, N, F0, albedo.rgb, metallicRoughness.r, metallicRoughness.g);
	//color += dirLightDiffuseSpec;

	// IBL
	mediump vec3 envLighting = computeEnvironmentLighting(N, V, R, albedo.rgb, F0, metallicRoughness.r, metallicRoughness.g);

	color += envLighting;
#ifdef MATERIAL_TEXTURES
	color += emissive;
#endif
    
	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tonemapping
	mediump vec3 ldrColor = color * ExposureBias;
	ldrColor = ldrColor / (1.0 + ldrColor);

#ifndef FRAMEBUFFER_SRGB
	ldrColor = pow(ldrColor, vec3(0.454545));
#endif

	outColor = vec4(ldrColor, albedo.a);
}