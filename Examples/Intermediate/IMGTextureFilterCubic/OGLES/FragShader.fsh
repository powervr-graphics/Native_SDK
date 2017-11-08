#version 310 es

layout(location=0) uniform highp sampler2D tex;
layout(location=1) uniform highp sampler2D cubicTex;

uniform highp float WindowWidth;

layout(location=0) out highp vec4 outColor;

in highp vec4 pos;

void main()
{
	highp vec2 texcoord = pos.xz * 0.01;

	highp float imageCoordX = gl_FragCoord.x - 0.5;
	highp float xPosition = imageCoordX / WindowWidth;

    outColor = xPosition < 0.5 ? texture(tex, texcoord) : xPosition == 0.5 ? vec4(1.0, 1.0, 1.0, 1.0) : texture(cubicTex, texcoord);
	outColor.a = 1.0;
}