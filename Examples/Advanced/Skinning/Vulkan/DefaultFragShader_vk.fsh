#version 440

layout(set = 0, binding = 0) uniform sampler2D sTexture;
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec3 vWorldNormal;
layout(location = 2) in vec3 vLightDir;
layout(location = 3) in float vOneOverAttenuation;

layout(location = 0) out vec4 oColor;

void main()
{
    vec3 norm = vWorldNormal;
	mediump float lightIntensity = clamp(dot(normalize(vLightDir), normalize(norm)) + .2, 0., 1.);
    lightIntensity *= vOneOverAttenuation;

    oColor = texture(sTexture, vTexCoord);
	oColor.xyz = oColor.xyz * lightIntensity;
    oColor.a = 1.0;
}
