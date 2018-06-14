attribute highp vec3 myVertex;
attribute vec3 normal;

uniform highp mat4 transform;
uniform highp mat4 viewMatrix;
uniform highp vec3 lightDir;
uniform lowp vec4 myColour;

varying lowp vec4 fragColour;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 1.0);

	vec3 N = normalize(mat3(viewMatrix) * normal);

	float D = max(dot(N, lightDir), 0.15);
	fragColour = vec4(myColour.rgb * D, myColour.a);
}