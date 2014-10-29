#version 300 es

#define DEFAULT_VERTEX_ARRAY	0
#define DEFAULT_TEXCOORD_ARRAY	1

layout (location = DEFAULT_VERTEX_ARRAY) in highp vec3	inVertex;
layout (location = DEFAULT_TEXCOORD_ARRAY) in mediump vec2	inTexCoord;

uniform highp   mat4 MVPMatrix;
uniform float	fUOffset;

out mediump vec2 TexCoord;

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Pass through texcoords
	TexCoord = inTexCoord;
	TexCoord.x += fUOffset;
}
 