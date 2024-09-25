#version 310 es

layout(binding = 0) uniform highp sampler2D albedoTexture;
layout(binding = 1) uniform highp sampler2D normalTexture;
layout(binding = 2) uniform highp sampler2D roughnessMetallicTextureIDTexture;
#if defined LOW_QUALITY_MATERIALS || defined HIGH_QUALITY_MATERIAL
layout(binding = 3) uniform highp samplerCube environmentTexture;
layout(binding = 4) uniform highp sampler2D brdfLUTmap;
#endif // LOW_QUALITY_MATERIALS || HIGH_QUALITY_MATERIAL

layout (location = 0) out mediump vec4 oColor;

uniform highp int uNumEnvironmentMipMap;

in highp vec2 vUV;
#if defined LOW_QUALITY_MATERIALS || defined HIGH_QUALITY_MATERIAL
in highp vec3 worldPosition;
in highp vec3 cameraPosition;
in highp vec3 normal;
#endif // LOW_QUALITY_MATERIALS || HIGH_QUALITY_MATERIAL

highp vec3 reinhardTonemapping(highp vec3 color)
{
	highp float gamma = 1.5;
	highp float exposure = 1.5;
	color *= exposure/(1.0 + color / exposure);
	color = pow(color, vec3(1.0 / gamma));
	return color;
}

#if defined HIGH_QUALITY_MATERIAL

mediump vec3 f_schlickR(mediump float cosTheta, mediump vec3 F0, mediump float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

#endif // HIGH_QUALITY_MATERIAL

void main()
{
	highp vec3 albedo = texture(albedoTexture, vUV).xyz;
#if defined LOW_QUALITY_MATERIALS

	// TODO: Adjust precission for performance
	highp vec3 I = normalize(worldPosition - cameraPosition);
	highp vec3 R = reflect(I, normalize(normal));
	highp vec4 roughnessMetallic = texture(roughnessMetallicTextureIDTexture, vUV);
	highp vec3 finalColor = albedo * mix(vec3(1.0), texture(environmentTexture, R).rgb, roughnessMetallic.y);
	oColor = vec4(reinhardTonemapping(finalColor), 1.0);

#elif defined HIGH_QUALITY_MATERIAL

    highp vec4 roughnessMetallic = texture(roughnessMetallicTextureIDTexture, vUV);
	highp float roughness = roughnessMetallic.x;
	highp float metallic = roughnessMetallic.y;
    highp vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    highp vec3 N = normalize(normal);
    highp vec3 V = normalize(cameraPosition - worldPosition);
    highp vec3 R = reflect(-V, N); 

    // Standard Fresnel implementation in PBR
    //highp vec3 F = f_schlickR(max(dot(N, V), 0.0), F0, roughness);

    // Avoid fresnel highlights at grazing angles
	highp vec3 specularColor = mix(F0, albedo, metallic);
	highp float reflectance0 = max(max(F0.x, F0.y), F0.z);
	highp float reflectance90 = clamp(reflectance0 * 0.1, 0.0, 1.0);
	highp vec3 F = F0 + (reflectance90 - F0) * pow(clamp(1.0 - max(dot(N, V), 0.0), 0.0, 1.0), 5.0);

    highp vec3 kS = F;
    highp vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    highp vec3 prefilteredColor = textureLod(environmentTexture, R,  0.0).rgb;
    highp vec2 brdf = texture(brdfLUTmap, vec2(max(dot(N, V), 0.0), 0.0)).rg;
    highp vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    highp vec3 ambient = (kD * albedo + specular);
    highp vec3 color = ambient;
    oColor = vec4(reinhardTonemapping(color), 1.0);

#else

	oColor = vec4(reinhardTonemapping(albedo), 1.0);

#endif
}
