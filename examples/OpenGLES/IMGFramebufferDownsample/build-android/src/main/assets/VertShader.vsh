#version 310 es

precision highp float;

in vec3 inVertex;

uniform mat4 MVPMatrix;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
}