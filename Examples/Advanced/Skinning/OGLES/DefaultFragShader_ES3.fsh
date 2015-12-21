#version 300 es

uniform sampler2D sTexture;

in mediump vec2 vTexCoord;
in mediump vec3 vWorldNormal;
in mediump vec3 vLightDir;
in mediump float vOneOverAttenuation;

layout (location = 0) out lowp vec4 oColor;

void main()
{
	mediump float lightIntensity = max(dot(normalize(vLightDir), normalize(vWorldNormal)), 0.);
    lightIntensity *= vOneOverAttenuation;

    oColor = texture(sTexture, vTexCoord);
	oColor.xyz = oColor.xyz * lightIntensity;
    oColor.a = 1.0;
}
