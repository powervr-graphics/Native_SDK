attribute highp   vec3  inVertex;
attribute highp   vec3	inNormal;
attribute highp   vec4  inColor;
attribute mediump vec2  inTexCoord;

uniform highp   mat4  MVPMatrix;

varying mediump vec2   TexCoord;
varying mediump vec4   Colors;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Pass through texcoords
	TexCoord = inTexCoord;
	Colors = inColor;
}
