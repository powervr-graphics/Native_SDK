#version 300 es

uniform sampler2D      SamplerTexture;

#ifdef DIFFUSE
in lowp float     fLightIntensity;
#endif
in mediump vec2   vTexCoord;

layout(location = 0) out lowp  vec4  oColour; 

void main()
{
#ifdef DIFFUSE
    oColour = texture(SamplerTexture, vTexCoord) * fLightIntensity;
#else
	oColour = texture(SamplerTexture, vTexCoord);
#endif
}
