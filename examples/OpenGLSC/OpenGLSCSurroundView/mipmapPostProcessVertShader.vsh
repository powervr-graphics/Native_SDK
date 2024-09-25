#version 310 es

out mediump vec2 UV;

void main()
{
	UV = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(UV * 2.0 + -1.0, 0.0, 1.0);
}
