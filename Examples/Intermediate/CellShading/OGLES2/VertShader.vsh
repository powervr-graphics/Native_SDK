attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;

uniform highp mat4  MVPMatrix;		// model view projection transformation
uniform highp vec3  LightDirection;	// light direction in model space
uniform highp vec3  EyePosition;	// eye position in model space

varying mediump vec2  TexCoord;

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex,1.0);
	
	mediump vec3 eyeDirection = normalize(EyePosition - inVertex);
	
	TexCoord.x = dot(LightDirection, inNormal);
	TexCoord.y = dot(eyeDirection, inNormal);
}
