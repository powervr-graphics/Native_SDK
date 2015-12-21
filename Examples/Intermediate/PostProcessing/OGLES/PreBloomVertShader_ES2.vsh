attribute highp   vec3  inVertex;
attribute mediump vec2  inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform mediump vec3  LightDirection;

varying mediump vec2   TexCoord;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);	

	// Use the light intensity as texture coords for the bloom mapping
	TexCoord = inTexCoord;
}