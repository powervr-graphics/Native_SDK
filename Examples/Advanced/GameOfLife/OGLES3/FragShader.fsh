#version 300 es
uniform sampler2D    sTexture;

in highp vec2 TexCoord;

out highp vec4 fragColor;

void main()
{
    mediump float smp = texture(sTexture, TexCoord).x;
	//Just giving it a little color.
	const lowp vec3 hue = vec3(.5, 1, .5);
	//const lowp vec3 hue = vec3(.9, .26, 1.);
	fragColor = vec4(hue * smp, 1.0); 
}
