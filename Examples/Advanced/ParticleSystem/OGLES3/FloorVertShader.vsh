#version 300 es

in highp vec3 inVertex;
in highp vec3 inNormal;

uniform highp mat4  uModelViewMatrix;
uniform highp mat3  uModelViewITMatrix;
uniform highp mat4  uModelViewProjectionMatrix;

uniform highp vec3  uLightPosition;

out highp vec3  vNormal;
out highp vec3  vLightDirection;

void main()
{
	highp vec4 position   = (uModelViewMatrix * vec4(inVertex, 1.0));

	gl_Position = uModelViewProjectionMatrix * vec4(inVertex, 1.0);
	
	vNormal         = uModelViewITMatrix * inNormal;
	vLightDirection = uLightPosition - position.xyz;
}
