#version 450

layout(location = 0) out vec2 vTexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	vTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(vTexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
}