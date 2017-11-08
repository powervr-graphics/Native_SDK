#version 450

out mediump vec2 vTexCoord;

void main()
{
	highp vec2 texcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	vTexCoord = texcoord;
}
