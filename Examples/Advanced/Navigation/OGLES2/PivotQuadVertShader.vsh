attribute highp   vec2  inVertex;
attribute mediump vec2  inWordIndex;
attribute mediump vec2  inTexCoords;

// inWordIndex: { horizontal multiplier | vertical muliplier }

varying mediump vec2    TexCoord;

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
