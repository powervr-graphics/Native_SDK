#version 450

layout(set = 1, binding = 0)uniform sampler2D sParaboloids;
layout(set = 2, binding = 0)uniform samplerCube sSkybox;
layout(location = 0)in mediump vec3 RefractDir;
layout(location = 0)out vec4 outColor;

void main()
{
	mediump vec3 Normalised;

	lowp vec4 RefractSky = texture(sSkybox, RefractDir);

	Normalised = normalize(RefractDir);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	lowp vec4 Refraction = texture(sParaboloids, Normalised.xy);

	Refraction.rgb = mix(RefractSky.rgb, Refraction.rgb, Refraction.a);
	outColor.rgb = Refraction.rgb;
	outColor.a = 1.0;
}
