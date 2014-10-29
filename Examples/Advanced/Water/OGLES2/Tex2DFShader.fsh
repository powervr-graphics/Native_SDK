uniform sampler2D Texture;

varying highp vec2 TexCoord;

void main()
{
	gl_FragColor = texture2D(Texture, TexCoord);
}