#version 450

layout(set = 0, binding = 1) uniform sampler2D sParaboloids;
layout(set = 0, binding = 2) uniform samplerCube sSkybox;

layout(location = 0) in highp vec3 ReflectDir;
layout(location = 1) in highp vec3 RefractDir;
layout(location = 2) in highp float ReflectFactor;

layout(location = 0) out vec4 outColor;
void main()
{
	mediump vec3 Normalised;

	// Sample reflection to skybox
	lowp vec4 ReflectSky = texture(sSkybox, ReflectDir);

	// Sample reflection to paraboloids
	lowp vec4 Reflection;

	highp vec3 vkReflectDir = ReflectDir;
	vkReflectDir.y = -vkReflectDir.y;
	Normalised = normalize(vkReflectDir);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	Reflection = texture(sParaboloids, Normalised.xy);

	// Combine skybox reflection with paraboloid reflection
	Reflection.rgb = mix(ReflectSky.rgb, Reflection.rgb, Reflection.a);


	lowp vec4 RefractSky = texture(sSkybox, RefractDir);

	highp vec3 vkRefractDir = RefractDir;
	vkRefractDir.y = -vkRefractDir.y;
	Normalised = normalize(vkRefractDir);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	lowp vec4 Refraction = texture(sParaboloids, Normalised.xy);

	Refraction.rgb = mix(RefractSky.rgb, Refraction.rgb, Refraction.a);
	// Combine reflection and refraction for final color
	outColor.rgb = mix(Refraction.rgb, Reflection.rgb, ReflectFactor);
	outColor.a = 1.0;
}
