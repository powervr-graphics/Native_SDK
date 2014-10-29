uniform sampler2D  sTexture;

varying mediump vec2  TexCoord;

const lowp vec3  cBaseColor = vec3(0.2, 1.0, 0.7);

void main()
{
	gl_FragColor = vec4(cBaseColor * texture2D(sTexture, TexCoord).rgb, 1.0);
}

