uniform sampler2D Texture;
uniform lowp vec4 vRGBA;

varying highp vec2 TexCoord;

void main()
{
	gl_FragColor = texture2D(Texture, TexCoord) * vRGBA;
}