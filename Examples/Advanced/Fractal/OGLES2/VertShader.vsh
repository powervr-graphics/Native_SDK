attribute highp   vec4  inVertex;
attribute mediump vec2  inTexCoord;

uniform mediump mat4  MVPMatrix;

varying mediump vec2  TexCoord;

void main()
{
	gl_Position = MVPMatrix * inVertex;
	TexCoord = inTexCoord;
}
