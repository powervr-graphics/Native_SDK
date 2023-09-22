#version 310 es

#define PI 3.1415926535897932384626433832795
#define ONE_OVER_PI (1.0 / 3.1415926535897932384626433832795)

layout(location = 0) in highp vec3 inWorldPos;
layout(location = 1) in highp vec3 inNormal;

layout(location = 0) out mediump vec4 outColor;

layout(std140, binding = 0) uniform UboDynamic
{
	highp mat4 VPMatrix;
	highp vec3 camPos;
} uboDynamic;


layout(std140, binding = 3) uniform Material
{
	mediump vec3 albedo;
	mediump float roughness;
} material;



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


mediump vec3 computeEnvironmentLighting(mediump vec3 N, mediump vec3 V, mediump vec3 R, mediump vec3 albedo,
    mediump vec3 F0, mediump float metallic, mediump float roughness, highp float distence)
{

	mediump vec3 F = f_schlickR(max(dot(N, V), 0.0), F0, roughness);

	mediump vec3 diffuseIR = vec3(1.5, 1.5, 1.5) / distence;
	mediump vec3 kD  = vec3(1.0 - F) * (1.0 - metallic);// Diffuse factor
	
	return albedo * kD  * diffuseIR;
}


void main()
{

	highp vec3 N = normalize(inNormal);
	highp float distence = length(uboDynamic.camPos - inWorldPos);
	// calculate the view direction, the direction from the surface towards the camera in world space.
	highp vec3 V = normalize(uboDynamic.camPos - inWorldPos);
	// calculate the reflection vector. The reflection vector is a reflected View vector on the surface.
	mediump vec3 R = -normalize(reflect(V, N));
	
	mediump float roughness = material.roughness;
	mediump float metallic = 0.4;

	mediump vec4 albedo = vec4(material.albedo, 1.0);



	// The base colour has two different interpretations depending on the value of metalness.
	// When the material is a metal, the base colour is the specific measured reflectance value at normal incidence (F0).
	// For a non-metal the base colour represents the reflected diffuse colour of the material.
	// In this model it is not possible to specify a F0 value for non-metals, and a linear value of 4% (0.04) is used.
	mediump vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);

	mediump vec3 color = vec3(0.0);

	// Compute the direction light diffuse and specular
	// highp vec3 dirLightDiffuseSpec = computeLight(V, N, F0, albedo.rgb, metallic, roughness);
	// colour = dirLightDiffuseSpec;

	// IBL
	mediump vec3 envLighting = computeEnvironmentLighting(N, V, R, albedo.rgb, F0, metallic, roughness, distence);

	color += envLighting;

	// This seemingly strange clamp is to ensure that the final colour stays within the constraints
	// of 16-bit floats (13848) with a bit to spare, as the tone mapping calculations squares 
	// this number. It does not affect the final image otherwise, as the clamp will only bring the value to
	// 50. This would already be very close to saturated (producing something like .99 after tone mapping), 
	// but it was trivial to tweak the tone mapping to ensure 50 produces a value >=1.0.
	// It is important to remember that this clamping must only happen last minute, as we need to have
	// the full brightness available for post processing calculations (e.g. bloom)

	mediump vec3 finalColor = min(color.rgb, 50. / 1.0);
	finalColor *= 1.0;

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Our favourite is the optimized formula by Jim Hejl and Richard Burgess-Dawson
	// We particularly like its high contrast and the fact that it also takes care
	// of Gamma.
	// As mentioned, we modified the values a bit to ensure it saturates at 50.

	mediump vec3 x = max(vec3(0.), finalColor - vec3(0.004));
	finalColor = (x * (6.2 * x + .49)) / (x * (6.175 * x + 1.7) + 0.06);

	outColor = vec4(finalColor, albedo.a);
}