#version 300 es

#define AXIS_ALIGNED_QUAD_VERTEX_ARRAY	0
#define AXIS_ALIGNED_QUAD_TEXCOORD_ARRAY	1

layout (location = AXIS_ALIGNED_QUAD_VERTEX_ARRAY) in highp vec2	inVertex;
layout (location = AXIS_ALIGNED_QUAD_TEXCOORD_ARRAY) in mediump vec2	inTexCoord;

out mediump vec2   TexCoord;

void main()
{
    // Pass through vertex
	gl_Position = vec4(inVertex, 0.0, 1.0);
	
	// Pass through texcoords
	TexCoord = inTexCoord;
}
