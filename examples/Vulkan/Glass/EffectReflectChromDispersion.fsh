#version 320 es

layout(set = 0, binding = 1) uniform mediump sampler2D sParaboloids;
layout(set = 0, binding = 2) uniform mediump samplerCube sSkybox;

layout(location = 0) in mediump vec3 ReflectDir;
layout(location = 1) in mediump vec3 RefractDirRed;
layout(location = 2) in mediump vec3 RefractDirGreen;
layout(location = 3) in mediump vec3 RefractDirBlue;
layout(location = 4) in mediump float ReflectFactor;

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

	// Sample refraction to skybox
	mediump vec4 RefractSky;
	RefractSky.r = texture(sSkybox, RefractDirRed).r;
	RefractSky.g = texture(sSkybox, RefractDirGreen).g;
	RefractSky.b = texture(sSkybox, RefractDirBlue).b;
	
	// Sample refraction to paraboloids
	mediump vec4 Refraction;

	// Red
	mediump vec3 vkRefractDirRed = RefractDirRed;
	vkRefractDirRed.y = -vkRefractDirRed.y;
	Normalised = normalize(vkRefractDirRed);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	mediump vec4 RefractRed = texture(sParaboloids, Normalised.xy);

	Refraction.r = mix(RefractSky.r, RefractRed.r, RefractRed.a);

	// Green
	mediump vec3 vkRefractDirGreen = RefractDirGreen;
	vkRefractDirGreen.y = -vkRefractDirGreen.y;
	Normalised = normalize(vkRefractDirGreen);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	mediump vec4 RefractGreen = texture(sParaboloids, Normalised.xy);

	Refraction.g = mix(RefractSky.g, RefractGreen.g, RefractGreen.a);

	// Blue
	mediump vec3 vkRefractDirBlue = RefractDirBlue;
	vkRefractDirBlue.y = -vkRefractDirBlue.y;
	Normalised = normalize(vkRefractDirBlue);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	mediump vec4 RefractBlue = texture(sParaboloids, Normalised.xy);

	Refraction.b = mix(RefractSky.b, RefractBlue.b, RefractBlue.a);
	// Combine reflection and refraction for final color
	outColor.rgb = mix(Refraction.rgb, Reflection.rgb, ReflectFactor);
	outColor.a = 1.0;
}
