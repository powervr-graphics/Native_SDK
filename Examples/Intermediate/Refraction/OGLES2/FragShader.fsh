uniform sampler2D  sTexture;

varying lowp    float  SpecularIntensity;
varying mediump vec2   RefractCoord;

void main()
{
	lowp vec3 refractColor = texture2D(sTexture, RefractCoord).rgb;	
	gl_FragColor = vec4(refractColor + SpecularIntensity, 1.0);
}