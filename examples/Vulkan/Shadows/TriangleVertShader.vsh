#version 320 es

layout (location = 0) out mediump vec2 vTexCoord;

void main(void)
{
    float x = -1.0 + float((gl_VertexIndex & 1) << 2);
	float y = -1.0 + float((gl_VertexIndex & 2) << 1);
    vTexCoord.x = (x + 1.0) * 0.5;
    vTexCoord.y = (y + 1.0) * 0.5;
    gl_Position = vec4(x, y, 0.0, 1.0);
}