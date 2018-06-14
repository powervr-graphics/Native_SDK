#version 450

layout (set = 0, binding = 0) uniform sampler2D sTexture;
layout (set = 1, binding = 0) uniform BloomConfig
{
	mediump float BloomIntensity; // Luminance threshold
};

layout (location = 0) in highp float LightIntensity;
layout (location = 1) in mediump vec2 TexCoord;

layout (location = 0) out highp vec4 oColor;
layout (location = 1) out highp vec4 oFilter;

mediump float luma(mediump vec3 color)
{	
	return dot(vec3(0.2126,0.7152,0.0722), color); 
}

void main()
{
    oColor = texture(sTexture, TexCoord) * LightIntensity;
    mediump float luminosityThreshold = .5;

	// multiply by alpha so that only the pixels belongs to the model get bloom
    oFilter = oColor;
    oFilter = oFilter * clamp(luma(oFilter.rgb) - luminosityThreshold, 0.0,1.0) * 
    					 (1.0 / (1.0 - luminosityThreshold)) * BloomIntensity * oFilter.a;
}