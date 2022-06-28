#version 310 es

out mediump vec4 oAmbient;

void main()
{
	mediump float ambient = 0.05;
	mediump vec3 diffuse = vec3(0.75);
	oAmbient = vec4(diffuse * ambient, 1.0);
}
