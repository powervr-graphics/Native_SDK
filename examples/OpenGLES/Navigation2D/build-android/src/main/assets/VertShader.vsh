#version 300 es

in highp vec2 myVertex;

uniform highp mat4 transform;
uniform lowp vec4 myColor;

out lowp vec4 fragColor;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 0.0, 1.0);
	fragColor = myColor;
}