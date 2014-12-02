attribute highp vec3  inVertex;
attribute highp vec3  inNormal;
attribute mediump vec2  inTexCoord;

uniform highp mat4 TexProjectionMatrix;
uniform	highp mat4 ProjectionMatrix;
uniform highp mat4 ModelViewMatrix;
uniform highp vec3 LightDirection;

varying highp vec4 vProjCoord;
varying mediump vec2 texCoord;
varying lowp vec3 LightIntensity;

void main()
{
	highp vec4 modelViewPos = ModelViewMatrix * vec4(inVertex, 1.0);
	gl_Position = ProjectionMatrix * modelViewPos;
	vProjCoord = TexProjectionMatrix * modelViewPos;

	texCoord = inTexCoord;
	
	// Simple diffuse lighting in model space
	LightIntensity = vec3(dot(inNormal, -LightDirection));
}
