#version 310 es

uniform highp sampler2D tex;

in mediump vec2 TexCoord;

layout (location = 0) out lowp vec4 oColor;

void main()
{
    oColor = textureLod(tex, TexCoord, 1.0);
}
