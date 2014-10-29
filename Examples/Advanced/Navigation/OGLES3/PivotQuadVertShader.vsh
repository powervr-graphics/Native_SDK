#version 300 es

#define VERTEX_ARRAY	0
#define WORDINDEX_ARRAY	1
#define ATTRIB_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec2	inVertex;
layout (location = WORDINDEX_ARRAY) in mediump vec2	inWordIndex;
layout (location = ATTRIB_ARRAY) in mediump vec2	inTexCoords;

// inWordIndex: { horizontal multiplier | vertical muliplier }

out mediump vec2    TexCoord;

uniform highp   mat4    ModelViewProjMatrix;
uniform mediump vec3    PivotDirection;
uniform mediump vec3    Up;

void main()
{
	// Span a quad depending on the texture coordinates and the camera's up and right vector		
	
	// Convert each vertex into projection-space and output the value
	mediump vec3 offset = PivotDirection * inWordIndex.x + Up * inWordIndex.y;		
	
	// Pass the texcoords
	TexCoord = inTexCoords;
	
	// Calculate the world position of the vertex
	highp vec4 vInVertex = vec4(vec3(inVertex, 0.0) + offset, 1.0);	
		
	// Transform the vertex
	gl_Position = ModelViewProjMatrix * vInVertex;	
}
