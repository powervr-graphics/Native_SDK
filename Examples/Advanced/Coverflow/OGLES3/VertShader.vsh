#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define COLOR_ARRAY		2
#define TEXCOORD_ARRAY	3

layout (location = VERTEX_ARRAY) in highp vec3 inVertex;
layout (location = NORMAL_ARRAY) in highp vec3 inNormal;
layout (location = COLOR_ARRAY) in highp vec4 inColour;
layout (location = TEXCOORD_ARRAY) in mediump vec2 inTexCoord;

uniform highp   mat4  MVPMatrix;

out mediump vec2   TexCoord;
out mediump vec4   Colours;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Pass through texcoords
	TexCoord = inTexCoord;
	Colours = inColour;
}
