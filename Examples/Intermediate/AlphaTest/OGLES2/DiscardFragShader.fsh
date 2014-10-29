uniform sampler2D  sTexture;

uniform lowp float  AlphaReference;

varying mediump vec2  TexCoord;

void main()
{
	lowp vec4 color = texture2D(sTexture, TexCoord);
	if (color.a < AlphaReference) 
	{
		discard;
	}
	gl_FragColor = color;
}
