attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;
attribute mediump vec2  inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform mediump vec3  LightDirection;

varying lowp    float  LightIntensity;
varying mediump vec2   TexCoord;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Pass through texcoords
	TexCoord = inTexCoord;
	
	// Simple diffuse lighting in model space
	LightIntensity = dot(inNormal, -LightDirection);
}