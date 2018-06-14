uniform mediump sampler2D  sTexture;

varying lowp vec4 fragColour;
varying mediump vec2 texCoordOut;

void main(void)
{
	lowp vec4 texColour = texture2D(sTexture, texCoordOut);
	gl_FragColor = vec4(fragColour.rgb * texColour.r, texColour.a);
}