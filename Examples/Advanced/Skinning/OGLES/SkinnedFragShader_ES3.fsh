#version 300 es

uniform sampler2D sTexture;
uniform sampler2D sNormalMap;

in mediump vec3 vLight;
in mediump vec2 vTexCoord;

layout (location = 0) out lowp vec4 oColor;

void main()
{
	/*
		Note:
		In the normal map red = y, green = x, blue = z which is why when we get the normal
		from the texture we use the swizzle .grb so the colors are mapped to the correct
		co-ordinate variable.
	*/

	mediump vec3 fNormal = texture(sNormalMap, vTexCoord).rgb;
	mediump float fNDotL = clamp(dot((fNormal - 0.5) * 2.0, normalize(vLight)) + .2, 0., 1.);
	
	oColor = texture(sTexture, vTexCoord) * fNDotL;
    oColor.a = 1.0;
}
