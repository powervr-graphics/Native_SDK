uniform sampler2D      SamplerTexture;

#ifdef DIFFUSE
varying lowp float     fLightIntensity;
#endif
varying mediump vec2   vTexCoord;

void main()
{
#ifdef DIFFUSE
    gl_FragColor = texture2D(SamplerTexture, vTexCoord) * fLightIntensity;
#else
	gl_FragColor = texture2D(SamplerTexture, vTexCoord);
#endif
}
