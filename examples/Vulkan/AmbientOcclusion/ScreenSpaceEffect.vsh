#version 320 es

// Rather than produce a input assembler, hardcore the values for a triangle that covers the entire screen
vec2 positions[3] = vec2[](vec2(-1.0, 1.0), // bottom left
	vec2(3.0, 1.0), // bottom right
	vec2(-1.0, -3.0) // top left
);

vec2 TexCoord[3] = vec2[](vec2(0.0, 1.0), vec2(2.0, 1.0), vec2(0.0, -1.0));

// Pass out the texture coordinates to the fragment shader to get the screen space location
layout(location = 0) out highp vec2 vTexCoord;

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	vTexCoord = TexCoord[gl_VertexIndex];
}
