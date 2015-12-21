#version 100

uniform sampler2D s2DMap;

varying highp vec2 TexCoords;
varying highp float LightIntensity;

void main()
{
	// Sample texture and shade fragment
	gl_FragColor = vec4(LightIntensity * texture2D(s2DMap, TexCoords).rgb, 1.0);
}
