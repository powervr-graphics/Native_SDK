#version 300 es

uniform sampler2D sTexture;
uniform sampler2D sNormalMap;
uniform bool bUseDot3;

in mediump vec2 TexCoord;
in mediump vec3 Light;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	if(bUseDot3)
	{
		/*
			Note:
			In the normal map red = y, green = x, blue = z which is why when we get the normal
			from the texture we use the swizzle .grb so the colours are mapped to the correct
			co-ordinate variable.
		*/

		mediump vec3 fNormal = texture(sNormalMap, TexCoord).grb;
		mediump float fNDotL = dot((fNormal - 0.5) * 2.0, Light);
		
		oColour = texture(sTexture, TexCoord) * fNDotL;
    }
    else
		oColour = texture(sTexture, TexCoord) * Light.x;
}
