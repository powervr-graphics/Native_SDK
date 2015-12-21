#version 300 es
in mediump vec2   TexCoord;
uniform sampler2D  sTexture;
uniform mediump float BloomIntensity;// Luminance threshol
layout (location = 0) out lowp vec4 oColor;
mediump float luma(mediump vec3 color)
{	
	return dot(vec3(0.2126,0.7152,0.0722), color); 
}

void main()
{
    mediump float lumThres = .5;
    oColor = texture(sTexture, TexCoord);
	oColor = oColor * clamp(luma(oColor.rgb) - lumThres, 0.0,1.0) * 
    					 (1.0 / (1.0 - lumThres)) * BloomIntensity * oColor.a /*multiply by alpha so that only the pixels belongs to the model get bloom*/; 
}