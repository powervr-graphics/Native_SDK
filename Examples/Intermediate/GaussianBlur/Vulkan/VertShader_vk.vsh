#version 450

layout(location = 0) out lowp vec2 UV_OUT;

void main()
{
	highp vec2 texcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);
	UV_OUT = texcoord;
}