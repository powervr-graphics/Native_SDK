attribute highp   vec4  inVertex;
attribute mediump vec2  inTexCoord;

uniform mediump mat4    MVPMatrix;
uniform highp mat4    TexSamplerPMatrix;
#ifdef NOISE
uniform mediump vec2    vNoiseLoc;
varying mediump vec2    vNoiseTexCoord;
#endif

varying mediump vec2    vTexCoord;

void main()
{
	gl_Position = MVPMatrix * inVertex;

#ifdef ANDROID
	vTexCoord   = (TexSamplerPMatrix * vec4(inTexCoord, 0, 1)).xy;
#else
	vTexCoord   = inTexCoord;
	vTexCoord.y = 1.0 - vTexCoord.y;  // Invert
#endif
	

#ifdef NOISE
	vNoiseTexCoord = (inTexCoord * 0.25) + vNoiseLoc;
#endif
}
