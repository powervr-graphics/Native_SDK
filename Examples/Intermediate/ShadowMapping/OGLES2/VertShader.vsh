attribute highp vec3  inVertex;
attribute highp vec3  inNormal;
attribute highp vec2  inTexCoord;

uniform highp mat4  ModelViewMatrix;
uniform highp mat4  ProjectionMatrix;
varying highp vec2  TexCoord;

void main()
{
	gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(inVertex, 1.0);
	TexCoord = inTexCoord;
}