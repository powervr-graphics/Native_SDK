#version 300 es
uniform sampler2D  sBloomMapping;

uniform mediump float fBloomIntensity;

in mediump vec2 TexCoord;
layout (location = 0) out lowp vec4 oColour;

void main()
{
    oColour = texture(sBloomMapping, TexCoord) * fBloomIntensity;
}
