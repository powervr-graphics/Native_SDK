#version 450
layout (set = 0, binding = 0)uniform mediump sampler2D sTexture;

/* 
  Separated Gaussian 5x5 filter, first row:

              1  5  6  5  1
*/


layout (location = 0)in mediump vec2 TexCoord0;
layout (location = 1)in mediump vec2 TexCoord1;
layout (location = 2)in mediump vec2 TexCoord2;

layout (location = 0) out mediump vec4 oColor;

void main()
{
    mediump vec3 color = texture(sTexture, TexCoord0).rgb * 0.52941176470588235294117647058823;
    color = color + texture(sTexture, TexCoord1).rgb * 0.35294117647058823529411764705882;
    color = color + texture(sTexture, TexCoord2).rgb * 0.35294117647058823529411764705882;    
    oColor.rgb = color;
	oColor.a = 1.;
}
