attribute highp vec3 inVertex;
attribute highp vec3 inNormal;

uniform highp mat4  uModelViewMatrix;
uniform highp mat3  uModelViewITMatrix;
uniform highp mat4  uModelViewProjectionMatrix;

uniform highp vec3  uLightPosition;

varying highp vec3  vNormal;
varying highp vec3  vLightDirection;

void main()
{
	gl_Position = uModelViewProjectionMatrix * vec4(inVertex, 1.0);
	vNormal = uModelViewITMatrix * inNormal;

	highp vec3 position = (uModelViewMatrix * vec4(inVertex, 1.0)).xyz;
	vLightDirection = uLightPosition - position;
}
