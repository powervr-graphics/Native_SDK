#version 450

layout(set = 0, binding = 1) uniform sampler2D sParaboloids;
layout(set = 0, binding = 2) uniform samplerCube sSkybox;
layout(location = 0) in mediump vec3 RefractDir;
layout(location = 0) out vec4 outColor;

void main()
{
	mediump vec3 Normalised;

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
	outColor.rgb = Refraction.rgb;
	outColor.a = 1.0;
}
