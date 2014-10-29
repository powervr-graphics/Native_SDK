attribute highp vec4  inVertex;
attribute highp vec3  inNormal;
attribute highp vec2  inTexCoord;

uniform highp mat4  MVPMatrix;
uniform highp vec3  LightDirection;
uniform highp vec3  EyePosition;

varying mediump float  CosViewAngle;
varying mediump float  LightIntensity;
varying mediump vec2   TexCoord;

void main()
{
	gl_Position = MVPMatrix * inVertex;
	
	highp vec3 eyeDirection = normalize(EyePosition - inVertex.xyz);
	
	// Simple diffuse lighting 
	LightIntensity = max(dot(LightDirection, inNormal), 0.0);

	// Cosine of the angle between surface normal and eye direction
	// We clamp at 0.1 to avoid ugly aliasing at near 90° angles
	CosViewAngle = max(dot(eyeDirection, inNormal), 0.1);
	
	TexCoord = inTexCoord;
}