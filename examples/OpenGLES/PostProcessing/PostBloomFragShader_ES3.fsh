#version 300 es
uniform mediump sampler2D  sTexture;
uniform sampler2D  sBlurTexture;

uniform mediump float sTexFactor;
uniform mediump float sBlurTexFactor;

const mediump float exposure = .5f;
const mediump float gamma = 2.2f;

in mediump vec2 TexCoord;

layout (location = 0) out lowp vec4 oColor;

void main()
{
	
	mediump vec4 hdrColor = texture(sTexture, TexCoord) * sTexFactor;
  
    // Exposure tone mapping
	hdrColor.rgb = vec3(1.0) - exp(-hdrColor.rgb * exposure);
    // Gamma correction 
    hdrColor.rgb = pow(hdrColor.rgb, vec3(1.0 / gamma));
	oColor = hdrColor + texture(sBlurTexture, TexCoord) * sBlurTexFactor;
}
