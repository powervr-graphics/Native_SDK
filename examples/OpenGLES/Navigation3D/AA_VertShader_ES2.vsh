attribute highp vec3 myVertex;
attribute mediump vec2	texCoord;

uniform highp mat4 transform;
uniform lowp vec4 myColour;

varying lowp vec4 fragColour;
varying mediump vec2 texCoordOut;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 1.0);
	fragColour = myColour;
	texCoordOut = texCoord;
}