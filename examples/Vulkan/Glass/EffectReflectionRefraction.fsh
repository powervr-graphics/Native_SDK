#version 320 es

layout(set = 0, binding = 1) uniform mediump sampler2D sParaboloids;
layout(set = 0, binding = 2) uniform mediump samplerCube sSkybox;

layout(location = 0) in mediump vec3 ReflectDir;
layout(location = 1) in mediump vec3 RefractDir;
layout(location = 2) in mediump float ReflectFactor;

layout(location = 0) out mediump vec4 outColor;
void main()
{
	// Sample reflection to skybox
	mediump vec4 ReflectSky = texture(sSkybox, ReflectDir);

	mediump vec3 vkReflectDir = ReflectDir;
	vkReflectDir.y = -vkReflectDir.y;
	mediump vec3 Normalised = normalize(vkReflectDir);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	// Sample reflection to paraboloids
	mediump vec4 Reflection = texture(sParaboloids, Normalised.xy);

	// Combine skybox reflection with paraboloid reflection
	Reflection.rgb = mix(ReflectSky.rgb, Reflection.rgb, Reflection.a);

	mediump vec4 RefractSky = texture(sSkybox, RefractDir);

	mediump vec3 vkRefractDir = RefractDir;
	vkRefractDir.y = -vkRefractDir.y;
	Normalised = normalize(vkRefractDir);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	mediump vec4 Refraction = texture(sParaboloids, Normalised.xy);

	Refraction.rgb = mix(RefractSky.rgb, Refraction.rgb, Refraction.a);
	// Combine reflection and refraction for final color
	outColor.rgb = mix(Refraction.rgb, Reflection.rgb, ReflectFactor);
	outColor.a = 1.0;
}
