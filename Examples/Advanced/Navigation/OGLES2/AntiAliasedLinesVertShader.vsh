attribute highp   vec3 inVertex;
attribute mediump vec2 inTexCoord;

varying mediump  vec2 TexCoord;

uniform highp mat4 ModelViewProjMatrix;

void main()
{
	// Convert each vertex into projection-space and output the value
	gl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);	
	TexCoord = inTexCoord;
}
