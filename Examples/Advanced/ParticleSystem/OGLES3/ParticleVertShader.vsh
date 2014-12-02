#version 300 es

#define POSITION_ARRAY	0
#define LIFESPAN_ARRAY	1

layout (location = POSITION_ARRAY) in highp vec3  inPosition;
layout (location = LIFESPAN_ARRAY) in highp float inLifespan;

uniform highp mat4 uModelViewProjectionMatrix;

out mediump float fTexCoord;

void main()
{
	gl_Position = uModelViewProjectionMatrix * vec4(inPosition, 1.0);
	gl_PointSize = 1.5;
	fTexCoord = inLifespan;
}
