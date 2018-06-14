#version 300 es
in mediump vec2   TexCoord;
uniform highp sampler2D sTexture;
uniform highp float BloomIntensity;
layout (location = 0) out highp vec4 oColor;
mediump float luma(highp vec3 color)
{	
	return max(dot(vec3(0.2126,0.7152,0.0722), color), 0.0001f); 
}

void main()
{
    mediump float lumThres = 1.0;
    oColor = texture(sTexture, TexCoord);
	oColor = oColor * clamp(luma(oColor.rgb) - lumThres, 0.0, 100.) * oColor.a * BloomIntensity;
	
	/*multiply by alpha so that only the pixels belongs to the model get bloom*/ 
}