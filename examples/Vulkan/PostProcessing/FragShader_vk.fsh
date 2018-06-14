#version 450
layout (set = 0, binding = 0) uniform sampler2D sTexture;

layout (location = 0) in highp float LightIntensity;
layout (location = 1) in mediump vec2 TexCoord;

layout (location = 0) out mediump vec4 oColor;

void main()
{
    oColor = texture(sTexture, TexCoord) * LightIntensity;
}
