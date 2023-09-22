#version 320 es

layout(location = 0) in highp vec3 vNormal;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	oColor = vec4(vNormal.xyz, 0.0);
}
